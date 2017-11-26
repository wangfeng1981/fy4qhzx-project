// image2xyz.cpp : 定义控制台应用程序的入口点。
//使用gdal基于图像的geotrans输出x y z的文本文件。 2017-11-7


#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include "../../sharedcodes/wftools.h"
#include <vector>
#include <string>
#include <iomanip>

using namespace std;



int main(int argc , char** argv )
{
	std::cout << "A program to use image's geotrans to output x,y,z ascii file. wangfeng1@piesat.cn." << endl;
	std::cout << "Version 0.1a .2017-11-7." << endl;
	std::cout << "Version 0.2a .region output.2017-11-7." << endl;
	std::cout << "Version 0.3a .output record info and yspace bug fixed. 2017-11-26." << endl;//2017-11-26

	if (argc == 1)
	{
		std::cout << "sample call:" << endl;

		std::cout << " image2xyz -in some.tif -out output.txt -valid0 -2 -valid1 10 [-scale 1] [-offset 0] [-xspace 1] [-yspace 1] [-nan NaN] [-x0 0 -x1 1 -y0 0 -y1 1]" << endl;

		std::cout << endl;
		std::cout << endl;
		std::cout << endl;

		exit(101);
	}


	string infile = "";
	wft_has_param(argc, argv, "-in", infile, true);
	string outfile = "";
	wft_has_param(argc, argv, "-out", outfile, true);

	string valid0str = "";
	wft_has_param(argc, argv, "-valid0", valid0str, true);
	string valid1str = "";
	wft_has_param(argc, argv, "-valid1", valid1str, true);

	string scalestr = "";
	wft_has_param2(argc, argv, "-scale", scalestr, false, "1");

	string offsetstr = "";
	wft_has_param2(argc, argv, "-offset", offsetstr, false, "0");

	string xspacestr = "";
	wft_has_param2(argc, argv, "-xspace", xspacestr, false, "1");
	string yspacestr = "";
	wft_has_param2(argc, argv, "-yspace", yspacestr, false, "1");

	string valNan = "NaN";
	wft_has_param2(argc, argv, "-nan", valNan, false, "NaN");//bugfixed 2017-11-26

	string x0str = "";
	wft_has_param2(argc, argv, "-x0", x0str, false, "");
	string x1str = "";
	wft_has_param2(argc, argv, "-x1", x1str, false, "");
	string y0str = "";
	wft_has_param2(argc, argv, "-y0", y0str, false, "");
	string y1str = "";
	wft_has_param2(argc, argv, "-y1", y1str, false, "");

	double valid0 = atof(valid0str.c_str());
	double valid1 = atof(valid1str.c_str());
	double scale = atof(scalestr.c_str());
	double offset = atof(offsetstr.c_str());
	int xspace = (int)atof(xspacestr.c_str());
	int yspace = (int)atof(yspacestr.c_str());

	double rx0, ry0, rx1, ry1;

	bool region = false;
	if (x0str != "" && x1str != "" && y0str != "" && y1str != "")
	{
		region = true;
		rx0 = atof(x0str.c_str());
		rx1 = atof(x1str.c_str());
		ry0 = atof(y0str.c_str());
		ry1 = atof(y1str.c_str());
	}

	GDALAllRegister();


	GDALDataset* inds = (GDALDataset*)GDALOpen(infile.c_str(), GDALAccess::GA_ReadOnly);
	const int rasterXSize = inds->GetRasterXSize();
	const int rasterYSize = inds->GetRasterYSize();

	std::cout << "raster x y size : " << rasterXSize << " " << rasterYSize << endl;//2017-11-26
	std::cout << "x y space : " << xspace << " " << yspace << endl;//2017-11-26
	int estxsize = rasterXSize / xspace;//2017-11-26
	int estysize = rasterYSize / yspace;//2017-11-26
	std::cout << "Estimate x y size : " << estxsize << " " << estysize << endl;//2017-11-26
	long  reccounts = estxsize * 1L * estysize ;//2017-11-26
	std::cout << "Estimate records count:" << reccounts << endl;//2017-11-26

	double trans[6];
	inds->GetGeoTransform(trans);

	double * buffer = new double[rasterXSize];

	ofstream ofs(outfile.c_str());
	ofs << "#x y z" << endl;

	int outRCount = 0;//2017-11-26
	int outYCount = 0;//2017-11-26
	for (int iy = 0; iy < rasterYSize; ++iy)
	{
		// what a bug 2017-11-26. if (iy %yspace == 0) continue;
		if (iy %yspace != 0) continue;
		inds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, rasterXSize, 1,
			buffer, rasterXSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
		++outYCount;//2017-11-26
		for (int ix = 0; ix < rasterXSize; ++ix)
		{
			if (ix%xspace == 0 )
			{
				double x = trans[0] + trans[1] * ix + trans[2] * iy;
				double y = trans[3] + trans[4] * ix + trans[5] * iy;
				if (region)
				{
					if (x < rx0 || x > rx1 || y < ry0 || y > ry1)
					{
						continue;
					}
				}
				++outRCount;//2017-11-26
				double val = buffer[ix];
				if (valid0 <= val && val <= valid1)
				{
					double val1 = val * scale + offset;
					ofs << fixed <<  x << "\t" << fixed<<  y << "\t" << val1 << endl;
				}
				else
				{
					ofs << fixed<< x << "\t" << fixed<< y << "\t" << valNan << endl;
				}
			}
			
		}
		ofs << endl;
		wft_term_progress(iy, rasterYSize);
	}

	delete[] buffer;
	GDALClose(inds);
	ofs.close();

	std::cout << "outYCount:" << outYCount << endl;//2017-11-26
	std::cout << "outRCount:" << outRCount << endl;//2017-11-26
	std::cout << "done." << endl;

    return 0;
}

