// fy4combine.cpp : 定义控制台应用程序的入口点。
//fy4遥感产品自动合成
//wangfeng1@piesat.cn 2017-10-16


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


int getYmdFromFy4Filepath(string filename )
{
	size_t nomPos = filename.rfind("NOM_");
	string ymd0str = filename.substr(nomPos + 4, 8);
	int ymd0 = (int)atof(ymd0str.c_str());
	return ymd0;
}

//2017-10-23
#define OUTTYPE_HALFHOUR 10
#define OUTTYPE_HOUR 11

#define OUTTYPE_DAY 1
#define OUTTYPE_FD 2
#define OUTTYPE_TD 3
#define OUTTYPE_MON 4
#define OUTTYPE_SEA 5
#define OUTTYPE_YEA 6


void getYmdRangeFromYmdAndType(int ymd, int type , int& refFromYmd , int& refToYmd )
{
	refFromYmd = ymd;
	refToYmd = ymd;
	int day = ymd % 100;
	int ym = (ymd / 100) * 100;
	if (type == OUTTYPE_DAY)
	{//daily
		refFromYmd = ymd;
		refToYmd = ymd;
	}
	else if (type == OUTTYPE_FD)
	{//five day
		if (day == 31) day = 30;
		int startday = ((day - 1) / 5) * 5 + 1;
		refFromYmd = ym + startday;
		if (startday == 26)
		{
			refToYmd = refFromYmd + 5;
		}
		else
		{
			refToYmd = refFromYmd + 4;
		}
	}
	else if (type == OUTTYPE_TD)
	{//ten day
		if (day == 31) day = 30;
		int startday = ((day - 1) / 10) * 10 + 1;
		refFromYmd = ym + startday;
		if (startday == 21)
		{
			refToYmd = refFromYmd + 10;
		}
		else
		{
			refToYmd = refFromYmd + 9;
		}
	}
	else if (type == OUTTYPE_MON)
	{//month
		int startday = 1;
		refFromYmd = ym + startday;
		refToYmd = refFromYmd + 31;
	}
	else if (type == OUTTYPE_SEA)
	{//season
		int year = ymd / 10000;
		int month = (ymd / 100) % 100;
		if (month <= 2)
		{
			refFromYmd = (year - 1) * 10000 + 1201 ;
			refToYmd = year * 10000 + 229; // last day of Febuary.
		}
		else if (month <= 5)
		{
			refFromYmd = year * 10000 + 301;
			refToYmd = year * 10000 + 531 ; 
		}
		else if (month <= 8)
		{
			refFromYmd = year * 10000 + 601;
			refToYmd = year * 10000 + 831;
		}
		else if (month <= 11)
		{
			refFromYmd = year * 10000 + 901;
			refToYmd = year * 10000 + 1130;
		}
		else
		{
			refFromYmd = year * 10000 + 1201 ;
			refToYmd = (year+1) * 10000 + 229 ;
		}
	}
	else if (type == OUTTYPE_YEA)
	{//year
		int year = (ymd / 10000) * 10000;
		refFromYmd = year + 101;
		refToYmd = year + 1231;
	}
	else
	{
		cout << "Error : unknown outtype:" << type << ". out." << endl;
		exit(104);
	}
}

