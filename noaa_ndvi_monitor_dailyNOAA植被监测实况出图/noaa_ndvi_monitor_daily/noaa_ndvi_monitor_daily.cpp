// noaa_ndvi_monitor_daily.cpp : 定义控制台应用程序的入口点。
//noaa植被指数日产品绘图入库程序 2017-11-9

#include <iostream>
#include <ctime>
#include <fstream>
#include "../../sharedcodes/wftools.h"
#include <vector>
#include <string>
using namespace std;






bool isvalidNoaaNdviFile(string filename)
{
	// AVHRR - Land_v004 - preliminary_AVH13C1_NOAA-19_20170102_c20170103133724.nc
	size_t pos0 = filename.find("AVHRR-Land_");
	size_t pos1 = filename.find("_AVH13C1_NOAA-");
	size_t pos2 = filename.find(".nc");

	if (pos0 != string::npos && pos1 != string::npos && pos2 != string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}

string indir = "D:/avhrr_ndvi/";
string outdir = "D:/avhrr_ndvi/";
string wgs84Program = "E:/coding/fy4qhzx-project/extras/makegridwgs84";
string gdalWarp = "gdalwarp";
string chinaShp = "E:/coding/fy4qhzx-project/extras/shp/cutchina.shp";
string txtProgram = "E:/coding/fy4qhzx-project/extras/image2xyz";

int processOneFile(
	string& filepath,
	string& indir,
	string& outdir,
	string& wgs84Program,
	string& gdalWarp,
	string& chinaShp,
	string& txtProgram,
	string& plotTemplate,
	string& plotProgram,
	string& dbProgram,
	string& host,
	string& user,
	string& pwd,
	string& db,
	string& tb,
	string& pid
	)
{
	string dspath = string("HDF5:\"") + filepath + "\"://NDVI";
	string filename = wft_base_name(filepath);
	string temp_wgs84path = outdir + filename + ".wgs84.tif";
	//step1 wgs84
	string cmd1 = wgs84Program + " -in " + dspath + " -out " + temp_wgs84path + " -left -179.975 -right 179.975 -top 89.975 -bottom -89.975 ";
	cout << cmd1 << endl;
	int res1 = system(cmd1.c_str());
	cout << "wgs84Program result:" << res1 << endl;

	if (wft_test_file_exists(temp_wgs84path))
	{
		string temp_cutfile = outdir + filename + ".cut.tif";
		//"gdalwarp - overwrite - dstnodata - 9999 - q - cutline E : / coding / fy4qhzx - project / extras / shp / cutchina.shp - tr 0.05 0.05 - of GTiff noaa2017m1w.tif noaa2017m1cn.tif"
		string cmd2 = gdalWarp + " -overwrite -dstnodata -9999 -q -cutline " + chinaShp + " -crop_to_cutline -tr 0.05 0.05 -of GTiff " + temp_wgs84path + " " + temp_cutfile;
		cout << cmd2 << endl;
		int res2 = system(cmd2.c_str());
		cout << "gdalwarp cut result:" << res2 << endl;
		if (wft_test_file_exists(temp_cutfile))
		{
			string temp_albersfile = outdir + filename + ".albers.tif";
			//"gdalwarp -overwrite -t_srs "+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47" -srcnodata -9999 -dstnodata -9999 ndvichina.tif alberschina.tif"
			string cmd3 = gdalWarp + " -overwrite -t_srs \"+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47\" -srcnodata -9999 -dstnodata -9999 " + temp_cutfile + " " + temp_albersfile;
			cout << cmd3 << endl;
			int res3 = system(cmd3.c_str());
			cout << "gdalwarp albers result:" << res3 << endl;
			if (wft_test_file_exists(temp_albersfile))
			{
				string temp_txtfile = outdir + filename + ".xyz.txt";
				string out_pngfile = outdir + filename + ".png";
				string cmd4 = txtProgram + " -in " + temp_albersfile + " -out " + temp_txtfile + " -valid0 -2000 -valid1 10000 -scale 0.0001 -offset 0.0 -nan NaN ";
				cout << cmd4 << endl;
				int res4 = system(cmd4.c_str());
				cout << "image2xyz result:" << res4 << endl;
				if (wft_test_file_exists(temp_txtfile))
				{
					int posNOAA = filename.find("NOAA-");
					if (posNOAA != string::npos && posNOAA + 15 < filename.length())
					{
						int posYmd = posNOAA + 8;
						string ymd = filename.substr(posYmd, 8);
						vector<string> varvector, repvector;
						varvector.push_back("{{{INFILE}}}");
						varvector.push_back("{{{OUTFILE}}}");
						varvector.push_back("{{{TITLE}}}");
						repvector.push_back(temp_txtfile);
						repvector.push_back(out_pngfile);
						repvector.push_back(ymd);
						string temp_plotfile = outdir + filename + ".plot";
						wft_create_file_by_template_with_replacement(temp_plotfile, plotTemplate, varvector, repvector);
						if (wft_test_file_exists(temp_plotfile))
						{
							string cmd6 = plotProgram + " " + temp_plotfile;
							int res6 = system(cmd6.c_str());
							cout << "plot result:" << res6 << endl;
							if (wft_test_file_exists(out_pngfile))
							{
								//insert db.
								string cmd7 = dbProgram + " -host " + host + " -user " + user
									+ " -pwd " + pwd + " -db " + db + " -tb " + tb
									+ " -datapath " + filepath
									+ " -dtloc " + wft_int2str(posYmd)
									+ " -dtlen 8 "
									+ " -thumb " + out_pngfile
									+ " -pid " + pid;

								cout << cmd7 << endl;
								int res7 = system(cmd7.c_str());
								cout << "insertdb result:" << res7 << endl;

								//delete temp files.
								wft_remove_file(temp_albersfile);
								wft_remove_file(temp_cutfile);
								wft_remove_file(temp_txtfile);
								wft_remove_file(temp_wgs84path);

								return 100;
							}
							else {
								cout << "*** Error : failed to make png file " << out_pngfile << endl;
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
						cout << "*** Error : failed to get ymd from filename " << filename << endl;
						return 0;
					}
				}
				else {
					cout << "*** Error : failed to make txt file " << temp_albersfile << endl;
					return 0;
				}
			}
			else
			{
				cout << "*** Error : failed to make albers file " << temp_cutfile << endl;
				return 0;
			}
		}
		else {
			cout << "*** Error : failed to cut file " << temp_wgs84path << endl;
			return 0;
		}
	}
	else {
		cout << "*** Error : failed to make file " << temp_wgs84path << endl;
		return 0;
	}
	return 0;
}


int main(int argc, char** argv)
{
	cout << "A program to plot noaa avhrr ndvi and insert db." << endl;
	cout << "Version 0.1a by wangfeng1@piesat.cn 2017-11-9." << endl;
	cout << "Version 0.2a add db support. by wangfeng1@piesat.cn 2017-11-9." << endl;

	cout << "Version 0.2.1a modify wgs84 left top bottom right lon/lat . by wangfeng1@piesat.cn 2017-11-9." << endl;

	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "noaa_ndvi_monitor_daily startup.txt" << endl;
		cout << "******** startup.txt sample ********" << endl;
		cout << "#indir" << endl;
		cout << "D:/avhrr_ndvi/" << endl;
		cout << "#outdir" << endl;
		cout << "D:/avhrr_ndvi/" << endl;
		cout << "#wgs84" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/makegridwgs84" << endl;
		cout << "#gdalwarp" << endl;
		cout << "gdalwarp" << endl;
		cout << "#chinashp" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/shp/cutchina.shp" << endl;
		cout << "#txt" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/image2xyz" << endl;
		cout << "#plottem" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/noaa-ndvi-daily.plot" << endl;
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
		cout << "tb_data_noaandvi" << endl;
		cout << "#pid" << endl;
		cout << "38" << endl;

		cout << "**** **** **** **** ****" << endl;

		exit(101);
	}

	string startupfile = argv[1];

	string indir = "";// "D:/avhrr_ndvi/";
	string outdir = "";// "D:/avhrr_ndvi/";
	string wgs84Program = "";// "E:/coding/fy4qhzx-project/extras/makegridwgs84";
	string gdalWarp = "";//"gdalwarp";
	string chinaShp = "";// "E:/coding/fy4qhzx-project/extras/shp/cutchina.shp";
	string txtProgram = "";//"E:/coding/fy4qhzx-project/extras/image2xyz";
	string plotTemplate = "";// "E:/coding/fy4qhzx-project/extras/noaa-ndvi-daily.plot";
	string plotProgram = "";//"gnuplot";
	string dbProgram = "";// "";

	indir = wft_getValueFromExtraParamsFile(startupfile, "#indir", true);
	outdir = wft_getValueFromExtraParamsFile(startupfile, "#outdir", true);
	wgs84Program = wft_getValueFromExtraParamsFile(startupfile, "#wgs84", true);

	gdalWarp = wft_getValueFromExtraParamsFile(startupfile, "#gdalwarp", true);
	chinaShp = wft_getValueFromExtraParamsFile(startupfile, "#chinashp", true);
	txtProgram = wft_getValueFromExtraParamsFile(startupfile, "#txt", true);

	plotTemplate = wft_getValueFromExtraParamsFile(startupfile, "#plottem", true);
	plotProgram = wft_getValueFromExtraParamsFile(startupfile, "#plot", true);
	dbProgram = wft_getValueFromExtraParamsFile(startupfile, "#insertprogram", true);

	string host = wft_getValueFromExtraParamsFile(startupfile, "#host", true);
	string user = wft_getValueFromExtraParamsFile(startupfile, "#user", true);
	string pwd = wft_getValueFromExtraParamsFile(startupfile, "#pwd", true);
	string db = wft_getValueFromExtraParamsFile(startupfile, "#db", true);

	string tb = wft_getValueFromExtraParamsFile(startupfile, "#tb", true);
	string pid = wft_getValueFromExtraParamsFile(startupfile, "#pid", true);


	vector<string> allfiles;
	wft_get_allfiles(indir, allfiles);
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		if (isvalidNoaaNdviFile(filename))
		{
			string pngfilepath = outdir + filename + ".png";
			if (wft_test_file_exists(pngfilepath))
			{
				continue;
			}
			else {
				cout << "find one:" << filename << endl;
				//HDF5:"AVHRR-Land_v004-preliminary_AVH13C1_NOAA-19_20170630_c20170701100114.nc"://NDVI
				int ret = processOneFile(filepath, indir, outdir, wgs84Program, gdalWarp, chinaShp, txtProgram,
					plotTemplate, plotProgram, dbProgram,
					host, user, pwd, db, tb, pid);
				if (ret == 100)
				{
					break;
				}
			}
		}
	}

	cout << "done." << endl;
	return 0;
}


