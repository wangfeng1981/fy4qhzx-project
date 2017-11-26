// wmosaic.cpp : 定义控制台应用程序的入口点。
//数据拼接 
//目前针对 modis 和fy3b 的影像拼接
//v1.1a 2017-9-30 拼接的输入数据可以通过txt文件逐行读取，不必只通过命令行传入。从新设计命令行参数格式。
//v1.1.1a 10-1修复获取第一个文件尺寸没有加prefix和tail的错误。
//
//v2.0 针对 modis mod10a2 积雪数据 500m分辨率进行拼接。 每个分幅大小为2400x2400


/*
中国区域modis分幅
 h23v03 ... ... h28v03
 ...   ...   ...  ...
 h23v07 ... ... h28v07



*/

#include "stdafx.h"
#include "gdal_priv.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "../../sharedcodes/wftools.h"
using namespace std;


string findstringFromVector(string find, vector<string>& vec)
{
	for (int i = 0; i < vec.size(); ++i)
	{
		if (vec[i].length() > 5)
		{
			string extname = vec[i].substr(vec[i].length() - 4, 4);
			if (vec[i].find(find) != string::npos && extname == ".hdf" )
			{
				return vec[i];
			}
		}
		
	}
	return "";
}


int main(int argc , char** argv )
{
	if (argc == 1)
	{
		std::cout << "*** Program description:" << std::endl;
		std::cout << "A program to mosic images. for mod10a2 only. resolution 500m , each tile has 2400x2400 pixels." << std::endl;
		std::cout << "Version 1.1.1a 2017-9-30 wangfengdev@163.com" << std::endl;
		std::cout << "V2.0 only for mod10a2 data 2017-10-24 wangfengdev@163.com" << std::endl;
		std::cout << "*** Sample call: *** " << std::endl;
		std::cout << " wmosaicmod10a2 -h0 0 -h1 35 -v0 0 -v1 17 -indir c:/somedir/ -out result.tif [-inprefix HDF5:] [-intail ://NDVI] -fill 255 " << std::endl;
		std::cout << " The count of ds files for mosaic must be equal to numXTiles times numYTiles. " << std::endl;
		std::cout << " a null string line for no fill at some tile. " << std::endl;
		std::cout << "*** No enough parameters, out.***" << std::endl;
		exit(101) ;
	}



	std::string inDir;
	wft_has_param(argc, argv, "-indir", inDir, true);

	string hh0, hh1, vv0, vv1;
	wft_has_param(argc, argv, "-h0", hh0, true);
	wft_has_param(argc, argv, "-h1", hh1, true);
	wft_has_param(argc, argv, "-v0", vv0, true);
	wft_has_param(argc, argv, "-v1", vv1, true);
	int h0, h1, v0, v1;
	h0 = (int)atof(hh0.c_str());
	h1 = (int)atof(hh1.c_str());
	v0 = (int)atof(vv0.c_str());
	v1 = (int)atof(vv1.c_str());

	vector<string> allfiles;
	wft_get_allfiles(inDir, allfiles);

	string firstValidFile = "";
	for (int iv = v0; iv <= v1; ++iv)
	{
		for (int ih = h0; ih <= h1; ++ih)
		{
			char hhvv[10];
			sprintf(hhvv, "h%02dv%02d", ih, iv);
			string hhvvstr(hhvv);
			string findfile = findstringFromVector(hhvvstr, allfiles);
			if (findfile.length() > 1)
			{
				firstValidFile = findfile;
				break;
			}

		}
		if (firstValidFile != "")
		{
			break;
		}
	}

	if (firstValidFile == "")
	{
		cout << "no valid file.out." << endl;
		exit(102);
	}
	else {
		cout << "First valid file:" << firstValidFile << endl;
	}


	std::string inputOutputFilepath ;
	wft_has_param(argc, argv, "-out", inputOutputFilepath, true);

	string inPrefix = "";
	string inTail = "";
	wft_has_param(argc, argv, "-inprefix", inPrefix, false);
	wft_has_param(argc, argv, "-intail", inTail, false);

	double fill = 255;
	string  fillstr = "";
	if (wft_has_param(argc, argv, "-fill", fillstr, true))
	{
		fill = atof(fillstr.c_str());
	}


	GDALAllRegister();
	int tileXSize = 0;
	int tileYSize = 0;
	GDALDataType dataType = GDALDataType::GDT_Byte;
	{
		string ds0path = inPrefix + firstValidFile + inTail;
		GDALDataset* tempDs = (GDALDataset*)GDALOpen(ds0path.c_str(), GDALAccess::GA_ReadOnly);
		tileXSize = tempDs->GetRasterXSize();
		tileYSize = tempDs->GetRasterYSize();
		dataType = tempDs->GetRasterBand(1)->GetRasterDataType();
		GDALClose(tempDs);
	}

	if (tileXSize == 0 || tileYSize == 0) {
		std::cout << "*** Error: Invalide tileXSize or tileYSize. out.";
		exit(104) ;
	}

	int inputNumXTiles = h1 - h0 + 1;
	int inputNumYTiles = v1 - v0 + 1;
	int totalXSize = tileXSize * inputNumXTiles;
	int totalYSize = tileYSize * inputNumYTiles;

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outDataset = driver->Create(inputOutputFilepath.c_str(), totalXSize, totalYSize, 1, dataType, nullptr);

	string lonFilepath = inputOutputFilepath + ".lon.tif";
	string latFilepath = inputOutputFilepath + ".lat.tif";

	GDALDataset* latDataset = driver->Create(latFilepath.c_str(), totalXSize, totalYSize, 1, GDALDataType::GDT_Float32, nullptr);
	GDALDataset* lonDataset = driver->Create(lonFilepath.c_str(), totalXSize, totalYSize, 1, GDALDataType::GDT_Float32, nullptr);

	//long lat 
	double resolution = 10.0 / tileXSize; // 10/2400
	int vertTileIndex = v0 ;
	int horiTileIndex = h0 ;


	double* tilebuff = new double[tileXSize];
	double * latValues = new double[tileXSize];
	double * lonValues = new double[tileXSize];
	for (int iv = v0; iv <= v1; ++iv)
	{
		for (int ih = h0; ih <= h1; ++ih)
		{
			int itilex = ih - h0;
			int itiley = iv - v0;

			char hhvv[10];
			sprintf(hhvv, "h%02dv%02d", ih, iv);
			string hhvvstr(hhvv);
			string findfile = findstringFromVector(hhvvstr, allfiles);
			if (findfile.length() > 1)
			{
				cout << hhvvstr << " is ok." << endl;
				string datasetpath = inPrefix + findfile + inTail;
				GDALDataset* dsone = (GDALDataset*)GDALOpen(datasetpath.c_str(), GDALAccess::GA_ReadOnly);
				int xsize1 = dsone->GetRasterXSize();
				int ysize1 = dsone->GetRasterYSize();
				if (xsize1 != tileXSize || ysize1 != tileYSize)
				{
					std::cout << "*** Error: " << findfile << " has a differenct xsize,ysize . out.";
					GDALClose(dsone);
					exit(105);
				}

				for (int iy = 0; iy < tileYSize; ++iy)
				{
					dsone->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, tileXSize, 1, tilebuff, tileXSize, 1,
						GDALDataType::GDT_Float64, 0, 0, nullptr);
					outDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, itilex*tileXSize, itiley*tileYSize + iy, tileXSize, 1,
						tilebuff, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);

					for (int ix = 0; ix < tileXSize; ++ix)
					{
						if ( tilebuff[ix] < 255 )
						{
							latValues[ix] = 90.0 - ((vertTileIndex + itiley) * tileXSize + iy - 0.5) * resolution;
							lonValues[ix] = ((horiTileIndex + itilex - 18) * tileXSize + ix + 0.5) * resolution / cos(latValues[ix] * M_PI / 180.0);
						}
						else {
							latValues[ix] = -999;
							lonValues[ix] = -999;
						}
					}
					lonDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, itilex*tileXSize, itiley*tileYSize + iy,
						tileXSize, 1, lonValues, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
					latDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, itilex*tileXSize, itiley*tileYSize + iy,
						tileXSize, 1, latValues, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
				}
				GDALClose(dsone);
			}
			else {
				cout << hhvvstr << " is null." << endl;
				for (int ix = 0; ix < tileXSize; ++ix)
				{
					tilebuff[ix] = fill;
					lonValues[ix] = -999;
					latValues[ix] = -999;
				}
				for (int iy = 0; iy < tileYSize; ++iy)
				{
					outDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, itilex*tileXSize, itiley*tileYSize + iy, tileXSize, 1,
						tilebuff, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
					lonDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, itilex*tileXSize, itiley*tileYSize + iy,
						tileXSize, 1, lonValues, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
					latDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, itilex*tileXSize, itiley*tileYSize + iy,
						tileXSize, 1, latValues, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
				}
			}
			cout << "tile done." << endl;
		}
	}

	delete[] tilebuff;
	delete[] lonValues;
	delete[] latValues;

	GDALClose(outDataset);
	GDALClose(latDataset);
	GDALClose(lonDataset);
	std::cout << "done." << std::endl;

    return 0;
}



