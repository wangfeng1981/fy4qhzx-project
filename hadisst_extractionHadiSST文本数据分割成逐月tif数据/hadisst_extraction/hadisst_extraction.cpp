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

void writeDatFile(std::string datfilepath, int* buff, std::ofstream& plotofs, std::string coastlinefile )
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
			if (buff[iy * 360 + ix] < -500)
			{
				datofs << lon << " " << lat << " " << "NaN" << std::endl;
			}else{
				datofs << lon << " " << lat << " " << buff[iy * 360 + ix] * 0.01 << std::endl;
			}
		}
		datofs << std::endl;
	}
	datofs.close();

	std::string temppngfile = datfilepath + ".png" ;
	plotofs << "set terminal png size 800, 350\n" << "set output '" << temppngfile << "'\n"
		<< "set border lw 1.5" << std::endl
		<< "set style line 1 lc rgb 'black' lt 1 lw 2" << std::endl
		<< "set rmargin screen 0.85" << std::endl
		<< "unset key" << std::endl
		<< "set tics scale 5" << std::endl
		<< "set pal maxcolors 10" << std::endl
		<< "set cbrange [-10:40]" << std::endl
		<< "set xrange[-179:179]" << std::endl
		<< "set yrange[-89:89]" << std::endl
		<< "set palette defined ( 0 '0x222255',10 '0x222255' , 10 '0x4575b4',20 '0x4575b4' , 20 '0x74add1',30 '0x74add1' , 30 '0xabd9e9',40 '0xabd9e9' , 40 '0xe0f3f8',50 '0xe0f3f8' , 50 '0xffffbf',60 '0xffffbf' , 60 '0xfee090',70 '0xfee090' , 70 '0xfdae61',80 '0xfdae61' , 80 '0xf46d43',90 '0xf46d43' , 90 '0xd73027',100 '0xd73027' )" << std::endl
		<< "plot '" << datfilepath
		<< "' u 1:2:3 w image, '" << coastlinefile
		<< "' with lines linestyle 1" << std::endl;
}


int main(int argc  , char** argv )
{
	GDALAllRegister();

	std::cout << "This program is used to split hadisst txt file into monthly tif file and plot the thumb image." << std::endl;
	std::cout << " Version 0.1a . by wangfengdev@163.com 2017-9-20." << std::endl;
	std::cout << "Sample call: hadisst_extraction hadisstfile.txt coastlinefile" << std::endl;

	std::string inputfile = "";// "E:/testdata/hadisst/HadISST1_SST_2017.txt/HadISST1_SST_2017.txt";
	std::string coastlinefile = "";// "E:/coding/fy4qhzx-project/extras/world_110m.txt";
	if (argc == 3)
	{
		inputfile = argv[1];
		coastlinefile = argv[2];
	}
	else {
		std::cout << "Error: Bad parameters. out." << std::endl;
		exit(101);
	}

	
	std::string plotfile = inputfile + ".plot";
	std::ifstream ifs(inputfile);
	std::ofstream plotfs(plotfile);

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
					writeDatFile(datfilepath, buff , plotfs , coastlinefile);
				}
				std::string outfilepath = ssout.str();
				lastTifFilepath = outfilepath;
				irows = 0;
				dsPtr = tifDriver->Create(outfilepath.c_str(), 360, 180, 1, GDALDataType::GDT_Int16, nullptr);
				std::cout << "creating file :" << outfilepath << std::endl;
			}
			else {
				std::string line1 = ReplaceAll(line, std::string("-32768"), std::string(" -32768 "));
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
			writeDatFile(datfilepath, buff, plotfs, coastlinefile);
		}
		ifs.close();
	}

	plotfs.close();

	std::string cmdline = std::string("gnuplot ") + plotfile ;
	std::system(cmdline.c_str());

    return 0;
}

