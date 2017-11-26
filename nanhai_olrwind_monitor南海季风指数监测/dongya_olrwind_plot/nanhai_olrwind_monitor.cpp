// nanhai_olrwind_monitor.cpp : 定义控制台应用程序的入口点。
//南海olr和纬向风监测折线图绘图

#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include "../../sharedcodes/wftools.h"
using namespace std;


string makeOutfilename( int ymd , string& prefix)
{
	return prefix + wft_int2str(ymd) + ".png";
}

string getValueFromFile(string& filepath)
{
	if (wft_test_file_exists(filepath))
	{
		ifstream ifs(filepath.c_str());
		string line;
		getline(ifs, line);
		ifs.close();
		return line;
	}
	else {
		return "NaN";
	}
}

int processOneFile(int startymd, int endymd, string& olrdir, string& olrprefix, string& uwinddir, string& windprefix,
	string outpath , string& gnuplot, string& templatefile , string& x0str,string& x1str , 
	string& dbProgram ,string& host, string& user , string& pwd , string& db , string& tb , string& pid )
{
	if (startymd > endymd)
	{
		cout << "Error : startymd " << startymd << " can not greater than endymd " << endymd << endl;
		return 10;
	}
	int startyear = startymd / 10000;
	int startmon = (startymd % 10000) / 100;
	int startday = startymd % 100;

	int endyear = endymd / 10000;
	int endmon = (endymd % 10000) / 100;
	int endday = endymd % 100;

	string temp_xyzfile = outpath + ".xyz.txt";
	ofstream ofs(temp_xyzfile.c_str());
	int mondays[] = { 0 , 31,28,31,30,31,30,31,31,30,31,30,31 };
	ofs << "#date olr uwind" << endl;
	for (int year = startyear; year <= endyear; ++year)
	{
		if (wft_is_leapyear(year)) mondays[2] = 29;
		else mondays[2] = 28;
		for (int mon = 1; mon <= 12; ++mon)
		{
			for (int day = 1; day <= mondays[mon] ; ++day)
			{
				int tymd = year * 10000 + mon * 100 + day;
				if (tymd >= startymd && tymd <= endymd)
				{
					string x = wft_int2str(year) + "-" + wft_int2str(mon) + "-" + wft_int2str(day);

					//dongya.uwind850aver.20170920.txt dongya.fy4olraver.20170920.txt
					string y1filepath = olrdir + olrprefix + wft_int2str(tymd) + ".txt";
					string y2filepath = uwinddir + windprefix + wft_int2str(tymd) + ".txt";
					string y1 = getValueFromFile(y1filepath);
					string y2 = getValueFromFile(y2filepath);
					ofs << x << " " << y1 << " " << y2 << endl;

				}
			}
		}
	}
	ofs.close();

	//ploting
	if (wft_test_file_exists(temp_xyzfile) == false )
	{
		cout << "Error : make " << temp_xyzfile << " failed." << endl;
		return 10;
	}

	vector<string> varvec;
	varvec.push_back("{{{INFILE}}}");
	varvec.push_back("{{{OUTFILE}}}");
	varvec.push_back("{{{X0}}}");
	varvec.push_back("{{{X1}}}");
	varvec.push_back("{{{TIME0}}}");
	varvec.push_back("{{{TIME1}}}");
	varvec.push_back("{{{MAKETIME}}}");

	vector<string> repvec;
	repvec.push_back(temp_xyzfile);
	repvec.push_back(outpath);
	repvec.push_back(x0str);
	repvec.push_back(x1str);
	repvec.push_back(wft_ymd_int2str(startymd));
	repvec.push_back(wft_ymd_int2str(endymd));
	repvec.push_back(wft_current_datetimestr());

	string plotfile = outpath + ".plot";
	wft_create_file_by_template_with_replacement(plotfile, templatefile, varvec, repvec);

	if (wft_test_file_exists(plotfile) == false)
	{
		cout << "Error : make " << plotfile << " failed." << endl;
		return 11;
	}

	string cmd1 = gnuplot + " " + plotfile;
	int res1 = system(cmd1.c_str());
	cout << "gnuplot result code : " << res1 << endl;

	if (wft_test_file_exists(outpath) == false)
	{
		cout << "Error : make " << outpath << " failed." << endl;
		return 12;
	}

	wft_remove_file(temp_xyzfile);

	//insert db.
	cout << "Warning in windows no insert db " << endl;
	return 0;
	string cmd7 = dbProgram + " -host " + host + " -user " + user
		+ " -pwd " + pwd + " -db " + db + " -tb " + tb
		+ " -datapath " + outpath
		+ " -dtloc 0 " 
		+ " -dtlen 0 "
		+ " -thumb " + outpath
		+ " -pid " + pid
		+ " -startdate " + wft_int2str(endymd)  ;

	cout << cmd7 << endl;
	int res7 = system(cmd7.c_str());
	cout << "insertdb result:" << res7 << endl;

	return 100;
}


