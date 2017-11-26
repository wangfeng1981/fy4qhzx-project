// fy4disk2grid.cpp : 定义控制台应用程序的入口点。
//风4圆盘投影数据转成grid等经纬格网数据。


#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "../../sharedcodes/wftools.h"
using namespace std;






int main(int argc, char** argv)
{
	cout << "A program to convert fy4 disk into grid data format." << endl;
	cout << "version 0.1a wangfeng1@piesat.cn 2017-10-19." << endl;
	cout << "version 0.1.1a . add fillgap ." << endl;
	cout << "version 0.1.2a . bugfix geotrans ." << endl;
	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "fy4disk2grid -in fy4.nc -lon fy4lon.tif -lat fy4lat.tif -out fy4grid.tif -valid0 0 -valid1 900 -fill -999 "
			" [-rx 0.05 -ry 0.05] " 
			" [-scale 1.0 -offset 0]"
			" [-fillgap 0]"
			<< endl;
		exit(101);
	}

	std::string inFile , lonFile , latFile , outFile ;
	wft_has_param(argc, argv, "-in", inFile, true);
	wft_has_param(argc, argv, "-lon", lonFile, true);
	wft_has_param(argc, argv, "-lat", latFile , true);
	wft_has_param(argc, argv, "-out", outFile , true);

	double rx(0.05), ry(0.05) , scale(1) , offset(0) ;
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
		if (wft_has_param(argc, argv, "-scale", temp, false))
		{
			scale = atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-offset", temp, false))
		{
			offset = atof(temp.c_str());
		}
	}
	double valid0, valid1 , fill ;
	{
		string temp;
		wft_has_param(argc, argv, "-valid0", temp, true);
		valid0 = atof(temp.c_str());
		wft_has_param(argc, argv, "-valid1", temp, true);
		valid1 = atof(temp.c_str());
		wft_has_param(argc, argv, "-fill", temp, true);
		fill = atof(temp.c_str());
	}
	int fillGap = 0;
	{
		string temp;
		if (wft_has_param(argc, argv, "-fillgap", temp, false))
		{
			fillGap = (int)atof(temp.c_str());
		}
	}

	int outXSize = (int)(360 / rx);
	int outYSize = (int)(180 / ry);

	GDALAllRegister();

	GDALDataset* inDs = (GDALDataset*)GDALOpen(inFile.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataType inDataType = inDs->GetRasterBand(1)->GetRasterDataType();
	int inXSize = inDs->GetRasterXSize();
	int inYSize = inDs->GetRasterYSize();
	GDALDataset* lonDs = (GDALDataset*)GDALOpen(lonFile.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* latDs = (GDALDataset*)GDALOpen(latFile.c_str(), GDALAccess::GA_ReadOnly);
	if (lonDs->GetRasterXSize() != inXSize || latDs->GetRasterYSize() != inYSize
		|| lonDs->GetRasterYSize() != inYSize || latDs->GetRasterXSize() != inXSize)
	{
		cout << "Error: Fy4 lon lat file do not have same x y size.out." << endl;
		exit(102);
	}
	string cntFile = outFile + ".cnt.tif";
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outDs = driver->Create(outFile.c_str(), outXSize, outYSize, 1, GDALDataType::GDT_Float32, nullptr);
	GDALDataset* cntDs = driver->Create(cntFile.c_str(), outXSize, outYSize, 1, GDALDataType::GDT_Int16, nullptr);
	{
		double adfGeoTrans[6] = {-180 , rx , 0 , 90 , 0 , -ry };//bugfixed 2017-10-19 version 0.1.2a.
		OGRSpatialReference osrs;
		char* pszSRS_WKT = 0;
		outDs->SetGeoTransform(adfGeoTrans);
		osrs.SetWellKnownGeogCS("EPSG:4326");
		osrs.exportToWkt(&pszSRS_WKT);
		outDs->SetProjection(pszSRS_WKT);
		CPLFree(pszSRS_WKT);
	}

	double* inBuffer = new double[inXSize];
	double* lonBuffer = new double[inXSize];
	double* latBuffer = new double[inXSize];
	for (int iy = 0; iy < inYSize; ++iy)
	{
		inDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, inXSize, 1, inBuffer, inXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
		lonDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, inXSize, 1, lonBuffer, inXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
		latDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, inXSize, 1, latBuffer, inXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);

		for (int ix = 0; ix < inXSize; ++ix)
		{
			float inval = inBuffer[ix];
			float lon = lonBuffer[ix];
			float lat = latBuffer[ix];
			if (lon > 180.f) lon = lon - 360;
			int gridx = (lon + 180) / rx;
			int gridy = (lat - 90) / (-ry);

			if (-181 < lon && lon < 361 && -91 < lat && lat < 91 )
			{

				if (gridx < outXSize && gridy < outYSize && gridx >= 0 && gridy >= 0)
				{
					float sumval = 0;
					short cntval = 0;

					outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, gridx, gridy, 1, 1, &sumval, 1, 1, GDALDataType::GDT_Float32, 0, 0, 0);
					cntDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, gridx, gridy, 1, 1, &cntval, 1, 1, GDALDataType::GDT_Int16, 0, 0, 0);

					if (valid0 <= inval && inval <= valid1)
					{
						if (cntval < 0) {
							sumval = 0;
							cntval = 0;
						}
						inval = inval * scale + offset;
						sumval += inval;
						cntval++;
						outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, gridx, gridy, 1, 1, &sumval, 1, 1, GDALDataType::GDT_Float32, 0, 0, 0);
						cntDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, gridx, gridy, 1, 1, &cntval, 1, 1, GDALDataType::GDT_Int16, 0, 0, 0);
					}
					else if( cntval == 0 ){
						short fillval = -1;
						cntDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, gridx, gridy, 1, 1, &fillval, 1, 1, GDALDataType::GDT_Int16, 0, 0, 0);
					}
				}
				else
				{
					cout << "find a input lon lat value out of grid range. continue ." << endl;
				}

				
				
				
			}
		}
		wft_term_progress(iy, inYSize);
	}
	delete[] inBuffer; inBuffer = 0;
	delete[] lonBuffer; lonBuffer = 0;
	delete[] latBuffer; latBuffer = 0;
	GDALClose(inDs);
	GDALClose(lonDs);
	GDALClose(latDs);

	//compute average.
	std::cout << "Averaging ... ..." << std::endl;
	float* outBuffer = new float[outXSize];
	short* outBuffer2 = new short[outXSize];
	for (int iy = 0; iy < outYSize; ++iy)
	{
		outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, outXSize, 1, outBuffer, outXSize, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
		cntDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, outXSize, 1, outBuffer2, outXSize, 1, GDALDataType::GDT_Int16, 0, 0, nullptr);
		for (int ix = 0; ix < outXSize; ++ix)
		{
			if (outBuffer2[ix] > 0 )
			{
				outBuffer[ix] = outBuffer[ix] / outBuffer2[ix];
			}
			else {
				outBuffer[ix] = fill;
			}
		}
		outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, outXSize, 1, outBuffer, outXSize, 1, GDALDataType::GDT_Float32, 0, 0, 0);
		wft_term_progress(iy, outYSize);
	}

	if (fillGap==1)
	{
		cout << "filling the gaps... ..." << endl;
		for (int iy = 0; iy < outYSize; ++iy)
		{
			outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, outXSize, 1, outBuffer, outXSize, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
			cntDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, outXSize, 1, outBuffer2, outXSize, 1, GDALDataType::GDT_Int16, 0, 0, nullptr);
			for (int ix = 0; ix < outXSize; ++ix)
			{
				if (ix > 0 && ix < outXSize - 2 && iy>0 && iy < outYSize - 2)
				{
					if (outBuffer2[ix] == 0)
					{
						float tval[9];
						short tcnt[9];
						outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, ix - 1, iy - 1, 3, 3, tval, 3, 3, GDALDataType::GDT_Float32, 0, 0, nullptr);
						cntDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, ix - 1, iy - 1, 3, 3, tcnt, 3, 3, GDALDataType::GDT_Int16, 0, 0, nullptr);
						bool hasInvalidAround = false;
						float aver = 0;
						int   validCnt = 0;
						for (int iround = 0; iround < 9; ++iround)
						{
							if (tcnt[iround] < 0) {
								hasInvalidAround = true;
								break;
							}
							else if( tcnt[iround]>0 ) {
								aver += tval[iround];
								validCnt++;
							}
						}
						if (hasInvalidAround == false && validCnt > 0 )
						{
							aver = aver / validCnt;
							outBuffer[ix] = aver;
						}
					}
				}
				
			}
			outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, outXSize, 1, outBuffer, outXSize, 1, GDALDataType::GDT_Float32, 0, 0, 0);
			wft_term_progress(iy, outYSize);
		}
	}
	
	

	delete[] outBuffer; outBuffer = 0;
	delete[] outBuffer2; outBuffer2 = 0;

	GDALClose(cntDs);
	GDALClose(outDs);


	std::cout << "done." << std::endl;
	return 0;
}

