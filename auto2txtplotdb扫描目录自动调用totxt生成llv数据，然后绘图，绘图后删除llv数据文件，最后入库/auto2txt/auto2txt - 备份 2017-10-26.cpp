// auto2txt.cpp : 定义控制台应用程序的入口点。
//扫描特定文件夹，找到符合要求的文件，制作llv.tmp文本文件。
//wangfeng1@piesat.cn 2017-10-20


#define UNICODE
#include "../../sharedcodes/tinydir.h"
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cmath>
#include "../../sharedcodes/wftools.h"
using namespace std;



void getFilesUnderDir(string dir, vector<string>& filesVector)
{
	wft_get_allfiles(dir, filesVector);
}



int main(int argc , char** argv )
{
	cout << "A program to auto make lon lat value txt file." << endl;
	cout << "Version 0.1a .2017-10-20" << endl;
	cout << "Version 0.1.1a . outfile path bugfixed 2017-10-20" << endl;
	cout << "Version 0.2a . accept multi fix tail string separated by ;" << endl;
	
	
	if (argc == 1)
	{
		cout << "sample call: " << endl;
		cout << "auto2txt params.startup" << endl;

		cout << "sample startupfile:" << endl;

		cout << "#indir" << endl;
		cout << "E:/testdata/fy4sst15min/" << endl;
		cout << "#filenamefixprefix" << endl;
		cout << "FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_" << endl;
		cout << "#filenamefixtail" << endl;
		cout << ".NC;_day.tif;_mon.tif" << endl;
		cout << "#totxtprogram" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy4totxt.exe" << endl;

		cout << "#dsprefix[optional]" << endl;
		cout << "HDF5:\"" << endl;
		cout << "#dstail[optional]" << endl;
		cout << "\"://SST" << endl;


		cout << "#outdir[optional use indir default]" << endl;
		cout << "E:/testdata/fy4sst15min/" << endl;
		cout << "#outtail[optional use .llv.tmp default]" << endl;
		cout << ".llv.tmp" << endl;

		cout << "#type" << endl;
		cout << "llfiles/llrange" << endl;

		cout << "#lonfile" << endl;
		cout << "fy4lon.tif" << endl;
		cout << "#latfile" << endl;
		cout << "fy4lat.tif" << endl;

		cout << "#left" << endl;
		cout << "-180" << endl;
		cout << "#right" << endl;
		cout << "180" << endl;
		cout << "#top" << endl;
		cout << "90" << endl;
		cout << "#bottom" << endl;
		cout << "-90" << endl;

		cout << "#valid0[optional]" << endl;
		cout << "-5" << endl;
		cout << "#valid1[optional]" << endl;
		cout << "45" << endl;

		cout << "#scale[optional]" << endl;
		cout << "1.0" << endl;
		cout << "#offset[optional]" << endl;
		cout << "0.0" << endl;

		cout << "#xspace[optional]" << endl;
		cout << "1" << endl;
		cout << "#yspace[optional]" << endl;
		cout << "1" << endl;

		cout << "#x0[optional]" << endl;
		cout << "0" << endl;
		cout << "#y0[optional]" << endl;
		cout << "0" << endl;
		cout << "#x1[optional]" << endl;
		cout << "100" << endl;
		cout << "#y1[optional]" << endl;
		cout << "50" << endl;

		cout << "" << endl;

		exit(101); 
	}

	string startupFile(argv[1]);

	string inDir = wft_getValueFromExtraParamsFile(startupFile, "#indir", true);
	string fixPrefix = wft_getValueFromExtraParamsFile(startupFile, "#filenamefixprefix", true);
	string fixTailLine = wft_getValueFromExtraParamsFile(startupFile, "#filenamefixtail", true);
	string toTxtProgram = wft_getValueFromExtraParamsFile(startupFile, "#totxtprogram", true);

	string dsPrefix = wft_getValueFromExtraParamsFile(startupFile, "#dsprefix", false);
	string dsTail = wft_getValueFromExtraParamsFile(startupFile, "#dstail", false);

	string outDir = wft_getValueFromExtraParamsFile(startupFile, "#outdir", false);
	if (outDir == "") outDir = inDir;
	if (*outDir.rbegin() != '/') outDir = outDir + "/";
	string outTailName = wft_getValueFromExtraParamsFile(startupFile, "#outtail", false);
	if (outTailName == "") outTailName = ".llv.tmp";

	string llType = wft_getValueFromExtraParamsFile(startupFile, "#type", true);
	string lonFile, latFile , left , right , top , bottom ;
	if (llType == "llfiles")
	{
		lonFile = wft_getValueFromExtraParamsFile(startupFile, "#lonfile", true);
		latFile = wft_getValueFromExtraParamsFile(startupFile, "#latfile", true);
	}
	else if (llType == "llrange")
	{
		left = wft_getValueFromExtraParamsFile(startupFile, "#left", true);
		right = wft_getValueFromExtraParamsFile(startupFile, "#right", true);
		top = wft_getValueFromExtraParamsFile(startupFile, "#top", true);
		bottom = wft_getValueFromExtraParamsFile(startupFile, "#bottom", true);
	}
	else {
		cout << "Error : unknown type :"<<llType  << endl;
		exit(102);
	}

	string valid0 = wft_getValueFromExtraParamsFile(startupFile, "#valid0", false);
	string valid1 = wft_getValueFromExtraParamsFile(startupFile, "#valid1", false);
	string scale = wft_getValueFromExtraParamsFile(startupFile, "#scale", false);
	string offset = wft_getValueFromExtraParamsFile(startupFile, "#offset", false);
	string xspace = wft_getValueFromExtraParamsFile(startupFile, "#xspace", false);
	string yspace = wft_getValueFromExtraParamsFile(startupFile, "#yspace", false);
	string x0 = wft_getValueFromExtraParamsFile(startupFile, "#x0", false);
	string x1 = wft_getValueFromExtraParamsFile(startupFile, "#x1", false);
	string y0 = wft_getValueFromExtraParamsFile(startupFile, "#y0", false);
	string y1 = wft_getValueFromExtraParamsFile(startupFile, "#y1", false);

	
	vector<string> allFiles;
	getFilesUnderDir(inDir, allFiles);

	vector<string> fixTailVec = wft_string_split(fixTailLine, ";");

	for (size_t ifile = 0; ifile < allFiles.size(); ++ifile)
	{
		string filename = wft_base_name(allFiles[ifile]);
		if (filename.find(fixPrefix) != string::npos)
		{
			bool hasATail = wft_string_has_tails(filename, fixTailVec);

			if (hasATail)
			{
				string outfile = outDir + filename + outTailName;//bugfixed 2017-10-20.
				if (wft_test_file_exists(outfile) == false)
				{
					string dsPath = dsPrefix + allFiles[ifile] + dsTail;
					string cmd = toTxtProgram + " -in " + dsPath;
					cmd = cmd + " -out " + outfile;
					cmd = cmd + " -type " + llType;
					if (llType == "llfiles")
					{
						cmd = cmd + " -lon " + lonFile;
						cmd = cmd + " -lat " + latFile;
					}
					else {
						cmd = cmd + " -left " + left;
						cmd = cmd + " -right " + right;
						cmd = cmd + " -top " + top;
						cmd = cmd + " -bottom " + bottom;
					}
					if (valid0 != "") cmd = cmd + " -valid0 " + valid0;
					if (valid1 != "") cmd = cmd + " -valid1 " + valid1;

					if (scale != "") cmd = cmd + " -scale " + scale;
					if (offset != "") cmd = cmd + " -offset " + offset;

					if (xspace != "") cmd = cmd + " -xspace " + xspace;
					if (yspace != "") cmd = cmd + " -yspace " + yspace;

					if (x0 != "") cmd = cmd + " -x0 " + x0;
					if (x1 != "") cmd = cmd + " -x1 " + x1;
					if (y0 != "") cmd = cmd + " -y0 " + y0;
					if (y1 != "") cmd = cmd + " -y1 " + y1;
					cout << "*** call ***" << endl;
					cout << cmd << endl;
					cout << "*** *** ***" << endl;
					int ret = system(cmd.c_str());
					cout << toTxtProgram << " return code :" << ret << endl;
					break;
				}
			}
		}
	}

	cout << "done." << endl;

    return 0;
}

