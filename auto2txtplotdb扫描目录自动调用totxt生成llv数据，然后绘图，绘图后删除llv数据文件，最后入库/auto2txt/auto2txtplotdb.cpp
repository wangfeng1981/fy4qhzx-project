// auto2txt.cpp : 定义控制台应用程序的入口点。
//扫描特定文件夹，找到符合要求的文件，制作llv.tmp文本文件。
//生产llv候，开始绘图，绘图完成后删除llv文件
//最后入库
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

//2017-10-26

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

void getCutLocLenPairs(string cutstr, vector<CutLocLen>& loclenVec)
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

void getCutReplaceTriples(string cpStr, vector <CutRepData>& cpVec)
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

vector<string> getAllCutString(string filename, vector<CutLocLen>& cvec, vector<CutRepData>& rvec)
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



string replaceStringByCut(string str, vector<string>& cpVec)
{
	for (int i = 0; i < cpVec.size(); ++i)
	{
		char buff[20];
		sprintf(buff, "{{{CUT%d}}}", i);
		string cutv(buff);
		str = wft_replaceString(str, cutv, cpVec[i]);
	}
	return str;
}


int makePlot(string llvfilepath , string pngfilepath ,string plotTem 
	, string plotProgram 
	, string title1tem, string title2tem, vector<CutLocLen>& cvec, vector<CutRepData>& rvec)
{
	if (wft_test_file_exists(llvfilepath) == true )
	{
		string filename = wft_base_name(llvfilepath);
		vector<string> allcutStrVec = getAllCutString(filename, cvec, rvec);

		string title1 = replaceStringByCut(title1tem, allcutStrVec);
		string title2 = replaceStringByCut(title2tem, allcutStrVec);

		vector<string> vec1;
		vec1.push_back("{{{TITLE1}}}");
		vec1.push_back("{{{TITLE2}}}");
		vec1.push_back("{{{INFILE}}}");
		vec1.push_back("{{{OUTFILE}}}");

		vector<string> vec2;
		vec2.push_back(title1);
		vec2.push_back(title2);
		vec2.push_back(llvfilepath);
		vec2.push_back(pngfilepath);

		string tempPlotFile = pngfilepath + ".plot";
		wft_create_file_by_template_with_replacement(tempPlotFile, plotTem, vec1, vec2);

		string cmd = plotProgram + " " + tempPlotFile;
		cout << "*** call ***" << endl;
		cout << cmd << endl;
		cout << "*** *** ***" << endl;
		int ret = system(cmd.c_str());
		cout << plotProgram << " return code :" << ret << endl;

		if (wft_test_file_exists(pngfilepath))
		{
			wft_remove_file(llvfilepath);
			if (wft_test_file_exists(llvfilepath))
			{
				cout << "Warning failed to delete : " << llvfilepath << endl;
			}
			else
			{
				cout << "Successfully delete : " << llvfilepath << endl;
			}
			return 0;
		}
		else
		{
			cout << "Error failed to make png : " << pngfilepath << endl;
			return 104;
		}
	}
	else {
		cout << "Error can not find file : " << llvfilepath << endl;
		return 103;
	}
}


int main(int argc , char** argv )
{
	cout << "A program to auto make lon lat value txt file and do plot make png." << endl;
	cout << "Version 0.1a .2017-10-20" << endl;
	cout << "Version 0.1.1a . outfile path bugfixed 2017-10-20" << endl;
	cout << "Version 0.2a . accept multi fix tail string separated by ;" << endl;
	cout << "Version 0.3a . make llv if no png found, after plot delete llv." << endl;
	
	if (argc == 1)
	{
		cout << "sample call: " << endl;
		cout << "auto2txtplot params.startup" << endl;

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

		//////////////////////////////// plot plot plot
		cout << "#plottemplate" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy4sst20171025.plot" << endl;
		cout << "#plotprogram" << endl;
		cout << "gnuplot" << endl;
		cout << "#filenamecut" << endl;
		cout << "44,8;69,3" << endl;
		cout << "#cutreplace" << endl;
		cout << "1,day,Daily;1,ten,Ten-days" << endl;
		cout << "#title1template" << endl;
		cout << "FY4A SST {{{CUT0}}} {{{CUT1}}}" << endl;
		cout << "#title2template" << endl;
		cout << "4km QC:all" << endl;

		//////////////////////////////// database database
		
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


	/////////////plot params
	string plotTemplate = wft_getValueFromExtraParamsFile(startupFile, "#plottemplate", true);
	string plotProgram = wft_getValueFromExtraParamsFile(startupFile, "#plotprogram", true);
	string filenameCutStr = wft_getValueFromExtraParamsFile(startupFile, "#filenamecut", true);
	string cutReplaceStr = wft_getValueFromExtraParamsFile(startupFile, "#cutreplace", true);
	string title1template = wft_getValueFromExtraParamsFile(startupFile, "#title1template", true);
	string title2template = wft_getValueFromExtraParamsFile(startupFile, "#title2template", true);
	vector<CutLocLen> cutLocLenVector;
	getCutLocLenPairs(filenameCutStr, cutLocLenVector);
	vector<CutRepData> cutRepVector;
	getCutReplaceTriples(cutReplaceStr, cutRepVector);

	
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
				string outllvfile = outDir + filename + outTailName;//bugfixed 2017-10-20.
				string outpngfile = outDir + filename + ".png";
				if (wft_test_file_exists(outpngfile) == false)
				{
					string dsPath = dsPrefix + allFiles[ifile] + dsTail;
					string cmd = toTxtProgram + " -in " + dsPath;
					cmd = cmd + " -out " + outllvfile;
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

					if (ret == 0)
					{
						int retplot = makePlot(outllvfile, outpngfile, plotTemplate, plotProgram, title1template,
							title2template, cutLocLenVector, cutRepVector);
						cout << "makePlot ret code:" << retplot << endl;
						if (retplot == 0)
						{
							break;
						}
						else {
							cout << "Warning retplot code is not Zero, do next." << endl;
						}
					}
					else {
						cout << "Warning ret code is not Zero, do next." << endl;
					}
				}
			}
		}
	}

	cout << "done." << endl;

    return 0;
}

