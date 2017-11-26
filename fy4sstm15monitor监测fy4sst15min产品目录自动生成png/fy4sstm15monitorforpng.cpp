// fy4sstm15monitor.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//¸Ä³ÌÐò²»¸ºÔð¼ì²éÊÇ·ñÈë¿âºÍÈë¿â²Ù×÷£¬Ö»¸ºÔðÉú³Épng
//¼à¿Øfy4 sst 15minÊý¾Ý²úÆ·£¬¶¨Ê±µ÷ÓÃ¸Ã³ÌÐò£¬É¨ÃèÄ¿Â¼£º
//1.¼ì²é¸ÃÎÄ¼þÊÇ·ñÒÑ¾­Éú³Épng£¿
//2.Î´Éú³ÉÔòÉú³Élonlatval.txt
//3.¸ù¾ÝÄ£°åÉú³Épdt.plot½Å±¾ÎÄ¼þ
//4.µ÷ÓÃgnuplot Éú³É.pdt.png
//5.¼ì²épngÊÇ·ñÉú³É³É¹¦
//6.³É¹¦µÄ»°ÍË³ö³ÌÐò
//7.²»³É¹¦µÄ»°¼ÇÂ¼µ½ÈÕÖ¾£¬´¦ÀíÏÂÒ»¸ö£¬Ö±ÖÁ´¦Àí³É¹¦Ò»¸ö£¬È»ºóÍË³ö
// wangfeng1@piesat.cn 2017-9-27

//Win #define UNICODE
//Win #include "../../sharedcodes/tinydir.h"
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include "../sharedcodes/wftools.h"
using namespace std;

extern int processOneFile(string undbFile, string fy4totxtProgram, string fy4lonFile, string fy4latFile, 
	string logDir, string programid,string	plotTemFile , string vrtProgram );

