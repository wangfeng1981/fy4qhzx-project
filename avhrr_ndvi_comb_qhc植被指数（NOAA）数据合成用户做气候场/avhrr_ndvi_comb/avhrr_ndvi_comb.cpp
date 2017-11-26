// avhrr_ndvi_comb.cpp : 定义控制台应用程序的入口点。
//针对NOAA AVHRR NDVI日数据产品，按照旬、月、季（12-2、3-5、6-8、9-11）、年进行组合。组合算法包括平均、最小值、最大值。
//候与旬的开始日期为每个月的第一天，每个月的第31天不计入统计，2月份最后一候或者旬有几天统计几天。
//wangfeng1@piesat.cn 2017-9-25.
//0.1a 2017-9-30 生成合成数据的时候，需要判断是否输出目录已经有了合成数据，如果有了就不要重复处理。
//0.2a 2017-9-30 输入目录与输出目录在同一个目录时，可以只针对avhrr原始数据进行整合，排除合成数据再次进行计算。
//0.3a 2017-10-9 使用datacombinationfornoaandvi进行数据合成，包含了对qa的判断。
//0.4a 2017-10-9 改程序用于制作气候场数据。


#define UNICODE
#include "tinydir.h"
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cmath>


struct WYmd {
	int year, month, day;
	bool isleapyear;
};

void printCurrentTime() {
	//#include <ctime>
	time_t time0;
	time(&time0);
	std::cout << asctime(localtime(&time0)) << std::endl;
}



//通过标签获取命令行参数
bool wft_has_param(int argc, char** argv, char* key, std::string& value, bool mustWithValue)
{
	value = "";
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], key) == 0) {
			if (i != argc - 1)
			{
				value = std::string(argv[i + 1]);
				return true;
			}
			else {
				if (mustWithValue) {
					std::cout << "Error: can not find value for key :" << key << ". out." << std::endl;
					exit(99);
				}
				return true;
			}
		}
	}
	if (mustWithValue) {
		std::cout << "Error: can not find value for key :" << key << ". out." << std::endl;
		exit(99);
	}
	return false;
}

std::string converTCHARStringToStdString(TCHAR* tstring)
{
	size_t origsize = wcslen(tstring) + 1;
	size_t convertedChars = 0;
	char strConcat[] = " (char *)";
	size_t strConcatsize = (strlen(strConcat) + 1) * 2;
	const size_t newsize = origsize * 2;
	char *nstring = new char[newsize + strConcatsize];
	wcstombs_s(&convertedChars, nstring, newsize, tstring, _TRUNCATE);
	std::string out = std::string(nstring);
	delete[] nstring;
	return out;
}


//获取文件夹中全部文件
void wft_get_allfiles( std::string absDir ,std::vector<std::string>& filesVector )
{
	std::vector<std::string> outVector;
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
			std::string tname = converTCHARStringToStdString(file.name);
			if ( tname == "." || tname==".." )
			{
				//donot process.
			}
			else {
				std::string subDir = converTCHARStringToStdString(file.path);
				wft_get_allfiles(subDir, filesVector);
			}
		}
		else {
			std::string tempfilepath = converTCHARStringToStdString( file.path ) ;
			filesVector.push_back(tempfilepath);
		}
	}
	tinydir_close(&dir);
	delete[] wcstring;
}

std::string getYmdFromAvhrrFilename(std::string filename)
{
	//AVHRR-Land_v004-preliminary_AVH13C1_NOAA-19_20161231_c20170106124028.nc
	//bug 2017-10-9 size_t posAvhrr = filename.find_last_of("AVHRR-Land");
	//bug 2017-10-9 size_t pos1 = filename.find_last_of("_");
	size_t posAvhrr = filename.rfind("AVHRR-Land");//bug fixed
	size_t pos1 = filename.rfind("_");//bug fixed
	size_t posavh = filename.rfind("AVH13C1");
	size_t posnc = filename.rfind(".nc");

	if ( posnc == filename.length()-3 && 
		pos1 != std::string::npos && 
		posAvhrr != std::string::npos &&
		posavh != std::string::npos
		&& pos1 > posavh  && posavh > posAvhrr )
	{
		size_t pos0 = pos1 - 8;
		std::string date0 = filename.substr(pos0, 8);
		return date0;
	}
	return "";
}



