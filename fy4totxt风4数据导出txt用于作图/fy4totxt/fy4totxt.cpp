// fy4totxt.cpp : 定义控制台应用程序的入口点。
//将风云四的数据输出成经度 纬度 值的文本文件格式.
//wangfeng1@piesat.cn

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "gdal_priv.h"
using namespace std;


#define LLFILES 0 //2017-11-14
#define LLRANGE 1  //2017-11-14
#define LLGEOTRANS 2 //2017-11-14


//通过标签获取命令行是否有某个tag
bool wft_has_tag(int argc, char** argv, const char* key )
{
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], key) == 0) {
			return true;
		}
	}
	return false;
}


//通过标签获取命令行参数
bool wft_has_param(int argc, char** argv, char* key, std::string& value, bool mustWithValue)
{
	value = "";
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], key) == 0) {
			if (i != argc - 1)
			{
				value = std::string(argv[i + 1]);
				return true;
			}
			else {
				if (mustWithValue) {
					std::cout << "Error: can not find value for key :" << key << ". out." << std::endl;
					exit(99);
				}
				return true;
			}
		}
	}
	if (mustWithValue) {
		std::cout << "Error: can not find value for key :" << key << ". out." << std::endl;
		exit(99);
	}
	return false;
}
//进度显示
void wft_term_progress(size_t curr, size_t total)
{
	static size_t term_progress_percent = -1;
	if (curr < total - 1)
	{
		size_t newper = curr * 100 / total;
		if (newper != term_progress_percent) {
			term_progress_percent = newper;
			if (term_progress_percent % 10 == 0) {
				std::cout << term_progress_percent;
			}
			else {
				std::cout << ".";
			}
		}
	}
	else {
		if (term_progress_percent != 100) {
			term_progress_percent = 100;
			std::cout << 100 << std::endl;
		}
	}
}




