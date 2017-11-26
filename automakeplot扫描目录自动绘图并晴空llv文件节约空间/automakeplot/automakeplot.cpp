// auto2txt.cpp : 
// 自动扫描文件夹找到llv文件生产绘图plot脚本，并调用gnuplot进行绘图
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

void makeEmptyFile(string filepath)
{
	FILE* pf = fopen(filepath.c_str(), "w");
	fclose(pf);
}

void getFilesUnderDir(string dir, vector<string>& filesVector)
{
	wft_get_allfiles(dir, filesVector);
}

int str2int(string str)
{
	return (int)atof(str.c_str());
}

struct CutLocLen
{
	int loc;
	int len;
};

struct CutRepData
{
	int cutid;
	string findStr;
	string replaceStr;
};

void getCutLocLenPairs(string cutstr, vector<CutLocLen>& loclenVec )
{
	vector<string> s1Vec = wft_string_split(cutstr, ";");
	for (int i1 = 0; i1 < s1Vec.size(); ++i1)
	{
		if (s1Vec[i1].length() > 0)
		{
			vector<string> s2Vec = wft_string_split(s1Vec[i1], ",");
			if (s2Vec.size() == 2)
			{
				CutLocLen ll;
				ll.loc = str2int(s2Vec[0]);
				ll.len = str2int(s2Vec[1]);
				loclenVec.push_back(ll);
			}
		}
	}
}

void getCutReplaceTriples(string cpStr, vector <CutRepData>& cpVec )
{
	vector<string> vec1 = wft_string_split(cpStr, ";");
	for (int i = 0; i < vec1.size(); ++i)
	{
		if (vec1[i].length() > 0)
		{
			vector<string> vec2 = wft_string_split(vec1[i], ",");
			if (vec2.size() == 3)
			{
				CutRepData crd;
				crd.cutid = str2int(vec2[0]);
				crd.findStr = vec2[1];
				crd.replaceStr = vec2[2];
				cpVec.push_back(crd);
			}
		}

	}
}

vector<string> getAllCutString(string filename, vector<CutLocLen>& cvec , vector<CutRepData>& rvec)
{
	vector<string> result;
	for (int i = 0; i < cvec.size(); ++i)
	{
		string s1 = filename.substr(cvec[i].loc, cvec[i].len);
		if (s1.length() > 0)
		{
			for (int j = 0; j < rvec.size(); ++j)
			{
				if (rvec[j].cutid == i)
				{
					if (s1 == rvec[j].findStr)
					{
						s1 = rvec[j].replaceStr;
						break;
					}
				}
			}
		}
		result.push_back(s1);
	}
	return result;
}



string replaceStringByCut(string str, vector<string>& cpVec )
{
	for (int i = 0; i < cpVec.size() ; ++i)
	{
		char buff[20];
		sprintf(buff, "{{{CUT%d}}}", i);
		string cutv(buff);
		str = wft_replaceString(str, cutv, cpVec[i]);
	}
	return str;
}