//查找avhrr数据
std::string lookingForAvhrrFilenameByDate8(std::vector<std::string>& filesVector ,  std::string dateYmd  )
{
	for (size_t i = 0; i < filesVector.size(); ++i)
	{
		// 	AVHRR-Land_v004_AVH13C1_NOAA-14_19950101_c20130923101030.nc
		size_t pos0 = filesVector[i].find("AVHRR-Land");
		size_t pos1 = filesVector[i].find("_AVH13C1_NOAA");
		size_t pos2 = filesVector[i].find(".nc");
		//bugfix 2017-9-30 only use avhrr orignal data file.
		if (pos0 != std::string::npos && pos1 != std::string::npos && pos2 == filesVector[i].length() - 3)
		{
			std::string ymd1 = getYmdFromAvhrrFilename(filesVector[i]);
			if (ymd1 == dateYmd) return filesVector[i];
		}
	}
	return "";
}

//查找avhrr数据2
std::string lookingForAvhrrFilenameByDate8v2(std::vector<std::string>& filesVector,std::vector<int>& ymdVector,int dateYmd)
{
	for (size_t i = 0; i < ymdVector.size(); ++i)
	{
		if (ymdVector[i] == dateYmd)
		{
			return filesVector[i];
		}
	}
	return "";
}






//从路径中截取文件名
std::string wft_base_name(std::string const & path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}

//是否闰年
bool wft_is_leapyear(int year)
{
	if (year % 400 == 0)
		return true;
	else if (year % 100 == 0)
		return false;
	else if (year % 4 == 0)
		return true;
	else
		return false;
}

//获取AVHRR产品年份
//AVHRR-Land_v004-preliminary_AVH13C1_NOAA-19_20170919_c20170920095925.nc
WYmd dateOfAvhrrFilename(std::string filename)
{
	WYmd ymd;
	ymd.year = -1;
	std::string ymdstr = getYmdFromAvhrrFilename(filename);
	if (ymdstr.length() == 8)
	{
		ymd.year = atof(ymdstr.substr(0, 4).c_str());
		ymd.month = atof(ymdstr.substr(4, 2).c_str());
		ymd.day = atof(ymdstr.substr(6, 2).c_str());
		ymd.isleapyear = wft_is_leapyear(ymd.year);
	}
	return ymd;
}


//判断文件是否存在
bool wft_test_file_exists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}


std::string makeAvhrrCombinationFilename(std::string filename0, std::string filename1 , std::string tail)
{
	filename0 = wft_base_name(filename0);
	filename1 = wft_base_name(filename1);
	size_t pos1 = filename0.rfind("_");
	size_t pos0 = pos1 - 8;

	std::string prefix = filename0.substr(0, pos0);
	std::string date0 = getYmdFromAvhrrFilename(filename0);
	std::string date1 = getYmdFromAvhrrFilename(filename1);
	std::string outfname = prefix + date0 + "_" + date1 + tail;
	return outfname;
}

extern void processAllSeasonQhc(std::vector<std::string>& filesVector, std::string outDir );
extern void processAllYearQhc(std::vector<std::string>& filesVector, std::string outDir );
extern void processAllMonthQhc(std::vector<std::string>& filesVector, std::string outDir  );
extern void processAllXunQhc(std::vector<std::string>& filesVector, std::string outDir);
extern void processAllDayQhc(std::vector<std::string>& filesVector, std::string outDir);

//命名规则 10d mon sea yea

