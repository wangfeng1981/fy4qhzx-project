// fy3ndvimonitorformosic.cpp : 定义控制台应用程序的入口点。
//监控某个文件夹下fy3b 植被指数数据，如果够拼接条件的话且输出文件夹没有拼接结果的话，就做拼接，并将结果保存到输出文件夹。
//wangfeng1@piesat.cn 2017-9-30
//
//v0.2a 2017-10-1 修复不能处理fy3月数据bug。

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../../sharedcodes/wftools.h"
using namespace std;




string get_dateymd_from_fy3file(string filepath)
{
	//FY3B_VIRRX_4080_L3_NVI_MLT_HAM_20170510_AOTD_1000M_MS.HDF
	string filename = wft_base_name(filepath);
	int pos0 = filename.find("FY3B_VIRRX_");
	int pos1 = filename.find("_NVI_MLT_HAM_");
	int pos2 = filename.find("_1000M_MS");
	int pos3 = filename.find(".HDF");
	if (pos0 != string::npos &&
		pos1 != string::npos && 
		pos2 != string::npos && 
		pos3 == filename.length()-4 )
	{
		int pos = pos1 + 13;
		string ymd = filename.substr(pos, 8);
		return ymd;
	}
	else {
		return std::string("");
	}
}



int main( int argc , char** argv )
{
	if (argc == 1)
	{
		//here 
		cout << "a program to monitor fy3 ndvi product dir and to make mosiac." << endl;
		cout << "Version 0.1a by wangfengdev@163.com 2017-9-30." << endl;
		cout << "Version 0.2a by wangfengdev@163.com 2017-10-1." << endl;
		cout << "sample call:" << endl;
		cout << "fy3ndvimonitorformosic starup.dat" << endl;
		cout << "out." << endl;
		exit(101);
	}


	string xtileArray[] = {
		"Z0" , "Y0" , "X0" , "W0" ,
		"V0" , "U0" , "T0" , "S0" , 
		"R0" , "Q0" , "P0" , "O0" , 
		"N0" , "M0" , "L0" , "K0" , 
		"J0" , "I0" , "00" , "10" , 
		"20" , "30" , "40" , "50" , 
		"60" , "70" , "80" , "90" , 
		"A0" , "B0" , "C0" , "D0" , 
		"E0" , "F0" , "G0" , "H0"
	};
	string ytileArray[] = {
		"80" , "70" , "60" , "50" ,
		"40" , "30" , "20" , "10" ,
		"00" , "90" , "A0" , "B0" ,
		"C0" , "D0" , "E0" , "F0" ,
		"G0" , "H0"
	};



	string startupFile = argv[1];
	string pid = "wmosaic";
	string mosicProgram , inDir , outDir ;
	mosicProgram = wft_getValueFromExtraParamsFile(startupFile, "#mosicprogram", true);
	inDir = wft_getValueFromExtraParamsFile(startupFile, "#indir", true);
	outDir = wft_getValueFromExtraParamsFile(startupFile, "#outdir", true); 
	string logDir = wft_getValueFromExtraParamsFile(startupFile, "#logdir", true);
	if (*inDir.rbegin() != '/') inDir += "/";
	if (*outDir.rbegin() != '/') outDir += "/";
	if (*logDir.rbegin() != '/') logDir += "/";

	string leftStr, rightStr, topStr, bottomStr , maxErrorStr ;
	int leftIndex(0), rightIndex(0), topIndex(0), bottomIndex(0);
	leftStr = wft_getValueFromExtraParamsFile(startupFile, "#lefttile", true);
	rightStr = wft_getValueFromExtraParamsFile(startupFile, "#righttile", true);
	topStr = wft_getValueFromExtraParamsFile(startupFile, "#toptile", true);
	bottomStr = wft_getValueFromExtraParamsFile(startupFile, "#bottomtile", true);
	leftIndex = wft_get_strindex_from_array(leftStr, xtileArray , 36  );
	rightIndex = wft_get_strindex_from_array(rightStr, xtileArray , 36 );
	topIndex = wft_get_strindex_from_array(topStr, ytileArray, 18);
	bottomIndex = wft_get_strindex_from_array(bottomStr, ytileArray, 18);
	maxErrorStr = wft_getValueFromExtraParamsFile(startupFile, "#maxerror", false );
	int maxError = fmax((int)atof(maxErrorStr.c_str()), 2);

	bool isDoWarp = false;
	string lutProgram, gdalWarp , vrtProgram , resMethod ;
	string doWarp = wft_getValueFromExtraParamsFile(startupFile, "#dowarp", false);
	if (doWarp == "true" )
	{
		isDoWarp = true;
		lutProgram = wft_getValueFromExtraParamsFile(startupFile, "#lonlatlutprogram", true);
		gdalWarp = wft_getValueFromExtraParamsFile(startupFile, "#gdalwarp", true);
		vrtProgram = wft_getValueFromExtraParamsFile(startupFile, "#vrtprogram", true);
		resMethod = wft_getValueFromExtraParamsFile(startupFile, "#resmethod", false );
		if (resMethod == "") resMethod = "near";
	}
	

	wft_log(logDir, pid, "begin");
	vector<string> filesVector;
	wft_get_allfiles(inDir, filesVector);

	int nfiles = filesVector.size();
	wft_log(logDir, pid, string("found file count:") + wft_int2str(nfiles) );
	int error = 0;
	for (int i = 0; i < nfiles; ++i)
	{
		string filepath1 = filesVector[i];
		string ymd = get_dateymd_from_fy3file(filepath1);
		if (ymd == "")
		{
			continue;
		}
		else {
			bool isok = true;
			string filename1 = wft_base_name(filepath1);
			int tilePos = filename1.find("VIRRX_") + 6 ;
			int pathEnd = filepath1.find(filename1);
			string inPath1 = filepath1.substr(0, pathEnd);
			string part0 = filename1.substr(0, tilePos);
			string part1 = filename1.substr(tilePos + 4);
			vector<string> mfilesVector;
			for (int ity = topIndex; ity <= bottomIndex; ++ity)
			{
				for (int itx = leftIndex; itx <= rightIndex; ++itx)
				{
					//FY3B_VIRRX_4080_L3_NVI_MLT_HAM_20170510_AOTD_1000M_MS.HDF
					string xtstr = xtileArray[itx];
					string ytstr = ytileArray[ity];
					string cfilename = part0 + ytstr  +   xtstr + part1;
					string cfilepath = inPath1 + cfilename;
					if (wft_test_file_exists(cfilepath))
					{
						mfilesVector.push_back(cfilepath);
					}
					else {
						isok = false;
						break;
					}
					
				}
				if (isok == false)
				{
					break;
				}
			}
			if (isok && mfilesVector.size()>0 ) {
				string outname = part0 + topStr  +leftStr  + "_" + bottomStr +  rightStr + part1 + ".tif";
				string outpath = outDir + outname;
				if (wft_test_file_exists(outpath) == false)
				{
					string flistFilepath = outpath + ".flist";
					ofstream ofs(flistFilepath.c_str());
					for (size_t im = 0; im < mfilesVector.size(); ++im)
					{
						ofs << mfilesVector[im] << endl;
					}
					ofs.close();
					int ntx = rightIndex - leftIndex + 1;
					int nty = bottomIndex - topIndex + 1;
					string cmd = mosicProgram;
					//月数据集
					//HDF5 : "FY3B_VIRRX_80Q0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF" ://1KM_Monthly_NDVI
					//十天数据集
					//HDF5 : "FY3B_VIRRX_4080_L3_NVI_MLT_HAM_20170510_AOTD_1000M_MS.HDF" ://1KM_10day_NDVI
					int posday10 = part1.find("AOTD");
					int posmonth = part1.find("AOAM");
					string prefix = "HDF5:\\\"";
					string tail = "";
					if (posday10 != string::npos)
					{
						tail = "\\\"://1KM_10day_NDVI";
					}
					else if (posmonth != string::npos)
					{
						tail = "\\\"://1KM_Monthly_NDVI";
					}
					else
					{
						cout << "Error : find a unstandard fy3 ndvi file :" << filename1 << endl;
						continue;
					}
					cmd = cmd + " -ntx " + wft_int2str(ntx) + " -nty " + wft_int2str(nty);
					cmd += " -out " + outpath + " -inprefix \"" + prefix + "\"" + " -intail \"" + tail + "\"";
					cmd += " -infilestxt " + flistFilepath;
					wft_log(logDir, pid, cmd);
					//拼接
					int ret = system(cmd.c_str());
					cout << "wmosaic return code : " << ret << endl;
					wft_log(logDir, pid, string("return code:") + wft_int2str(ret));
					if (ret != 0)
					{
						error++;
						if (error >= maxError) {
							wft_log(logDir, pid, string("reach max error.out.") );
							break;
						}
					}
					else
					{
						//生成经纬度查找表
						if( isDoWarp ){
							//create lonlatlut for fy3
							std::cout << "create fy3 lonlatlut ..." << std::endl;
							std::string fy3lon = outpath + ".fy3lon.tif";
							std::string fy3lat = outpath + ".fy3lat.tif";

							std::string cmd3 = lutProgram + " -type fy3b -lefttoptilename " + topStr + leftStr + " -ntx " + wft_int2str(ntx)
								+ " -nty " + wft_int2str(nty) + " -outlon " + fy3lon + " -outlat " + fy3lat
								+ " -inprefix HDF5:\\\" -intail " + tail;
							cmd3 = cmd3 + " -infilestxt " + flistFilepath;

							int res3 = std::system(cmd3.c_str());
							std::cout << "create fy3 lonlatlut return code : " << res3 << std::endl;
							if (res3 != 0) exit(104);

							//create vrt here 
							std::string vrtfile = outpath + ".warp.vrt";
							std::string cmd4 = vrtProgram + " -type llfile  -in " + outpath;
							cmd4 = cmd4 + " -lonfile " + fy3lon + " -latfile " + fy3lat;
							cmd4 = cmd4 + " -outvrt " + vrtfile + " -llnodata -999 ";
							cmd4 = cmd4 + " -pdtnodata -32768 -pdtlut -32768:-32768,-10001:-32768,-10000:-10000,10000:10000 ";
							int res4 = std::system(cmd4.c_str());
							std::cout << "create vrt result code :" << res4 << endl;
							if (res4 != 0) exit(105);

							//gdalwarp
							std::string wgs84file = outpath + ".wgs84.tif";
							std::string cmd5 = gdalWarp + " -geoloc -t_srs \"+proj=longlat +ellps=WGS84\" -tr 0.01 0.01 -r "+resMethod+" -overwrite " 
								+ vrtfile + " "
								+ wgs84file;
							int res5 = std::system(cmd5.c_str());
							std::cout << "gdalwarp vrt result code :" << res5 << endl;
							if (res4 != 0) exit(105);

						}

						break;
					}
				}

			}
		}
	}

	wft_log(logDir, pid, "done." );
    return 0;
}

