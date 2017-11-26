// f3noaandvionestep.cpp : 定义控制台应用程序的入口点。
//风3b与noaa产品植被指数质检一步计算程序
//wangfeng1@piesat.cn 2017-9-30

//FY3B_VIRRX_8090_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF
//AVHRR-Land_v004-preliminary_AVH13C1_NOAA-19_20170621_20170630_10d.tif 合成数据
//AVHRR-Land_v004-preliminary_AVH13C1_NOAA-19_20170626_c20170627095719.nc
//HDF5:"AVHRR-Land_v004-preliminary_AVH13C1_NOAA-19_20170630_c20170701100114.nc"://NDVI
#define UNICODE
#define _UNICODE
#include "tinydir.h"
#include <string>
#include <vector>
#include "gdal_priv.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <cassert>
#include "../../sharedcodes/wftools.h"
using namespace std;

/*
fy3中国范围
5060,5070,5080,5090,50A0,50B0
4060,4070,4080,4090,40A0,40B0
3060,3070,3080,3090,30A0,30B0
2060,2070,2080,2090,20A0,20B0

modis中国范围
h23v03,h24v03,h25v03,h26v03,h27v03,h28v03
h23v04,h24v04,h25v04,h26v04,h27v04,h28v04
h23v05,h24v05,h25v05,h26v05,h27v05,h28v05
h23v06,h24v06,h25v06,h26v06,h27v06,h28v06
h23v07,h24v07,h25v07,h26v07,h27v07,h28v07

*/




 

//二月28天                          1    2      3     4     5     6     7    8     9     10    11     12
std::string firstDayOfMon28[] = { "001","032","060","091","121","152","182","213","244","274","305","335" };
std::string endDayOfMon28[] = { "0131","0228","0331","0430","0531","0630","0731","0831","0930","1031","1130","1231" };
//二月29天
std::string firstDayOfMon29[] = {"001","032","061","092","122","153","183","214","245","275","306","336"};
std::string endDayOfMon29[] = { "0131","0229","0331","0430","0531","0630","0731","0831","0930","1031","1130","1231" };

std::vector<std::string> fy3TilesNameVector = {
	"5060","5070","5080","5090","50A0","50B0",
	"4060","4070","4080","4090","40A0","40B0",
	"3060","3070","3080","3090","30A0","30B0",
	"2060","2070","2080","2090","20A0","20B0"
};
std::vector<std::string> modTilesNameVector = {
	"h23v03","h24v03","h25v03","h26v03","h27v03","h28v03",
	"h23v04","h24v04","h25v04","h26v04","h27v04","h28v04",
	"h23v05","h24v05","h25v05","h26v05","h27v05","h28v05",
	"h23v06","h24v06","h25v06","h26v06","h27v06","h28v06",
	"h23v07","h24v07","h25v07","h26v07","h27v07","h28v07"
};

extern std::string lookingForModisFilenameByPrefix(std::string absDir, std::string prefix);