int main( int argc  , char** argv )
{
	if (argc == 1)
	{
		cout << "V0.2a Fy4 sst 15min product monitor." << endl;
		cout << "by wangfengdev@163.com 2017-9-27." << endl;
		cout << "Sample call:" << endl;
		cout << " fy4sstm15monitor startup.params " << endl;
		cout << "No enought parameters. out." << endl;
		exit(101);
	}
	const std::string programid = "fy4sstm15monitorforpng" ;
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
	int maxError = 2;
	{//×î´ó´íÎóÊýÁ¿£¬´ïµ½ºóÍË³ö
		std::string tstr = wft_getValueFromExtraParamsFile(startupFile, "#maxerror", false);
		if (tstr != "") maxError = max(1, (int)atof(tstr.c_str()) );
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

	wft_log(logDir,programid , "begin");

	//FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_20170815000000_20170815001459_4000M_V0001.NC
	//1.¼ì²é¸ÃÎÄ¼þÊÇ·ñÒÑ¾­Éú³Épng£¿ÒÑÓÐpngÌø¹ý¡£ÕÒµ½Ò»¸öÎ´×öpngµÄÎÄ¼þ
	string undbFile = "";
	vector<string> dirFiles;
	wft_linux_get_dir_files(inDir, dirFiles);
	int errorCount = 0;
	for (size_t i = 0; i < dirFiles.size(); ++i)
	{
		string tempfile = dirFiles[i];
		size_t pos0 = tempfile.rfind("FY4A-_AGRI--_N_DISK");
		size_t pos1 = tempfile.rfind("L2-_SST-_MULT_NOM");
		size_t pos2 = tempfile.rfind(".NC");
		if (pos0 != string::npos && pos1 != string::npos && pos2 != string::npos && pos0 < pos1 && pos1 < pos2
			&& pos2 == tempfile.length() - 3 )
		{//ÓÐÐ§µÄ·çËÄsstÊý¾Ý
			bool noPng = true ;
			string thepngfile = tempfile + ".png";
			if (wft_test_file_exists(thepngfile)) {
				noPng = false;
			}
			if (noPng)
			{
				undbFile = tempfile;
				int ret = processOneFile(undbFile, fy4totxtProgram, fy4lonFile, fy4latFile, logDir, programid, plotTemFile , vrtProgram );
				if (ret == 100) {
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

	wft_log(logDir, programid , "done");
	cout << "done." << endl;
    return 0;
}


int processOneFile(string undbFile , string fy4totxtProgram , string fy4lonFile , string fy4latFile ,
	string logDir , string programid , string plotTemFile ,
	string vrtProgram )
{
	//2.Î´Èë¿â£¬Éú³Élonlatval.txt
	string llvFile = undbFile + ".llv.txt";
	{
		//HDF5:"E:/testdata/fy4sst15min/FY4A-_AGRI--_N_DISK_1047E_L2-_SST-
		//_MULT_NOM_20170815000000_20170815001459_4000M_V0001.NC"://SST
		//2.1 create vrt
		string vrtFile = undbFile + ".warp.vrt";
		string wgs84tif = undbFile + ".wgs84.tif";
		string cmd21=vrtProgram +		
			" -type llfile "
			+ " -in HDF5:\"" + undbFile +  "\"://SST "
			+ " -lonfile "+fy4lonFile 
			+ " -latfile " + fy4latFile 
			+ " -pdtnodata 65535 "
			+ " -pdtlut -6:65536,-5:-5,45:45,46:65535 "
			+ " -outvrt " + vrtFile ;
		int ret21 = std::system(cmd21.c_str());
		cout << "createvrtprogram return code :" << ret21 << endl;
		if (wft_test_file_exists(vrtFile) == false) {
			wft_log(logDir, programid, llvFile + " is failed to create. out.");
			return 121;
		}

		string cmd22 = "gdalwarp -geoloc -t_srs \"+proj=longlat +ellps=WGS84\" -r bilinear -tr 0.05 0.05 -overwrite ";
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
			+ " -valid0 -5 -valid1 45 -xspace 2 -yspace 2";
		int ret = std::system(cmd2.c_str());
		cout << "fy4totxtProgram return code :" << ret << endl;

		if (wft_test_file_exists(llvFile) == false) {
			wft_log(logDir, programid, llvFile + " is failed to create. out.");
			return 123;
		}
	}

	//3.¸ù¾ÝÄ£°åÉú³É.plot½Å±¾ÎÄ¼þ #OUTPNGFILE,LONLATVALTXTFILE,TITLE1,TITLE
	string theplotfile = undbFile + ".plot";
	string thepngfile = undbFile + ".png";
	{
		string productName = wft_base_name(undbFile);
		string title1 = productName.substr(0, productName.length() / 2);
		string title2 = productName.substr(productName.length() / 2);
		vector<string> varVector ;
		varVector.push_back("{{{OUTPNGFILE}}}") ;
		varVector.push_back("{{{LONLATVALTXTFILE}}}") ;
		varVector.push_back("{{{TITLE1}}}") ;
		varVector.push_back("{{{TITLE2}}}") ;

		vector<string> repVector ;
		repVector.push_back(thepngfile) ;
		repVector.push_back(llvFile) ;
		repVector.push_back(title1) ;
		repVector.push_back(title2) ;

		wft_create_file_by_template_with_replacement(theplotfile, plotTemFile, varVector, repVector);
		if (wft_test_file_exists(theplotfile) == false) {
			wft_log(logDir, programid, theplotfile + " is failed to create. out.");
			return 124;
		}
	}

	//4.µ÷ÓÃgnuplot Éú³É.pdt.png
	//5.¼ì²épngÊÇ·ñÉú³É³É¹¦
	//5.5 ²»³É¹¦µÄ»°¼ÇÂ¼µ½ÈÕÖ¾£¬´¦ÀíÏÂÒ»¸ö£¬Ö±ÖÁ´¦Àí³É¹¦Ò»¸ö£¬È»ºóÍË³ö
	{
		string cmd4 = "gnuplot ";
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

