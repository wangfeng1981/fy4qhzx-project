std::string wft_replaceString(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

void writeDatFile(std::string datfilepath, int* buff, std::ofstream& plotofs, std::string coastlinefile)
{
	std::ofstream datofs(datfilepath);
	datofs << "# lon lat hadisst_value " << std::endl;
	for (int iy = 0; iy < 180; ++iy)
	{
		bool hasval = false;
		for (int ix = 0; ix < 360; ++ix)
		{
			double lon = ix - 180 + 0.5;
			double lat = 90 - iy - 0.5;
			if (buff[iy * 360 + ix] < -500)
			{
				datofs << lon << " " << lat << " " << "NaN" << std::endl;
			}
			else {
				datofs << lon << " " << lat << " " << buff[iy * 360 + ix] * 0.01 << std::endl;
			}
		}
		datofs << std::endl;
	}
	datofs.close();

	std::string temppngfile = datfilepath + ".png";
	plotofs << "set terminal png size 800, 350\n" << "set output '" << temppngfile << "'\n"
		<< "set border lw 1.5" << std::endl
		<< "set style line 1 lc rgb 'black' lt 1 lw 2" << std::endl
		<< "set rmargin screen 0.85" << std::endl
		<< "unset key" << std::endl
		<< "set tics scale 5" << std::endl
		<< "set pal maxcolors 10" << std::endl
		<< "set cbrange [-10:40]" << std::endl
		<< "set xrange[-179:179]" << std::endl
		<< "set yrange[-89:89]" << std::endl
		<< "set palette defined ( 0 '0x222255',10 '0x222255' , 10 '0x4575b4',20 '0x4575b4' , 20 '0x74add1',30 '0x74add1' , 30 '0xabd9e9',40 '0xabd9e9' , 40 '0xe0f3f8',50 '0xe0f3f8' , 50 '0xffffbf',60 '0xffffbf' , 60 '0xfee090',70 '0xfee090' , 70 '0xfdae61',80 '0xfdae61' , 80 '0xf46d43',90 '0xf46d43' , 90 '0xd73027',100 '0xd73027' )" << std::endl
		<< "plot '" << datfilepath
		<< "' u 1:2:3 w image, '" << coastlinefile
		<< "' with lines linestyle 1" << std::endl;
}


void printCurrentTime() {
	//#include <ctime>
	time_t time0;
	time(&time0);
	std::cout << asctime(localtime(&time0)) << std::endl;
}

//read extra params file by key. note the key should like the same as text file write do not use space.
std::string getValueFromExtraParamsFile(std::string extrafile, std::string key , bool musthas=false )
{
	std::string res = "";
	bool find = false;
	std::ifstream  ifs(extrafile.c_str());
	while (getline(ifs, res)) {
		if (res == key) {
			getline(ifs, res);
			find = true;
			break;
		}
	}
	ifs.close();
	if (find == false )
	{
		std::cout << "Error : Not found any params with key : " << key << std::endl;
		if (musthas)
		{
			std::cout << "Error : 缺少必须参数:"<<key<<"，程序无法运行。退出中...  " << std::endl;
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





//在一个目录中寻找文件
std::string lookingForModisFilenameByPrefix(std::string absDir, std::string prefix)
{
	//#define UNICODE
	//#include "tinydir.h"
	//the two line above must be place before other include. In linux remove #define UNICODE staff.
	assert(prefix.length() >0);
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

//通过标签获取命令行是否有某个tag
bool wft_has_tag(int argc, char** argv, char* key  )
{
	value = "";
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], key) == 0) {
			return true ;
		}
	}
	return false;
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


//逐行分析
void wft_file_linebyline(){
	std::string line;
	while (std::getline(infile, line))
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

//日期结构体
struct WYmd {
	int year, month, day;
	bool isleapyear;
};