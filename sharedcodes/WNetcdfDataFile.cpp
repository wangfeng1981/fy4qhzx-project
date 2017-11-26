
#include "WNetcdfDataFile.h"
#include "gdal_priv.h"
#include <iostream>
#include "cpl_error.h"
#include "cpl_conv.h"
#include "cpl_vsi.h"
#include <algorithm>
#include <string>



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

std::string WNetcdfDataFile::createUpperString(const std::string str) {
	std::string strUpper = str ;
	std::transform(strUpper.begin(), strUpper.end(), strUpper.begin(), ::toupper);
	return strUpper;
}


GDALDataset* WNetcdfDataFile::extractDataset(std::string subDsName)
{
	if (this->m_datasetPtr != nullptr)
	{
		size_t lenOfSubDsName = subDsName.length();
		std::string upperSubDsName = WNetcdfDataFile::createUpperString(subDsName);

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
					size_t maoHaoPos = subDatasetString.find_last_of(':');
					if (maoHaoPos != std::string::npos)
					{
						size_t lenOfString = subDatasetString.length();
						std::string theRightWord = subDatasetString.substr(lenOfString- lenOfSubDsName);
						std::string theRightWordUpper = WNetcdfDataFile::createUpperString(theRightWord);
						if (theRightWordUpper == upperSubDsName)
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
