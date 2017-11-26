// extractncepwind.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include "../../sharedcodes/wftools.h"
using namespace std;





void outputXyzfile(string outfilepath, int yearday, int year, int month, int day , int xsize , int ysize , float* buffer ,float* buffer1 )
{
	//-125.0f -- 160.0f
	ofstream ofs(outfilepath.c_str());
	ofs << "#x y uwnd vwnd yearday:"<<yearday<<" => year-month-day:"<<year<< "-"<<month<<"-"<<day << endl;
	float x0 = 0.0f;
	float y0 = 90.0;
	for (int iy = 0; iy < ysize; ++iy)
	{
		for (int ix = 0; ix < xsize; ++ix)
		{
			float x = x0 + ix * 2.5;
			x = x - 180;
			float y = y0 - iy * 2.5;
			if (buffer[iy*xsize + ix] >= -125.0f && buffer[iy*xsize + ix] <= 160.f)
			{
				ofs << x << " " << y << " " << buffer[iy*xsize + ix] << " " << buffer1[iy*xsize +ix] << endl;
			}
			else
			{
				ofs << x << " " << y << " NaN NaN " << endl;
			}

		}
		ofs << endl;
	}
	ofs.close();

}

void processOneNcFile(string uncfilepath , string vncfilepath , string outdir )
{
	int numLevels = 17;
	int levelIndex = 2;
	string levelName = "850";
	string ncfilename = wft_base_name(uncfilepath);
	string fileYearStr = ncfilename.substr(5, 4);
	int fileYear = (int)atof(fileYearStr.c_str());

	string udspath = string("NETCDF:\"") + uncfilepath + "\":uwnd" ;
	string vdspath = string("NETCDF:\"") + vncfilepath + "\":vwnd" ;

	GDALDataset* uds = (GDALDataset*)GDALOpen(udspath.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* vds = (GDALDataset*)GDALOpen(vdspath.c_str(), GDALAccess::GA_ReadOnly);
	const int rasterXSize = uds->GetRasterXSize();
	const int rasterYSize = uds->GetRasterYSize();
	int nband = uds->GetRasterCount();
	int nband1 = vds->GetRasterCount();
	if (nband == nband1)
	{
		cout << "x , y , nband  " << endl;
		cout << rasterXSize << " " << rasterYSize << " " << nband << endl;

		float* buffer = new float[rasterXSize*rasterYSize];
		float* buffer1 = new float[rasterXSize*rasterYSize];

		int numberOfDays = (int)(nband / numLevels);
		cout << "number of days in ncfile:" << numberOfDays << endl;

		char fnamebuffer[2048];
		for (int iday = 0; iday < numberOfDays; ++iday)
		{
			int dayOfYear = iday + 1;
			int month, day;
			wft_convertdayofyear2monthday(fileYear, dayOfYear, month, day);
			sprintf(fnamebuffer, "%sncepwind.%d%02d%02d.%s.xyuv.txt", outdir.c_str(), fileYear, month, day, levelName.c_str());
			string xyzfilepath = string(fnamebuffer);
			if (wft_test_file_exists(xyzfilepath) == false)
			{
				cout << "it is making " << xyzfilepath << endl;
				int bandindex = iday * numLevels + levelIndex + 1;
				uds->GetRasterBand(bandindex)->RasterIO(GDALRWFlag::GF_Read, 0, 0, rasterXSize, rasterYSize,
					buffer, rasterXSize, rasterYSize, GDALDataType::GDT_Float32, 0, 0, nullptr);
				vds->GetRasterBand(bandindex)->RasterIO(GDALRWFlag::GF_Read, 0, 0, rasterXSize, rasterYSize,
					buffer1, rasterXSize, rasterYSize, GDALDataType::GDT_Float32, 0, 0, nullptr);
				outputXyzfile(xyzfilepath, dayOfYear, fileYear, month, day, rasterXSize, rasterYSize, buffer, buffer1 );
			}
		}

		delete[] buffer; buffer = 0;
		delete[] buffer1; buffer1 = 0;

		GDALClose(uds);
		GDALClose(vds);
		//change nc file name 
		string ymd = wft_current_dateymd();
		string uncfilepathback = uncfilepath + "." + ymd + ".backup";
		string vncfilepathback = vncfilepath + "." + ymd + ".backup";
		rename(uncfilepath.c_str(), uncfilepathback.c_str());
		rename(vncfilepath.c_str(), vncfilepathback.c_str());
	}
	else {
		cout << "Error: uwnd band " << nband << " is not equal vwnd band " << nband1 << endl;
		GDALClose(uds);
		GDALClose(vds);
	}
}




int main()
{
	//这个地方还需要修改，参数要可以输入的 2017-11-10
	wft_is_leapyear(2014);
	string ncfile = "E:/testdata/testingplots/wind-overlay/uwnd.2017.nc";
	string outdir = "d:/test-wind/";
	string indir = "E:/testdata/testingplots/wind-overlay/";

	GDALAllRegister();

	for (int i = 1000; i < 3000 ; ++i)
	{
		string yearStr = wft_int2str(i);
		string uwndfilepath = indir + "uwnd." + yearStr + ".nc";
		string vwndfilepath = indir + "vwnd." + yearStr + ".nc";
		if (wft_test_file_exists(uwndfilepath) && wft_test_file_exists(vwndfilepath))
		{
			processOneNcFile(uwndfilepath , vwndfilepath , outdir);
		}

	}
	cout << "done." << endl;
    return 0;
}