//HDF4_EOS:EOS_GRID:\"mod.h27v05.hdf\":MOD_Grid_monthly_1km_VI:\"1km monthly NDVI;

/* modis
std::vector<std::string> inputDatasetsPathArray = {
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h23v03.006.2017234112103.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h24v03.006.2017234112824.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h25v03.006.2017234111604.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h26v03.006.2017234112024.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h27v03.006.2017234112330.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h28v03.006.2017234111307.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",

"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h23v04.006.2017234112057.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h24v04.006.2017234112252.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h25v04.006.2017234111711.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h26v04.006.2017234111536.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h27v04.006.2017234112219.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h28v04.006.2017234112236.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",

"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h23v05.006.2017234114920.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h24v05.006.2017234112129.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h25v05.006.2017234111857.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h26v05.006.2017234111843.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h27v05.006.2017234111713.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h28v05.006.2017234112619.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",

"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h23v06.006.2017234111524.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h24v06.006.2017234111930.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h25v06.006.2017234111202.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h26v06.006.2017234112354.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h27v06.006.2017234111702.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h28v06.006.2017234112616.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",

"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h23v07.006.2017234111647.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h24v07.006.2017234111333.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h25v07.006.2017234112141.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h26v07.006.2017234111818.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h27v07.006.2017234111119.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h28v07.006.2017234112353.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI"
};
std::string inputOutputFilepath = "E:/testdata/modisndvi/china182.tif";
*/

/* fy3b
std::vector<std::string> inputDatasetsPathArray = {

"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_5070_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_5080_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_5090_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_50A0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_50B0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",


"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_4060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_4070_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_4080_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_4090_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_40A0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_40B0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",


"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_3060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_3070_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_3080_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_3090_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_30A0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_30B0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",


"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_2060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_2070_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_2080_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_2090_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_20A0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_20B0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI"
};*/