//2017-10-23
void getYmdRangeFromYmdAndType2(const int ymd,const int hm , int type, int& refFromYmd, int& refToYmd, int& refFromHm , int& refToHm )
{
	refFromYmd = ymd;
	refToYmd = ymd;
	refFromHm = 0;
	refToHm = 2359;
	int day = ymd % 100;
	int ym = (ymd / 100) * 100;
	int hour0 = hm / 100;
	int minu0 = hm % 100;
	if (type == OUTTYPE_HALFHOUR)
	{
		refFromYmd = ymd;
		refToYmd = ymd;
		if (minu0 < 30)
		{
			refFromHm = hour0 * 100;
			refToHm = refFromHm + 29;
		}
		else {
			refFromHm = hour0 * 100 + 30 ;
			refToHm = refFromHm + 29;
		}
	}
	else if (type == OUTTYPE_HOUR)
	{
		refFromYmd = ymd;
		refToYmd = ymd;
		refFromHm = hour0 * 100;
		refToHm = refFromHm + 59;
	}
	else if (type == OUTTYPE_DAY)
	{//daily
		refFromYmd = ymd;
		refToYmd = ymd;
	}
	else if (type == OUTTYPE_FD)
	{//five day
		if (day == 31) day = 30;
		int startday = ((day - 1) / 5) * 5 + 1;
		refFromYmd = ym + startday;
		if (startday == 26)
		{
			refToYmd = refFromYmd + 5;
		}
		else
		{
			refToYmd = refFromYmd + 4;
		}
	}
	else if (type == OUTTYPE_TD)
	{//ten day
		if (day == 31) day = 30;
		int startday = ((day - 1) / 10) * 10 + 1;
		refFromYmd = ym + startday;
		if (startday == 21)
		{
			refToYmd = refFromYmd + 10;
		}
		else
		{
			refToYmd = refFromYmd + 9;
		}
	}
	else if (type == OUTTYPE_MON)
	{//month
		int startday = 1;
		refFromYmd = ym + startday;
		refToYmd = refFromYmd + 31;
	}
	else if (type == OUTTYPE_SEA)
	{//season
		int year = ymd / 10000;
		int month = (ymd / 100) % 100;
		if (month <= 2)
		{
			refFromYmd = (year - 1) * 10000 + 1201;
			refToYmd = year * 10000 + 229; // last day of Febuary.
		}
		else if (month <= 5)
		{
			refFromYmd = year * 10000 + 301;
			refToYmd = year * 10000 + 531;
		}
		else if (month <= 8)
		{
			refFromYmd = year * 10000 + 601;
			refToYmd = year * 10000 + 831;
		}
		else if (month <= 11)
		{
			refFromYmd = year * 10000 + 901;
			refToYmd = year * 10000 + 1130;
		}
		else
		{
			refFromYmd = year * 10000 + 1201;
			refToYmd = (year + 1) * 10000 + 229;
		}
	}
	else if (type == OUTTYPE_YEA)
	{//year
		int year = (ymd / 10000) * 10000;
		refFromYmd = year + 101;
		refToYmd = year + 1231;
	}
	else
	{
		cout << "Error : unknown outtype:" << type << ". out." << endl;
		exit(104);
	}
}

string makeOutFilePath(string dir, string prefix, int ymd, string type , string outid , string outtail )
{
	if (*dir.rbegin() != '/') dir = dir + "/";
	string outFilePath;
	if (outtail == "")
	{
		outFilePath = dir + prefix + wft_int2str(ymd) + "_" + outid + "_combination_" + type + ".tif";
	}
	else {
		outFilePath = dir + prefix + wft_int2str(ymd) + outtail ;
	}
	
	return outFilePath;
}

//2017-10-23
string makeOutFilePath2(string dir, string prefix, int ymd , int hm , string type, string outid, string outtail)
{
	if (*dir.rbegin() != '/') dir = dir + "/";
	string outFilePath;
	char buff[8];
	sprintf(buff, "%04d", hm);
	string hmstr(buff);
	if (outtail == "")
	{
		outFilePath = dir + prefix + wft_int2str(ymd) +hmstr + "_" + outid + "_combination_" + type + ".tif";
	}
	else {
		outFilePath = dir + prefix + wft_int2str(ymd) + hmstr + outtail;
	}

	return outFilePath;
}


void getFilesUnderDir(string dir, vector<string>& filesVector)
{
	wft_get_allfiles(dir, filesVector);
}


int doCombine(vector<string>& filesVector, string fixPrefix, string fixTail, string dsPrefix, string dsTail,
	int ymdLoc, int hmLoc, int fromYmd, int toYmd, int fromHm, int toHm
	, string outputFilepath
	, string combineProgram
	, string combMethod
	, string combMin
	, string combMax);


string g_programid = "fy4cmobine";
string g_logdir = "";

