// nanhai_fy4olr_aver.cpp : 定义控制台应用程序的入口点。
//计算南海区域fy4-olr平均值，并输出到文件 
#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include "../../sharedcodes/wftools.h"
using namespace std;



bool isvalidFy4OlrDailyFile(string& filename)
{
	// FY4A-_AGRI--_N_DISK_1047E_L2-_LPW-_MULT_NOM_20171006_TPW_combination_day.tif
	size_t pos0 = filename.find("FY4A-_AGRI--_N_DISK_1047E_L2-_OLR-_MULT_NOM_");
	size_t pos1 = filename.find("_OLR_combination_day.tif");
	size_t pos2 = filename.find("_day.tif");
	if (pos0 != string::npos && pos1 != string::npos  && pos2 == filename.length() - 8)//bugfixed 2017-11-10
	{
		return true;
	}
	else
	{
		return false;
	}
}


void readDongyaRegion(string& filepath, vector<int>& ivec)
{
	ifstream ifs(filepath.c_str());
	std::string line;
	while (std::getline(ifs, line))
	{
		if (line.length() > 0)
		{
			std::istringstream iss(line);
			int a;
			if (!(iss >> a))
			{
				continue;
			}
			else {
				ivec.push_back(a);
			}
		}
	}
	ifs.close();
}

string makeOutfilename(string& filename , string& outprefix)
{
	string ymd = filename.substr(44, 8);
	string outname = outprefix + ymd + ".txt";
	return outname;
}

int processOnefile(string& filepath, vector<int>& ivec, string& outpath   )
{
	GDALDataset* olrds = (GDALDataset*)GDALOpen(filepath.c_str(), GDALAccess::GA_ReadOnly);
	int xsize = olrds->GetRasterXSize();
	int ysize = olrds->GetRasterYSize();
	float* buffer = new float[xsize * ysize];
	int size2d = xsize * ysize;
	olrds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, 0, xsize, ysize,
		buffer, xsize, ysize, GDALDataType::GDT_Float32, 0, 0, 0);
	double sum = 0;
	int cnt = 0;
	for (int i = 0; i < ivec.size(); ++i)
	{
		int px = ivec[i];
		if (px < size2d)
		{
			if (buffer[px] >= 40 && buffer[px] <= 450)
			{
				sum += buffer[px];
				++cnt;
			}
		}
		else {
			cout << "Warning some point's index("<<px<<") is out of image("<<xsize<<"x"<<ysize<< " = " << size2d<< " )."  << endl;
		}
	}
	delete[] buffer;
	GDALClose(olrds);

	double aver = 0;
	if (cnt > 0) aver = sum / cnt;
	ofstream ofs(outpath.c_str());
	ofs << aver << endl;
	ofs.close();

	return 100;
}


int main(int argc , char** argv )
{
	cout << "A program to extract average value of Nanhai region from fy4-olr.2017-11-16. by wangfeng1@piesat.cn" << endl;
	cout << "Version 1.0a 2017-11-16." << endl;
	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "nanhai_fy4olr_aver startup.txt" << endl;
		cout << "************* sample startup.txt *************" << endl;
		cout << "#olrdir" << endl;
		cout << "E:/testdata/fy4olr15min/2017/" << endl;
		cout << "#pixellist" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy4-1047E-nanhai-region.txt" << endl;
		cout << "#outdir" << endl;
		cout << "E:/testdata/fy4olr15min/nanhai-olr/" << endl;
		cout << "#outprefix" << endl;
		cout << "nanhai.fy4olraver." << endl;
		cout << "" << endl;
		cout << "************* *****************  *************" << endl;
		exit(101);
	}
	string startupfile = argv[1];
	string olrdir =   wft_getValueFromExtraParamsFile(startupfile, "#olrdir", true);
	string dyregion =   wft_getValueFromExtraParamsFile(startupfile, "#pixellist", true);
	string outdir = wft_getValueFromExtraParamsFile(startupfile, "#outdir", true);
	string outprefix = wft_getValueFromExtraParamsFile(startupfile, "#outprefix", true);

	vector<int> regionindex;
	readDongyaRegion(dyregion, regionindex);
	cout << "nanhai region points count:" << regionindex.size() << endl;

	vector<string> allfiles;
	wft_get_allfiles(olrdir, allfiles);
	GDALAllRegister();
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		if (isvalidFy4OlrDailyFile(filename))
		{
			string outname = makeOutfilename(filename , outprefix);
			string outpath = outdir + outname;
			if (wft_test_file_exists(outpath))
			{
				continue;
			}
			else {
				int ret = processOnefile(filepath, regionindex, outpath);
				cout << "make " << outname << " result : " << ret << endl;
			}
		}
	}
    return 0;
}