int g_mon0(-1), g_mon1(-1), g_day0(-1), g_day1(-1);
bool g_onlyFileList = false;
std::string g_useQA="1";
int main(int argc, char** argv)
{
	if (argc == 1)
	{
		std::cout << "A program to generate bat file for avhrr combination." << std::endl;
		std::cout << "Version 0.2a " << std::endl;
		std::cout << " by wangfengdev@163.com 2017-9-25." << std::endl;
		std::cout << " v0.3a by wangfengdev@163.com 2017-10-9. use datacombinationfornoaandvi include QA filter." << std::endl;
		std::cout << " v0.4a by wangfengdev@163.com 2017-10-9. only for QiHouChang." << std::endl;
		std::cout << " v0.5a by wangfengdev@163.com 2017-10-10. can pick month for working." << std::endl;
		std::cout << " v0.6a by wangfengdev@163.com 2017-10-11. add year season month xun. remove month pick." << std::endl;
		std::cout << " v0.7a by wangfengdev@163.com 2017-10-24. add day QHC." << std::endl;
		std::cout << " v0.7.1a by wangfengdev@163.com 2017-10-25. user can pick month range and day range for daily qhc." << std::endl;
		std::cout << " v0.7.2a by wangfengdev@163.com 2017-10-26. can only out filelist.txt , change out filename 1981_2010." << std::endl;
		std::cout << " v0.7.3a by wangfengdev@163.com 2017-10-26. use QA or not." << std::endl;
		std::cout << "Sample call:" << std::endl;
		std::cout << "  avhrr_ndvi_comb_qhc  -indir c:/datas/ -outdir c:/out/ -type day/d10/mon/yea/sea [-mon0 2 -mon1 2 -day0 12 -day1 12] "
			" [-onlyfilelist 0/1]"
			" [-qa 0/1] "
			<< std::endl;
		exit(101);
	}
	
	std::string inDir,outDir,  type ;
	wft_has_param(argc, argv, "-indir", inDir, true);
	wft_has_param(argc, argv, "-outdir", outDir, true);
	wft_has_param(argc, argv, "-type", type, true);

	std::string tempstr1;
	if (wft_has_param(argc, argv, "-mon0", tempstr1, false))
	{
		g_mon0 = (int)atof(tempstr1.c_str());
	}
	if (wft_has_param(argc, argv, "-mon1", tempstr1, false))
	{
		g_mon1 = (int)atof(tempstr1.c_str());
	}
	if (wft_has_param(argc, argv, "-day0", tempstr1, false))
	{
		g_day0 = (int)atof(tempstr1.c_str());
	}
	if (wft_has_param(argc, argv, "-day1", tempstr1, false))
	{
		g_day1 = (int)atof(tempstr1.c_str());
	}

	//2017-10-26
	std::string onlyfilelistStr;
	if (wft_has_param(argc, argv, "-onlyfilelist", onlyfilelistStr, false))
	{
		if (onlyfilelistStr != "0") g_onlyFileList = true;
	}
	std::string useQA;
	if (wft_has_param(argc, argv, "-qa", useQA, false))
	{
		g_useQA = useQA;
	}
	else {
		g_useQA = "1";
	}

	std::vector<std::string> allFiles;
	wft_get_allfiles(inDir , allFiles );
	if (allFiles.size() == 0)
	{
		std::cout << "No files under "<<inDir<<" . out." << std::endl;
		exit(102);
	}

	printCurrentTime();
	std::cout << "processing... " << std::endl;

	//processAllSeason(allFiles, outDir, method, minYear, maxYear);
	//processAllYear(allFiles, outDir, method, minYear, maxYear);
	if (type == "day")
	{
		processAllDayQhc(allFiles, outDir);
	}
	else if (type == "d10")
	{
		processAllXunQhc(allFiles, outDir);
	}
	else if (type == "mon")
	{
		processAllMonthQhc(allFiles, outDir  );
	}
	else if (type == "sea")
	{
		processAllSeasonQhc(allFiles, outDir);
	}
	else if (type == "yea")
	{
		processAllYearQhc(allFiles, outDir);
	}
	else
	{
		std::cout << "Error: invalid type " << type << std::endl;
	}

	printCurrentTime();
	std::cout << "done." << std::endl;

	return 0;
}