int main(int argc , char** argv )
{
	std::cout << "Version 0.1.2a. 2017-9-30 wangfengdev@163.com." << endl;
	std::cout << "Version 0.2a. can set output rect by pixels coordinate." << endl;
	std::cout << "Version 0.3a. can set output rect by lonlat coordinate, lonlat source add geotrans .2017-11-14." << endl;//2017-11-14
	
	if (argc == 1) {
		
		std::cout << "sample call:" << endl;
		std::cout << "fy4totxt -in HDF5:xxx.hdf:SST -out xxx.txt -type llfiles/llrange/geotrans " //2017-11-14
			" [-lon fy4lon.tif -lat fy4lat.tif]/[-left lon0 -right lon1 -top lat0 -bottom lat1] "
			" [-valid0 v0] [-valid1 v1] " 
			" [-scale 1.0] [-offset 0.0] "
			" [-xspace 2] [-yspace 2] "
			" [-nonan] "
			" [-x0 0 -x1 100 -y0 0 -y1 50] "
			" [-cutlon0 -180 -cutlon1 180 -cutlat0 -90 -cutlat1 90] " //2017-11-14
			<< endl;
		exit(101);
	}

	std::string inputDatasetpath;
	wft_has_param(argc, argv, "-in", inputDatasetpath, true);
	std::string outputFile ;
	wft_has_param(argc, argv, "-out", outputFile , true);

	std::string llType = "llrange" ;
	wft_has_param(argc, argv, "-type", llType, false);
	std::string lonFile, latFile;
	double left(-180), right(180), top(90), bottom(-90);
	double xStride(1.0), yStride(1.0);

	int llTypeInt = LLGEOTRANS ;//2017-11-14
	if (llType == "llfiles")
	{
		llTypeInt = LLFILES ;//2017-11-14
		wft_has_param(argc, argv, "-lon", lonFile, true);
		wft_has_param(argc, argv, "-lat", latFile, true);
	}
	else if( llType == "llrange" ) {
		llTypeInt = LLRANGE ;//2017-11-14
		std::string s1 ;
		if (wft_has_param(argc, argv, "-left", s1, false)) left = atof(s1.c_str());
		if (wft_has_param(argc, argv, "-right", s1, false)) right = atof(s1.c_str());
		if (wft_has_param(argc, argv, "-top", s1, false)) top = atof(s1.c_str());
		if (wft_has_param(argc, argv, "-bottom", s1, false)) bottom = atof(s1.c_str());
	}
	else {//2017-11-14
		llTypeInt = LLGEOTRANS;//2017-11-14
	}//2017-11-14

	int useValidType = 0;
	double valid0(0), valid1(0);
	{
		std::string s1,s2;
		if (wft_has_param(argc, argv, "-valid0", s1, false) && wft_has_param(argc, argv, "-valid1", s2, false))
		{
			valid0 = atof(s1.c_str());
			valid1 = atof(s2.c_str());
			useValidType = 1;
		}
	}
	double scale(1.0), offset(0);
	{
		std::string s1;
		if (wft_has_param(argc, argv, "-scale", s1, false)) scale = atof(s1.c_str());
		if (wft_has_param(argc, argv, "-offset", s1, false)) offset = atof(s1.c_str());
	}
	int xSpace = 1;
	int ySpace = 1;
	{
		std::string s1;
		if (wft_has_param(argc, argv, "-xspace", s1, false)) xSpace = atof(s1.c_str());
		if (wft_has_param(argc, argv, "-yspace", s1, false)) ySpace = atof(s1.c_str());
	}

	bool noNanOutput = false;
	if (wft_has_tag(argc, argv, "-nonan") ) {
		noNanOutput = true;
	}

	double outX0, outX1, outY0, outY1;
	int useRect = 0;
	{
		std::string s1;
		if (wft_has_param(argc, argv, "-x0", s1, false)) { outX0 = atof(s1.c_str()); useRect++; }
		if (wft_has_param(argc, argv, "-x1", s1, false)) { outX1 = atof(s1.c_str()); useRect++; }
		if (wft_has_param(argc, argv, "-y0", s1, false)) { outY0 = atof(s1.c_str()); useRect++; }
		if (wft_has_param(argc, argv, "-y1", s1, false)) { outY1 = atof(s1.c_str()); useRect++; }
	}

	//2017-11-14 start
	double cutlon0, cutlon1, cutlat0, cutlat1;
	int useLonLatRect = 0;
	{
		std::string s1;
		if (wft_has_param(argc, argv, "-cutlon0", s1, false)) { cutlon0 = atof(s1.c_str()); useLonLatRect++; }
		if (wft_has_param(argc, argv, "-cutlon1", s1, false)) { cutlon1 = atof(s1.c_str()); useLonLatRect++; }
		if (wft_has_param(argc, argv, "-cutlat0", s1, false)) { cutlat0 = atof(s1.c_str()); useLonLatRect++; }
		if (wft_has_param(argc, argv, "-cutlat1", s1, false)) { cutlat1 = atof(s1.c_str()); useLonLatRect++; }
	}
	//2017-11-14 end

	GDALAllRegister();

	GDALDataset* inputDs = 0;
	GDALDataset* lonDs = 0;
	GDALDataset* latDs = 0;
	inputDs = (GDALDataset*)GDALOpen(inputDatasetpath.c_str(), GDALAccess::GA_ReadOnly);
	if (inputDs == 0)
	{
		std::cout << "Error : can not open input file "<< inputDatasetpath << ". out." << std::endl;
		exit(102);
	}
	int xSize = inputDs->GetRasterXSize();
	int ySize = inputDs->GetRasterYSize();
	xStride = (right - left) / xSize;
	yStride = (bottom - top) / ySize;

	double geotrans[6];//2017-11-14

	if (llType == "llfiles")
	{
		lonDs = (GDALDataset*)GDALOpen(lonFile.c_str(), GDALAccess::GA_ReadOnly);
		latDs = (GDALDataset*)GDALOpen(latFile.c_str(), GDALAccess::GA_ReadOnly);
		if (lonDs == 0 || latDs == 0)
		{
			std::cout << "Error : can not open lon lat files. out." << std::endl;
			exit(103);
		}
		int nx1 = lonDs->GetRasterXSize();
		int ny1 = lonDs->GetRasterYSize();
		int nx2 = latDs->GetRasterXSize();
		int ny2 = latDs->GetRasterYSize();
		if (xSize != nx1 || xSize != ny1 || ySize != ny1 || ySize != ny2)
		{
			std::cout << "Error :lon lat files do not have same size with input file. out." << std::endl;
			exit(104);
		}
	}
	//2017-11-14 start
	else if (llType == "geotrans")
	{
		inputDs->GetGeoTransform(geotrans);
	}
	//2017-11-14 end 

	ofstream outfs(outputFile.c_str());
	outfs << "#lon lat val" << std::endl;
	double * buffer = new double[xSize];
	double * lonbuffer = 0; 
	double * latbuffer = 0; 
	if (llType == "llfiles")
	{
		lonbuffer = new double[xSize];
		latbuffer = new double[xSize];
	}
	for (int iy = 0; iy < ySize; ++iy)
	{
		if (useRect == 4) {
			if (outY0 > iy || iy > outY1) continue;
		}
		inputDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy,
											xSize, 1,buffer, xSize, 1, GDALDataType::GDT_Float64, 0, 0);
		if( lonbuffer ){
			lonDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy,
				xSize, 1, lonbuffer, xSize, 1, GDALDataType::GDT_Float64, 0, 0);
			latDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy,
				xSize, 1, latbuffer, xSize, 1, GDALDataType::GDT_Float64, 0, 0);
		}
		if (iy % ySpace != 0) continue;
		bool rowHasValue = false;
		for (int ix = 0; ix < xSize; ++ix)
		{
			if (useRect == 4) {
				if (outX0 > ix || ix > outX1) continue;
			}

			if (ix % xSpace == 0)
			{
				double value = buffer[ix];
				double tlon = -999;
				double tlat = -999;
				if (lonbuffer)
				{
					tlon = lonbuffer[ix];
					tlat = latbuffer[ix];
				}
				else if( llTypeInt==LLRANGE ){//2017-11-14
					tlon = left + (ix + 0.5)*xStride;
					tlat = top + (iy + 0.5)*yStride;
				}
				//2017-11-14 start
				else {
					tlon = geotrans[0] + geotrans[1] * ix + geotrans[2] * iy;
					tlat = geotrans[3] + geotrans[4] * ix + geotrans[5] * iy;
				}
				//2017-11-14 end

				if (tlon > -900 && tlat > -900)
				{
					//2017-11-14 start
					bool isLonLatOk = true;
					if (useLonLatRect == 4)
					{
						if (tlon >= cutlon0 && tlon <= cutlon1 &&
							tlat >= cutlat0 && tlat <= cutlat1)
						{
							isLonLatOk = true;
						}
						else
						{
							isLonLatOk = false;
						}
					}
					//2017-11-14 end

					if (isLonLatOk)//2017-11-14
					{//2017-11-14
						rowHasValue = true;
						if (valid0 <= value && value <= valid1)
						{
							value = value * scale + offset;
							outfs << tlon << " " << tlat << " " << value << endl;
						}
						else if (noNanOutput == false) {
							outfs << tlon << " " << tlat << " " << "NaN" << endl;
						}
					}//2017-11-14
				}
			}
		}
		wft_term_progress(iy, ySize );
		if (rowHasValue)
		{
			outfs << endl;
		}
	}
	delete[] buffer;  
	if( lonbuffer ) delete[] lonbuffer;  
	if( latbuffer ) delete[] latbuffer;  
	GDALClose(inputDs);
	if (lonDs) GDALClose(lonDs);
	if (latDs) GDALClose(latDs);
	outfs.close();

	std::cout << "done." << endl;

    return 0;
}

