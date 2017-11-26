// jifeng_monitor_daily.cpp : 定义控制台应用程序的入口点。
//季风日产品监测，绘图需要水汽总量和风场数据 2017-11-10
//能够叠加OLR

#include <iostream>
#include <ctime>
#include <fstream>
#include "../../sharedcodes/wftools.h"
#include <vector>
#include <string>
using namespace std;


#define INPUT_TYPE_TPW 0
#define INPUT_TYPE_OLR 1



bool isvalidFy4TpwDailyFile(string filename)
{
	// FY4A-_AGRI--_N_DISK_1047E_L2-_LPW-_MULT_NOM_20171006_TPW_combination_day.tif
	size_t pos0 = filename.find("FY4A-_AGRI--_N_DISK_1047E_L2-_LPW-_MULT_NOM_");
	size_t pos1 = filename.find("_TPW_combination_day.tif");
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

//2017-11-15 start
bool isvalidFy4OlrDailyFile(string filename)
{
	//
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
//2017-11-15 end


int processOneFile(
	int inputType,
	string& waterfilepath ,
	string& windfilepath ,
	string& outdir,
	string& outpngpath , 
	string& wgs84Program,
	string& lonfile , 
	string& latfile ,
	string& gdalWarp,
	string& txtProgram,
	string& plotTemplate,
	string& plotProgram,
	string& dbProgram,
	int ymdloc ,
	string& host,
	string& user,
	string& pwd,
	string& db,
	string& tb,
	string& pid
	)
{
	string dspath = waterfilepath;
	string filename = wft_base_name(waterfilepath);
	string temp_wgs84path = outdir + filename + ".wgs84.tif";
	//2017-11-15 start.
	string lutstr = "";
	string pdtnodata = "";
	string valid0 = "";
	string valid1 = "";
	if (inputType == INPUT_TYPE_TPW)
	{
		pdtnodata = " 65535 ";
		lutstr = " -1:65535,0:0,10:10,11:65535 ";
		valid0 = "0";
		valid1 = "10";
	}
	else
	{ 
		pdtnodata = " 32766 ";
		lutstr = " 39:32766,40:40,450:450,451:32766 ";
		valid0 = "40";
		valid1 = "450";
	}
	//2017-11-15 end.	

	//step1 wgs84
	string cmd1 = wgs84Program + " -in " + dspath + " -outtif " + temp_wgs84path + " -type llfile " +
		" -lonfile " + lonfile +
		" -latfile " + latfile
		+ " -llnodata -999 "
		+ " -pdtnodata  " + pdtnodata 
		+ " -pdtlut " + lutstr 
		+ " -gdalwarp " + gdalWarp
		+ " -orx 0.04 -ory 0.04 ";
	cout << cmd1 << endl;
	int res1 = system(cmd1.c_str());
	cout << "wgs84Program result:" << res1 << endl;

	if (wft_test_file_exists(temp_wgs84path))
	{
		string temp_txtfile = outdir + filename + ".xyz.txt";
		string cmd4 = txtProgram + " -in " + temp_wgs84path + " -out " + temp_txtfile 
			+ " -valid0 " + valid0  // 2017-11-15
			+ " -valid1 " + valid1  // 2017-11-15
			+ " -scale 1 -offset 0.0 -nan NaN -x0 30 -x1 180 -y0 -20  -y1 50 ";
		cout << cmd4 << endl;
		int res4 = system(cmd4.c_str());
		cout << "image2xyz result:" << res4 << endl;
		if (wft_test_file_exists(temp_txtfile))
		{

			int posYmd = ymdloc ;
			string ymd = filename.substr(posYmd, 8);
			vector<string> varvector, repvector;
			varvector.push_back("{{{INFILE1}}}");
			varvector.push_back("{{{INFILE2}}}");
			varvector.push_back("{{{OUTFILE}}}");
			varvector.push_back("{{{TITLE}}}");
			repvector.push_back(temp_txtfile);
			repvector.push_back(windfilepath);
			repvector.push_back(outpngpath);
			repvector.push_back(ymd);
			string temp_plotfile = outdir + filename + ".plot";
			wft_create_file_by_template_with_replacement(temp_plotfile, plotTemplate, varvector, repvector);
			if (wft_test_file_exists(temp_plotfile))
			{
				string cmd6 = plotProgram + " " + temp_plotfile;
				int res6 = system(cmd6.c_str());
				cout << "plot result:" << res6 << endl;
				if (wft_test_file_exists(outpngpath))
				{
					return 0;
					//insert db.
					string cmd7 = dbProgram + " -host " + host + " -user " + user
						+ " -pwd " + pwd + " -db " + db + " -tb " + tb
						+ " -datapath " + waterfilepath
						+ " -dtloc " + wft_int2str(posYmd)
						+ " -dtlen 8 "
						+ " -thumb " + outpngpath
						+ " -pid " + pid;

					cout << cmd7 << endl;
					int res7 = system(cmd7.c_str());
					cout << "insertdb result:" << res7 << endl;

					//delete temp files.
					wft_remove_file(temp_txtfile);
					wft_remove_file(temp_wgs84path);

					return 100;
				}
				else {
					cout << "*** Error : failed to make png file " << outpngpath << endl;
					return 0;
				}
			}
			else {
				cout << "*** Error : failed to make plot file " << temp_plotfile << endl;
				return 0;
			}
		}
		else
		{
			cout << "*** Error : failed to make txt file  " << temp_txtfile << endl;
			return 0;
		}


	}
	else {
		cout << "*** Error : failed to make file " << temp_wgs84path << endl;
		return 0;
	}
	return 0;
}

bool hasNcepWindFile(string& tpwfilename,int ymdloc, string& winddir , string& windfilepath )
{
	string ymdstr = tpwfilename.substr(ymdloc, 8);
	string windfilename = string("ncepwind.") + ymdstr + ".850.xyuv.txt";
	string windpath = winddir + windfilename;
	if (wft_test_file_exists(windpath))
	{
		windfilepath = windpath;
		return true;
	}
	else {
		windfilepath = "";
		return false;
	}
}




int main(int argc, char** argv)
{
	cout << "A program to plot jifeng(TPW/OLR + Wind) and insert db." << endl;
	cout << "Version 0.1a by wangfeng1@piesat.cn 2017-11-10." << endl;
	cout << "Version 0.1.1a bugfixed for finding tpw file. 2017-11-10." << endl;
	cout << "Version 0.2a add support of OLR. 2017-11-15." << endl;//2017-11-15

	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "jifeng_monitor_daily startup.txt" << endl;
		cout << "******** startup.txt sample ********" << endl;
		cout << "#tpwdir" << endl;//2017-11-15
		cout << "D:/water/" << endl;
		cout << "#olrdir" << endl;//2017-11-15
		cout << "D:/olr/" << endl;//2017-11-15
		cout << "#winddir" << endl;
		cout << "D:/wind/" << endl;

		cout << "#lonfile" << endl;
		cout << "fy4lon.tif" << endl;

		cout << "#latfile" << endl;
		cout << "fy4lat.tif" << endl;


		cout << "#outdir" << endl;
		cout << "D:/out/" << endl;

		cout << "#wgs84" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/createwarpvrt" << endl;

		cout << "#gdalwarp" << endl;
		cout << "gdalwarp" << endl;

		cout << "#txt" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/image2xyz" << endl;
		cout << "#plottem" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/template.plot" << endl;
		cout << "#plot" << endl;
		cout << "gnuplot" << endl;
		cout << "#insertprogram" << endl;
		cout << "/root/ncc-fy4-project/produce_codes/insertdb/insertdb" << endl;
		cout << "#host" << endl;
		cout << "localhost" << endl;
		cout << "#user" << endl;
		cout << "htht" << endl;
		cout << "#pwd" << endl;
		cout << "htht000000" << endl;
		cout << "#db" << endl;
		cout << "qhzx_uus" << endl;
		cout << "#tb" << endl;
		cout << "tb_product_data" << endl;
		cout << "#pid" << endl;
		cout << "39/51" << endl;//2017-11-15

		cout << "**** **** **** **** ****" << endl;

		exit(101);
	}

	string startupfile = argv[1];
	/*
	string tpwindir = "E:/testdata/fy4lwpm15/2017/";// "D:/avhrr_ndvi/";
	string winddir = "E:/testdata/ncep-wind/";
	string outdir = "D:/";// "D:/avhrr_ndvi/";
	string wgs84Program = "E:/coding/fy4qhzx-project/extras/createwarpvrt";// "E:/coding/fy4qhzx-project/extras/makegridwgs84";
	string gdalWarp = "gdalwarp";//"gdalwarp";
	string txtProgram = "E:/coding/fy4qhzx-project/extras/image2xyz";//"E:/coding/fy4qhzx-project/extras/image2xyz";
	string plotTemplate = "E:/coding/fy4qhzx-project/extras/jifeng20171110.plot";// "E:/coding/fy4qhzx-project/extras/noaa-ndvi-daily.plot";
	string plotProgram = "gnuplot";//"gnuplot";
	string dbProgram = "";// "";
	string lonfile = "E:/coding/fy4qhzx-project/extras/fy4lon.tif";
	string latfile = "E:/coding/fy4qhzx-project/extras/fy4lat.tif";

	string host ="";
	string user = "";
	string pwd ="";
	string db = "";

	string tb = "";
	string pid = "";

	*/
	
	int ymdloc = 44;

	string tpwindir = wft_getValueFromExtraParamsFile(startupfile, "#tpwdir", false);//2017-11-15
	string olrindir = wft_getValueFromExtraParamsFile(startupfile, "#olrdir", false);//2017-11-15
	string winddir = wft_getValueFromExtraParamsFile(startupfile, "#winddir", true);
	//2017-11-15 start.
	int inputType = 0;
	if (tpwindir == "" && olrindir == "")
	{
		cout << "Tpw dir and olr dir both empty. out." << endl;
		exit(102);
	}
	else if (tpwindir != "")
	{
		inputType = INPUT_TYPE_TPW;
	}
	else {
		inputType = INPUT_TYPE_OLR;
	}
	//2017-11-15 end.


	string lonfile = wft_getValueFromExtraParamsFile(startupfile, "#lonfile", true);
	string latfile = wft_getValueFromExtraParamsFile(startupfile, "#latfile", true);

	string outdir = wft_getValueFromExtraParamsFile(startupfile, "#outdir", true);
	string wgs84Program = wft_getValueFromExtraParamsFile(startupfile, "#wgs84", true);

	string gdalWarp = wft_getValueFromExtraParamsFile(startupfile, "#gdalwarp", true);

	string txtProgram = wft_getValueFromExtraParamsFile(startupfile, "#txt", true);

	string plotTemplate = wft_getValueFromExtraParamsFile(startupfile, "#plottem", true);
	string plotProgram = wft_getValueFromExtraParamsFile(startupfile, "#plot", true);
	string dbProgram = wft_getValueFromExtraParamsFile(startupfile, "#insertprogram", true);

	string host = wft_getValueFromExtraParamsFile(startupfile, "#host", true);
	string user = wft_getValueFromExtraParamsFile(startupfile, "#user", true);
	string pwd = wft_getValueFromExtraParamsFile(startupfile, "#pwd", true);
	string db = wft_getValueFromExtraParamsFile(startupfile, "#db", true);

	string tb = wft_getValueFromExtraParamsFile(startupfile, "#tb", true);
	string pid = wft_getValueFromExtraParamsFile(startupfile, "#pid", true);




	vector<string> allfiles;
	if (inputType == INPUT_TYPE_TPW)
	{
		wft_get_allfiles(tpwindir, allfiles);
	}
	else
	{
		wft_get_allfiles(olrindir, allfiles);
	}

	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);

		if ( ( inputType == INPUT_TYPE_TPW && isvalidFy4TpwDailyFile(filename)) || (inputType == INPUT_TYPE_OLR && isvalidFy4OlrDailyFile(filename)) )		//2017-11-15
		{
			string pngfilepath = outdir + filename + ".monsoons.png";
			if (wft_test_file_exists(pngfilepath))
			{
				continue;
			}
			else {
				cout << "find one:" << filename << endl;
				string windfilepath;
				if (hasNcepWindFile(filename, ymdloc, winddir, windfilepath))
				{//find wind file.
					string windname = wft_base_name(windfilepath);
					cout << " find wind : " << windname << endl;

					int ret = processOneFile(//2017-11-15
						inputType , //2017-11-15
						filepath , //2017-11-15
						windfilepath , outdir , 
						pngfilepath , 
						wgs84Program,
						lonfile, 
						latfile , 
						gdalWarp , txtProgram,
						plotTemplate, plotProgram, dbProgram,
						ymdloc , 
						host, user, pwd, db, tb, pid );
					if (ret == 100)
					{
						break;
					}

				}
				
			}
		}
	}

	cout << "done." << endl;
	return 0;
}


