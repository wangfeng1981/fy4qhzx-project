// jiaohuhuitu.cpp : 定义控制台应用程序的入口点。
//交互绘图，输入时间和经纬度范围，和输出文件路径


#include <iostream>
#include <ctime>
#include <fstream>
#include "../../sharedcodes/wftools.h"
#include <vector>
#include <string>
#include "gdal_priv.h"
using namespace std;


bool isValidFile(string& filename, string& fixprefix, string& fixtail, int fixmidloc, int fixmidlen, string& fixmid,
	int timeloc, int timelen, int fromymd, int toymd)
{
	int pos0 = filename.find(fixprefix);
	if (pos0 != string::npos)
	{
		bool tailGood = false;
		if (fixtail == "") tailGood = true;
		if (tailGood == false)
		{
			int pos1 = filename.find(fixtail);
			if (pos1 != string::npos && pos1 == filename.length() - fixtail.length()) tailGood = true;
		}
		if (tailGood)
		{
			bool midgood = false;
			if (fixmidloc < 0 || fixmidlen <= 0 && fixmid == "") midgood = true;
			if (midgood == false)
			{
				if (filename.length() >= fixmidloc + fixmidlen)
				{
					string tempmid = filename.substr(fixmidloc, fixmidlen);
					if (tempmid == fixmid)
					{
						midgood = true;
					}
				}
			}
			if (midgood)
			{
				if (timeloc >= 0 && timelen > 0 && timeloc + timelen < filename.length())
				{
					string ymdstr = filename.substr(timeloc, timelen);
					int tymd = (int)atof(ymdstr.c_str());
					if (tymd >= fromymd && tymd <= toymd)
					{
						return true;
					}
				}
			}	
		}
	}
	return false;
}

void writeresultinfo(string& filepath, int code, string description)
{
	ofstream ofs(filepath.c_str());
	ofs << code << endl;
	ofs << description << endl;
	ofs.close();
}

string makeYmdStr(string year, string mon, string day)
{
	int y = (int)atof(year.c_str());
	int m = (int)atof(mon.c_str());
	int d = (int)atof(day.c_str());
	return wft_int2str(y * 10000 + m * 100 + d);
}

