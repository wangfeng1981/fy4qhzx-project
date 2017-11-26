#ifndef WFTOOLS_H
#define WFTOOLS_H

//
#include <vector>
#include <string>
#include <ctime>
#include <sstream>
#include <iostream>
#include "tinydir.h"




//从字符串数组中找到索引
int wft_get_strindex_from_array(std::string find, std::string array[] , int size)
{
	for (int i = 0; i < size; ++i)
	{
		if (find == array[i])
		{
			return i;
		}
	}
	return -1;
}

//整形转字符串
std::string wft_int2str(int val)
{
	std::stringstream ss;
	ss << val;
	return ss.str();
}
//float形转字符串
std::string wft_float2str(float val)
{
	std::stringstream ss;
	ss << val;
	return ss.str();
}
//double形转字符串
std::string wft_double2str(double val)
{
	std::stringstream ss;
	ss << val;
	return ss.str();
}


//替换字符串
std::string wft_replaceString(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

//获取当前时间 
std::string wft_get_current_time() {
	//#include <ctime>
	time_t time0;
	time(&time0);
	return std::string(asctime(localtime(&time0)));
}

//打印当前时间
void wft_print_current_time() {
	std::cout << wft_get_current_time() << std::endl;
}

//从配置文件获取参数
//read extra params file by key. note the key should like the same as text file write do not use space.
std::string wft_getValueFromExtraParamsFile(std::string extrafile, std::string key , bool musthas=false )
{
	std::string res = "";
	bool find = false;
	std::ifstream  ifs(extrafile.c_str());
	while (getline(ifs, res)) {
		if (res.length() > 0)//bugfixed 2017-10-25 for empty line.
		{
			if ((int)res[res.length() - 1] == 0)
			{
				res = res.substr(0, res.length() - 1);
			}
			if (res == key) {
				getline(ifs, res);
				if (res.length() > 0)//bugfixed 2017-11-10 for empty line.
				{
					if ((int)res[res.length() - 1] == 0)
					{
						res = res.substr(0, res.length() - 1);
					}
				}
				find = true;
				break;
			}
		}
	}
	ifs.close();
	if (find == false )
	{
		res = "" ;//bugfixed 2017-10-27
		std::cout << "Warning : Not found any params with key : " << key << std::endl;
		if (musthas)
		{
			std::cout << "Error : can not find params of "<<key<<", the program can not run. outing...  " << std::endl;
			exit(111);
		}
	}
	return res;
}

//从路径中截取文件名
std::string wft_base_name(std::string const & path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}

//判断文件是否存在
bool wft_test_file_exists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
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
	if (absDir.length() < 1) return;
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

//2017-11-23
void wft_get_allSelectedFiles(std::string inDir, std::string fnFixPrefix,
	std::string fnFixTail, int fnFixMidLoc, std::string fnFixMidStr,
	std::vector<std::string>& selectedFiles)
{
	std::vector<std::string> allfiles;
	wft_get_allfiles(inDir, allfiles);
	int nfiles = allfiles.size();
	for (int i = 0; i < nfiles; ++i)
	{
		std::string filepath = allfiles[i];
		std::string filename = wft_base_name(filepath);
		int pos0 = filename.find(fnFixPrefix);
		if (pos0 == 0)
		{
			int pos1 = filename.rfind(fnFixTail);
			if (pos1 != std::string::npos && pos1 == filename.length() - fnFixTail.length())
			{
				if (fnFixMidLoc >= 0 && fnFixMidStr != "")
				{
					std::string mid1 = filename.substr(fnFixMidLoc, fnFixMidStr.length());
					if (mid1 == fnFixMidStr)
					{
						selectedFiles.push_back(filepath);
					}
				}
				else {
					selectedFiles.push_back(filepath);
				}
			}
		}
	}
}






//在一个目录中寻找文件
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
		printf("%s", file.name);
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




//通过标签获取命令行参数
bool wft_has_param(int argc, char** argv, char* key, std::string& value , bool mustWithValue )
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

//通过标签获取命令行参数
bool wft_has_param2(int argc, char** argv, char* key, std::string& value, bool mustWithValue , std::string defaultForNoValue )
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
	value = defaultForNoValue;
	return false;
}


