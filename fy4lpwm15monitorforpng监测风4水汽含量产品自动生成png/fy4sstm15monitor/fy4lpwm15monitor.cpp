// fy4sstm15monitor.cpp : 定义控制台应用程序的入口点。
//改程序不负责检查是否入库和入库操作，只负责生成png
//监控fy4 水汽含量 15min数据产品，定时调用该程序，扫描目录：
//1.检查该文件是否已经生成png？
//2.0 生成vrt
//2.1 gdalwarp校正成wgs84.tif
//2.2 生成lonlatval.txt
//3.根据模板生成pdt.plot脚本文件
//4.调用gnuplot 生成.pdt.png
//5.检查png是否生成成功
//6.成功的话退出程序
//7.不成功的话记录到日志，处理下一个，直至处理成功一个，然后退出
// wangfeng1@piesat.cn 2017-9-27

#define UNICODE
#include "../../sharedcodes/tinydir.h"
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include "../../sharedcodes/wftools.h"
using namespace std;

extern int processOneFile(string undbFile, string fy4totxtProgram, string fy4lonFile, string fy4latFile, 
	string logDir, string programid,string	plotTemFile , string vrtProgram ,string gdalwarp, string gnuplot, string dsName,double valid0,double valid1);

int main( int argc  , char** argv )
{
	if (argc == 1)
	{
		cout << "V0.1a Fy4 LPW 15min product monitor." << endl;
		cout << "by wangfengdev@163.com 2017-10-12." << endl;

		cout << "Sample call:" << endl;
		cout << " fy4lpwm15monitorforpng startup.params " << endl;
		cout << "No enought parameters. out." << endl;
		exit(101);
	}

	std::string startupFile = argv[1];

	string inDir, outDir, plotTemFile , logDir , 
		fy4lonFile , fy4latFile , fy4totxtProgram , 
		vrtProgram ;
	inDir = wft_getValueFromExtraParamsFile(startupFile, "#indir", true);
	outDir = wft_getValueFromExtraParamsFile(startupFile, "#outdir", true);
	plotTemFile = wft_getValueFromExtraParamsFile(startupFile, "#plotfile", true);
	fy4lonFile = wft_getValueFromExtraParamsFile(startupFile, "#fy4lon", true);
	fy4latFile = wft_getValueFromExtraParamsFile(startupFile, "#fy4lat", true);
	logDir = wft_getValueFromExtraParamsFile(startupFile, "#logdir", true);
	fy4totxtProgram = wft_getValueFromExtraParamsFile(startupFile, "#fy4totxtprogram", true);
	vrtProgram = wft_getValueFromExtraParamsFile(startupFile, "#vrtprogram", true); 
	
	string gdalwarp = wft_getValueFromExtraParamsFile(startupFile, "#gdalwarpprogram", true);
	string gnuplot = wft_getValueFromExtraParamsFile(startupFile, "#plotprogram", true);

	int maxError = 2;
	{//最大错误数量，达到后退出
		std::string tstr = wft_getValueFromExtraParamsFile(startupFile, "#maxerror", false);
		if (tstr != "") maxError = max(1, atof(tstr.c_str()) );
	}

	if (*inDir.rbegin() != '/') {
		inDir += "/";
	}
	if (*outDir.rbegin() != '/') {
		outDir += "/";
	}
	if (*logDir.rbegin() != '/') {
		logDir += "/";
	}
	const string programid = wft_getValueFromExtraParamsFile(startupFile, "#programid", true); 
	wft_log(logDir,programid , "begin");

	//FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_20170815000000_20170815001459_4000M_V0001.NC
	//1.检查该文件是否已经生成png？已有png跳过。找到一个未做png的文件
	string undbFile = "";
	vector<string> dirFiles;
	wft_get_allfiles(inDir, dirFiles);
	int errorCount = 0;
	for (size_t i = 0; i < dirFiles.size(); ++i)
	{
		string tempfile = dirFiles[i];
		size_t pos0 = tempfile.rfind("FY4A-_AGRI--_N_DISK");
		size_t pos1 = tempfile.rfind("L2-_LPW-_MULT_NOM");
		size_t pos2 = tempfile.rfind(".NC");
		if (pos0 != string::npos && pos1 != string::npos && pos2 != string::npos && pos0 < pos1 && pos1 < pos2
			&& pos2 == tempfile.length() - 3 )
		{//有效的风四sst数据
			bool noPng = true ;
			string thepngfile = tempfile + ".png";
			if (wft_test_file_exists(thepngfile)) {
				noPng = false;
			}
			if (noPng)
			{
				undbFile = tempfile;
				int ret = processOneFile(undbFile, fy4totxtProgram, fy4lonFile, fy4latFile, logDir, programid, plotTemFile , vrtProgram,gdalwarp,gnuplot,"",0,10 );
				int ret1 = processOneFile(undbFile, fy4totxtProgram, fy4lonFile, fy4latFile, logDir, programid, plotTemFile, vrtProgram, gdalwarp, gnuplot, "LPW_HIGH",0,10);
				int ret2 = processOneFile(undbFile, fy4totxtProgram, fy4lonFile, fy4latFile, logDir, programid, plotTemFile, vrtProgram, gdalwarp, gnuplot, "LPW_LOW",0,10);
				int ret3 = processOneFile(undbFile, fy4totxtProgram, fy4lonFile, fy4latFile, logDir, programid, plotTemFile, vrtProgram, gdalwarp, gnuplot, "LPW_MID",0,10);
				if (ret == 100 && ret1==100 && ret2==100 && ret3==100) {
					break;
				}
				else {
					errorCount++;
					if (errorCount >= maxError)
					{
						wft_log(logDir, programid, "Error count reach maxError. out.");
						exit(103);
					}
				}
			}
		}
	}

	if (undbFile == "")
	{
		wft_log(logDir , programid , "every file is in db, no need to continue." );
		exit(102);
	}

	wft_log(logDir, programid, "done");
	cout << "done." << endl;
    return 0;
}


