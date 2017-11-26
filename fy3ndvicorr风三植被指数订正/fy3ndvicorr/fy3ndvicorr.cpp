// fy3ndvicorr.cpp : 定义控制台应用程序的入口点。
//根据不同的拟合公式类型fit type position使用对应的线性拟合公式对fy3植被指数数据进行修正。2017-10-16


#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "../../sharedcodes/wftools.h"
using namespace std;

void readCorrEquations(string file, std::vector<double>& slopeVector, std::vector<double>& interVector)
{
	ifstream fs1(file.c_str());
	string line;
	while (std::getline(fs1, line))
	{
		if (line.length() > 1 && line[0] != '#')
		{
			stringstream ss(line);
			double v1, v2;
			ss >> v1 >> v2;
			slopeVector.push_back(v1);
			interVector.push_back(v2);
		}
	}
	fs1.close();
}



int main(int argc , char** argv )
{


	std::cout << "Description: A program to do correction for fy3b ndvi. " << std::endl;
	std::cout << "Version 0.1a . wangfeng1@piesat.cn 2017-10-16. " << std::endl;
	std::cout << "Version 0.2a . add valid range -valid0 -valid1. " << std::endl;
	if (argc == 1)
	{

		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "fy3ndvicorr -in fy3ndvi.tif -ceq correquations.txt -ftp fittypeposition.tif -out corrndvi.tif -valid0 -0.1 -valid1 0.1" << endl;
		std::cout << "a sample corr EQuation file:" << std::endl;
		std::cout << "#the equation was computed by fy3bndvi(May) vs AVHRRndvi(May) 2017-10-11\n"
			"#slope inter\n"
			"1	0\n"
			"0.7846	0.2745\n"
			"1.1056 -0.0207\n"
			"0.9206 -0.1258\n"
			<< std::endl;
		std::cout << "no enough parameters. out." << std::endl;
		return 101;
	}

	string inFile, ceqFile, ftpFile, outFile , v0str , v1str ;
	wft_has_param(argc, argv, "-in", inFile, true);
	wft_has_param(argc, argv, "-ceq", ceqFile, true);
	wft_has_param(argc, argv, "-ftp", ftpFile, true);
	wft_has_param(argc, argv, "-out", outFile, true);
	wft_has_param(argc, argv, "-valid0", v0str, true);
	wft_has_param(argc, argv, "-valid1", v1str, true);

	double valid0 = atof(v0str.c_str());
	double valid1 = atof(v1str.c_str());

	vector<double> slopevec, intervec;
	readCorrEquations(ceqFile, slopevec, intervec);
	const int fitcount = slopevec.size();
	GDALAllRegister();

	GDALDataset* inDs = (GDALDataset*)GDALOpen(inFile.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* ftpDs = (GDALDataset*)GDALOpen(ftpFile.c_str(), GDALAccess::GA_ReadOnly);
	const int xSize = inDs->GetRasterXSize();
	const int ySize = inDs->GetRasterYSize();
	{
		if (xSize != ftpDs->GetRasterXSize() || ySize != ftpDs->GetRasterYSize())
		{
			std::cout << "Error : sizes are not equal between infile and fit type postion. out." << std::endl;
			return 102;
		}
	}
	const GDALDataType theDataType = inDs->GetRasterBand(1)->GetRasterDataType();


	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outDs = driver->Create(outFile.c_str(), xSize, ySize, 1, theDataType, nullptr);
	double trans[6];
	inDs->GetGeoTransform(trans);
	outDs->SetGeoTransform(trans);
	outDs->SetProjection(inDs->GetProjectionRef());

	double* inbuffer = new double[xSize];
	int* ftpbuffer = new int[xSize];
	double* outbuffer = new double[xSize];

	for (int iy = 0; iy < ySize; ++iy)
	{
		inDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, xSize, 1,
			inbuffer, xSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
		ftpDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, xSize, 1,
			ftpbuffer, xSize, 1, GDALDataType::GDT_Int32, 0, 0, nullptr);
		for (int ix = 0; ix < xSize; ++ix)
		{
			if (inbuffer[ix] >= valid0 && inbuffer[ix] <= valid1)
			{
				int ifit = ftpbuffer[ix];
				double slope = 1.0;
				double inter = 0.0;
				if (ifit >= 0 && ifit < fitcount)
				{
					slope = slopevec[ifit];
					inter = intervec[ifit];
				}
				double newval = inbuffer[ix] * slope + inter;
				outbuffer[ix] = newval;
			}
			else
			{
				outbuffer[ix] = inbuffer[ix];
			}
			
		}
		outDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, xSize, 1,
			outbuffer, xSize, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
		wft_term_progress(iy, ySize);
	}

	delete[] inbuffer; inbuffer = 0;
	delete[] ftpbuffer; ftpbuffer = 0;
	delete[] outbuffer; outbuffer = 0;

	GDALClose(inDs);
	GDALClose(outDs);
	GDALClose(ftpDs);

	cout << "done." << endl;

    return 0;
}




