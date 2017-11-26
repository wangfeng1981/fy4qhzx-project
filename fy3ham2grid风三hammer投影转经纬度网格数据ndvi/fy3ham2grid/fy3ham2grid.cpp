// fy3sin2noaa.cpp : 定义控制台应用程序的入口点。
//fy3sin投影数据转成noaa avh13ch1 0.05度等经纬格网数据。


#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "../../sharedcodes/wftools.h"
using namespace std;


// FY3B_VIRRX_VVHH_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS
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
	cout << "A program to convert fy3-ndvi into noaa avh13ch1 0.05x0.05 grid data format." << endl;
	cout << "version 0.1a wangfeng1@piesat.cn 2017-10-13." << endl;


	//HDF5:"FY3B_VIRRX_80Q0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF" ://1KM_Monthly_NDVI
	//HDF5 : "FY3B_VIRRX_4080_L3_NVI_MLT_HAM_20170510_AOTD_1000M_MS.HDF" ://1KM_10day_NDVI
	// /root/data/fy3/ndvi/FY3B_VIRRX_8090_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS
	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "fy3ham2grid -indir /root/data/ -type AOAM/AOTD -ymd 20170510 -out out.tif [-rx 0.05 -ry 0.05] " << endl;
			exit(101);
	}

	std::string inDir , inType , inYmd , outFile ;
	wft_has_param(argc, argv, "-indir", inDir, true);
	wft_has_param(argc, argv, "-type", inType, true);
	wft_has_param(argc, argv, "-ymd", inYmd , true);
	wft_has_param(argc, argv, "-out", outFile , true);

	if ( *inDir.rbegin() != '/' ) inDir = inDir + "/";

	double rx(0.05), ry(0.05);
	{
		string temp;
		if (wft_has_param(argc, argv, "-rx", temp, false))
		{
			rx = atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-ry", temp, false))
		{
			ry = atof(temp.c_str());
		}
	}


	string dsName = "1KM_Monthly_NDVI";
	if (inType == "AOAM" ) {
		dsName = "1KM_Monthly_NDVI";
	}
	else if (inType == "AOTD" )
	{
		dsName = "1KM_10day_NDVI";
	}
	else {
		cout << "Error : not supported type  "<< inType << endl;
		exit(102);
	}

	int ocean = -32750;
	int fill = -32768;
	int tileXSize = 1000;//fy3b
	int tileYSize = 1000;
	double fy3resolution = 1002.228;
	const double earthRadius = 6378137;
	const float scale = 0.0001f;
	const float offset = 0.f;

	vector<string> validDsPath;
	vector<int> vtileIndexVector;
	vector<int> htileIndexVector;
	for (int iv = 0; iv < 18; ++iv)
	{
		string vname = fyVertFenFuHaoArray[iv];
		for (int ih = 0; ih < 36; ++ih)
		{
			//HDF5:"FY3B_VIRRX_80Q0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF" ://1KM_Monthly_NDVI
			//HDF5 : "FY3B_VIRRX_4080_L3_NVI_MLT_HAM_20170510_AOTD_1000M_MS.HDF" ://1KM_10day_NDVI
			string hname = fyHoriFenFuHaoArray[ih];
			string filepath = inDir + "FY3B_VIRRX_" + vname + hname + "_L3_NVI_MLT_HAM_" + inYmd + "_"+inType + "_1000M_MS.HDF" ;
			if (wft_test_file_exists(filepath))
			{
				string dspath = string("HDF5:\"") + filepath + "\"://" + dsName;
				validDsPath.push_back(dspath);
				vtileIndexVector.push_back(iv);
				htileIndexVector.push_back(ih);
			}
		}
	}

	if (validDsPath.size() == 0) {
		cout << "Error : no valid fy3 file founded. out." << endl;
		exit(103);
	}

	int outXSize = (int)(360 / rx);
	int outYSize = (int)(180 / ry);

	std::string cntFile = outFile + ".cnt.tif";
	std::string stdevFile = outFile + ".stdev.tif";
	std::string ndviFile = outFile + ".ndvi.tif";
	GDALAllRegister();
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outDs = driver->Create(outFile.c_str(), outXSize, outYSize, 1, GDALDataType::GDT_Int32, nullptr);
	GDALDataset* cntDs = driver->Create(cntFile.c_str(), outXSize, outYSize, 1, GDALDataType::GDT_Int16, nullptr);

	GDALDataset* ndviDs = driver->Create(ndviFile.c_str(), outXSize, outYSize, 1, GDALDataType::GDT_Float32, nullptr);
	GDALDataset* stdevDs = driver->Create(stdevFile.c_str(), outXSize, outYSize, 1, GDALDataType::GDT_Float32, nullptr);

	{
		//#include "ogr_spatialref.h"
		double adfGeoTrans[6] = {-180 , rx , 0 , 90 , 0 , -ry };
		OGRSpatialReference osrs;
		char* pszSRS_WKT = 0;
		outDs->SetGeoTransform(adfGeoTrans);
		cntDs->SetGeoTransform(adfGeoTrans);
		ndviDs->SetGeoTransform(adfGeoTrans);
		stdevDs->SetGeoTransform(adfGeoTrans);
		osrs.SetWellKnownGeogCS("EPSG:4326");
		osrs.exportToWkt(&pszSRS_WKT);
		outDs->SetProjection(pszSRS_WKT);
		cntDs->SetProjection(pszSRS_WKT);
		ndviDs->SetProjection(pszSRS_WKT);
		stdevDs->SetProjection(pszSRS_WKT);
		CPLFree(pszSRS_WKT);
	}

	int* tileBuffer = new int[tileXSize];

	for (size_t ids = 0; ids < validDsPath.size(); ++ids)
	{
		cout << "tile " << ids << "/" << validDsPath.size() << endl;
		GDALDataset* tileDs = (GDALDataset*) GDALOpen( validDsPath[ids].c_str() , GDALAccess::GA_ReadOnly ) ;
		double ffLeftBottomX0 = (htileIndexVector[ids]- 18) * tileXSize * fy3resolution + fy3resolution / 2;
		double ffLeftBottomY0 = (8 - vtileIndexVector[ids] )* tileYSize * fy3resolution + fy3resolution / 2;

		for (int iy = 0; iy < tileYSize; ++iy)
		{
			tileDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, tileXSize, 1, tileBuffer, tileXSize, 1, GDALDataType::GDT_Int32, 0, 0, nullptr);

			for (int ix = 0; ix < tileXSize; ++ix)
			{
				if (tileBuffer[ix] != fill)
				{
					double mapx = ffLeftBottomX0 + ix * fy3resolution;
					double mapy = ffLeftBottomY0 + (tileYSize - 1 - iy)*fy3resolution;
					double mapx2 = mapx / earthRadius;
					double mapy2 = mapy / earthRadius;
					//=SQRT(1-(O2/4)*(O2/4)-(O3/2)*(O3/2))
					double tempZ = sqrt(1.0 - (mapx2 / 4)*(mapx2 / 4) - (mapy2 / 2)*(mapy2 / 2));
					//= 2 * ATAN(O5*O2 / 2 / (2 * O5*O5 - 1)) * 180 / 3.14;
					double fy3lon = 2 * atan(tempZ*mapx2 / 2 / (2 * tempZ*tempZ - 1)) * 180 / M_PI;
					//=ASIN(O5*O3)*180/3.14
					double fy3lat = asin(tempZ*mapy2) * 180 / M_PI;


					int gridx = ( fy3lon + 180.0) / rx;//
					int gridy = ( 90.0 - fy3lat ) / ry;//

					if (gridx >= 0 && gridx < outXSize && gridy >= 0 && gridy < outYSize)
					{
						int ndviSum = 0;
						short ndviCnt = 0;
						
						outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, gridx, gridy, 1, 1, &ndviSum, 1, 1, GDALDataType::GDT_Int32, 0, 0, 0);
						cntDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, gridx, gridy, 1, 1, &ndviCnt, 1, 1, GDALDataType::GDT_Int16, 0, 0, 0);
						

						if (tileBuffer[ix] == ocean)
						{
							if (ndviCnt == 0)
							{
								ndviSum = tileBuffer[ix];
							}
						}
						else {
							float fndvi = tileBuffer[ix] * scale + offset;
							float fndvi2 = fndvi*fndvi;

							float fndvi2sum = 0;
							stdevDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, gridx, gridy, 1, 1, &fndvi2sum, 1, 1, GDALDataType::GDT_Float32, 0, 0, 0);
							

							if (ndviCnt == 0)
							{
								ndviSum = tileBuffer[ix];
								fndvi2sum = fndvi2;
							}
							else {
								ndviSum += tileBuffer[ix];
								fndvi2sum += fndvi2;
							}
							++ndviCnt;
							stdevDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, gridx, gridy, 1, 1, &fndvi2sum, 1, 1, GDALDataType::GDT_Float32, 0, 0, 0);
						}
						outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, gridx, gridy, 1, 1, &ndviSum, 1, 1, GDALDataType::GDT_Int32, 0, 0, 0);
						cntDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, gridx, gridy, 1, 1, &ndviCnt, 1, 1, GDALDataType::GDT_Int16, 0, 0, 0);
						
					}
					else {
						std::cout << "A exception grid xy found " << gridx << " " << gridy << std::endl;
					}

				}
			}
			wft_term_progress(iy, tileYSize);
		}

		GDALClose(tileDs);
	}

	delete[] tileBuffer; tileBuffer = 0;

	//compute average.
	std::cout << "Averaging ... ..." << std::endl;
	int* outBuffer = new int[outXSize];
	int* outBuffer2 = new int[outXSize];
	float* outBufferNdvi = new float[outXSize];
	float* outBufferStdev = new float[outXSize];
	for (int iy = 0; iy < outYSize; ++iy)
	{
		outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, outXSize, 1, outBuffer, outXSize, 1, GDALDataType::GDT_Int32, 0, 0, nullptr);
		cntDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, outXSize, 1, outBuffer2, outXSize, 1, GDALDataType::GDT_Int32, 0, 0, nullptr);
		stdevDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, outXSize, 1, outBufferStdev, outXSize, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
		for (int ix = 0; ix < outXSize; ++ix)
		{
			if (outBuffer2[ix] > 0 )
			{
				double sum2 = outBuffer[ix] *1.0 * scale + offset;
				sum2 = sum2 * sum2;//样本和的平方

				double ndvi2Sum = outBufferStdev[ix];//样本平方的和
				double stdev = sqrt( outBuffer2[ix] * ndvi2Sum - sum2 ) / outBuffer2[ix] ;

				outBuffer[ix] = outBuffer[ix] / outBuffer2[ix];
				outBufferNdvi[ix] = outBuffer[ix] * scale + offset;
				outBufferStdev[ix] = (float)stdev;
			}
			else {
				outBufferNdvi[ix] = -9;
				outBufferStdev[ix] = 0;
				if (outBuffer[ix] != ocean)
				{
					outBuffer[ix] = fill;
				}
			}
		}
		outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, outXSize, 1, outBuffer, outXSize, 1, GDALDataType::GDT_Int32, 0, 0, 0);
		ndviDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, outXSize, 1, outBufferNdvi, outXSize, 1, GDALDataType::GDT_Float32, 0, 0, 0);
		stdevDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, outXSize, 1, outBufferStdev, outXSize, 1, GDALDataType::GDT_Float32, 0, 0, 0);
		wft_term_progress(iy, outYSize);
	}

	delete[] outBuffer; outBuffer = 0;
	delete[] outBuffer2; outBuffer2 = 0;
	delete[] outBufferNdvi; outBufferNdvi = 0;
	delete[] outBufferStdev; outBufferStdev = 0;

	GDALClose(cntDs);
	GDALClose(outDs);
	GDALClose(ndviDs);
	GDALClose(stdevDs);

	std::cout << "done." << std::endl;
	return 0;
}

