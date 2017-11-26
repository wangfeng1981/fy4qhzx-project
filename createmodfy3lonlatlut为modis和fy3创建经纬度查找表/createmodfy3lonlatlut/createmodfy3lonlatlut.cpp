// createmodfy3lonlatlut.cpp : 定义控制台应用程序的入口点。
//用于为modis sinu或fy3b hammer分幅与拼接数据创建经纬度查找表。
//wangfeng1@piesat.cn 2017-9-15
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

//0.1a 修改命令行参数格式，接受文本文件描述的数据列表。

#include "stdafx.h"
#include "gdal_priv.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cmath>
#include "../../sharedcodes//wftools.h"
using namespace std;

std::string fyVertFenFuHaoArray[18] = { "80","70","60","50","40",
"30","20","10","00",
"90","A0","B0","C0","D0",
"E0","F0","G0","H0" };

std::string fyHoriFenFuHaoArray[36] = {
	"Z0","Y0","X0","W0","V0" ,
	"U0","T0","S0","R0","Q0",
	"P0","O0","N0","M0","L0",
	"K0","J0","I0",
	"00","10","20","30","40",
	"50","60","70","80","90",
	"A0","B0","C0","D0","E0",
	"F0","G0","H0" };

int getMatchedStringIndex(std::string ffstr, std::string strArr[], int arrSize)
{
	for (int i = 0; i < arrSize; ++i)
	{
		if (strArr[i] == ffstr) {
			return i;
		}
	}
	return -1;
}


