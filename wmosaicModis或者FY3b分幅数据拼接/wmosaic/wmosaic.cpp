// wmosaic.cpp : 定义控制台应用程序的入口点。
//数据拼接 
//目前针对 modis 和fy3b 的影像拼接
//v1.1a 2017-9-30 拼接的输入数据可以通过txt文件逐行读取，不必只通过命令行传入。从新设计命令行参数格式。
//v1.1.1a 10-1修复获取第一个文件尺寸没有加prefix和tail的错误。


/*
中国区域modis分幅
 h23v03 ... ... h28v03
 ...   ...   ...  ...
 h23v07 ... ... h28v07

 中国区fy3b分幅
 5060  5070  ...  50A0  50B0
 ... ... ... ...
 2060  2070  ...  20A0  20B0


*/

#include "stdafx.h"
#include "gdal_priv.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "../../sharedcodes/wftools.h"
using namespace std;




int main(int argc , char** argv )
{
	if (argc == 1)
	{
		std::cout << "*** Program description:" << std::endl;
		std::cout << "A program to mosic images." << std::endl;
		std::cout << "Version 1.1.1a 2017-9-30 wangfengdev@163.com" << std::endl;
		std::cout << "*** Sample call: *** " << std::endl;
		std::cout << " wmosaic -ntx numXTiles -nty numYTiles -out result.tif [-inprefix HDF5:] [-intail ://NDVI] -infiles file0.hdf file1.hdf ... " << std::endl;
		std::cout << " wmosaic -ntx numXTiles -nty numYTiles -out result.tif [-inprefix HDF5:] [-intail ://NDVI] -infilestxt filelist.txt " << std::endl;
		std::cout << " The count of ds files for mosaic must be equal to numXTiles times numYTiles. " << std::endl;
		std::cout << "*** No enough parameters, out.***" << std::endl;
		exit(101) ;
	}


	int inputNumXTiles = 0;
	int inputNumYTiles = 0;
	{
		string str1;
		wft_has_param(argc, argv, "-ntx", str1, true);
		inputNumXTiles = (int)atof(str1.c_str());
	}
	{
		string str1;
		wft_has_param(argc, argv, "-nty", str1, true);
		inputNumYTiles = (int)atof(str1.c_str());
	}

	std::string inputOutputFilepath ;
	wft_has_param(argc, argv, "-out", inputOutputFilepath, true);

	string inPrefix = "";
	string inTail = "";
	wft_has_param(argc, argv, "-inprefix", inPrefix, false);
	wft_has_param(argc, argv, "-intail", inTail, false);

	bool isInFiles = wft_has_tag(argc, argv, "-infiles");
	bool isFilelist = wft_has_tag(argc, argv, "-infilestxt");
	if (isInFiles == false && isFilelist == false)
	{
		std::cout << "Error : -infiles or -infilestxt at least has one." << std::endl;
		exit(102);
	}

	std::vector<std::string> inputFileVector;
	if (isInFiles)
	{
		int i0 = 0;
		for (int i = 0; i < argc; ++i)
		{
			if (strcmp(argv[i], "-infiles") == 0) {
				i0 = i + 1;
				break;
			}
		}
		for (int i = i0; i < argc; ++i)
		{
			std::string tempdspath = std::string(argv[i]);
			inputFileVector.push_back(tempdspath);
		}
	}
	else
	{
		string fileListFile;
		wft_has_param(argc, argv, "-infilestxt", fileListFile, true);
		ifstream ifs(fileListFile.c_str());
		std::string line;
		while (std::getline(ifs, line))
		{
			if (line.length() > 1)
			{
				inputFileVector.push_back(line);
			}
		}
		ifs.close();
	}

	int numDatasets = (int)inputFileVector.size();
	if (numDatasets != inputNumXTiles * inputNumYTiles) {
		std::cout << "*** Error: Number of datasets is not equal with numXTiles * numYTiles. out.";
		exit(103);
	}
	GDALAllRegister();
	int tileXSize = 0;
	int tileYSize = 0;
	GDALDataType dataType = GDALDataType::GDT_Byte;
	{
		string ds0path = inPrefix + inputFileVector[0] + inTail;
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

	int totalXSize = tileXSize * inputNumXTiles;
	int totalYSize = tileYSize * inputNumYTiles;
	int ids = 0;

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outDataset = driver->Create(inputOutputFilepath.c_str(), totalXSize, totalYSize, 1, dataType, nullptr);

	double* tilebuff = new double[tileXSize];
	for (int itiley = 0; itiley < inputNumYTiles; ++itiley)
	{
		for (int itilex = 0; itilex < inputNumXTiles; ++itilex)
		{
			string datasetpath = inPrefix + inputFileVector[ids] + inTail;
			GDALDataset* dsone = (GDALDataset*)GDALOpen( datasetpath.c_str(), GDALAccess::GA_ReadOnly);
			int xsize1 = dsone->GetRasterXSize();
			int ysize1 = dsone->GetRasterYSize();
			if (xsize1 != tileXSize || ysize1 != tileYSize)
			{
				std::cout << "*** Error: "<< inputFileVector[ids] << " has a differenct xsize,ysize . out.";
				GDALClose(dsone);
				exit(105) ;
			}

			for (int iy = 0; iy < tileYSize; ++iy)
			{
				dsone->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, tileXSize, 1, tilebuff, tileXSize, 1,
					GDALDataType::GDT_Float64, 0, 0, nullptr);
				outDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, itilex*tileXSize , itiley*tileYSize+iy, tileXSize, 1,
					tilebuff, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
			}

			GDALClose(dsone);
			std::cout << "tile " << ids << " of "<< numDatasets<<" is done." << std::endl;
			++ids;
			
		}
	}
	delete[] tilebuff;

	GDALClose(outDataset);
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