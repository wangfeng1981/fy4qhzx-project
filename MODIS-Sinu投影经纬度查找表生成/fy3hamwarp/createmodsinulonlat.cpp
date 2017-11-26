// createmodsinulonlat.cpp : 定义控制台应用程序的入口点。
//为modis sinu投影产品生成经纬度查找表Tif数据。
// 2017-9-14 wangfengdev@163.com wangfeng1@piesat.cn
/*
GDAL打开HDF，像素原点0，0为图像的左上角的点

-3000 为填充值 ； -2000到10000为有效值

modis sinu tile 图像坐标与经纬度坐标计算工具
https://landweb.modaps.eosdis.nasa.gov/cgi-bin/developer/tilemap.cgi


the left top tile is vert,hori 0,0 
[0,1,...,35] 36 tiles in hori
[0,1,...,17] 18 tiles in vert

each modis tile is 1200*1200
赤道像素像素分辨率x，y 0.00833333333 , 0.00833333333
任意像元经纬度计算公式:
lat = 90 - （ vertTileIndex * 1200 + lines - 0.5 ) * 0.00833333333  
lon = [ ( horiTileIndex - 18 ) * 1200 + samples + 0.5 ] * 0.00833333333 / cos(lat*pi/180)
上述结果单位均为角度

*/

#include "stdafx.h"
#include "gdal_priv.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cmath>




int main(int argc , char** argv )
{
	
	if (argc != 7) {
		std::cout << "*** Program description:" << std::endl;
		std::cout << "A program to generate lon,lat lookup tables of tif file for warping MODIS product from sinu projection to wgs84." << std::endl;
		std::cout << "Version 1.0 2017-9-14 wangfengdev@163.com" << std::endl;
		std::cout << "*** Fixed parameters:" << std::endl;
		std::cout << "pixel resolution at equator x,y:0.00833333333 degree" << std::endl;
		std::cout << "tile size x,y:1200,1200" << std::endl;
		std::cout << "earth radius:6371007.181 meter" << std::endl;
		std::cout << "*** Sample call:" << std::endl;
		std::cout << "createmodsinulonlat" << std::endl;
		std::cout << "\tHDF4_EOS:EOS_GRID:\"mod.h27v05.hdf\":MOD_Grid_monthly_1km_VI:\"1 km monthly NDVI\"" << std::endl;
		std::cout << "\tfenfuhao[h27v05] " << std::endl; 
		std::cout << "\toutput_root[mod.h27v05.hdf] " << std::endl;
		std::cout << "\tvalid_range0[-2000] " << std::endl; 
		std::cout << "\tvalid_range1[10000] " << std::endl;
		std::cout << "\toutput_invalid_lonlat[-999]" << std::endl;

		std::cout << "*** No enough parameters, out.***" << std::endl;
		return 1;
	}

	std::cout << "start working..." << std::endl;

	std::string inputDspath = std::string(argv[1]);// HDF4_EOS:EOS_GRID:\"mod.h27v05.hdf\":MOD_Grid_monthly_1km_VI:\"1 km monthly NDVI;
	std::string inputFenFuHao = std::string(argv[2]);// "h27v05";//分幅号可以自动获取
	std::string inputOutputFileRoot = std::string(argv[3]);// "mod.h27v05.hdf";
	double inputValidDataValue0 = atof(argv[4]);// -2000;
	double inputValidDataValue1 = atof(argv[5]);// 10000;
	double inputOutputNoDataValue = atof(argv[6]);// -999;

	const double resolution = 10.0/1200 ;
	//useless const double earthRadius = 6371007.181;
	const int ffXSize = 1200;
	const int ffYSize = 1200;

	std::string vFenFu = inputFenFuHao.substr(4, 2);
	std::string hFenFu = inputFenFuHao.substr(1, 2);

	int vertTileIndex = atof(vFenFu.c_str()) ;
	int horiTileIndex = atof(hFenFu.c_str()) ;

	std::string latFilepath = inputOutputFileRoot + ".lat.tif";
	std::string lonFilepath = inputOutputFileRoot + ".lon.tif";

	GDALAllRegister();
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");

	GDALDataset* latDataset = driver->Create(latFilepath.c_str(), ffXSize, ffYSize, 1, GDALDataType::GDT_Float32, nullptr);
	GDALDataset* lonDataset = driver->Create(lonFilepath.c_str(), ffXSize, ffYSize, 1, GDALDataType::GDT_Float32, nullptr);

	GDALDataset* dataDataset = (GDALDataset* )GDALOpen(inputDspath.c_str(), GDALAccess::GA_ReadOnly);
	int nband = dataDataset->GetRasterCount();

	double * latValues = new double[ffXSize];
	double * lonValues = new double[ffXSize];
	double * dataValues = new double[ffXSize];
	for (int iy = 0 ; iy < ffYSize ; ++iy)
	{
		dataDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, ffXSize, 1, dataValues, ffXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
		for (int ix = 0; ix < ffXSize; ++ix)
		{
			if (dataValues[ix] >= inputValidDataValue0 && dataValues[ix] <= inputValidDataValue1)
			{
				latValues[ix] = 90.0 - (vertTileIndex * 1200 + iy - 0.5 ) * resolution ;
				lonValues[ix] = ( ( horiTileIndex - 18 )*1200 + ix + 0.5 ) * resolution / cos(latValues[ix]*M_PI/180.0) ;
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
	
	std::cout << "generate modis sinu lon lat done." << std::endl;


    return 0;
}