int main(int argc , char** argv )
{
	wft_print_current_time();
	std::cout << "processing..." << std::endl;
	if (argc == 1)
	{
		std::cout << "Sample call: f3noaandvionestep startup.dat" << std::endl;
		std::cout << "Error: no enough params. out." << std::endl;
		exit(101);
	}

	//2017-9-30 here 暂停在这里，先写fy3植被指数自动拼接程序

	std::string startupFile = argv[1];
	string fy3dir = wft_getValueFromExtraParamsFile( startupFile , "" , true ) ;




	int year = (int)atof(inputYearMonth.substr(0, 4).c_str());
	int month = (int)atof(inputYearMonth.substr(4).c_str());
	bool use29 = false;
	if (year % 4 == 0 && year % 100 != 0)
	{
		use29 = true;
	}
	if (year % 400 == 0) {
		use29 = true;
	}

	std::string fy3Date = inputYearMonth.substr(0,4) + endDayOfMon28[month-1] ;
	std::string modDate = inputYearMonth.substr(0, 4) + firstDayOfMon28[month - 1];
	if (use29)
	{
		fy3Date = inputYearMonth.substr(0, 4) + endDayOfMon29[month - 1];
		modDate = inputYearMonth.substr(0, 4) + firstDayOfMon29[month - 1];
	}
	std::cout << "fy3date modisdate:" << fy3Date << " " << modDate << std::endl;

	std::string fy3Dir = getValueFromExtraParamsFile(startupFile, "#fy3dir", true); 
	std::string modDir = getValueFromExtraParamsFile(startupFile, "#moddir", true);
	std::string outDir = getValueFromExtraParamsFile(startupFile, "#outdir", true);
	std::string computeLutProgram = getValueFromExtraParamsFile(startupFile, "#createlutprogram", true);
	std::string mosicProgram = getValueFromExtraParamsFile(startupFile, "#mosicprogram" , true);
	std::string vrtProgram = getValueFromExtraParamsFile(startupFile, "#createvrtfiles", true);
	std::string qcPrograme = getValueFromExtraParamsFile(startupFile, "#qcprogram", true);
	std::string plotTemplate = getValueFromExtraParamsFile(startupFile, "#plottemplate", true);

	std::vector<std::string> fy3filepathVector;
	std::vector<std::string> fy3DspathVector;
	std::cout << "checking if all tiles data file available..." << std::endl;
	for (size_t i = 0; i < fy3TilesNameVector.size(); ++i)
	{//FY3B_VIRRX_8090_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF
	 //HDF5:"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF":"//1KM_Monthly_NDVI"
		std::string tempfile = fy3Dir + "FY3B_VIRRX_" + fy3TilesNameVector[i] + "_L3_NVI_MLT_HAM_"+fy3Date+"_AOAM_1000M_MS.HDF";
		std::ifstream tempifs(tempfile.c_str());
		if (tempifs.good() == false)
		{
			std::cout << "Error : can not find file " << tempfile << " . out." << std::endl;
			exit(102);
		}
		tempifs.close();
		fy3filepathVector.push_back(tempfile);
		std::cout << "find fy3 tile:" << wft_base_name(tempfile) << std::endl;

		std::string dspath = std::string("HDF5:\"") + tempfile + "\"://1KM_Monthly_NDVI";
		fy3DspathVector.push_back(dspath);
	}

	std::vector<std::string> modfilepathVector;
	std::vector<std::string> modDspathVector;
	for (size_t i = 0; i < modTilesNameVector.size(); ++i)
	{//MOD13A3.A2017182.h20v07.006.2017234112120.hdf
	//"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h23v03.006.2017234112103.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",
		
		std::string modPrefix1 = std::string("MOD13A3.A") + modDate +"."+ modTilesNameVector[i] ;
		std::string modfilenameOnly = lookingForModisFilenameByPrefix(modDir, modPrefix1);
		if (modfilenameOnly.length() == 0)
		{
			std::cout << "Error : can not find modis file with prefix " << modPrefix1 << " . out." << std::endl;
			exit(102);
		}
		std::string tempfile = modDir + modfilenameOnly ;
		std::ifstream tempifs(tempfile.c_str());
		if (tempifs.good() == false)
		{
			std::cout << "Error : can not find file " << tempfile << " . out." << std::endl;
			exit(103);
		}
		tempifs.close();
		modfilepathVector.push_back(tempfile);
		std::cout << "find modis tile:" << wft_base_name(tempfile) << std::endl;

		std::string dspath = std::string("HDF4_EOS:EOS_GRID:\"") + tempfile + "\":\"MOD_Grid_monthly_1km_VI:1 km monthly NDVI\"";
		modDspathVector.push_back(dspath);
	}

	//output root path
	std::string outRoot = outDir + wft_base_name(fy3filepathVector[0]);

	//mosaic fy3
	std::cout << "mosaic fy3 ..." << std::endl;
	std::string outfy3MosicTifFile = outRoot + ".mosic.tif";
	std::string cmdlineMosicfy3 = mosicProgram + " 6 4 " + outfy3MosicTifFile;
	for (size_t i = 0; i < fy3DspathVector.size(); ++i)
	{
		cmdlineMosicfy3 += " " + fy3DspathVector[i];
	}
	int res1 = std::system(cmdlineMosicfy3.c_str());
	std::cout << "mosaic fy3 return code : " << res1 << std::endl;
	if (res1 != 0) return 71;
	

	//mosaic modis
	std::cout << "mosaic modis ..." << std::endl;
	std::string outmodMosicTifFile = outDir + wft_base_name(modfilepathVector[0]) + ".mosic.tif";
	std::string cmdlineMosicMod = mosicProgram + " 6 5 " + outmodMosicTifFile;
	for (size_t i = 0; i < modDspathVector.size(); ++i)
	{
		cmdlineMosicMod += " " + modDspathVector[i];
	}
	int res2 = std::system(cmdlineMosicMod.c_str());
	std::cout << "mosaic mod return code : " << res2 << std::endl;
	if (res2 != 0) return 72;

	//create lonlatlut for fy3
	std::cout << "create fy3 lonlatlut ..." << std::endl;
	std::string fy3lon = outRoot + ".fy3lon.tif";
	std::string fy3lat = outRoot + ".fy3lat.tif";
	std::string cmd3 = computeLutProgram + " -fy3b 5060 6 4 " + fy3lon + " " + fy3lat;
	for (size_t i = 0; i < fy3filepathVector.size(); ++i)
	{
		cmd3 += " " + fy3filepathVector[i];
	}
	int res3 = std::system(cmd3.c_str());
	std::cout << "create fy3 lonlatlut return code : " << res3 << std::endl;
	if (res3 != 0) return 73;

	//create lonlatlut for modis
	std::cout << "create modis lonlatlut ..." << std::endl;
	std::string modlon = outRoot + ".modlon.tif";
	std::string modlat = outRoot + ".modlat.tif";
	std::string cmd4 = computeLutProgram + "  -mod h23v03 6 5  " + modlon + " " + modlat;
	for (size_t i = 0; i < modfilepathVector.size(); ++i)
	{
		cmd4 += " " + modfilepathVector[i];
	}
	int res4 = std::system(cmd4.c_str());
	std::cout << "create modis lonlatlut return code : " << res4 << std::endl;
	if (res4 != 0) return 74;

	//create modis vrt for warping, this step is create a modvrtroot.warp.vrt
	std::string modvrtroot = outRoot + ".modvrt";
	std::cout << "create modis vrt files ..." << std::endl;
	std::string cmd5 = vrtProgram + " -in " + outmodMosicTifFile + " -type llfile "
		+ " -lonfile " + modlon + " -latfile " + modlat + " -temproot " + modvrtroot
		+ " -llnodata -999 -pdtnodata -3000 -pdtlut -2001:-3000,-2000:-2000,10000:10000 -outtype Float32 ";
	int res5 = std::system(cmd5.c_str());
	std::cout << "create modis vrt files return code : " << res5 << std::endl;
	if (res5 != 0) return 75;
	

	//warp modis wgs84
	std::cout << "warp modis to wgs84..." << std::endl;
	std::string modwarpvrt = modvrtroot + ".warp.vrt";
	std::string modwarpfile = modvrtroot + ".wgs84.tif";
	std::string cmd6 = std::string("gdalwarp -overwrite -of GTiff -tr 0.01 0.01 ") + modwarpvrt + " " + modwarpfile ;
	int res6 = std::system(cmd6.c_str());
	std::cout << "warp modis to wgs84 return code : " << res6 << std::endl;
	if (res6 != 0) return 76;

	//qc compute
	std::cout << "QC calculating..." << std::endl;
	std::string cmd7 = qcPrograme + " -type geotrans -inf "+ outfy3MosicTifFile + " -reff " + modwarpfile;
	cmd7 += " -inds " + outfy3MosicTifFile + " -refds " + modwarpfile;
	cmd7 += " -inlon " + fy3lon + " -inlat " + fy3lat;
	cmd7 += " -outroot " + outRoot;
	cmd7 += " -iv0 -10000 -iv1 10000 -rv0 -2000 -rv1 10000 -isca 0.0001 -iofs 0 -rsca 0.0001 -rofs 0 ";
	cmd7 += " -fill -99 -nomat -88 -pcode f3mndvi -coast null  -xbk x1 -xcv x2 -xdf x3 -xge x4 -xpd x5 -xpv x6 ";
	cmd7 += " -outxml " + outRoot+".server.xml" + " -plot " + plotTemplate;
	int res7 = std::system(cmd7.c_str());
	std::cout << "QC calculation return code : " << res7 << std::endl;
	if (res7 != 0) return 77;

	//done
	printCurrentTime();
	std::cout << " All done!" << std::endl;
	
    return 0;
}