int processOneFile(string ncFile , string fy4totxtProgram , string fy4lonFile , string fy4latFile ,
	string logDir , string programid , string plotTemFile ,
	string vrtProgram , string gdalwarp , string gnuplot , string dsName , double valid0 , double  valid1 )
{
	string outroot = ncFile;
	if (dsName != "")
	{
		outroot = ncFile + "." + dsName;
	}
	else {
		dsName = "TPW";
	}
		
	//2.未入库，生成lonlatval.txt
	string llvFile = outroot + ".llv.txt";
	{
		//HDF5:"E:/testdata/fy4sst15min/FY4A-_AGRI--_N_DISK_1047E_L2-_SST-
		//_MULT_NOM_20170815000000_20170815001459_4000M_V0001.NC"://SST
		//2.1 create vrt
		string vrtFile = outroot + ".warp.vrt";
		string wgs84tif = outroot + ".wgs84.tif";
		string cmd21=vrtProgram +		
			" -type llfile "
			+ " -in HDF5:\"" + ncFile +  "\"://" + dsName
			+ " -lonfile "+fy4lonFile 
			+ " -latfile " + fy4latFile 
			+ " -pdtnodata 65535 "
			//+ " -pdtlut -1:65536,0:0,10:10,11:65535 "
			+" -pdtlut " + wft_float2str(valid0-1) + ":65536,"+ wft_float2str(valid0) +":"+wft_float2str(valid0)+","+wft_float2str(valid1)+":"+ wft_float2str(valid1) +","+ wft_float2str(valid1 + 1) +":65535 "
			+ " -outvrt " + vrtFile ;
		int ret21 = std::system(cmd21.c_str());
		cout << "createvrtprogram return code :" << ret21 << endl;
		if (wft_test_file_exists(vrtFile) == false) {
			wft_log(logDir, programid, llvFile + " is failed to create. out.");
			return 121;
		}

		string cmd22 = gdalwarp + " -geoloc -t_srs \"+proj=longlat +ellps=WGS84\" -r bilinear -tr 0.1 0.1 -overwrite ";
		cmd22 += vrtFile + " " + wgs84tif;
		int ret22 = std::system(cmd22.c_str());
		cout << "gdalwarp return code :" << ret22 << endl;
		if (wft_test_file_exists(wgs84tif) == false) {
			wft_log(logDir, programid, wgs84tif + " is failed to create. out.");
			return 122;
		}

		string cmd2 = fy4totxtProgram + " -in ";
		cmd2 += wgs84tif + " -out " + llvFile 
			+ " -type llrange -left 23.94716 -right 185.49716 " 
			+ " -top 73.98172 -bottom -75.01828 " 
			+ " -valid0 "+ wft_float2str(valid0) +" -valid1 "+wft_float2str(valid1)+" -xspace 2 -yspace 2";
		int ret = std::system(cmd2.c_str());
		cout << "fy4totxtProgram return code :" << ret << endl;

		if (wft_test_file_exists(llvFile) == false) {
			wft_log(logDir, programid, llvFile + " is failed to create. out.");
			return 123;
		}
	}

	//3.根据模板生成.plot脚本文件 #OUTPNGFILE,LONLATVALTXTFILE,TITLE1,TITLE
	string theplotfile = outroot + ".plot";
	string thepngfile = outroot + ".png";
	{
		string productName = wft_base_name(ncFile) + ":" +  dsName ;
		string title1 = productName.substr(0, productName.length() / 2);
		string title2 = productName.substr(productName.length() / 2);
		vector<string> varVector = { "{{{OUTPNGFILE}}}" , "{{{LONLATVALTXTFILE}}}" , "{{{TITLE1}}}" , "{{{TITLE2}}}" };
		vector<string> repVector = { thepngfile , llvFile  , title1 , title2 };
		wft_create_file_by_template_with_replacement(theplotfile, plotTemFile, varVector, repVector);
		if (wft_test_file_exists(theplotfile) == false) {
			wft_log(logDir, programid, theplotfile + " is failed to create. out.");
			return 124;
		}
	}

	//4.调用gnuplot 生成.pdt.png
	//5.检查png是否生成成功
	//5.5 不成功的话记录到日志，处理下一个，直至处理成功一个，然后退出
	{
		string cmd4 = gnuplot + " " ;
		cmd4 += theplotfile;
		int ret = std::system(cmd4.c_str());
		cout << "gnuplot return code :" << ret << endl;

		if (wft_test_file_exists(thepngfile) == false) {
			wft_log(logDir, programid, thepngfile + " is failed to create. out.");
			return 125;
		}
	}




	return 100;
}

