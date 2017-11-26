//avhrr_ndvi_xulie 计算时间序列
//wangfeng1@piesat.cn 2017-11-07


#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include "../../sharedcodes/wftools.h"
#include <vector>
#include <string>
using namespace std;



bool isValidNoaaNdvi(double vmin, double vmax, double inval, short qaval)
{
	short mask = 14;
	short maskout = qaval & mask;
	//std::string qabinary = std::bitset<16>(qaval).to_string();
	// if (inputValidMin <= inval && inval <= inputValidMax && qaval == 128 )//????QA????128±í??avhrr?????¨???????§?????????¤??????0??2017-10-9.
	if (vmin <= inval && inval <= vmax && maskout == 0) return true; //????QA?? 1,2,3 bit ??????1.
	else
	{
		return false;
	}
}




int main(int argc , char** argv )
{
	std::cout << "Compute avhrr ndvi time sequence value." << std::endl;
	std::cout << "Version 0.1a . by wangfengdev@163.com 2017-11-7." << std::endl;
	std::cout << "Version 0.2a . add season calculating 2017-11-8." << std::endl;
	std::cout << "Version 0.3a . add year calculating 2017-11-14." << std::endl;
	wft_print_current_time();
	std::cout << "processing..." << std::endl;
	if (argc == 1 )
	{
		
		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "avhrr_ndvi_xulie -indir /datas/ -year 2016 [-month 1/.../12]  [-season 12/3/6/9] -out output.txt -mask chinamask.tif" << std::endl;
		std::cout << "no enough parameters. out." << std::endl;
		return 101;
	}
	
	string inDir, inYear, inMonth, outfile, maskfile , inSeason ;
	wft_has_param(argc, argv, "-indir", inDir, true);
	wft_has_param(argc, argv, "-year", inYear, true);
	wft_has_param2(argc, argv, "-month", inMonth, false , "" );
	wft_has_param2(argc, argv, "-season", inSeason, false , "" );
	wft_has_param(argc, argv, "-out", outfile, true);
	wft_has_param(argc, argv, "-mask", maskfile, true);

	bool calYear = false;
	if (inMonth == "" && inSeason == "" )
	{
		calYear = true;
	}

	
	vector<string> allfiles;
	wft_get_allfiles(inDir, allfiles);

	vector<int> allYmds;
	for (int i = 0; i < allfiles.size(); ++i)
	{
		//AVHRR-Land_v004_AVH13C1_NOAA-07_19810624_c20130815112445
		//AVHRR-Land_v004-preliminary_AVH13C1_NOAA-19_20160101_c20160104131939
		string filename = wft_base_name(allfiles[i]);
		int pos = filename.find("NOAA-");
		if (pos != string::npos)
		{
			int newpos = pos + 8;
			string ymd1 = filename.substr(newpos, 8);
			int ymd2 = (int)atof(ymd1.c_str());
			allYmds.push_back(ymd2);
		}
		else {
			allYmds.push_back(0);
		}
	}


	int year = (int)atof(inYear.c_str()) ;
	int month = 0; 
	int season = 0;
	if (inMonth!="") {
		month = (int)atof(inMonth.c_str());
	}
	else {
		season = (int)atof(inSeason.c_str());
	}

	vector<string> selectedFiles;
	if (month > 0)
	{
		for (int iday = 1; iday < 32; ++iday)
		{
			int ymd0 = year * 10000 + month * 100 + iday;
			for (int ifile = 0; ifile < allYmds.size(); ++ifile)
			{
				if (allYmds[ifile] == ymd0)
				{
					selectedFiles.push_back(allfiles[ifile]);
					break;
				}
			}
		}
	}
	else if (season > 0) {
		int mon0 = 1;
		int mon1 = 2;
		int year1 = year;
		if (season == 12) {
			mon0 = 1;
			mon1 = 2;
			for (int iday = 1; iday < 32; ++iday)
			{
				int ymd0 = year * 10000 + 1200 + iday;
				for (int ifile = 0; ifile < allYmds.size(); ++ifile)
				{
					if (allYmds[ifile] == ymd0)
					{
						selectedFiles.push_back(allfiles[ifile]);
						break;
					}
				}
			}
			year1 = year + 1;
		}
		else if( season ==3 )
		{
			mon0 = 3;
			mon1 = 5;
		}
		else if (season == 6)
		{
			mon0 = 6;
			mon1 = 8;
		}
		else if (season == 9)
		{
			mon0 = 9;
			mon1 = 11;
		}
		else
		{
			std::cout << "unsupported season value :"<<season << ". only supported: 12/3/6/9. out." << std::endl;
			return 103;
		}
		for (int imon = mon0 ; imon <= mon1 ; ++imon)
		{
			for (int iday = 1; iday < 32; ++iday)
			{
				int ymd0 = year1 * 10000 + imon * 100 + iday;
				for (int ifile = 0; ifile < allYmds.size(); ++ifile)
				{
					if (allYmds[ifile] == ymd0)
					{
						selectedFiles.push_back(allfiles[ifile]);
						break;
					}
				}
			}
		}
	}
	else
	{
		int mon0 = 1;
		int mon1 = 12;
		for (int imon = mon0; imon <= mon1; ++imon)
		{
			for (int iday = 1; iday < 32; ++iday)
			{
				int ymd0 = year * 10000 + imon * 100  + iday;
				for (int ifile = 0; ifile < allYmds.size(); ++ifile)
				{
					if (allYmds[ifile] == ymd0)
					{
						selectedFiles.push_back(allfiles[ifile]);
						break;
					}
				}
			}
		}
	}


	cout << "target year : " << year << endl;
	cout << "target month:" << month << endl;
	cout << "target season:" << season << endl;
	cout << " find number of days : " << selectedFiles.size() << endl;;

	if (selectedFiles.size() == 0)
	{
		cout << "no files are available.out." << endl;
		exit(104);
	}

	GDALAllRegister();

	//get output parameters.

	GDALDataset* maskds = (GDALDataset*)GDALOpen(maskfile.c_str(), GDALAccess::GA_ReadOnly);
	const int rasterXSize = maskds->GetRasterXSize();
	const int rasterYSize = maskds->GetRasterYSize();
	int num = rasterXSize * rasterYSize;
	short* max2d = new short[rasterXSize * rasterYSize];
	bool* cnt2d = new bool[rasterXSize * rasterYSize];
	for (int i = 0; i < num ; ++i)
	{
		cnt2d[i] = false ;
	}

	short* mask2d = new short[rasterXSize * rasterYSize];
	short* bufferNd2d = new short[rasterXSize * rasterYSize];
	short* bufferQa2d = new short[rasterXSize * rasterYSize];

	maskds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, 0, rasterXSize, rasterYSize,
		mask2d, rasterXSize, rasterYSize, GDALDataType::GDT_Int16, 0, 0, nullptr);
	GDALClose(maskds);


	for (int i = 0; i < selectedFiles.size(); ++i)
	{
		cout << "comparing " << i << "/" << selectedFiles.size() << " ... " << endl;
		string dspath = "HDF5:\"" + selectedFiles[i] + "\"://NDVI";
		string qapath = "HDF5:\"" + selectedFiles[i] + "\"://QA";
		//string dspath =   selectedFiles[i] ;
		//string qapath = "D:/test1/demoqa.tif";
		GDALDataset* ndvids = (GDALDataset*)GDALOpen(dspath.c_str(), GDALAccess::GA_ReadOnly);
		GDALDataset* qads = (GDALDataset*)GDALOpen(qapath.c_str(), GDALAccess::GA_ReadOnly);

		ndvids->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, 0, rasterXSize, rasterYSize,
			bufferNd2d, rasterXSize, rasterYSize, GDALDataType::GDT_Int16, 0, 0, nullptr);
		qads->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, 0, rasterXSize, rasterYSize,
			bufferQa2d, rasterXSize, rasterYSize, GDALDataType::GDT_Int16, 0, 0, nullptr);

		GDALClose(ndvids);
		GDALClose(qads);

		for (int ip = 0; ip < num; ++ ip )
		{
			if (mask2d[ip] > 0)
			{
				if (isValidNoaaNdvi(-2000, 10000, bufferNd2d[ip], bufferQa2d[ip]))
				{
					if (cnt2d[ip] == false )
					{
						max2d[ip] = bufferNd2d[ip];
						cnt2d[ip] = true;
					}
					else if( bufferNd2d[ip] > max2d[ip] ){
						max2d[ip] = bufferNd2d[ip];
					}
				}
			}

		}

	}


	double allSum = 0;
	int allCount = 0;
	double theAver =0;
	for (int i = 0; i < num; ++i)
	{
		if (cnt2d[i] == true)
		{
			double val = max2d[i] * 0.0001;
			allSum += val;
			allCount++;
		}
	}

	delete[] max2d;
	delete[] cnt2d;
	delete[] bufferNd2d;
	delete[] bufferQa2d;
	delete[] mask2d;


	if (allCount > 0)
	{
		theAver = allSum / allCount;
		ofstream ofs(outfile.c_str());
		ofs << theAver << endl;
		ofs.close();

		FILE* file = fopen("alldata.txt", "a");
		if (season > 0)
		{
			fprintf(file, "%s-%s season \n%f\n", inYear.c_str(), inSeason.c_str(), theAver);
		}
		else if( month>0 ) {
			fprintf(file, "%s-%s month \n%f\n", inYear.c_str(), inMonth.c_str(), theAver);
		}
		else {
			fprintf(file, "%s year \n%f\n", inYear.c_str() , theAver);
		}
		
		fclose(file);

	}
	else
	{
		cout << "allCount is zero. out." << endl;
		exit(105);
	}



	/*
	

	double* dataBuffer = new double[rasterXSize];
	double* dataMaxBuffer = new double[rasterXSize];
	int*    countBuffer = new int[rasterXSize];
	short*  qaBuffer = new short[rasterXSize];
	short* maskeBuffer = new short[rasterXSize];

	GDALDataset** ndvidsArray = new GDALDataset*[selectedFiles.size()];
	GDALDataset** qadsArray = new GDALDataset*[selectedFiles.size()];
	for (int i = 0; i < selectedFiles.size(); ++i)
	{
		string dspath = "HDF5:\"" + selectedFiles[i] + "\"://NDVI";
		string qapath = "HDF5:\"" + selectedFiles[i] + "\"://QA";
		GDALDataset* ndvids = (GDALDataset*)GDALOpen(dspath.c_str(), GDALAccess::GA_ReadOnly);
		GDALDataset* qads = (GDALDataset*)GDALOpen(qapath.c_str(), GDALAccess::GA_ReadOnly);
		ndvidsArray[i] = ndvids;
		qadsArray[i] = qads;
	}

	for (int iy = 0; iy < rasterYSize; ++iy )
	{
		maskds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, rasterXSize, 1,
			maskeBuffer, rasterXSize, 1, GDALDataType::GDT_Int16, 0, 0, nullptr);
		int maskstartx = -1;
		for (int it = 0; it < rasterXSize; ++it)
		{
			dataMaxBuffer[it] = 0 ;
			countBuffer[it] = 0;
			if (maskeBuffer[it] > 0 && maskstartx == -1) maskstartx = it;
		}

		if (maskstartx == -1) continue;

		for (int ids = 0; ids < selectedFiles.size() ; ++ids)
		{
			//string dspath = "HDF5:\"" + selectedFiles[ids] + "\"://NDVI";
			//string qapath = "HDF5:\"" + selectedFiles[ids] + "\"://QA";
			GDALDataset* ndvids = ndvidsArray[ids];
			GDALDataset* qads = qadsArray[ids];

			ndvids->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy , rasterXSize, 1 ,
				dataBuffer, rasterXSize, 1 , GDALDataType::GDT_Float64, 0, 0, nullptr);
			qads->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, rasterXSize, 1,
				qaBuffer, rasterXSize, 1, GDALDataType::GDT_Int16, 0, 0, nullptr);
			
			for (int ix = maskstartx ; ix < rasterXSize; ++ix)
			{
				if (maskeBuffer[ix] > 0)
				{//在中国范围内
					double inval = dataBuffer[ix];
					short qaval = qaBuffer[ix];
					if (isValidNoaaNdvi( -2000 , 10000, inval, qaval))//
					{
						//最大值合成
						if (countBuffer[ix] == 0) {
							dataMaxBuffer[ix] = inval;
							countBuffer[ix] = 1;
						}
						else if( inval > dataMaxBuffer[ix] ) {
							dataMaxBuffer[ix] = inval;
						}
					}
				}
			}

		}


		//汇总求和
		for (int ix = 0; ix < rasterXSize; ++ix)
		{
			if (countBuffer[ix] > 0 && maskeBuffer[ix]>0 )
			{
				allSum += dataMaxBuffer[ix] * 0.0001 ;//ndvi-dn -> ndvi-float
				++allCount;
			}
		}
		cout << iy << "/" << rasterYSize << "  ";
		//wft_term_progress(iy, rasterYSize);
	}
	cout << "" << endl;

	for (int i = 0; i < selectedFiles.size(); ++i)
	{
		GDALClose(ndvidsArray[i])  ;
		GDALClose(qadsArray[i])  ;

	}
	delete [] dataBuffer; dataBuffer = nullptr;
	delete[] dataMaxBuffer; dataMaxBuffer = nullptr;
	delete [] countBuffer; countBuffer = nullptr;
	delete[] qaBuffer; qaBuffer = nullptr;
	delete[] maskeBuffer; countBuffer = nullptr;

	*/

	
	
	std::cout << "All done." << std::endl;
	wft_print_current_time();
    return 0;
}