int main(int argc , char** argv )
{
	cout << "A program to plot some data with combined and subseted data. 2017-11-14. wangfeng1@piesat.cn" << endl;
	cout << "Version 0.1a 2017-11-14" << endl;
	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "jiaohuhuitu startup.txt" << endl;

		cout << "********* example startup.txt *************" << endl;
		cout << "#productdir" << endl;
		cout << "E:/testdata/fy4sst15min/2017/" << endl;
		cout << "#fixprefix" << endl;
		cout << "FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_" << endl;
		cout << "#fixtail" << endl;
		cout << "_SST_combination_day.tif" << endl;
		cout << "#fixmidloc" << endl;
		cout << "0" << endl;
		cout << "#fixmidlen" << endl;
		cout << "0" << endl;
		cout << "#fixmid" << endl;
		cout << " " << endl;
		cout << "#dsprefix" << endl;
		cout << " " << endl;
		cout << "#dstail" << endl;
		cout << " " << endl;
		cout << "#lltype[llfiles/llrange/geotrans]" << endl;
		cout << "llfiles" << endl;
		cout << "#lonfile" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy4lon.tif" << endl;
		cout << "#latfile" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy4lat.tif" << endl;
		cout << "#outrx" << endl;
		cout << "0.04" << endl;
		cout << "#outry" << endl;
		cout << "0.04" << endl;
		cout << "#gridleft" << endl;
		cout << "0" << endl;
		cout << "#gridright" << endl;
		cout << "0" << endl;
		cout << "#gridtop" << endl;
		cout << "0" << endl;
		cout << "#gridbottom" << endl;
		cout << "0" << endl;
		cout << "#year0" << endl;
		cout << "2017" << endl;
		cout << "#month0" << endl;
		cout << "10" << endl;
		cout << "#day0" << endl;
		cout << "1" << endl;
		cout << "#year1" << endl;
		cout << "2017" << endl;
		cout << "#month1" << endl;
		cout << "10" << endl;
		cout << "#day1" << endl;
		cout << "2" << endl;
		cout << "#valid0" << endl;
		cout << "-5" << endl;
		cout << "#valid1" << endl;
		cout << "50" << endl;
		cout << "#scale" << endl;
		cout << "1.0" << endl;
		cout << "#offset" << endl;
		cout << "0.0" << endl;
		cout << "#cutleft" << endl;
		cout << "120" << endl;
		cout << "#cutright" << endl;
		cout << "140" << endl;
		cout << "#cuttop" << endl;
		cout << "20" << endl;
		cout << "#cutbottom" << endl;
		cout << "0" << endl;
		cout << "#cb0" << endl;
		cout << "15" << endl;
		cout << "#cb1" << endl;
		cout << "35" << endl;
		cout << "#cblevelnum" << endl;
		cout << "10" << endl;
		cout << "#fntimeloc" << endl;
		cout << "44" << endl;
		cout << "#fntimelen" << endl;
		cout << "8" << endl;
		cout << "#gnuplot" << endl;
		cout << "gnuplot" << endl;
		cout << "#plottem" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/jiaohu-fy4sst-20171114.plot" << endl;
		cout << "#combinetype[min/max/aver]" << endl;
		cout << "aver" << endl;
		cout << "#outputpng" << endl;
		cout << "d:/demo.png" << endl;
		cout << "#outputinfo" << endl;
		cout << "d:/demo.info.txt" << endl;
		cout << "#combineprogram" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/datacombination" << endl;
		cout << "#txtprogram" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy4totxt" << endl;
		cout << "#warpprogram" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/createwarpvrt" << endl;
		cout << "#gdalwarp" << endl;
		cout << "gdalwarp" << endl;
		cout << "#pdtnodata" << endl;
		cout << "65535" << endl;
		cout << "**************** end *************" << endl;

		exit(101);
	}
	

	string startupfile = argv[1];
	string productdir =  wft_getValueFromExtraParamsFile(startupfile, "#productdir", true);

	string filenameFixPrefix = wft_getValueFromExtraParamsFile(startupfile, "#fixprefix", true);
	string filenameFixTail = wft_getValueFromExtraParamsFile(startupfile, "#fixtail", true);
	string filenameFixMidLoc = wft_getValueFromExtraParamsFile(startupfile, "#fixmidloc", true);
	string filenameFixMidLen = wft_getValueFromExtraParamsFile(startupfile, "#fixmidlen", true);
	string filenameFixMid = wft_getValueFromExtraParamsFile(startupfile, "#fixmid", true);

	string datasetPrefix = wft_getValueFromExtraParamsFile(startupfile, "#dsprefix", true);
	string datasetTail = wft_getValueFromExtraParamsFile(startupfile, "#dstail", true);

	string lonlatType = wft_getValueFromExtraParamsFile(startupfile, "#lltype", true); //llfiles,llrange,geotrans
	string lonfile = wft_getValueFromExtraParamsFile(startupfile, "#lonfile", true);
	string latfile = wft_getValueFromExtraParamsFile(startupfile, "#latfile", true);

	string warprx = wft_getValueFromExtraParamsFile(startupfile, "#outrx", true);
	string warpry = wft_getValueFromExtraParamsFile(startupfile, "#outry", true);

	string gridLeft = wft_getValueFromExtraParamsFile(startupfile, "#gridleft", true);
	string gridRight = wft_getValueFromExtraParamsFile(startupfile, "#gridright", true);
	string gridTop = wft_getValueFromExtraParamsFile(startupfile, "#gridtop", true);
	string gridBottom = wft_getValueFromExtraParamsFile(startupfile, "#gridbottom", true);

	string year0 = wft_getValueFromExtraParamsFile(startupfile, "#year0", true);
	string month0 = wft_getValueFromExtraParamsFile(startupfile, "#month0", true);
	string day0 = wft_getValueFromExtraParamsFile(startupfile, "#day0", true);
	string year1 = wft_getValueFromExtraParamsFile(startupfile, "#year1", true);
	string month1 = wft_getValueFromExtraParamsFile(startupfile, "#month1", true);
	string day1 = wft_getValueFromExtraParamsFile(startupfile, "#day1", true);

	string valid0str = wft_getValueFromExtraParamsFile(startupfile, "#valid0", true);
	string valid1str = wft_getValueFromExtraParamsFile(startupfile, "#valid1", true);
	

	string scalestr =  wft_getValueFromExtraParamsFile(startupfile, "#scale", true);
	string offsetstr = wft_getValueFromExtraParamsFile(startupfile, "#offset", true);

	string cutleft = wft_getValueFromExtraParamsFile(startupfile, "#cutleft", true);
	string cutright = wft_getValueFromExtraParamsFile(startupfile, "#cutright", true);
	string cuttop = wft_getValueFromExtraParamsFile(startupfile, "#cuttop", true);
	string cutdown = wft_getValueFromExtraParamsFile(startupfile, "#cutbottom", true);

	string value0Str = wft_getValueFromExtraParamsFile(startupfile, "#cb0", true);
	string value1Str = wft_getValueFromExtraParamsFile(startupfile, "#cb1", true);
	string valueLevelNum = wft_getValueFromExtraParamsFile(startupfile, "#cblevelnum", true);

	string filenameTimeLoc = wft_getValueFromExtraParamsFile(startupfile, "#fntimeloc", true);
	string filenameTimeLen = wft_getValueFromExtraParamsFile(startupfile, "#fntimelen", true);

	string gnuplot = wft_getValueFromExtraParamsFile(startupfile, "#gnuplot", true);
	string plotTemplate = wft_getValueFromExtraParamsFile(startupfile, "#plottem", true);

	string combineType = wft_getValueFromExtraParamsFile(startupfile, "#combinetype", true);
	string outputpng = wft_getValueFromExtraParamsFile(startupfile, "#outputpng", true);
	string outputinfo = wft_getValueFromExtraParamsFile(startupfile, "#outputinfo", true);

	string combineProgram = wft_getValueFromExtraParamsFile(startupfile, "#combineprogram", true);
	string txtProgram =     wft_getValueFromExtraParamsFile(startupfile, "#txtprogram", true);
	string warpProgram =    wft_getValueFromExtraParamsFile(startupfile, "#warpprogram", true);
	string gdalwarp =       wft_getValueFromExtraParamsFile(startupfile, "#gdalwarp", true);
	string pdtNoData =      wft_getValueFromExtraParamsFile(startupfile, "#pdtnodata", true);

	int fixmidloc = (int)atof(filenameFixMidLoc.c_str());
	int fixmidlen = (int)atof(filenameFixMidLen.c_str());
	double valid0 = atof(valid0str.c_str());
	double valid1 = atof(valid1str.c_str());
	double scale = atof(scalestr.c_str());
	double offset = atof(offsetstr.c_str());
	string ymd0str = makeYmdStr(year0, month0, day0);
	string ymd1str = makeYmdStr(year1, month1, day1);
	int fromYmd = (int)atof(ymd0str.c_str());
	int toYmd = (int)atof(ymd1str.c_str());
	int timeloc = (int)atof(filenameTimeLoc.c_str());
	int timelen = (int)atof(filenameTimeLen.c_str());

	vector<string> allfiles;
	wft_get_allfiles(productdir, allfiles);

	vector<string> selectedFiles;
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath1 = allfiles[i];
		string filename1 = wft_base_name(filepath1);
		cout << filename1 << endl;
		if (isValidFile(filename1, filenameFixPrefix, filenameFixTail, fixmidloc, fixmidlen, filenameFixMid, timeloc, timelen, fromYmd, toYmd))
		{
			selectedFiles.push_back(filepath1);
		}
	}

	if (selectedFiles.size() == 0)
	{
		string desc = string("No files from ") + ymd0str + " to " + ymd1str;
		writeresultinfo(outputinfo, 10, desc);
		exit(102);
	}

	string outputpngfilename = wft_base_name(outputpng);
	string temp_filelistforcombine = outputpng + ".combinefilelist.txt";
	wft_write_file_linebyline(temp_filelistforcombine, selectedFiles, datasetPrefix, datasetTail);

	string temp_combinefile = outputpng + ".combine.tif";
	string cmd1 = combineProgram + " -method " + combineType + " -vmin " + valid0str + " -vmax " + valid1str
		+ " -out " + temp_combinefile + " -infilestxt " + temp_filelistforcombine;
	int res1 = system(cmd1.c_str());
	cout << "combine result code:" << res1 << endl;

	if (wft_test_file_exists(temp_combinefile) == false)
	{
		string desc = string("making ") + temp_combinefile + " failed.";
		writeresultinfo(outputinfo, 103, desc);
		exit(103);
	}


	//warping
	string temp_warpfile = "";
	if (lonlatType == "llfiles")
	{
		temp_warpfile = outputpng + ".warp.tif";
		string cmd15 = warpProgram + " -type llfile "
			+ " -in " + temp_combinefile
			+ " -lonfile " + lonfile
			+ " -latfile " + latfile
			+ " -llnodata -999 "
			+ " -pdtnodata " + pdtNoData
			+ " -outtif " + temp_warpfile
			+ " -gdalwarp " + gdalwarp
			+ " -orx " + warprx + " -ory " + warpry;
		int res15 = system(cmd15.c_str());
		cout << "warp return code : " << res15 << endl;
		if (wft_test_file_exists(temp_warpfile) == false)
		{
			string desc = string("making ") + temp_warpfile + " failed.";
			writeresultinfo(outputinfo, 115, desc);
			exit(115);
		}
	}

	string temp_combinefilecopy = "";
	if (temp_warpfile != "")
	{
		lonlatType = "geotrans";
		temp_combinefilecopy = temp_combinefile;
		temp_combinefile = temp_warpfile;
	}
	
	string temp_txtfile = outputpng + ".xyz.txt";
	string cmd2 = txtProgram + " -in " + temp_combinefile
		+ " -out " + temp_txtfile
		+ " -type " + lonlatType 
		+ " -lon " + lonfile
		+ " -lat " + latfile
		+ " -left " + gridLeft
		+ " -right " + gridRight
		+ " -top " + gridTop
		+ " -bottom " + gridBottom
		+ " -valid0 " + valid0str
		+ " -valid1 " + valid1str
		+ " -scale " + scalestr
		+ " -offset " + offsetstr
		+ " -xspace 1 -yspace 1 "
		+ " -cutlon0 " + cutleft
		+ " -cutlon1 " + cutright
		+ " -cutlat0 " + cutdown
		+ " -cutlat1 " + cuttop;

	int res2 = system(cmd2.c_str());
	cout << "totxt result code:" << res2 << endl;

	if (wft_test_file_exists(temp_txtfile) == false)
	{
		string desc = string("making ") + temp_txtfile + " failed.";
		writeresultinfo(outputinfo, 104, desc);
		exit(104);
	}


	string temp_plotfile = outputpng + ".plot";
	vector<string> varVec;
	vector<string> repVec;

	string datetimeNow = wft_current_datetimestr();

	varVec.push_back("{{{INFILE}}}");
	varVec.push_back("{{{OUTFILE}}}");
	varVec.push_back("{{{MAKETIME}}}");
	varVec.push_back("{{{X0}}}");
	varVec.push_back("{{{X1}}}");
	varVec.push_back("{{{Y0}}}");
	varVec.push_back("{{{Y1}}}");
	varVec.push_back("{{{VAL0}}}");
	varVec.push_back("{{{VAL1}}}");
	varVec.push_back("{{{VALLVL}}}");


	repVec.push_back(temp_txtfile);
	repVec.push_back(outputpng);
	repVec.push_back(datetimeNow);
	repVec.push_back(cutleft);
	repVec.push_back(cutright);
	repVec.push_back(cutdown);
	repVec.push_back(cuttop);
	repVec.push_back(value0Str);
	repVec.push_back(value1Str);
	repVec.push_back(valueLevelNum);


	wft_create_file_by_template_with_replacement(temp_plotfile, plotTemplate, varVec, repVec);
	if (wft_test_file_exists(temp_plotfile) == false)
	{
		string desc = string("making ") + temp_plotfile + " failed.";
		writeresultinfo(outputinfo, 105, desc);
		exit(105);
	}

	string cmd3 = gnuplot + " " + temp_plotfile;
	int res3 = system(cmd3.c_str());
	cout << "gnuplot result code :" << res3 << endl;

	if (wft_test_file_exists(outputpng) == false)
	{
		string desc = string("making ") + outputpng + " failed.";
		writeresultinfo(outputinfo, 106, desc);
		exit(106);
	}



	//delete temp files
	wft_remove_file(temp_combinefile);
	wft_remove_file(temp_filelistforcombine);
	wft_remove_file(temp_txtfile);
	wft_remove_file(temp_combinefilecopy);

	writeresultinfo(outputinfo, 100 , string("OK") );

    return 0;
}