int main(int argc, char** argv)
{

	if (argc == 1 ) {
		std::cout << "*** Program description:" << std::endl;
		std::cout << "A program to generate lon,lat lookup tables of tif file for Modis Sinu or Fy3b hammer product." << std::endl;
		std::cout << "Version 0.0.1a 2017-9-15 wangfengdev@163.com" << std::endl;
		std::cout << "Version 0.1a 2017-10-1 wangfengdev@163.com" << std::endl;
		std::cout << "Version 0.1.1a 2017-10-1 wangfengdev@163.com" << std::endl;
		std::cout << "*** Sample call: ***" << std::endl;
		std::cout << "createmodfy3blonlatlut -type mod/fy3b -lefttoptilename 5060 "
			" -ntx numXtiles -nty numYTiles "
			" [-inprefix HDF5:\\\"] [-intail \\\":1km_Monthly_NDVI] "
			" -outlon outlonpath -outlat outlatpath " 
			" [-infiles file0 file1  ...] / [-infilestxt filelist.txt]" << std::endl;
		std::cout << "*** No enough parameters, out.***" << std::endl;
		exit(101);
	}

	std::cout << "start working..." << std::endl;

	std::string inputMode;
	wft_has_param(argc, argv, "-type", inputMode, true);

	std::string inputFenFuHao  ;
	wft_has_param(argc, argv, "-lefttoptilename", inputFenFuHao, true);


	int inputNumXTiles = 0;
	int inputNumYTiles =0 ;
	{
		string tstr;
		wft_has_param(argc, argv, "-ntx", tstr , true);
		inputNumXTiles = atof(tstr.c_str());
	}
	{
		string tstr;
		wft_has_param(argc, argv, "-nty", tstr, true);
		inputNumYTiles = atof(tstr.c_str());
	}


	std::string lonFilepath;
	wft_has_param(argc, argv, "-outlon", lonFilepath, true);
	std::string latFilepath;
	wft_has_param(argc, argv, "-outlat", latFilepath, true);

	std::string inPrefix;
	wft_has_param(argc, argv, "-inprefix", inPrefix, false);
	std::string inTail;
	wft_has_param(argc, argv, "-intail", inTail, false);


	std::vector<std::string> inputHdfVector;
	if (wft_has_tag(argc , argv , "-infiles") )
	{
		int i0 = wft_tag_index(argc, argv, "-infiles") + 1;
		for (int i = i0; i < argc; ++i)
		{
			std::string temphdf = std::string(argv[i]);
			inputHdfVector.push_back(temphdf);
		}
	}
	else if (wft_has_tag(argc, argv, "-infilestxt"))
	{
		string txtfile;
		wft_has_param(argc, argv, "-infilestxt", txtfile , true);
		inputHdfVector = wft_get_filelist_from_file(txtfile, "", "");
	}
	else {
		std::cout << "Error : not found -infiles or -infilestxt parameter. out." << std::endl;
		exit(102);
	}

	if( inputNumXTiles * inputNumYTiles != inputHdfVector.size() ) {
		std::cout << "*** Error: inputNumXTiles * inputNumYTiles is not equal hdf file count . out." << std::endl;
		return 1001;
	}

	double validDataValue0 = 0; 
	double validDataValue1 = 0; 
	double outputLonLatNoDataValue = -999; 
	double resolution = 0;
	int tileXSize = 0;
	int tileYSize = 0;
	int vertTileIndex = 0;
	int horiTileIndex = 0;

	const double earthRadius = 6378137;
	if (inputMode == "mod")
	{
		tileXSize = 1200;
		tileYSize = 1200;
		resolution = 10.0 / 1200; // 0.008333333333333333
		validDataValue0 = -2001;
		validDataValue1 = 10000;
		//inputDspath = std::string("HDF4_EOS:EOS_GRID:\"") + inputHdf + "\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI";

		std::string vFenFu = inputFenFuHao.substr(4, 2);
		std::string hFenFu = inputFenFuHao.substr(1, 2);
		vertTileIndex = atof(vFenFu.c_str());
		horiTileIndex = atof(hFenFu.c_str());

	}
	else if (inputMode == "fy3b")
	{
		tileXSize = 1000;
		tileYSize = 1000;
		resolution = 1002.228;
		validDataValue0 = -32751;// -32751;
		validDataValue1 =  32750 ;// 32750;
		//inputDspath = std::string("HDF5:\"") + inputHdf + "\"://1KM_Monthly_NDVI";

		std::string vFenFu = inputFenFuHao.substr(0, 2);
		std::string hFenFu = inputFenFuHao.substr(2, 2);
		vertTileIndex = getMatchedStringIndex(vFenFu, fyVertFenFuHaoArray, 18);
		horiTileIndex = getMatchedStringIndex(hFenFu, fyHoriFenFuHaoArray, 36);

	}
	else {
		std::cout << "*** Error: Unkown mode :"<<inputMode << std::endl;
		exit(103) ;
	}

	GDALAllRegister();
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");

	int totalXSize = tileXSize * inputNumXTiles;
	int totalYSize = tileYSize * inputNumYTiles;

	GDALDataset* latDataset = driver->Create(latFilepath.c_str(), totalXSize, totalYSize, 1, GDALDataType::GDT_Float32, nullptr);
	GDALDataset* lonDataset = driver->Create(lonFilepath.c_str(), totalXSize, totalYSize, 1, GDALDataType::GDT_Float32, nullptr);

	double * latValues = new double[tileXSize];
	double * lonValues = new double[tileXSize];
	double * dataValues = new double[tileXSize];
	int ifile = 0;
	for (int iTileY = 0; iTileY < inputNumYTiles; ++iTileY)
	{
		for (int iTileX = 0; iTileX < inputNumXTiles; ++iTileX)
		{
			double ffLeftBottomX0 = 0;//used for fy3
			double ffLeftBottomY0 = 0;//used for fy3
			std::string hdfDatasetPath = "";
			if (inputMode == "mod")
			{
				//2017-10-1 hdfDatasetPath = std::string("HDF4_EOS:EOS_GRID:\"") + inputHdfVector[ifile] + "\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI";
				hdfDatasetPath = inPrefix + inputHdfVector[ifile] + inTail ;
				
			}
			else{
				//2017-10-1 hdfDatasetPath = std::string("HDF5:\"") + inputHdfVector[ifile] + "\"://1KM_Monthly_NDVI";
				hdfDatasetPath = inPrefix + inputHdfVector[ifile] + inTail ;
				ffLeftBottomX0 = ( horiTileIndex+iTileX - 18)*tileXSize*resolution + resolution / 2;
				ffLeftBottomY0 = (8 - (vertTileIndex + iTileY) )*tileYSize*resolution + resolution / 2;

			}
			GDALDataset* dataDataset = (GDALDataset*)GDALOpen(hdfDatasetPath.c_str(), GDALAccess::GA_ReadOnly);
			int xs = dataDataset->GetRasterXSize();
			int ys = dataDataset->GetRasterYSize();
			if (tileXSize != xs || tileYSize != ys) {
				std::cout << "*** Error: the dataset's xsize or ysize is not equal with others. skip." << std::endl;
			}
			else {
				for (int iy = 0; iy < tileYSize; ++iy)
				{
					dataDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, tileXSize, 1, dataValues, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
					if (inputMode == "mod") {
						for (int ix = 0; ix < tileXSize; ++ix)
						{
							if (dataValues[ix] >= validDataValue0 && dataValues[ix] <= validDataValue1)
							{
								latValues[ix] = 90.0 - ( (vertTileIndex+iTileY) * 1200 + iy - 0.5) * resolution;
								lonValues[ix] = ((horiTileIndex+iTileX - 18) * 1200 + ix + 0.5) * resolution / cos(latValues[ix] * M_PI / 180.0);
							}
							else {
								latValues[ix] = outputLonLatNoDataValue;
								lonValues[ix] = outputLonLatNoDataValue;
							}
						}
					}
					else {
						for (int ix = 0; ix < tileXSize; ++ix)
						{
							if (dataValues[ix] >= validDataValue0 && dataValues[ix] <= validDataValue1)
							{
								double mapx = ffLeftBottomX0 + ix * resolution;
								double mapy = ffLeftBottomY0 + (tileYSize - 1 - iy)*resolution;

								double mapx2 = mapx / earthRadius;
								double mapy2 = mapy / earthRadius;
								//=SQRT(1-(O2/4)*(O2/4)-(O3/2)*(O3/2))
								double tempZ = sqrt(1.0 - (mapx2 / 4)*(mapx2 / 4) - (mapy2 / 2)*(mapy2 / 2));
								//= 2 * ATAN(O5*O2 / 2 / (2 * O5*O5 - 1)) * 180 / 3.14;
								double templon = 2 * atan(tempZ*mapx2 / 2 / (2 * tempZ*tempZ - 1)) * 180 / M_PI;
								//=ASIN(O5*O3)*180/3.14
								double templat = asin(tempZ*mapy2) * 180 / M_PI;
								latValues[ix] = templat;
								lonValues[ix] = templon;
							}
							else {
								latValues[ix] = outputLonLatNoDataValue;
								lonValues[ix] = outputLonLatNoDataValue;
							}
						}
					}
					lonDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, iTileX*tileXSize, iTileY*tileYSize+iy,
						tileXSize, 1, lonValues, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
					latDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, iTileX*tileXSize, iTileY*tileYSize+iy,
						tileXSize, 1, latValues, tileXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
				}
			}
			std::cout << "file " << ifile << " is done." << std::endl;
			++ifile;
			GDALClose(dataDataset);
		}
	}

	delete[] latValues;
	delete[] lonValues;
	delete[] dataValues;

	GDALClose(latDataset);
	GDALClose(lonDataset);

	std::cout << "All done." << std::endl;


	return 0;
}


