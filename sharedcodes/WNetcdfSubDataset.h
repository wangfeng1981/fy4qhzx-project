#pragma once
#include "gdal_priv.h"
#include "WNetcdfDataFile.h"

class WNetcdfSubDataset
{
public:
	WNetcdfSubDataset();
	~WNetcdfSubDataset();

	bool load(std::string filepath, std::string subDsName  );
	void close();
	void warpToWgs84ByNcHdf(
		std::string lonFilePath, std::string lonDsName,
		std::string latFilePath, std::string latDsName);
	void warpToWgs84ByTif(
		std::string lonFilePath ,
		std::string latFilePath );
	int warpToWgs84ByDs(GDALDataset* lonDs, GDALDataset* latDs, std::string lonFilePath, std::string latFilePath);


private:
	std::string m_filePath;
	std::string m_subDatasetName;
	std::string m_lonFilePath;
	std::string m_lonSubDatasetName;
	std::string m_latFilePath;
	std::string m_latSubDatasetName;
	GDALDataset* m_dataSubDatasetPtr;
	GDALDataset* m_lonSubDatasetPtr;
	GDALDataset* m_latSubDatasetPtr;

};