int main(int argc, char** argv)
{
	cout << "A program to auto make plot script and call gnuplot." << endl;
	cout << "Version 0.1a .2017-10-20" << endl;
	cout << "Version 0.2a .2017-10-25. clear llv.tmp file after plot." << endl;
	cout << "Version 0.2.1a .2017-10-26. empty the llv file." << endl;
	if (argc == 1)
	{
		cout << "sample call: " << endl;
		cout << "automakeplot params.startup" << endl;

		cout << "sample startupfile:" << endl;

		cout << "#indir" << endl;
		cout << "E:/testdata/fy4sst15min/2017/" << endl;
		cout << "#filenamefixprefix" << endl;
		cout << "FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_" << endl;
		cout << "#filenamefixtail" << endl;
		cout << "_day.tif.llv.tmp" << endl;
		cout << "#plottemplate" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/sst-sphere-20171018.plot" << endl;
		cout << "#plotprogram" << endl;
		cout << "gnuplot" << endl;

		cout << "#filenamecut" << endl;
		cout << "44,8;0,2;5,4[data pair]" << endl;

		cout << "#cutreplace" << endl;
		cout << "0,day,Daily;0,mon,Monthly;0,ten,Ten-days[data triple cutindex,find,replace]" << endl;

		cout << "#title1template" << endl;
		cout << "FY4A AGRI {{{CUT0}}}{{{CUT1}}}{{{CUT2}}}{{{CUT3}}}" << endl;
		cout << "#title2template" << endl;
		cout << "Resolution 4km {{{CUT4}}} QC:all" << endl;

		cout << "#outdir[optional]" << endl;
		cout << "E:/testdata/fy4sst15min/2017/" << endl;
		cout << "#outtail[optional]" << endl;
		cout << ".png" << endl;
		
		cout << "" << endl;

		exit(101);
	}

	string startupFile(argv[1]);

	string inDir = wft_getValueFromExtraParamsFile(startupFile, "#indir", true);
	string fixPrefix = wft_getValueFromExtraParamsFile(startupFile, "#filenamefixprefix", true);
	string fixTail = wft_getValueFromExtraParamsFile(startupFile, "#filenamefixtail", true);

	string plotTemplate = wft_getValueFromExtraParamsFile(startupFile, "#plottemplate", true);
	string plotProgram = wft_getValueFromExtraParamsFile(startupFile, "#plotprogram", true);

	string filenameCutStr = wft_getValueFromExtraParamsFile(startupFile, "#filenamecut", true);
	string cutReplaceStr = wft_getValueFromExtraParamsFile(startupFile, "#cutreplace", true);
	string title1template = wft_getValueFromExtraParamsFile(startupFile, "#title1template", true);
	string title2template = wft_getValueFromExtraParamsFile(startupFile, "#title2template", true);

	string outDir = wft_getValueFromExtraParamsFile(startupFile, "#outdir", false);
	if (outDir == "") outDir = inDir;
	if (*outDir.rbegin() != '/') outDir = outDir + "/";
	string outTailName = wft_getValueFromExtraParamsFile(startupFile, "#outtail", false);
	if (outTailName == "") outTailName = ".png";


	vector<CutLocLen> cutLocLenVector;
	getCutLocLenPairs(filenameCutStr, cutLocLenVector);

	vector<CutRepData> cutRepVector;
	getCutReplaceTriples(cutReplaceStr, cutRepVector);

	vector<string> allFiles;
	getFilesUnderDir(inDir, allFiles);

	int tailLen = fixTail.length();
	for (size_t ifile = 0; ifile < allFiles.size(); ++ifile)
	{
		string filename = wft_base_name(allFiles[ifile]);
		if (filename.find(fixPrefix) != string::npos)
		{
			bool hastail = wft_string_has_tail(filename, fixTail);
			if ( hastail )
			{
				string outfile = outDir + filename + outTailName;//
				if (wft_test_file_exists(outfile) == false)
				{
					vector<string> allcutStrVec = getAllCutString(filename, cutLocLenVector, cutRepVector);

					string title1 = replaceStringByCut( title1template , allcutStrVec) ;
					string title2 = replaceStringByCut(title2template, allcutStrVec);

					vector<string> vec1;
					vec1.push_back("{{{TITLE1}}}");
					vec1.push_back("{{{TITLE2}}}");
					vec1.push_back("{{{INFILE}}}");
					vec1.push_back("{{{OUTFILE}}}");

					vector<string> vec2;
					vec2.push_back(title1);
					vec2.push_back(title2);
					vec2.push_back(allFiles[ifile]);
					vec2.push_back(outfile);

					string tempPlotFile =   outDir + filename + ".plot" ;
					wft_create_file_by_template_with_replacement(tempPlotFile, plotTemplate, vec1, vec2);

					string cmd = plotProgram + " " + tempPlotFile;
					cout << "*** call ***" << endl;
					cout << cmd << endl;
					cout << "*** *** ***" << endl;
					int ret = system(cmd.c_str());
					cout << plotProgram << " return code :" << ret << endl;

					//2017-10-26
					if (ret == 0)
					{
						makeEmptyFile(allFiles[ifile]);
					}

					break;
				}

			}
		}
	}

	cout << "done." << endl;

	return 0;
}