//通过标签获取命令行是否有某个tag
bool wft_has_tag(int argc, char** argv, char* key  )
{
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], key) == 0) {
			return true ;
		}
	}
	return false;
}
//通过标签获取命令行是否有某个tag
int wft_tag_index(int argc, char** argv, char* key)
{
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], key) == 0) {
			return i ;
		}
	}
	return -1;
}


//进度显示
void wft_term_progress(size_t curr, size_t total)
{
	static size_t term_progress_percent = -1;
	if (curr < total-1)
	{
		size_t newper = curr * 100 / total;
		if (newper != term_progress_percent) {
			term_progress_percent = newper;
			if (term_progress_percent % 10 == 0) {
				std::cout << term_progress_percent;
			}
			else {
				std::cout << ".";
			}
		}
	}
	else{
		if (term_progress_percent != 100) {
			term_progress_percent = 100;
			std::cout << 100 << std::endl;
		}
	}
}

//获取当前日期，格式YYYYMMDD
std::string wft_current_dateymd()
{
	time_t theTime = time(NULL);
	struct tm *aTime = localtime(&theTime);
	int day = aTime->tm_mday;
	int month = aTime->tm_mon + 1;  
	int year = aTime->tm_year + 1900; 
	char buff[10];
	sprintf(buff, "%04d%02d%02d", year, month, day);
	return std::string(buff);
}

//获取当前日期int，格式YYYYMMDD
int wft_current_dateymd_int()
{
	time_t theTime = time(NULL);
	//struct tm *aTime = localtime(&theTime);//use utc
	struct tm *aTime = gmtime(&theTime);//modify for utc  2017-10-23.
	int day = aTime->tm_mday;
	int month = aTime->tm_mon + 1;  
	int year = aTime->tm_year + 1900; 
	return year * 10000 + month * 100 + day ;
}

//获取当前日期和时间int，格式YYYYMMDD 2017-10-23
int wft_current_dateymd_int2(int& hhmm)
{
	time_t theTime = time(NULL);
	struct tm *aTime = gmtime(&theTime);
	int day = aTime->tm_mday;
	int month = aTime->tm_mon + 1;
	int year = aTime->tm_year + 1900;

	int hour = aTime->tm_hour;
	int minu = aTime->tm_min;
	hhmm = hour * 100 + minu;

	return year * 10000 + month * 100 + day;
}

//获取当前日期，格式YYYYMMDD HH:mm:ss 2017-11-14
std::string wft_current_datetimestr()
{
	time_t theTime = time(NULL);
	struct tm *aTime = localtime(&theTime);
	int day = aTime->tm_mday;
	int month = aTime->tm_mon + 1;
	int year = aTime->tm_year + 1900;

	int hour = aTime->tm_hour;
	int minu = aTime->tm_min;
	int sec = aTime->tm_sec;

	char buff[30];
	sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day , hour ,minu , sec );
	return std::string(buff);
}

//获取 日期，格式YYYYMMDD 2017-11-15
std::string wft_ymd_int2str( int ymdi )
{
	int year = ymdi / 10000;
	int mon = (ymdi % 10000) / 100;
	int day = ymdi % 100;

	char buff[30];
	sprintf(buff, "%04d-%02d-%02d", year, mon, day);
	return std::string(buff);
}



//写入日志
void wft_log(std::string logdir , std::string programid, std::string content)
{
	if (logdir != "")
	{
		if (*logdir.rbegin() != '/') {
			logdir += "/";
		}
	}
	std::string ymd = wft_current_dateymd();
	std::string logfile = logdir + programid + "-" + ymd + ".log" ;
	std::ofstream outfs;
	if (wft_test_file_exists(logfile))
	{
		outfs.open(logfile, std::ofstream::app);
	}
	else {
		outfs.open(logfile);
	}
	std::string currTime = wft_get_current_time();
	outfs << currTime << " -> " << content << std::endl;
	std::cout<<content<<std::endl ;
	outfs.close();

}