void combinationQhc(std::vector<std::string> fileVector, std::string outDir , std::string outfilenameOnly   )
{

	if (fileVector.size() == 0) return;

	if (*outDir.rbegin() != '/')
	{
		outDir += "/";
	}

	std::string fname0 = fileVector[0];
	std::string fname1 = *fileVector.rbegin();

	//std::string outFilename = outDir + makeAvhrrCombinationFilename(fname0, fname1, tailname );//"_monqhc.tif"
	std::string outFilename = outDir + outfilenameOnly;
	if (wft_test_file_exists(outFilename) == false)
	{//结果文件不存在，继续运算
		std::string temptxt = outFilename + ".txt";
		std::ofstream txtfs(temptxt.c_str());
		for (size_t iday = 0; iday < fileVector.size(); ++iday)
		{
			txtfs << fileVector[iday] << std::endl;
		}
		txtfs.close();
		std::string cbMethod = "aver";
		std::string cmd = "datacombinationfornoaandvi ";
		cmd += std::string(" -method ") + cbMethod + " -vmin -1000 -vmax 10000 ";
		cmd += std::string(" -out ") + outFilename;
		cmd += std::string(" -infilestxt ") + temptxt;
		cmd += std::string(" -qa ") + g_useQA;

		if (g_onlyFileList)
		{
			std::cout << "only out filelist ok." << std::endl;
		}
		else
		{
			int ret = 0;
			ret = std::system(cmd.c_str());
			std::cout << "datacombinationfornoaandvi call return code:" << ret << std::endl;
		}
		
	}
	else {
		std::cout << outFilename << " file exist , no need to process." << std::endl;
	}


}



std::vector<int> getYmdVec(std::vector<std::string>& filesVector)
{
	std::vector<int> ymdVector;
	for (size_t i = 0; i < filesVector.size(); ++i)
	{
		size_t pos0 = filesVector[i].find("AVHRR-Land");
		size_t pos1 = filesVector[i].find("_AVH13C1_NOAA");
		size_t pos2 = filesVector[i].find(".nc");
		//bugfix 2017-9-30 only use avhrr orignal data file.
		if (pos0 != std::string::npos && pos1 != std::string::npos && pos2 == filesVector[i].length() - 3)
		{
			std::string ymd1 = getYmdFromAvhrrFilename(filesVector[i]);
			int ymdi = atof(ymd1.c_str());
			ymdVector.push_back(ymdi);
		}
		else {
			ymdVector.push_back(-1);
		}
	}
	return ymdVector;
}


void processAllDayQhc(std::vector<std::string>& filesVector, std::string outDir)
{
	std::vector<int> ymdVector = getYmdVec(filesVector);
	std::vector<std::string> dayVectorArray[372]; //366  31*12=372
	for (int year = 1981; year <= 2010; ++year)
	{
		for (int imon = 1; imon <= 12; ++imon)
		{
			for (int iday = 1; iday <= 31; ++iday)
			{
				if (g_mon0 > 0 && g_mon1 > 0 && g_day0 > 0 && g_day1 > 0)
				{
					if (imon < g_mon0 || imon > g_mon1 || iday < g_day0 || iday > g_day1)
					{
						continue;
					}
				}
				int cymd = year * 10000 + imon * 100 + iday ;
				std::cout << cymd << "\t";
				std::string filepath = lookingForAvhrrFilenameByDate8v2(filesVector, ymdVector, cymd);

				if (filepath.length() > 1) {
					int dayindex = (imon - 1) * 31 + (iday-1);
					dayVectorArray[dayindex].push_back(filepath);
				}
			}//end of day

		}
	}

	for (int imon = 0; imon < 12; ++imon )
	{
		for (int iday = 0; iday < 31; ++iday)
		{
			int dayindex = imon * 31 + iday;
			if (dayVectorArray[dayindex].size() > 0)
			{
				char buff[100];
				sprintf(buff, "qhc_day_1981_2010%02d%02d.tif", imon+1 , iday + 1);
				combinationQhc(dayVectorArray[dayindex], outDir, buff);
			}
		}
	}

}



void processAllXunQhc (std::vector<std::string>& filesVector ,std::string outDir  )
{
	std::vector<int> ymdVector = getYmdVec(filesVector);
	std::vector<std::string> xunVectorArray[36]; //12*3
	for (int year = 1981; year <= 2010; ++year)
	{
		for (int imon = 1; imon <=12 ; ++imon)
		{
			for (int iday = 1; iday <=31; ++iday)
			{
				int cymd = year * 10000 + imon * 100 + iday;
				std::cout << cymd << "\t";
				std::string filepath = lookingForAvhrrFilenameByDate8v2(filesVector, ymdVector, cymd);
				
				if (filepath.length() > 1) {
					int xunindex = (imon - 1) * 3;
					if (iday <= 10) {
						xunindex += 0;
					}
					else if( iday<=20 ) {
						xunindex += 1;
					}
					else {
						xunindex += 2;
					}
					xunVectorArray[xunindex].push_back(filepath);
				}
			}//end of day
			
		}
	}

	for (int ixun = 0; ixun < 36; ++ixun)
	{
		if (xunVectorArray[ixun].size() > 0)
		{
			int imon = ixun / 3 + 1;
			int iday = ixun % 3 + 1;
			char buff[100];
			sprintf(buff, "qhc_xun_1981_2010%02d_%02d%02d.tif", ixun+1 , imon , iday );
			combinationQhc(xunVectorArray[ixun], outDir , buff );
		}
	}
	
}



