#pragma once
#include <string>
#include "gdal_priv.h"


class WNetcdfDataFile
{
public:
	WNetcdfDataFile();
	~WNetcdfDataFile();

	bool open(std::string filepath);
	GDALDataset* extractDataset(std::string subDsName);
	void close();

private:
	std::string m_filepath;
	GDALDataset* m_datasetPtr;

};

