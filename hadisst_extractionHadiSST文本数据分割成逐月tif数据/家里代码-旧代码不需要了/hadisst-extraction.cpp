// hadisst-extraction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gdal_priv.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

void writeDatFile(std::string datfilepath , int* buff)
{
	std::ofstream datofs(datfilepath);
	datofs << "# lon lat hadisst_value " << std::endl;
	for (int iy = 0; iy < 180; ++iy)
	{
		bool hasval = false;
		for (int ix = 0;ix < 360; ++ix)
		{
			double lon = ix - 180 + 0.5;
			double lat = 90 - iy - 0.5;
			datofs << lon << " " << lat << " " << buff[iy * 360 + ix] << std::endl;
		}
		datofs << std::endl;
	}
	datofs.close();
}


int main()
{
	GDALAllRegister();

	std::string inputfile = "E:/developing/hadisst-extraction/HadISST1_SST_2017.txt";

	std::ifstream ifs(inputfile);

	if (ifs.is_open())
	{
		std::string line;
		std::string lastTifFilepath = "";
		int irows = 0;
		int buff[360 * 180];
		GDALDataset* dsPtr = nullptr;
		GDALDriver* tifDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
		while (std::getline(ifs, line)) {
			if (line.length() < 2) continue;
			if (line.length() < 100)
			{
				
				std::istringstream iss(line);
				int day, mon, year;
				iss >> day >> mon >> year;
				std::stringstream ssout;
				ssout << inputfile << "." << year << "-" << mon << "-" << day << ".tif";
				if (dsPtr != nullptr)
				{
					assert(irows == 180);
					dsPtr->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, 0, 360, 180, buff, 360, 180, GDALDataType::GDT_Int32, 0, 0, nullptr);
					GDALClose(dsPtr);
					dsPtr = nullptr;

					std::string datfilepath = lastTifFilepath + ".dat";
					writeDatFile(datfilepath, buff);
				}
				std::string outfilepath = ssout.str();
				lastTifFilepath = outfilepath;
				irows = 0;
				dsPtr = tifDriver->Create(outfilepath.c_str(), 360, 180, 1, GDALDataType::GDT_Int16, nullptr);
				std::cout << "creating file :" << outfilepath << std::endl;
			}
			else {
				std::string line1 = ReplaceAll(line, std::string("-32768"), std::string(" -5000 "));
				std::istringstream iss(line1);
				int x = 0;
				int val = 0;
				while (iss >> val) {
					buff[irows * 360 + x] = val;
					++x;
				}
				assert(x == 360);
				++irows;
			}
		}
		if (dsPtr != nullptr)
		{
			assert(irows == 180);
			dsPtr->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, 0, 360, 180, buff, 360, 180, GDALDataType::GDT_Int32, 0, 0, nullptr);
			GDALClose(dsPtr);
			dsPtr = nullptr;

			std::string datfilepath = lastTifFilepath + ".dat";
			writeDatFile(datfilepath, buff);
		}


		ifs.close();
	}
	


    return 0;
}