int main(int argc , char** argv )
{
	
	cout << "A program to do auto combination of fy4 products." << endl;
	cout << "Version 0.1a . nice work. wangfeng1@piesat.cn 2017-10-16" << endl;
	cout << "Version 0.1.1a . add startup file description." << endl;
	cout << "Version 0.1.2a . fix 31-40 bug." << endl;
	cout << "Version 0.1.3a . 2017-10-18 add output tail name param in startup file. It's optional." << endl;
	cout << "Version 0.1.4a . 2017-10-18 . #outtail bug fixed." << endl;
	cout << "Version 0.1.5a . 2017-10-23 . add 30min , 1hour period combination type. startup add hour minu location param. add log." << endl;
	cout << "Version 0.1.6a . 2017-11-10 . bugfixed for wftools for empty line bug.." << endl;
	if (argc == 1)
	{
		cout << "sample call: " << endl;
		cout << "fy4combine params.startup" << endl;

		cout << "combination type descripton:" << endl;
		cout << "hah:half-hour hou:hour day:day fiv:five-days ten:ten-days mon:month sea:season yea:year " << endl;

		cout << "sample startupfile:" << endl;
		
		cout << "#indir" << endl;
		cout << "E:/testdata/fy4sst15min/"<<endl;
		cout << "#filenamefixprefix"<<endl;
		cout << "FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_" << endl;
		cout << "#filenamefixtail" << endl;
		cout << ".NC" << endl;
		cout << "#outtype" << endl;
		cout << "hah/hou/day/fiv/ten/mon/sea/yea" << endl;
		cout << "#combineprogram" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/datacombination.exe" << endl;
		cout << "#filenameymdstartlocation" << endl;
		cout << "44" << endl;
		cout << "#filenamehmstartlocation[optional]" << endl;
		cout << "52" << endl;
		cout << "#outdir" << endl;
		cout << "E:/testdata/fy4sst15min/" << endl;
		cout << "#dsprefix" << endl;
		cout << "HDF5:\""<<endl;
		cout << "#dstail" << endl;
		cout << "\"://SST"<<endl;
		cout << "#outtail" << endl;
		cout << "_OPTIONAL.tif" << endl;
		cout << "#outid" << endl;
		cout << "SST" << endl;
		cout << "#combinemethod" << endl;
		cout << "aver" << endl;
		cout << "#combinevmin" << endl;
		cout << "-20" << endl;
		cout << "#combinevmax" << endl;
		cout << "50" << endl;
		cout << "#logdir" << endl;
		cout << "/extras/" << endl;
		cout << "" << endl;

		exit(101);
	}

	string startupFile(argv[1]);
	 
	string inDir =  wft_getValueFromExtraParamsFile(startupFile, "#indir", true);
	string fixPrefix = wft_getValueFromExtraParamsFile(startupFile, "#filenamefixprefix", true);
	string fixTail = wft_getValueFromExtraParamsFile(startupFile, "#filenamefixtail", true);
	string outType = wft_getValueFromExtraParamsFile(startupFile, "#outtype", true);
	string combinationProgram = wft_getValueFromExtraParamsFile(startupFile, "#combineprogram", true);
	string ymdLocStr = wft_getValueFromExtraParamsFile(startupFile, "#filenameymdstartlocation", true);

	string outDir = wft_getValueFromExtraParamsFile(startupFile, "#outdir", true);
	string dsPrefix = wft_getValueFromExtraParamsFile(startupFile, "#dsprefix", true);
	string dsTail = wft_getValueFromExtraParamsFile(startupFile, "#dstail", true);
	string outId = wft_getValueFromExtraParamsFile(startupFile, "#outid", true);
	string combineMethod = wft_getValueFromExtraParamsFile(startupFile, "#combinemethod", true);
	string combineMin = wft_getValueFromExtraParamsFile(startupFile, "#combinevmin", true);
	string combineMax = wft_getValueFromExtraParamsFile(startupFile, "#combinevmax", true);
	string outTailName = wft_getValueFromExtraParamsFile(startupFile, "#outtail", false);

	g_logdir = wft_getValueFromExtraParamsFile(startupFile, "#logdir", false);
	

	if (*outDir.rbegin() != '/') outDir = outDir + "/";

	const int ymdLoc = (int)atof(ymdLocStr.c_str());
	int todayYmd = wft_current_dateymd_int();
	int nowYmd, nowHm;
	nowYmd = wft_current_dateymd_int2(nowHm);


	int outTypeVal = OUTTYPE_DAY;
	if( outType == "day" ) outTypeVal = OUTTYPE_DAY;
	else if (outType == "fiv") outTypeVal = OUTTYPE_FD;
	else if (outType == "ten") outTypeVal = OUTTYPE_TD;
	else if (outType == "mon") outTypeVal = OUTTYPE_MON;
	else if (outType == "sea") outTypeVal = OUTTYPE_SEA;
	else if (outType == "yea") outTypeVal = OUTTYPE_YEA;
	else if (outType == "hah") outTypeVal = OUTTYPE_HALFHOUR;//2017-10-23
	else if (outType == "hou") outTypeVal = OUTTYPE_HOUR;//2017-10-23
	else
	{
		cout << "Error : unknown outtype:" << outType << ". out." << endl;
		wft_log(g_logdir, g_programid, string("Error : unknown out type ") + outType);
		exit(105);
	}

	//小时分钟在文件命中的起始位置
	int hmLoc = -1;
	if (outTypeVal == OUTTYPE_HALFHOUR || outTypeVal == OUTTYPE_HOUR)
	{
		string hmLocStr = wft_getValueFromExtraParamsFile(startupFile, "#filenamehmstartlocation", true);
		hmLoc = (int)atof(hmLocStr.c_str());
	}

	vector<string> filesVector;
	//get all files.
	getFilesUnderDir(inDir, filesVector);


	int tailLen = fixTail.length();
	int maxErrorCount = 5;
	int errorCount = 0;
	for (size_t ifile = 0; ifile < filesVector.size(); ++ifile)
	{
		//FY4A-_AGRI--_N_DISK_1047E_L2-_OLR-_MULT_NOM_20170919234500_20170919235959_4000M_V0001.NC
		string filename = wft_base_name(filesVector[ifile]);
		if (filename.find(fixPrefix) != string::npos)
		{
			if (filename.length() > tailLen)
			{
				string tTail = filename.substr(filename.length() - tailLen);
				if (tTail == fixTail)
				{
					if (outTypeVal == OUTTYPE_HALFHOUR || outTypeVal == OUTTYPE_HOUR)
					{
						string ymdStr = filename.substr(ymdLoc, 8);
						string hmStr = filename.substr( hmLoc , 4);
						int ymd = (int)atof(ymdStr.c_str());
						int hm = (int)atof(hmStr.c_str());
						
						int fromYmd(0), toYmd(0), fromHm(0), toHm(0);
						getYmdRangeFromYmdAndType2(ymd, hm, outTypeVal, fromYmd, toYmd, fromHm, toHm);
						if (fromYmd <= todayYmd && todayYmd <= toYmd && nowHm <= toHm )
						{//如果时间范围包含了当前时间。
							continue;
						}
						else {
							string outFilePath2 = makeOutFilePath2(outDir, fixPrefix, fromYmd , fromHm , outType, outId, outTailName);
							if (wft_test_file_exists(outFilePath2))
							{
								continue;
							}
							else
							{
								int retcode = doCombine(filesVector, fixPrefix, fixTail, dsPrefix, dsTail, ymdLoc, hmLoc, fromYmd, toYmd,
									fromHm, toHm, outFilePath2, combinationProgram, combineMethod, combineMin, combineMax);
								if (retcode != 0) {
									errorCount++;
									if (errorCount >= maxErrorCount)
									{
										wft_log( g_logdir , g_programid ,  "Error: reach max error count.");
									}
								}
								else {
									break;
								}
							}
						}
					}
					else {
						string ymdStr = filename.substr(ymdLoc, 8);
						int ymd = (int)atof(ymdStr.c_str());
						int fromYmd = ymd;
						int toYmd = ymd;
						getYmdRangeFromYmdAndType(ymd, outTypeVal, fromYmd, toYmd);
						if (fromYmd <= todayYmd && todayYmd <= toYmd)
						{//如果时间范围包含了今天，考虑到数据可能不完整，跳过不处理。
							continue;
						}
						else
						{//查看输出文件是否已经生成了。

							string outFilePath = makeOutFilePath(outDir, fixPrefix, fromYmd, outType, outId, outTailName);
							if (wft_test_file_exists(outFilePath))
							{
								continue;
							}
							else
							{
								int retcode = doCombine(filesVector, fixPrefix, fixTail, dsPrefix, dsTail, ymdLoc, hmLoc, fromYmd, toYmd,
									0, 9999, outFilePath, combinationProgram, combineMethod, combineMin, combineMax);
								if (retcode != 0) {
									errorCount++;
									if (errorCount >= maxErrorCount)
									{
										wft_log(g_logdir, g_programid, "Error: reach max error count.");
									}
								}
								else {
									break;
								}
							}
						}
					}
				}
			}
		}
	}




	cout << "done." << endl;
    return 0;
}


