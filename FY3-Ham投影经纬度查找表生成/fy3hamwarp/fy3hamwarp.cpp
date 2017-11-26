// fy3hamwarp.cpp : 定义控制台应用程序的入口点。
//为FY3Ham投影产品生成经纬度查找表Tif数据。
// 2017-9-14 wangfengdev@163.com wangfeng1@piesat.cn
/*
bugfixed
2017-9-15 修复bug-915-1,bug-915-2

*/


/*
GDAL打开HDF，像素原点0，0为图像的左上角的点

fy3b-ndvi -32768 表示没有数据像元；-32750表示非陆地像元没有ndvi值;


*/

#include "stdafx.h"
#include "gdal_priv.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cmath>


std::string vertFenFuHaoArray[18] = {"80","70","60","50","40",
								"30","20","10","00",
								"90","A0","B0","C0","D0",
								"E0","F0","G0","H0"};

std::string horiFenFuHaoArray[36] = {
								"Z0","Y0","X0","W0","V0" ,
								"U0","T0","S0","R0","Q0",
								"P0","O0","N0","M0","L0",
								"K0","J0","I0",
								"00","10","20","30","40",
								"50","60","70","80","90",
								"A0","B0","C0","D0","E0",
								"F0","G0","H0"};

int getMatchedStringIndex(std::string ffstr, std::string strArr[] , int arrSize )
{
	for (int i = 0; i < arrSize; ++i)
	{
		if (strArr[i] == ffstr) {
			return i;
		}
	}
	return -1;
}

int main(int argc , char** argv )
{
	
	if (argc != 7) {
		std::cout << "*** Program description:" << std::endl;
		std::cout << "A program to generate lon,lat lookup tables of tif file for warping FY3 product from hammer projection to wgs84." << std::endl;
		std::cout << "Version 1.0.2 2017-9-14 wangfengdev@163.com" << std::endl;
		std::cout << "*** Fixed parameters:" << std::endl;
		std::cout << "pixel resolution x,y:1002.228 meter" << std::endl;
		std::cout << "tile size x,y:1000,1000" << std::endl;
		std::cout << "earth radius:6378137 meter" << std::endl;
		std::cout << "*** Sample call:" << std::endl;
		std::cout << "createfy3hamwarplonlat hdf5:'fy3bxxxx.hdf'://product_name fenfuhao[00A0] output_root[fy3bxxxx.hdf] valid_range0[-372751] valid_range1[32750] output_invalid_lonlat[-999]" << std::endl;

		std::cout << "*** No enough parameters, out.***" << std::endl;
		return 1;
	}

	std::cout << "start working..." << std::endl;

	std::string inputFy3HamFilepath = std::string(argv[1]);// "HDF5:\"E:/testdata/fy3bvirrndvi/fy3b-0000.hdf\"://1KM_Monthly_NDVI";
	std::string inputFenFuHao = std::string(argv[2]);// "0000";//分幅号可以自动获取
	std::string inputOutputFileRoot = std::string(argv[3]);// "E:/testdata/fy3bvirrndvi/fy3b-0000.hdf";
	double inputValidDataValue0 = atof(argv[4]);// -32751;
	double inputValidDataValue1 = atof(argv[5]);// 32750;
	double inputOutputNoDataValue = atof(argv[6]);// -999;


	

	const double resolution = 1002.228;
	const double earthRadius = 6378137;
	const int ffXSize = 1000;
	const int ffYSize = 1000;

	std::string vFenFu = inputFenFuHao.substr(0, 2);
	std::string hFenFu = inputFenFuHao.substr(2, 2);

	int vffIndex = getMatchedStringIndex(vFenFu, vertFenFuHaoArray, 18);
	int hffIndex = getMatchedStringIndex(hFenFu, horiFenFuHaoArray, 36);

	//bug-915-1 double ffLeftBottomX0 = (hffIndex - 18)*ffYSize*resolution + resolution / 2;  
	//bug-915-2 double ffLeftBottomY0 = (8 - vffIndex)*ffXSize*resolution + resolution / 2;
	double ffLeftBottomX0 = (hffIndex - 18)*ffXSize*resolution + resolution / 2; //bugfixed 915-1
	double ffLeftBottomY0 = (8 - vffIndex)*ffYSize*resolution + resolution / 2;//bugfixed 915-2

	std::string latFilepath = inputOutputFileRoot + ".lat.tif";
	std::string lonFilepath = inputOutputFileRoot + ".lon.tif";

	GDALAllRegister();
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");

	GDALDataset* latDataset = driver->Create(latFilepath.c_str(), ffXSize, ffYSize, 1, GDALDataType::GDT_Float32, nullptr);
	GDALDataset* lonDataset = driver->Create(lonFilepath.c_str(), ffXSize, ffYSize, 1, GDALDataType::GDT_Float32, nullptr);

	GDALDataset* dataDataset = (GDALDataset* )GDALOpen(inputFy3HamFilepath.c_str(), GDALAccess::GA_ReadOnly);

	double * latValues = new double[ffXSize];
	double * lonValues = new double[ffXSize];
	double * dataValues = new double[ffXSize];
	for (int iy = ffYSize-1 ; iy >= 0 ; --iy)
	{
		dataDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, ffXSize, 1, dataValues, ffXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
		for (int ix = 0; ix < ffXSize; ++ix)
		{
			if (dataValues[ix] >= inputValidDataValue0 && dataValues[ix] <= inputValidDataValue1)
			{
				double mapx = ffLeftBottomX0 + ix * resolution;
				double mapy = ffLeftBottomY0 + (ffYSize - 1- iy )*resolution;

				double mapx2 = mapx / earthRadius;
				double mapy2 = mapy / earthRadius;
				//=SQRT(1-(O2/4)*(O2/4)-(O3/2)*(O3/2))
				double tempZ = sqrt(1.0 - (mapx2 / 4)*(mapx2 / 4) - (mapy2 / 2)*(mapy2 / 2));
				//= 2 * ATAN(O5*O2 / 2 / (2 * O5*O5 - 1)) * 180 / 3.14;
				double templon = 2*atan(tempZ*mapx2/2/(2*tempZ*tempZ-1))*180/M_PI;
				//=ASIN(O5*O3)*180/3.14
				double templat = asin(tempZ*mapy2)*180/M_PI;
				latValues[ix] = templat;
				lonValues[ix] = templon;
			}
			else {
				latValues[ix] = inputOutputNoDataValue;
				lonValues[ix] = inputOutputNoDataValue;
			}
		}
		lonDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, ffXSize, 1, lonValues, ffXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
		latDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, ffXSize, 1, latValues, ffXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
	}

	delete[] latValues;
	delete[] lonValues;
	delete[] dataValues;

	GDALClose(latDataset);
	GDALClose(lonDataset);
	GDALClose(dataDataset);
	
	std::cout << "generate fy3 lon lat done." << std::endl;


    return 0;
}