std::string lookingForModisFilenameByPrefix(std::string absDir, std::string prefix)
{
	//#define UNICODE
	//#include "tinydir.h"
	//the two line above must be place before other include. In linux remove #define UNICODE staff.
	if (*absDir.rbegin() == '/') {
		absDir = absDir.substr(0, absDir.length() - 1);
	}
	tinydir_dir dir;

	//msvc
	size_t newsize = strlen(absDir.c_str()) + 1;
	wchar_t * wcstring = new wchar_t[newsize];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring, newsize, absDir.c_str(), _TRUNCATE);
	//msvc

	tinydir_open_sorted(&dir, (TCHAR*)wcstring);
	std::string resultFilenameOnly = "";
	for (int i = 0; i < dir.n_files; i++)
	{
		tinydir_file file;
		tinydir_readfile_n(&dir, &file, i);
		if (file.is_dir)
		{
		}
		else {

			//msvc
			size_t origsize = wcslen(file.name) + 1;
			size_t convertedChars = 0;
			char strConcat[] = " (char *)";
			size_t strConcatsize = (strlen(strConcat) + 1) * 2;
			const size_t newsize = origsize * 2;
			char *nstring = new char[newsize + strConcatsize];
			wcstombs_s(&convertedChars, nstring, newsize, file.name, _TRUNCATE);
			//msvc

			std::string tempfilename(nstring) ;
			std::string tempPrefix = tempfilename.substr(0, prefix.length());
			if (tempPrefix == prefix) {
				resultFilenameOnly = tempfilename;
				break;
			}
		}
	}
	tinydir_close(&dir);
	return resultFilenameOnly ;
}