//2017-10-23
int doCombine(vector<string>& filesVector ,string fixPrefix, string fixTail,string dsPrefix,string dsTail,
	int ymdLoc , int hmLoc , int fromYmd , int toYmd , int fromHm , int toHm 
	, string outputFilepath 
	, string combineProgram
	, string combMethod
	,string combMin 
	,string combMax )
{
	cout << "need combination." << endl;
	cout << "fromYmd:" << fromYmd << " toYmd:" << toYmd << endl;
	cout << "fromHm:" << fromHm << " toHm:" << toHm << endl;
	int tailLen = fixTail.length();
	vector<string> useFilesVector;
	for (int ifile = 0; ifile < filesVector.size(); ++ifile)
	{
		string filename = wft_base_name(filesVector[ifile]);
		if (filename.find(fixPrefix) != string::npos)
		{
			if (filename.length() > tailLen)
			{
				string tTail = filename.substr(filename.length() - tailLen);
				if (tTail == fixTail)
				{
					string ymdStr = filename.substr(ymdLoc, 8);
					int ymd = (int)atof(ymdStr.c_str());
					if (hmLoc >= 0)
					{
						string hmStr = filename.substr(hmLoc, 4);
						int hm = (int)atof(hmStr.c_str());
						if (fromYmd <= ymd && ymd <= toYmd && fromHm <= hm && hm <= toHm)
						{
							string dsPath = dsPrefix + filesVector[ifile] + dsTail;
							useFilesVector.push_back(dsPath);
						}
					}
					else {
						if (fromYmd <= ymd && ymd <= toYmd )
						{
							string dsPath = dsPrefix + filesVector[ifile] + dsTail;
							useFilesVector.push_back(dsPath);
						}
					}
				}
			}
		}
	}

	if (useFilesVector.size() > 0)
	{
		std::string temptxt = outputFilepath + ".filelist.tmp";
		std::ofstream txtfs(temptxt.c_str());
		for (size_t iday = 0; iday < useFilesVector.size(); ++iday)
		{
			txtfs << useFilesVector[iday] << std::endl;
		}
		txtfs.close();

		string cmd = combineProgram + " -method " + combMethod ;
		cmd = cmd + " -vmin " + combMin ;
		cmd = cmd + " -vmax " + combMax ;
		cmd = cmd + " -out " + outputFilepath;
		cmd = cmd + " -infilestxt " + temptxt;

		wft_log(g_logdir, g_programid, cmd);

		int ret = system(cmd.c_str());
		cout << "datacombination return code :" << ret << endl;

		wft_log(g_logdir, g_programid, string("return code ") + wft_int2str(ret) );

		return ret;
	}
	else
	{
		cout << "Error : No files for combination from " << fromYmd << " to " << toYmd << endl;
		return 106;
	}
}

