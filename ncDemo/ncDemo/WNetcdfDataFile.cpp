#include "stdafx.h"
#include "WNetcdfDataFile.h"
#include "gdal_priv.h"
#include <iostream>
#include "cpl_error.h"
#include "cpl_conv.h"
#include "cpl_vsi.h"

WNetcdfDataFile::WNetcdfDataFile():m_datasetPtr(nullptr)
{
}


WNetcdfDataFile::~WNetcdfDataFile()
{
	this->close();
}

bool WNetcdfDataFile::open(std::string filepath) 
{
	this->m_filepath = filepath;
	if (this->m_datasetPtr != nullptr) {
		this->close();
	}
	try 
	{
		this->m_datasetPtr = (GDALDataset*)GDALOpen(this->m_filepath.c_str(), GDALAccess::GA_ReadOnly);
		if (this->m_datasetPtr == nullptr) {
			std::string errorText = "打开NetCDF文件(";
			errorText = errorText + this->m_filepath;
			errorText = errorText + ")失败。";
			throw errorText;
		}
	}
	catch (std::string ex )
	{
	 
	}

}

void WNetcdfDataFile::close()
{
	if (m_datasetPtr != nullptr)
	{
		GDALClose(m_datasetPtr);
		m_datasetPtr = nullptr;
	}
}


GDALDataset* WNetcdfDataFile::extractDataset(std::string subDsName)
{
	if (this->m_datasetPtr != nullptr)
	{
		char** plist = this->m_datasetPtr->GetMetadata("SUBDATASETS");
		int i = 0;
		bool findSubDataset = false;
		std::string subDatasetPath = "";
		while (plist[i] != nullptr) {
			std::string subDatasetString = std::string(plist[i]) ;
			size_t dengHaoPos = subDatasetString.find_first_of('=');
			if ( dengHaoPos != std::string::npos )
			{
				std::string dengHaoLeftFourString = subDatasetString.substr(dengHaoPos - 4, 4);
				if (dengHaoLeftFourString == std::string("NAME"))
				{
					size_t doubleSlashPos = subDatasetString.find_last_of('/');
					if (doubleSlashPos != std::string::npos)
					{
						std::string doubleSlashRightString = subDatasetString.substr(doubleSlashPos + 1);
						if (doubleSlashRightString == subDsName)
						{
							findSubDataset = true;
							subDatasetPath = subDatasetString.substr(dengHaoPos + 1);
						}
					}
				}
			}
			++i;
		}
		if (findSubDataset == false)
		{
			std::string error = "没有找到NetCDF文件中";
			error = error + subDsName;
			error = error + "数据集。";
			throw error ;
		}
		else {
			GDALDataset* subDatasetPtr =(GDALDataset*) GDALOpen(subDatasetPath.c_str(), GDALAccess::GA_ReadOnly);
			return subDatasetPtr;
		}
	}
}
