// nanhai_uwind_aver.cpp : 定义控制台应用程序的入口点。
//提取南海范围的uwind平均值，范围10N-20N，110E-120E

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../../sharedcodes//wftools.h"
using namespace std;


bool isValidUwindFile(string& filename)
{
	int pos0 = filename.find("ncepwind.");
	int pos1 = filename.find(".xyuv.txt");
	if (pos0 != string::npos && pos1 != string::npos && pos1 == filename.length() - 9)
	{
		return true;
	}
	else {
		return false;
	}
}

int processOneFile(string& filepath , string& outpath )
{
	ifstream ifs(filepath.c_str());

	//逐行分析
	std::string line;
	std::getline(ifs, line);//description line.
	double sum = 0;
	int cnt = 0;
	while (std::getline(ifs, line))
	{
		if (line.length() > 7)
		{
			std::istringstream iss(line);
			double lon, lat, u, v;
			if (!(iss >> lon >> lat >> u >> v))
			{
				continue;
			}
			else {
				if (lon >= 110 && lon <= 120 && lat >= 10 && lat <= 20)
				{
					sum += u;
					++cnt;
				}
			}
		}
	}
	ifs.close();

	double aver = 0;
	if( cnt > 0 ) aver = sum / cnt;
	ofstream ofs(outpath.c_str());
	ofs << aver;
	ofs.close();

	return 100;
}

string makeOutfilename(string& filename ,string& outprefix)
{
	//ncepwind.20170920.850.xyuv  string("nanhai.uwind850aver.") +
	string ymd = filename.substr(9, 8);
	string outname = outprefix + ymd + ".txt";
	return outname;
}

int main(int argc , char** argv )
{
	cout << "A program to extract average value of Nanhai region from ncep-uwind text file.2017-11-16. by wangfeng1@piesat.cn" << endl;
	cout << "Version 1.0a 2017-11-16." << endl;
	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "nanhai_uwind_aver startup.txt" << endl;
		cout << "************* sample startup.txt *************" << endl;
		cout << "#indir" << endl;
		cout << "E:/wind/" << endl;
		cout << "#outdir" << endl;
		cout << "E:/outdir/" << endl;
		cout << "#outprefix" << endl;
		cout << "nanhai.uwind850aver." << endl;
		cout << "" << endl;
		cout << "************* *****************  *************" << endl;
		exit(101);
	}

	string startupfile = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startupfile, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startupfile, "#outdir", true);
	string outprefix = wft_getValueFromExtraParamsFile(startupfile, "#outprefix", true);


	vector<string> allfiles;
	wft_get_allfiles(indir, allfiles);
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		if (isValidUwindFile(filename))
		{
			string outname = makeOutfilename(filename , outprefix );
			string outpath = outdir + outname;
			if (wft_test_file_exists(outpath)==false)
			{
				int ret = processOneFile(filepath, outpath);
				cout << "make " << outname << " : " << ret << endl;
			}
		} 
	}
	cout << "done." << endl;
    return 0;
}

