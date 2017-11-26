// ncDemo.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include <cstring>
#include <string>
#include "C:\gdaldev\include\gdal_priv.h"
#include "WNetcdfDataFile.h"
#include "WNetcdfSubDataset.h"

int main()
{
	std::cout << "This is a demo for study netcdf." << std::endl;
	GDALAllRegister();

	std::cout << "Input netcdf filepath:";
	std::string filepath = "E:/testdata/fy4-sst.nc";
	//std::cin >> filepath;
	std::cout << "Subdataset Name:";
	std::string subDsName = "LST" ;
	//std::cin >> subDsName;

	std::cout << filepath << " ; " << subDsName << std::endl;

	/*
	
	WNetcdfDataFile ncfile;
	
	ncfile.open(filepath);

	GDALDataset* pDs = ncfile.extractDataset(subDsName);
	std::cout << "Sub Dataset :" << pDs->GetRasterXSize() << " , " << pDs->GetRasterYSize() << std::endl;
	GDALClose(pDs);

	ncfile.close();
	*/
	WNetcdfSubDataset product;
	product.load(filepath, subDsName);
	product.warpToWgs84ByTif("E:/testdata/fy4lon-good.tif", "E:/testdata/fy4lat-good.tif");

	


	std::cout << "Press any key to quit..." << std::endl;
	getchar();
	getchar();
	getchar();

	return 0;

}