/*
老的合并文件代码2017-10-23以前
if (useCombination)
{
cout << "need combination." << endl;
cout << "fromYmd:" << useFromYmd << " toYmd:" << useToYmd << endl;

vector<string> useFilesVector;
for (int ifile = 0; ifile < filesVector.size(); ++ifile)
{
string filename = wft_base_name(filesVector[ifile]);
if (filename.find(fixPrefix) != string::npos)
{
if (filename.length() > tailLen)
{
string tTail = filename.substr(filename.length() - tailLen);
if (tTail == fixTail)
{
string ymdStr = filename.substr(ymdLoc, 8);
int ymd = (int)atof(ymdStr.c_str());
if (useFromYmd <= ymd && ymd <= useToYmd)
{
string dsPath = dsPrefix + filesVector[ifile] + dsTail;
useFilesVector.push_back(dsPath);
}
}
}
}
}

if (useFilesVector.size() > 0)
{
std::string temptxt = useOutputFilepath + ".filelist.tmp";
std::ofstream txtfs(temptxt.c_str());
for (size_t iday = 0; iday < useFilesVector.size(); ++iday)
{
txtfs << useFilesVector[iday] << std::endl;
}
txtfs.close();

string cmd = combinationProgram + " -method " + combineMethod;
cmd = cmd + " -vmin " + combineMin;
cmd = cmd + " -vmax " + combineMax;
cmd = cmd + " -out " + useOutputFilepath;
cmd = cmd + " -infilestxt " + temptxt;

int ret = system(cmd.c_str());
cout << "datacombination return code :" << ret << endl;
}
else
{
cout << "Error : No files for combination from "<<useFromYmd<<" to "<<useToYmd << endl;
exit(106);
}
}
*/