void processAllMonthQhc(std::vector<std::string>& filesVector, std::string outDir   )
{//用于计算气候场，从1981年至2010年，每个月份一个气候场结果，气候场计算方式采用1981到2010年同一个月份所有天数数值的平均值计算得到。
	std::vector<int> ymdVector = getYmdVec(filesVector) ;

	std::vector<std::string> monthVector[12];
	for (int year = 1981; year <= 2010; ++year)
	{
		//for (int imon = 1; imon <= 12; ++imon)
		for (int imon = 1; imon <= 12; ++imon)
		{
			for (int iday = 1; iday <= 31 ; ++iday)
			{
				int cymd = year * 10000 + imon * 100 + iday;
				std::cout << cymd << "\t";
				std::string filepath = lookingForAvhrrFilenameByDate8v2(filesVector ,ymdVector , cymd);
				if (filepath.length() > 1) {
					monthVector[imon-1].push_back(filepath); 
				}
			}//end of day
		}
	}
	for (int imon = 0; imon < 12; ++imon)
	{
		if (monthVector[imon].size()>0)
		{
			char buff[100];
			sprintf(buff, "qhc_month_1981_2010%02d.tif", imon + 1);
			combinationQhc(monthVector[imon], outDir, buff );
		}
	}
}


void processAllYearQhc(std::vector<std::string>& filesVector, std::string outDir)
{//每一年份的所有数据都在一个文件夹中

	std::vector<int> ymdVector = getYmdVec(filesVector);
	std::vector<std::string> yearVec ;
	for (int year = 1981; year <= 2010; ++year)
	{
		for (int imon = 1; imon <= 12; ++imon)
		{
			for (int iday = 1; iday <= 31; ++iday)
			{
				int cymd = year * 10000 + imon * 100 + iday;
				std::cout << cymd << "\t";
				std::string filepath = lookingForAvhrrFilenameByDate8v2(filesVector, ymdVector, cymd);

				if (filepath.length() > 1) {
					yearVec.push_back(filepath);
				}
			}//end of day
		}
	}
	if (yearVec.size() > 0)
	{
		combinationQhc(yearVec, outDir , "qhc_year_1981_2010.tif");
	}
	
}



void processAllSeasonQhc(std::vector<std::string>& filesVector, std::string outDir )
{
	std::vector<int> ymdVector = getYmdVec(filesVector);
	std::vector<std::string> seasonVecArray[4] ;
	for (int year = 1981; year <= 2010; ++year)
	{
		for (int imon = 1; imon <= 12; ++imon)
		{
			for (int iday = 1; iday <= 31; ++iday)
			{
				int cymd = year * 10000 + imon * 100 + iday;
				std::cout << cymd << "\t";
				std::string filepath = lookingForAvhrrFilenameByDate8v2(filesVector, ymdVector, cymd);

				if (filepath.length() > 1) {
					int isea = 0;
					if (imon == 1 || imon == 2 || imon == 12)
					{
						isea = 0;
					}
					else if (imon <= 5)
					{
						isea = 1;
					}
					else if (imon <= 8)
					{
						isea = 2;
					}
					else {
						isea = 3;
					}
					seasonVecArray[isea].push_back(filepath);
				}
			}//end of day
		}
	}

	for (int isea = 0; isea < 4; ++isea)
	{
		if (seasonVecArray[isea].size() > 0)
		{
			std::string outname = "";
			if (isea == 0)
			{
				outname = "qhc_season_wint.tif";
			}
			else if (isea == 1)
			{
				outname = "qhc_season_spri.tif";
			}
			else if (isea == 2)
			{
				outname = "qhc_season_summ.tif";
			}
			else
			{
				outname = "qhc_season_fall.tif";
			}
			combinationQhc(seasonVecArray[isea], outDir, outname);
		}
	}
	
}


