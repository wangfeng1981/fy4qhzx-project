#pragma once
#include <string>
#include "gdal_priv.h"


struct WValueRange
{
	double lowValue, highValue;
	inline WValueRange(double low, double high) :lowValue(low), highValue(high) {};
	inline bool isInside(double val) { return (val >= lowValue&&val <= highValue) ? true : false; };
};

struct WDataPair
{
	inline WDataPair(double x, double ref) :value_x(x), value_ref(ref) {};
	double value_x;
	double value_ref;
};
struct WDataTripple
{
	inline WDataTripple(double x, double y,double z) :mx(x),my(y),mz(z) {};
	double mx,my,mz;
};


class WNetcdfDataFile
{
public:
	WNetcdfDataFile();
	~WNetcdfDataFile();

	bool open(std::string filepath);
	GDALDataset* extractDataset(std::string subDsName);
	void close();

	static std::string createUpperString(const std::string str);

private:
	std::string m_filepath;
	GDALDataset* m_datasetPtr;

};