//逐行分析
void wft_file_linebyline(std::string infile) {
	std::ifstream infs(infile.c_str());
	std::string line;
	while (std::getline(infs, line))
	{
		std::istringstream iss(line);
		int a, b;
		if (!(iss >> a >> b)) { break; } // error

										 // process pair (a,b)
	}
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

//2017-11-10 年月日转换日序数当年。
int wft_convertymd2dayofyear(int mon, int day, int year)
{
	int days[2][13] = {
		{ 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
		{ 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
	};
	int leap =  (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	return days[leap][mon] + day;
}
//2017-11-10 日年转换为月份和日
void wft_convertdayofyear2monthday(int year , int dayofyear , int& month , int& day )
{
	month = 0;
	day = 0;
	int days[2][13] = {
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 , 367 },
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 , 367 }
	};
	int leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	for (int imon = 1; imon <= 12; ++imon)
	{
		if (dayofyear <= days[leap][imon])
		{
			month = imon;
			day = dayofyear - days[leap][imon - 1];
			return;
		}
	}
	std::cout << "Error: day of year " << dayofyear << " is invalid." << std::endl;
}


//日期结构体
struct WYmd {
	int year, month, day;
	bool isleapyear;
};


//复制文件并替换掉变量
int wft_create_file_by_template_with_replacement(std::string targetFile, std::string temfile,
	std::vector<std::string> variableVector,
	std::vector<std::string> replaceStrVector)
{
	std::ofstream ofs(targetFile.c_str());
	std::ifstream ifs(temfile.c_str());

	std::string line;
	while (std::getline(ifs, line))
	{
		for (size_t ir = 0; ir < variableVector.size(); ++ir)
		{
			line = wft_replaceString(line, variableVector[ir], replaceStrVector[ir]);
		}
		ofs << line << std::endl;
	}

	ofs.close();
	ifs.close();
	return 100;
}

//从文件列表文件中读取数组
std::vector<std::string> wft_get_filelist_from_file(std::string filepath , std::string prefix , std::string tail )
{
	std::vector<std::string> res;
	std::ifstream tfs(filepath.c_str());
	std::string line;
	while (std::getline(tfs, line))
	{
		if (line.length() > 1)
		{
			std::string tempInDsPath = prefix + line + tail;
			res.push_back(tempInDsPath);
		}
	}
	tfs.close();
	return res;
}

//逐行写入一个文件
void wft_write_file_linebyline(std::string filepath, std::vector<std::string>& vec, std::string prefix, std::string tail)
{
	std::ofstream tfs(filepath.c_str());
	for (size_t i = 0; i < vec.size(); ++ i )
	{
		tfs << prefix << vec[i] <<tail  << std::endl;
	}
	tfs.close();

}

void wft_compare_minmax(double val, double& vmin, double& vmax)
{
	if (val < vmin) vmin = val;
	if (val > vmax) vmax = val;
}

void wft_make_histfile(double vmin, double vmax, double binwid, std::vector<double> vec, std::string outfile)
{
	double tmin = ((int)(vmin / binwid))*binwid;
	double tmax = (1+(int)(vmax / binwid))*binwid;
	int numbin = (tmax - tmin) / binwid;
	if (numbin == 0)
	{
		std::cout << "Warning : the number of bin is zero. no hist file will be generated." << std::endl;
		return;
	}
	double* histArray = new double[numbin];
	for (int i = 0; i < numbin; ++i) histArray[i] = 0;
	for (size_t it = 0; it < vec.size(); ++it)
	{
		int ibin = (vec[it] - tmin) / binwid;
		if (ibin < 0) ibin = 0;
		if (ibin >= numbin) ibin = numbin - 1;
		histArray[ibin] += 1;
	}
	int vsize = (int)vec.size();
	std::ofstream histOfs(outfile.c_str());
	histOfs << "#x h" << std::endl;
	for (int i = 0; i < numbin; ++i)
	{
		histArray[i] = histArray[i] / vsize * 100.0 ;//use percent %.
		histOfs << (tmin + i*binwid) << " " << histArray[i] << std::endl;
	}
	histOfs.close();
	delete[] histArray;
}

//ASCII 字符串分割 2017-10-25
std::vector<std::string> wft_string_split(std::string wholeString, std::string sep)
{
	std::vector<std::string> result;

	int pos0 = 0;
	while (pos0 < wholeString.length())
	{
		int pos1 = wholeString.find(sep,pos0);
		if (pos1 == std::string::npos)
		{
			std::string substr = wholeString.substr(pos0);
			if (substr.length() > 0) result.push_back(substr);
			break;
		}
		else {
			int len1 = pos1 - pos0;
			std::string substr = wholeString.substr(pos0 , len1);
			if (substr.length() > 0) result.push_back(substr);
			pos0 = pos1 + 1;
		}
	}
	return result;
}

//find index of string in vector , -1 for not find 2017-10-25
int wft_get_strindex_from_vector(std::string find, std::vector<std::string> vec)
{
	for (int i = 0; i < vec.size(); ++i)
	{
		if (find == vec[i])
		{
			return i;
		}
	}
	return -1;
}

//2017-10-25
bool wft_string_has_tail(std::string str, std::string tail)
{
	if (str.length() >= tail.length())
	{
		std::string tempTail = str.substr(str.length() - tail.length());
		if (tempTail == tail)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else {
		return false;
	}
}
//2017-10-25
bool wft_string_has_tails(std::string str, std::vector<std::string>& tailvec)
{
	for (int i = 0; i < tailvec.size(); ++i)
	{
		if (wft_string_has_tail(str, tailvec[i]))
		{
			return true;
		}
	}
	return false;
}

//2017-10-26
int wft_remove_file(std::string filepath)
{
	if (remove(filepath.c_str()) != 0)
	{
		std::cout << "Error deleting file:" << filepath << std::endl;
		return 101;
	}
	else
	{
		std::cout << "Successfully deleting file:" << filepath << std::endl;
		return 0;
	}
}


/*

//GDAL 数据缓存
struct DataBlock
{
	DataBlock(GDALRasterBand* bandPtr, int bufferHeight);
	~DataBlock();
	int m_bufferHeight;
	GDALRasterBand* m_bandPtr;
	int m_imageXSize, m_imageYSize;
	int m_y0 , m_hei ;
	double* m_data ;
	
	double getValueByImagePixelXY(int imgx, int imgy);
	bool isInsideCurrentBlock( int y );
	bool isInsideImage(int x ,int y);
private:
	void loadDataBlockByCenterY(int centerY);
};

DataBlock::~DataBlock()
{
	if (m_data != 0) {
		delete[] m_data;
		m_data = 0;
	}
	m_bandPtr = 0;
}
DataBlock::DataBlock(GDALRasterBand* bandPtr, int bufferHeight)
{
	m_bufferHeight = bufferHeight;
	m_bandPtr = bandPtr;
	m_imageXSize = m_bandPtr->GetXSize();
	m_imageYSize = m_bandPtr->GetYSize();
	m_data = new double[m_imageXSize * m_bufferHeight];
	m_y0 = -1;
	this->loadDataBlockByCenterY(0);
}

void DataBlock::loadDataBlockByCenterY( int centerY )
{
	m_y0 = MAX(0, centerY -  2);
	m_hei = MIN(m_bufferHeight , m_imageYSize - m_y0 );
	m_bandPtr->RasterIO(GDALRWFlag::GF_Read, 0, m_y0, m_imageXSize , m_hei,
		m_data, m_imageXSize, m_bufferHeight, GDALDataType::GDT_Float64, 0, 0, 0);
}
double DataBlock::getValueByImagePixelXY(int imgx, int imgy)
{
	if (isInsideImage(imgx, imgy))
	{
		if (this->isInsideCurrentBlock(imgy) == false ) {
			this->loadDataBlockByCenterY(imgy);
		}
		int yoffset = imgy - m_y0;
		return this->m_data[yoffset*m_imageXSize + imgx] ;
	}
	else {
		std::cout << "Error : " << imgx << "," << imgy << " is outside of image. out." << std::endl;
		exit(77);
		return 0;
	}
}
bool DataBlock::isInsideCurrentBlock( int y)
{
	if (y >= m_y0 && y < m_y0 + m_hei) {
		return true;
	}
	else {
		return false;
	}
}

bool DataBlock::isInsideImage(int x, int y)
{
	if (x < 0 || y < 0) return false;
	if (x >= m_imageXSize || y >= m_imageYSize) return false;
	return true;
}
*/


#endif