int main(int argc , char** argv)
{
	cout << "A program to monitor South China Sea OLR and Uwind. Use points and lines. 2017-11-16. by wangfeng1@piesat.cn." << endl;
	cout << "Version 1.0a " << endl;
	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "nanhai_olrwind_monitor startup.txt" << endl;

		cout << "******************* sample startup.txt ************************" << endl;
		cout << "#nanhaiwinddir" << endl;
		cout << "E:/testdata/ncep-wind/nanhai-wind/" << endl;
		cout << "#nanhaiwindprefix" << endl;
		cout << "nanhai.uwind850aver." << endl;
		cout << "#nanhaiolrdir" << endl;
		cout << "E:/testdata/fy4olr15min/nanhai-olr/" << endl;
		cout << "#nanhaiolrprefix" << endl;
		cout << "nanhai.fy4olraver." << endl;
		cout << "#outdir" << endl;
		cout << "E:/testdata/jifeng-out/" << endl;
		cout << "#outprefix" << endl;
		cout << "nanhai.olruwind." << endl;
		cout << "#startyear" << endl;
		cout << "2017" << endl;
		cout << "#plot" << endl;
		cout << "gnuplot" << endl;
		cout << "#plottem" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/nanhai2017-11-16.plot" << endl;
		cout << "#insertprogram" << endl;
		cout << "/root/ncc-fy4-project/produce_codes/insertdb/insertdb" << endl;
		cout << "#host" << endl;
		cout << "localhost" << endl;
		cout << "#user" << endl;
		cout << "htht" << endl;
		cout << "#pwd" << endl;
		cout << "htht123456" << endl;
		cout << "#db" << endl;
		cout << "qhzx_uus" << endl;
		cout << "#tb" << endl;
		cout << "tb_product_data" << endl;
		cout << "#pid" << endl;
		cout << "52" << endl;

		cout << "******************* *****************  ************************" << endl;
		exit(101);
	}
	
	string startupfile = argv[1];
	string dywinddir = wft_getValueFromExtraParamsFile(startupfile, "#nanhaiwinddir", true);
	string windprefix = wft_getValueFromExtraParamsFile(startupfile, "#nanhaiwindprefix", true);
	string dyolrdir = wft_getValueFromExtraParamsFile(startupfile, "#nanhaiolrdir", true);
	string olrprefix = wft_getValueFromExtraParamsFile(startupfile, "#nanhaiolrprefix", true);
	string outdir = wft_getValueFromExtraParamsFile(startupfile, "#outdir", true);
	string outprefix = wft_getValueFromExtraParamsFile(startupfile, "#outprefix", true);
	string startYearStr = wft_getValueFromExtraParamsFile(startupfile, "#startyear", true);
	string temfile = wft_getValueFromExtraParamsFile(startupfile, "#plottem", true);
	string gnuplot = wft_getValueFromExtraParamsFile(startupfile, "#plot", true);

	string dbProgram = wft_getValueFromExtraParamsFile(startupfile, "#insertprogram", true);
	string host = wft_getValueFromExtraParamsFile(startupfile, "#host", true);
	string user = wft_getValueFromExtraParamsFile(startupfile, "#user", true);
	string pwd = wft_getValueFromExtraParamsFile(startupfile, "#pwd", true);
	string db = wft_getValueFromExtraParamsFile(startupfile, "#db", true);
	string tb = wft_getValueFromExtraParamsFile(startupfile, "#tb", true);
	string pid = wft_getValueFromExtraParamsFile(startupfile, "#pid", true);

	//jiaohuhuitu
	//string jiaohuStartYmd = "";
	//string jiaohuEndYmd = "";
	//string jiaohuOutpng = "";

	int startYear = (int)atof(startYearStr.c_str());
	int ymd = wft_current_dateymd_int();
	int cyear = ymd / 10000;
	int mondays[] = { 0 , 31,28,31,30,31,30,31,31,30,31,30,31 };

	string x0str = wft_ymd_int2str( cyear*10000+101 ) ;
	string x1str = wft_ymd_int2str(cyear * 10000 + 1231);

	for (int year = startYear; year <= cyear; ++year)
	{
		if (wft_is_leapyear(year)) mondays[2] = 29;
		else mondays[2] = 28;
		for (int mon = 1; mon <= 12; ++mon)
		{
			for (int day = 1; day <= mondays[mon]; ++day)
			{
				int tymd = year * 10000 + mon * 100 + day;
				if (tymd >= ymd) break;
				string outname = makeOutfilename(tymd, outprefix);
				string outpath = outdir + outname;
				if (wft_test_file_exists(outpath))
				{
					continue;
				}
				else {
					int tstart = year * 10000 + 101;
					int ret = processOneFile(tstart, tymd, dyolrdir, olrprefix , dywinddir , windprefix , outpath , gnuplot , temfile ,x0str,x1str ,
						dbProgram , host , user , pwd , db , tb , pid );
					cout << "make " << outname << " result : " << ret << endl;
				}
			}
		}
	}

	cout << "done." << endl;
    return 0;
}

