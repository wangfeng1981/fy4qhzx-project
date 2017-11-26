// qcbylonlatlut.cpp : 定义控制台应用程序的入口点。
//待检验数据使用经纬度查找表，检验源数据为等经纬度坐标系并使用双线性差值计算参考值。
//wangfeng1@piesat.cn 2017-9-15
//plot文件中使用-outroot输出根路径替换调掉模板中{{{ROOT}}}的值.plot文档中的输入文件与输出文件均根据{{{ROOT}}}加后缀名得到。
//例：海温plot模板中 散点图的输入文件定义为 {{{ROOT}}}.scatter.txt   散点图固定输出路径为{{{ROOT}}}.scatter.png
//
//0.3a 对输入数据按行处理，参考数据按块缓存
//0.4a 增加一个root.linear.txt文件用于gnuplot计算线性回归公式的两个点
//0.5a 散点图最多20万个点，超过这个数目进行重采样到20万点。
//0.5a.1 bufix scatter.txt.
//0.6a 增加参考数据匹配方法选项 " [-matchmethod nn/bil/aver] " 可选项，值有最邻近nn，双线性插值bil，平均值aver


//-type llrange -inf fy4sst0807.NC -reff avhrr-only-v2.20170807.nc -inds HDF5:"fy4sst0807.NC"://SST -refds NETCDF:"avhrr-only-v2.20170807.nc":sst -inlon E:/coding/fy4qhzx-project/extras/fy4lon-good.tif -inlat E:/coding/fy4qhzx-project/extras/fy4lat-good.tif -rlon0 0 -rlon1 360 -rlat0 90 -rlat1 -90 -outroot demoout -iv0 -5 -iv1 45 -rv0 -300 -rv1 4500 -isca 1 -iofs 0 -rsca 0.01 -rofs 0 -fill -99 -nomat -88 -pcode sst -coast null -xbk x1 -xcv x2 -xdf x3 -xge x4 -xpd x5 -xpv x6 -outxml server.xml -plot E:/coding/fy4qhzx-project/extras/sst.plot
//
/*
-type geotrans -inf E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.mosic.tif
-reff E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.modvrt.wgs84.tif
-inds E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.mosic.tif
-refds E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.modvrt.wgs84.tif
-inlon E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.fy3lon.tif
-inlat E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.fy3lat.tif
-outroot E:/testdata/testdatandvi/22test/
-iv0 -10000 -iv1 10000 -rv0 -2000 -rv1 10000 -isca 0.0001 -iofs 0 -rsca 0.0001 -rofs 0 
-fill -99 -nomat -88 -pcode f3mndvi -coast null  -xbk x1 -xcv x2 -xdf x3 -xge x4 -xpd x5 -xpv x6
-outxml cdxml.xml 
-plot E:/coding/fy4qhzx-project/extras/ndvi.plot
*/
 
#include "gdal_priv.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <cassert>

struct WTriple
{
	double val0, val1, val2;
	inline WTriple(double v0, double v1, double v2) :val0(v0), val1(v1), val2(v2) {  };
};

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
	else {
		std::cout << 100 << std::endl;
	}

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


std::string wft_replaceString(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}



int main(int argc , char** argv )
{
	std::cout << "Version 0.6a " << std::endl;
	std::cout << "by wangfengdev@163.com 2017 - 9 - 21. " << std::endl;
	
	if (argc == 1 )
	{
		std::cout << "Sample call:" << std::endl;
		std::cout << "qcbylonlatlut -type geotrans -inf input.tif -reff ref.tif " 
			" -inds inds.tif -refds refds.tif "  
			" -inlon inlon.tif -inlat inlat.tif "
			" -outroot outroot.tif "
			" -iv0 -10000 -iv1 10000 -rv0 -2000 -rv1 10000  "
			" -isca 0.0001 -iofs 0 -rsca 0.0001 -rofs 0 "
			" -fill -999  -nomat -888 "
			" -pcode sst "
			" -coast world_110m.txt "
			" -xbk x1 -xcv x2 -xdf x3 -xge x4 -xpd x5 -xpv x6  "
			" -outxml serverprovide.xml "
			" -plot someascii.plot "
			" [-matchmethod nn/bil/aver] "
			<< std::endl ;
		std::cout << std::endl;
		std::cout << "qcbylonlatlut -type llrange -inf input.tif -reff ref.tif "
			" -inds inds.tif -refds refds.tif "
			" -inlon inlon.tif -inlat inlat.tif "
			" -rlon0 -180 -rlon1 180 -rlat0 90 -rlat1 -90 "
			" -outroot outroot.tif "
			" -iv0 -10000 -iv1 10000 -rv0 -2000 -rv1 10000  "
			" -isca 0.0001 -iofs 0 -rsca 0.0001 -rofs 0 "
			" -fill -999  -nomat -888 "
			" -pcode sst "
			" -coast world_110m.txt "
			" -xbk x1 -xcv x2 -xdf x3 -xge x4 -xpd x5 -xpv x6  "
			" -outxml serverprovide.xml "
			" -plot someascii.plot "
			" [-matchmethod nn/bil/aver] "
			<< std::endl;

		std::cout << "outputs:" << std::endl;
		std::cout << "root.biasimg.tif \t root.matchedpoints.txt \t root.lonlatbias.txt " << std::endl;
		std::cout << "root.biashist.txt \t root.scatter.txt \t root.plot " << std::endl;
		std::cout << "root.scatter.png \t root.heatmap.png \t root.hist.png " << std::endl;
		std::cout << "root.linear.txt \t  " << std::endl;
		std::cout << "serverprovidefilepath.xml " << std::endl;


		std::cout << "*** Error: no enough parameters. out." << std::endl;
		return 1001;
	}

	//type
	std::string intype;
	wft_has_param(argc, argv, "-type", intype, true);

	//inputfilename
	std::string inputInputFilename = "";
	wft_has_param(argc, argv, "-inf", inputInputFilename, true);
	//inputdatasetpath
	std::string inputbaseDataPath = "";
	wft_has_param(argc, argv, "-inds", inputbaseDataPath, true);

	//inputvalid0
	double inputbaseDataValid0 = 0 ;
	{
		std::string temp;
		wft_has_param(argc, argv, "-iv0", temp, true);
		inputbaseDataValid0 = atof(temp.c_str());
	}

	//inputvalid1
	 double inputbaseDataValid1 = 0;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-iv1", temp, true);
		 inputbaseDataValid1 = atof(temp.c_str());
	 }

	//inputdatascale
	 double inputbaseDataScale =0 ;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-isca", temp, true);
		 inputbaseDataScale = atof(temp.c_str());
	 }

	//inputdataoffset
	 double inputbaseDataOffset = 0;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-iofs", temp, true);
		 inputbaseDataOffset = atof(temp.c_str());
	 }

	//inputlontif
	 std::string inputbaseLonPath ;
	 wft_has_param(argc, argv, "-inlon", inputbaseLonPath, true);


	//inputlattif
	 std::string inputbaseLatPath ;
	 wft_has_param(argc, argv, "-inlat", inputbaseLatPath, true);

	//reffilename
	std::string inputRefFilename  ;
	wft_has_param(argc, argv, "-reff", inputRefFilename, true);


	//refdatasetpath
	 std::string inputrefDataPath  ;
	 wft_has_param(argc, argv, "-refds", inputrefDataPath, true);


	//refvalid0
	 double inputrefDataValid0 = 0;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-rv0", temp, true);
		 inputrefDataValid0 = atof(temp.c_str());
	 }


	//refvalid1
	 double inputrefDataValid1 =0;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-rv1", temp, true);
		 inputrefDataValid1 = atof(temp.c_str());
	 }


	//refdatascale
	 double inputrefDataScale = 0;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-rsca", temp, true);
		 inputrefDataScale = atof(temp.c_str());
	 }


	//refdataoffset
	 double inputrefDataOffset = 0;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-rofs", temp, true);
		 inputrefDataOffset = atof(temp.c_str());
	 }


	//refleftlon
	double refLeftLon = 0;
	if (intype == "llrange")
	{
		std::string temp;
		wft_has_param(argc, argv, "-rlon0", temp, true);
		refLeftLon = atof(temp.c_str());
	}


	//refrightlon
	double refRightLon = 0;
	if( intype == "llrange" )
	{
		std::string temp;
		wft_has_param(argc, argv, "-rlon1", temp, true);
		refRightLon = atof(temp.c_str());
	}


	//reftoplat
	double refTopLat = 0;
	if (intype == "llrange")
	{
		std::string temp;
		wft_has_param(argc, argv, "-rlat0", temp, true);
		refTopLat = atof(temp.c_str());
	}


	//refbottomlat
	double refBottomLat = 0;
	if (intype == "llrange")
	{
		std::string temp;
		wft_has_param(argc, argv, "-rlat1", temp, true);
		refBottomLat = atof(temp.c_str());
	}


	//outputrootpath
	 std::string inputOutputFileRoot  ;
	 wft_has_param(argc, argv, "-outroot", inputOutputFileRoot, true);

	//filledvalue
	 double inputOutFilledValue = 0 ;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-fill", temp, true);
		 inputOutFilledValue = atof(temp.c_str());
	 }

	//nomatchedfillvalue
	 double inputOutNotMatchedValue = 0;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-nomat", temp, true);
		 inputOutNotMatchedValue = atof(temp.c_str());
	 }

	//productcode
	 std::string inputProductType  ;
	 wft_has_param(argc, argv, "-pcode", inputProductType, true);

	//coastline
	 std::string inputCoastlineFilepath  ;
	 wft_has_param(argc, argv, "-coast", inputCoastlineFilepath, true);

	//bechekvariety
	 std::string xp1  ;
	 wft_has_param(argc, argv, "-xbk", xp1, true);

	//checkVariety
	 std::string xp2  ;
	 wft_has_param(argc, argv, "-xcv", xp2, true);

	//dataFormat
	 std::string xp3  ;
	 wft_has_param(argc, argv, "-xdf", xp3, true);

	//GERO
	 std::string xp4  ;
	 wft_has_param(argc, argv, "-xge", xp4, true);

	//productDate
	 std::string xp5  ;
	 wft_has_param(argc, argv, "-xpd", xp5, true);

	//productVariety
	 std::string xp6  ;
	 wft_has_param(argc, argv, "-xpv", xp6, true);


	//thetargetresultxmlfilepath
	 std::string serverProvideXmlPath  ;
	 wft_has_param(argc, argv, "-outxml", serverProvideXmlPath, true);

	//plottemplate
	 std::string plotTemplateFile;
	 wft_has_param(argc, argv, "-plot", plotTemplateFile, true);

	 //参考数据匹配方法
	 int matchMethod = 0; //0-nn 1-bil 2-aver
	 {
		 std::string mm = "";
		 bool hasMatchMethod = wft_has_param(argc, argv, "-matchmethod", mm, false);
		 if (hasMatchMethod) {
			 matchMethod = 0;
			 if (mm == "bil") matchMethod = 1;
			 if (mm == "aver") matchMethod = 2;
		 }
	 }
	

	std::string outputbiasfile = inputOutputFileRoot + ".biasimg.tif";
	std::string outputhistfile = inputOutputFileRoot + ".matchedpoints.txt";
	std::string lonlatbiasfile = inputOutputFileRoot + ".lonlatbias.txt";

	std::ofstream outHistStream(outputhistfile.c_str());
	std::ofstream outLonLatBiasStream(lonlatbiasfile.c_str());
	//打印表头
	outHistStream << "#ibx iby inlon inlat inval refx refy match interval val00 val10 val01 val11 bias" << std::endl;
	outHistStream << "#inlon inlat bias" << std::endl;


	GDALAllRegister();

	GDALDataset* baseDataDs = (GDALDataset*)GDALOpen(inputbaseDataPath.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* baseLonDs = (GDALDataset*)GDALOpen(inputbaseLonPath.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* baseLatDs = (GDALDataset*)GDALOpen(inputbaseLatPath.c_str(), GDALAccess::GA_ReadOnly);

	GDALDataset* refDataDs = (GDALDataset*)GDALOpen(inputrefDataPath.c_str(), GDALAccess::GA_ReadOnly);

	int baseXSize = baseDataDs->GetRasterXSize();
	int baseYSize = baseDataDs->GetRasterYSize();
	{
		int xs = baseLonDs->GetRasterXSize();
		int ys = baseLonDs->GetRasterYSize();
		int x1 = baseLatDs->GetRasterXSize();
		int y1 = baseLatDs->GetRasterYSize();
		if (baseXSize != xs || baseXSize != x1 || baseYSize != ys || baseYSize != y1)
		{
			std::cout << "*** Error: base data size is not equal with lon or lat data size . out." << std::endl;
			return 1002;
		}
	}

	int refXSize = refDataDs->GetRasterXSize();
	int refYSize = refDataDs->GetRasterYSize();

	double referenceGeoTransform[6];
	//GDAL default transform is (0,1,0,0,0,1) 
	//Xp(lon) = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
	//Yp(lat) = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];

	if( intype == "llrange" )
	{
		referenceGeoTransform[2] = 0;
		referenceGeoTransform[1] = (refRightLon - refLeftLon) / refXSize;
		//fix referenceGeoTransform[0] = refLeftLon + referenceGeoTransform[1] / 2;//fix 原点坐标使用边缘坐标，不再使用中心坐标
		referenceGeoTransform[0] = refLeftLon;

		referenceGeoTransform[5] = (refBottomLat - refTopLat) / refYSize ;
		referenceGeoTransform[4] = 0 ;
		//fix referenceGeoTransform[3] = refTopLat + referenceGeoTransform[5] / 2;//fix 原点坐标使用边缘坐标，不再使用中心坐标
		referenceGeoTransform[3] = refTopLat;
	}
	else if (intype == "geotrans")
	{
		GDALDataset* tempDs =(GDALDataset*) GDALOpen( inputrefDataPath.c_str() , GDALAccess::GA_ReadOnly );
		tempDs->GetGeoTransform(referenceGeoTransform);
		//fix 原点坐标使用边缘坐标，不再使用中心坐标 referenceGeoTransform[0] = referenceGeoTransform[0] + referenceGeoTransform[1] / 2;
		//fix 原点坐标使用边缘坐标，不再使用中心坐标 referenceGeoTransform[3] = referenceGeoTransform[3] + referenceGeoTransform[5] / 2;
		GDALClose(tempDs);
	}
	else {
		std::cout << "Error : Unsupported type " << intype << std::endl;
		exit(91);
	}

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputBiasDataset = driver->Create(outputbiasfile.c_str(), baseXSize, baseYSize , 1, GDALDataType::GDT_Float32, 0);

	float * outBiasRowValues = new float[baseXSize];

	//统计信息
	std::vector<WTriple> dataTripleVector;
	std::vector<double> biasVector;
	double validPixelCount = 0;
	double averageInputValue = 0;
	double averageRefValue = 0;
	double averageBias = 0.0;
	double averageAbsBias = 0.0;
	double rmse = 0;
	double inputMaxValue = -99999;
	double inputMinValue = 99999;
	double biasMaxValue = -99999;
	double biasMinValue = 99999;
	double refMinValue = 99999;
	double refMaxValue = -99999;
	double fitxySum = 0;
	double fitxSum = 0;
	double fitySum = 0;
	double fitxxSum = 0;

	DataBlock refDataBlock(refDataDs->GetRasterBand(1) , 200 );
	DataBlock inDataBlock(baseDataDs->GetRasterBand(1),  200);
	DataBlock lonDataBlock(baseLonDs->GetRasterBand(1), 200);
	DataBlock latDataBlock(baseLatDs->GetRasterBand(1), 200);

	int lonlatbiasStridex = 1;
	int lonlatbiasStridey = 1;
	if (baseXSize > 1000) {
		lonlatbiasStridex = baseXSize / 1000;
	}
	if (baseYSize > 1000) {
		lonlatbiasStridey = baseYSize / 1000;
	}

	for (int iby = 0; iby < baseYSize; ++iby)
	{
		std::stringstream ssbuffer;
		std::stringstream lonLatBiasBuffer;

		for (int ibx = 0; ibx < baseXSize; ++ibx)
		{
			//初始化输出bias数组
			outBiasRowValues[ibx] = inputOutFilledValue ;

			double baseValue = inDataBlock.getValueByImagePixelXY(ibx,iby);// dataValues[ibx];
			double blonValue = lonDataBlock.getValueByImagePixelXY(ibx, iby);//中心坐标
			double blatValue = latDataBlock.getValueByImagePixelXY(ibx, iby);//中心坐标
			int matchRefType = 0;//匹配类型 0-未匹配，1-匹配一个值，2-匹配X轴两个值，3-匹配Y轴两个值，4-匹配四个值
			if (baseValue >= inputbaseDataValid0 && baseValue <= inputbaseDataValid1 && blonValue>-998 && blatValue>-998 )
			{
				double tempInputValue = baseValue * inputbaseDataScale + inputbaseDataOffset;
				outBiasRowValues[ibx] = inputOutNotMatchedValue;
				const double* t = referenceGeoTransform;
				int targetRefX = 0;
				int targetRefY = 0;
				double refMatchedValue = 0;
				if (matchMethod == 0)
				{//最邻近法
					targetRefX = (int)((blonValue*t[5] - blatValue*t[2] - t[0] * t[5] + t[2] * t[3]) / (t[1] * t[5] - t[2] * t[4]));
					targetRefY = (int)((blonValue*t[4] - blatValue*t[1] - t[0] * t[4] + t[1] * t[3]) / (t[2] * t[4] - t[1] * t[5]));

				}
				else if (matchMethod == 1)
				{//双线性法
					targetRefX = (int)((blonValue*t[5] - blatValue*t[2] - t[0] * t[5] + t[2] * t[3]) / (t[1] * t[5] - t[2] * t[4]));
					targetRefY = (int)((blonValue*t[4] - blatValue*t[1] - t[0] * t[4] + t[1] * t[3]) / (t[2] * t[4] - t[1] * t[5]));
				}
				else {
					//平均值法
					targetRefX = (int)((blonValue*t[5] - blatValue*t[2] - t[0] * t[5] + t[2] * t[3]) / (t[1] * t[5] - t[2] * t[4]));
					targetRefY = (int)((blonValue*t[4] - blatValue*t[1] - t[0] * t[4] + t[1] * t[3]) / (t[2] * t[4] - t[1] * t[5]));
				}
				double bias = 0.0;				
				if (targetRefX >= 0 && targetRefX < refXSize-1 && targetRefY >= 0 && targetRefY < refYSize-1 )
				{//4

					float tempRefValueArr[4] ;
					tempRefValueArr[0] = refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY);
					tempRefValueArr[1] = refDataBlock.getValueByImagePixelXY(targetRefX+1, targetRefY);
					tempRefValueArr[2] = refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY+1);
					tempRefValueArr[3] = refDataBlock.getValueByImagePixelXY(targetRefX+1, targetRefY+1);

					if (inputrefDataValid0 <= tempRefValueArr[0] && tempRefValueArr[0] <= inputrefDataValid1)
					{
						int tnvalid = 1;
						matchRefType = 4;
						refMatchedData[0] = tempRefValueArr[0];
						refMatchedData[1] = tempRefValueArr[0];
						refMatchedData[2] = tempRefValueArr[1];
						refMatchedData[3] = tempRefValueArr[2];
						refMatchedData[4] = tempRefValueArr[3];
						for (int iref = 1; iref < 4; ++iref)
						{
							if (inputrefDataValid0 <= tempRefValueArr[iref] && tempRefValueArr[iref] <= inputrefDataValid1)
							{
								++tnvalid;
							}
						}
						
						if (tnvalid == 4)
						{//4个值都是有效值，做双线性差值
							
							double tval0 = tempRefValueArr[0] + (blonValue - (targetRefX * t[1] + t[0])) / t[1]*(tempRefValueArr[1] - tempRefValueArr[0]);
							double tval1 = tempRefValueArr[2] + (blonValue - (targetRefX * t[1] + t[0])) / t[1]*(tempRefValueArr[3] - tempRefValueArr[2]);
							double tval = tval0 + (blatValue - (targetRefY*t[5] + t[3])) / t[5] * (tval1 - tval0);
							tval = tval * inputrefDataScale + inputrefDataOffset;
							bias = tempInputValue - tval;
							refMatchedData[0] = tval;
						}
						else
						{//4个值出现无效值，使用最邻近的值
							double tempRefValue = tempRefValueArr[0] * inputrefDataScale + inputrefDataOffset;
							bias = tempInputValue - tempRefValue;
							refMatchedData[0] = tempRefValue;
						}
					}

				}
				else if (targetRefX == refXSize - 1 && targetRefY >= 0 && targetRefY < refYSize - 1)
				{//2

					float tempRefValueArr[2];
					tempRefValueArr[0] = refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY);
					tempRefValueArr[1] = refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY+1);

					if (inputrefDataValid0 <= tempRefValueArr[0] && tempRefValueArr[0] <= inputrefDataValid1)
					{
						matchRefType = 3;
						refMatchedData[0] = tempRefValueArr[0];
						refMatchedData[1] = tempRefValueArr[0];
						refMatchedData[3] = tempRefValueArr[1];
						if (inputrefDataValid0 <= tempRefValueArr[1] && tempRefValueArr[1] <= inputrefDataValid1)
						{
							double tval0 = tempRefValueArr[0] + (blatValue - (targetRefY * t[5] + t[3])) / t[5] * (tempRefValueArr[1] - tempRefValueArr[0]);
							tval0 = tval0 * inputrefDataScale + inputrefDataOffset;
							bias = tempInputValue - tval0;
							refMatchedData[0] = tval0;
						}
						else {
							double tempRefValue = tempRefValueArr[0] * inputrefDataScale + inputrefDataOffset;
							bias = tempInputValue - tempRefValue;
							refMatchedData[0] = tempRefValue;
						}
					}
				}
				else if (targetRefY == refYSize - 1 && targetRefX >= 0 && targetRefX < refXSize - 1)
				{//2
					float tempRefValueArr[2];
					tempRefValueArr[0] = refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY);
					tempRefValueArr[1] = refDataBlock.getValueByImagePixelXY(targetRefX+1, targetRefY);
					if (inputrefDataValid0 <= tempRefValueArr[0] && tempRefValueArr[0] <= inputrefDataValid1)
					{
						matchRefType = 2;
						refMatchedData[0] = tempRefValueArr[0];
						refMatchedData[1] = tempRefValueArr[0];
						refMatchedData[2] = tempRefValueArr[1];
						if (inputrefDataValid0 <= tempRefValueArr[1] && tempRefValueArr[1] <= inputrefDataValid1)
						{
							double tval0 = tempRefValueArr[0] + (blonValue - (targetRefX * t[1] + t[0])) / t[1] * (tempRefValueArr[1] - tempRefValueArr[0]);
							tval0 = tval0 * inputrefDataScale + inputrefDataOffset;
							bias = tempInputValue - tval0;
							refMatchedData[0] = tval0;
						}
						else {
							double tempRefValue = tempRefValueArr[0] * inputrefDataScale + inputrefDataOffset;
							bias = tempInputValue - tempRefValue;
							refMatchedData[0] = tempRefValue;
						}
					}
				}
				else if (targetRefX == refXSize - 1 && targetRefY == refYSize - 1)
				{//1
					float tempRefValue =  refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY);
					if (inputrefDataValid0 <= tempRefValue && tempRefValue <= inputrefDataValid1)
					{
						matchRefType = 1;
						refMatchedData[1] = tempRefValue;
						tempRefValue = tempRefValue * inputrefDataScale + inputrefDataOffset;
						bias = tempInputValue - tempRefValue;
						refMatchedData[0] = tempRefValue;
					}
				}
				else {
					//no matched.
				}

				if (matchRefType > 0)
				{//找到匹配值

					//统计信息
					++validPixelCount;
					fitxySum += tempInputValue * refMatchedData[0];
					fitxSum += tempInputValue;
					fitySum += refMatchedData[0];
					fitxxSum += tempInputValue*tempInputValue ;
					dataTripleVector.push_back(WTriple(tempInputValue, refMatchedData[0], bias));
					biasVector.push_back(bias);
					averageInputValue += tempInputValue;
					averageRefValue += refMatchedData[0];
					averageBias += bias;
					averageAbsBias += ABS(bias);
					rmse += bias * bias;
					if (tempInputValue > inputMaxValue) {
						inputMaxValue = tempInputValue;
					}
					if (tempInputValue < inputMinValue) {
						inputMinValue = tempInputValue;
					}
					if (refMatchedData[0] < refMinValue)
					{
						refMinValue = refMatchedData[0];
					}
					if (refMatchedData[0] > refMaxValue)
					{
						refMaxValue = refMatchedData[0];
					}
					if (bias > biasMaxValue)
					{
						biasMaxValue = bias;
					}
					if (bias < biasMinValue)
					{
						biasMinValue = bias;
					}

					outBiasRowValues[ibx] = bias;

					if (ibx % lonlatbiasStridex == 0 && iby % lonlatbiasStridey == 0)
					{
						//no need to write all points down. just save 1000.
						ssbuffer << ibx << " "
							<< iby << " "
							<< blonValue << " "
							<< blatValue << " "
							<< tempInputValue << " "
							<< targetRefX << " "
							<< targetRefY << " "
							<< matchRefType << " "
							<< refMatchedData[0] << " "
							<< refMatchedData[1] << " "
							<< refMatchedData[2] << " "
							<< refMatchedData[3] << " "
							<< refMatchedData[4] << " "
							<< bias << std::endl;
						lonLatBiasBuffer << blonValue << " " << blatValue << " " << bias << std::endl;
					}
				}
				
			}
			if (matchRefType == 0)
			{
				if (blonValue>-998 && blatValue>-998 && matchRefType == 0) {
					if (ibx % lonlatbiasStridex == 0 && iby % lonlatbiasStridey == 0)
					{
						lonLatBiasBuffer << blonValue << " " << blatValue << " NaN" << std::endl;
					}
				}
			}
			
		}//一行处理结束
		//写入文件
		if (ssbuffer.str().length() > 1)
		{
			outHistStream << ssbuffer.str();
		}
		if (lonLatBiasBuffer.str().length() > 1)
		{
			outLonLatBiasStream << lonLatBiasBuffer.str() << std::endl;
		}
		
		outputBiasDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iby, baseXSize, 1, 
				outBiasRowValues , baseXSize , 1, GDALDataType::GDT_Float32, 0, 0, 0);
		wft_term_progress(iby, baseYSize);
	}

	//delete [] latValues  ;
	//delete[] lonValues  ;
	//delete[] dataValues  ;
	//delete[] refDataBuffer;


	GDALClose(baseDataDs);  
	GDALClose(baseLonDs);
	GDALClose(baseLatDs);

	GDALClose(refDataDs);
	outHistStream.close();
	outLonLatBiasStream.close();
	GDALClose(outputBiasDataset);

	std::cout << "calculating quality parameters..." << std::endl;

	//线性拟合公式
	double linearSlope = 0; 
	double linearIntercept = 0;  
	//计算指标
	double corr = 0;
	double skew = 0;
	double kurt = 0;
	double middle = 0;
	double stddev = 0;
	if (validPixelCount > 1)
	{

		linearSlope =  (validPixelCount * fitxySum - fitxSum * fitySum) / (validPixelCount * fitxxSum - fitxSum * fitxSum);
		linearIntercept =   ( fitySum - linearSlope * fitxSum) / validPixelCount;

		averageInputValue = averageInputValue / validPixelCount;
		averageRefValue = averageRefValue / validPixelCount;
		averageBias = averageBias / validPixelCount;
		averageAbsBias = averageAbsBias / validPixelCount;
		rmse = sqrt(rmse / validPixelCount);
		size_t numPixels = dataTripleVector.size();
		for (std::vector<WTriple>::iterator it = dataTripleVector.begin(); it != dataTripleVector.end(); ++it)
		{
			WTriple data = *it;
			double cz = data.val2 - averageBias;
			stddev += cz*cz;
		}
		stddev = sqrt(stddev / validPixelCount);

		double corr_0 = 0;
		double corr_1 = 0;
		double corr_2 = 0;
		size_t curr = 0;
		for (std::vector<WTriple>::iterator it = dataTripleVector.begin(); it != dataTripleVector.end(); ++it)
		{
			WTriple data = *it;
			double cz0 = data.val0 - averageInputValue;
			double cz1 = data.val1 - averageRefValue;
			double czbias = data.val2 - averageBias;
			corr_0 += cz0*cz1;
			corr_1 += cz0*cz0;
			corr_2 += cz1*cz1;

			skew += czbias*czbias*czbias;
			kurt += czbias*czbias*czbias*czbias;
			wft_term_progress(++curr, numPixels);
		}
		corr = corr_0 / (sqrt(corr_1)*sqrt(corr_2));
		skew = skew / validPixelCount / (stddev*stddev*stddev);
		kurt = kurt / validPixelCount / (stddev*stddev*stddev*stddev) - 3.0;
		//求中值  
		std::sort(biasVector.begin(), biasVector.end()); 
		if (numPixels % 2 == 1) {
			size_t middleIndex = numPixels / 2;
			middle = biasVector[middleIndex];
		}
		else {
			size_t middleIndex0 = numPixels / 2;
			size_t middleIndex1 = numPixels / 2 - 1;
			middle = (biasVector[middleIndex0] + biasVector[middleIndex1]) / 2.0;
		}
	}
	else {
		linearSlope = -999;
		linearIntercept = -999;
		averageInputValue = -999;
		averageRefValue = -999;
		averageBias = -999;
		averageAbsBias = -999;
		rmse = -999;
		corr = -999;
		skew = -999;
		kurt = -999;
		middle = -999;
		stddev = -999;
	}

	std::string biasDistFilepath = inputOutputFileRoot + ".biashist.txt";
	std::string scaFilepath = inputOutputFileRoot + ".scatter.txt";
	std::string linearEquationFile = inputOutputFileRoot + ".linear.txt";
	//输出直方图
	{
		int histBinCount = 31;//fixed
		float histStride = (biasMaxValue - biasMinValue + 1) / histBinCount;//fixed
		float * histBinArray = new float[histBinCount] ;
		for (int ih = 0; ih < histBinCount; ++ih) histBinArray[ih] = 0.f;
		std::cout << "saving bias histogram and scatter values..." << std::endl;

		std::ofstream linearStream(linearEquationFile.c_str());
		linearStream << "#invalue  refvalue" << std::endl;
		linearStream << inputMinValue << "  " << inputMinValue * linearSlope + linearIntercept << std::endl;
		linearStream << inputMaxValue << "  " << inputMaxValue * linearSlope + linearIntercept << std::endl;
		linearStream.close();

		std::ofstream scaOfs(scaFilepath.c_str());
		scaOfs << "#invalue  refvalue" << std::endl;
		std::ofstream biasdistOfs(biasDistFilepath.c_str());
		size_t pxCount = biasVector.size();
		biasdistOfs << "#x bias" << std::endl;
		int scaStride = pxCount / 200000;
		for (size_t it = 0; it < pxCount; ++it)
		{
			// biasdistOfs << biasVector[it] << std::endl;
			int ihist = (biasVector[it] - biasMinValue) / histStride;
			++ histBinArray[ihist];
			if (it % scaStride == 0) {
				scaOfs << dataTripleVector[it].val0 << " " << dataTripleVector[it].val1 << std::endl;
			}
			wft_term_progress(it, pxCount);
		}
		scaOfs.close();

		for (int ih = 0; ih < histBinCount; ++ih)
		{
			biasdistOfs << biasMinValue + (ih+0.5)* histStride  << " " << histBinArray[ih] / validPixelCount << std::endl;
		}
		delete[] histBinArray;
		biasdistOfs.close();
		
	}

	std::string histPlotPng = inputOutputFileRoot + ".hist.png";
	std::string scatterPlotPng = inputOutputFileRoot + ".scatter.png";
	std::string fenbuPlotPng = inputOutputFileRoot + ".heatmap.png";
	//生成绘图gnuplot文件
	{
		std::cout << "creating plots script files...." << std::endl;

		std::ifstream tplot(plotTemplateFile);
		std::string plotFile = inputOutputFileRoot + ".plot";
		std::ofstream plotOfs(plotFile.c_str());
		
		std::string line;
		while (getline(tplot, line)) {
			std::string newline = wft_replaceString(line, "{{{ROOT}}}", inputOutputFileRoot);
			plotOfs << newline << std::endl;
		}

		tplot.close();
		plotOfs.close();

		//绘图
		std::string cmd1 = std::string("gnuplot ");
		cmd1 += plotFile;
		int res1 = std::system(cmd1.c_str());
		std::cout << "Gnuplot result code:" << res1 << std::endl;
	}

	{//输出xml
		std::ofstream xmlStream(serverProvideXmlPath.c_str() );
		xmlStream << "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>" << std::endl;
		xmlStream << "<XML identify=\"productoutput\">" << std::endl;
		xmlStream << "<OutputFiles>" << std::endl;
		xmlStream << "<OutputFile>" << std::endl;

		xmlStream << "<InputFilename>" << inputInputFilename.c_str() << "</InputFilename>" << std::endl;
		xmlStream << "<referenceFilename>" << inputRefFilename.c_str() << "</referenceFilename>" << std::endl;
		xmlStream << "<OutputFilename>" << outputbiasfile.c_str() << "</OutputFilename>" << std::endl;//输出偏差tif数据
		xmlStream << "<OutputFilename>" << histPlotPng.c_str() << "</OutputFilename>" << std::endl;//直方图图片
		xmlStream << "<OutputFilename>" << scatterPlotPng.c_str() << "</OutputFilename>" << std::endl;//散点图图片
		xmlStream << "<OutputFilename>" << fenbuPlotPng.c_str() << "</OutputFilename>" << std::endl;//分布图图片地球图

		xmlStream << "<Deviationstatic bechekVariety=\"" << xp1 << "\" checkVariety = \"" << xp2
			<< "\" dataFormat = \"" << xp3 <<
			"\" dataType = \""<< xp4 << 
			"\" productDate = \""<< xp5
			<<"\" productVariety = \""<< xp6 << "\" " <<
			" values=\"{AVBIAS:"<<averageAbsBias
			<<",CORR:"<<corr<<",KURT:"<<kurt
			<<",MAX:"<<biasMaxValue<<",MEDIAN:"<<middle
			<<",MIN:"<<biasMinValue<<",RMSE:"<<rmse<<",SKEW:"<<skew<< "}\"/>" << std::endl;

		xmlStream << "<OutputFilename>" << outputhistfile.c_str() << "</OutputFilename>" << std::endl;//matched point
		xmlStream << "<OutputFilename>" << lonlatbiasfile.c_str() << "</OutputFilename>" << std::endl; //绘制地球的lonlatbias数据
		xmlStream << "<OutputFilename>" << biasDistFilepath.c_str() << "</OutputFilename>" << std::endl;//直方图数据
		xmlStream << "<OutputFilename>" << scaFilepath.c_str() << "</OutputFilename>" << std::endl;//散点图数据

		xmlStream << "<matched_count>" << (size_t)validPixelCount << "</matched_count>" << std::endl;
		xmlStream << "<input_min_value>" << inputMinValue << "</input_min_value>" << std::endl;
		xmlStream << "<input_max_value>" << inputMaxValue << "</input_max_value>" << std::endl;
		xmlStream << "<input_average>" << averageInputValue << "</input_average>" << std::endl;
		xmlStream << "<ref_min_value>" << refMinValue << "</ref_min_value>" << std::endl;
		xmlStream << "<ref_max_value>" << refMaxValue << "</ref_max_value>" << std::endl;
		xmlStream << "<ref_average>" << averageRefValue << "</ref_average>" << std::endl;
		xmlStream << "<average_bias>" << averageBias << "</average_bias>" << std::endl;
		xmlStream << "<average_abs_bias>" << averageAbsBias << "</average_abs_bias>" << std::endl;
		xmlStream << "<rmse>" << rmse << "</rmse>" << std::endl;
		xmlStream << "<corr>" << corr << "</corr>" << std::endl;
		xmlStream << "<skew>" << skew << "</skew>" << std::endl;
		xmlStream << "<kurt>" << kurt << "</kurt>" << std::endl;
		xmlStream << "<bias_median>" << middle << "</bias_median>" << std::endl;
		xmlStream << "<stddev>" << stddev << "</stddev>" << std::endl;
		xmlStream << "<linearslope>" << linearSlope << "</linearslope>" << std::endl;
		xmlStream << "<linearintercept>" << linearIntercept << "</linearintercept>" << std::endl;
		xmlStream << "<OutputFile>" << std::endl;
		xmlStream << "<OutputFiles>" << std::endl;
		xmlStream << "</XML>" << std::endl;
		xmlStream.close();

	}



	std::cout << "done." << std::endl;

    return 0;
}



/*
计算线性拟合公式
std::vector<double> wft_linear_fit(const std::vector<T>& data)
{
T xSum = 0, ySum = 0, xxSum = 0, xySum = 0, slope, intercept;
std::vector<T> xData;
for (long i = 0; i < data.size(); i++)
{
xData.push_back(static_cast<T>(i));
}
for (long i = 0; i < data.size(); i++)
{
xSum += xData[i];
ySum += data[i];
xxSum += xData[i] * xData[i];
xySum += xData[i] * data[i];
}
slope = (data.size() * xySum - xSum * ySum) / (data.size() * xxSum - xSum * xSum);
intercept = (ySum - slope * xSum) / data.size();
std::vector<T> res;
res.push_back(slope);
res.push_back(intercept);
return res;
}
*/





/*
之前版本的命令行说明，不再使用。
if (argc != 32)
{
std::cout << "*** Program description:" << std::endl;
std::cout << "A program to do baseimage(Lon Lat Lut) subtract refimage(Equal lon,lat coordinate) and compute related bias et al." << std::endl;
std::cout << "Version 0.1.3a 2017-9-15 wangfengdev@163.com" << std::endl;
std::cout << "*** Sample call: ***" << std::endl;
std::cout << "qcbylonlatlut basedatasetpath basedataValid0 basedataValid1 basedataScale basedataOffset  baselon.tif baselat.tif " << std::endl;
std::cout << "\t refdatasetpath refValid0 refValid1 refdataScale refdataOffset refLeftLon(左端像素经度值不是像素中心经度，后同) "
"refRightLon refTopLat(顶端像素边缘纬度值不是像素中心纬度值，后同) refBottomLat outputFileRoot(输出根路径) "
" outfilledValue（填充值） outNotMatchedValue（未匹配填充值） productType[sst]（质检产品代码，用于从extrafile中取得相关参数）"
" coastlinefilepath(海岸线txt文件) inputfile（输入文件名无路径） reffile（参考文件名无路径） "
" xbv（bechekVariety） xcv（checkVariety） xdf（dataFormat） xdt（GERO） xpd（productDate) xpv(productVariety)  " // 24 -- 29
" thetargetresultxmlfilepath（服务器传入的结果xml保存路径） " //30
" extrafilepath（额外配置信息文件路径） " //额外配置信息文件路径
<< std::endl;
std::cout << "*** Error: no enough parameters. out." << std::endl;
return 1001;
}

*/


/*
老版绘图plot代码，不够灵活，不再使用。
std::string palletes;
int maxcolors = 10;

std::string plotFile = inputOutputFileRoot + ".plot";
std::ofstream plotOfs(plotFile.c_str());

plotOfs << "#输出直方图" << std::endl;
plotOfs << "reset"<< std::endl;
plotOfs << "set terminal png size 800, 600" << std::endl;
plotOfs << "set output \""<< histPlotPng <<"\""<< std::endl;
plotOfs << "set xrange[-5:5]" << std::endl;
plotOfs << "binwidth = 5" << std::endl;
plotOfs << "plot '"<< biasDistFilepath <<"' smooth freq with boxes" << std::endl;
plotOfs << "unset output" << std::endl;

plotOfs << "#输出散点图" << std::endl;
plotOfs << "reset" << std::endl;
plotOfs << "set terminal pngcairo size 800,600" << std::endl;
plotOfs << "set output \""<< scatterPlotPng << "\"" << std::endl;
plotOfs << "#拟合曲线" << std::endl;
plotOfs << "f(x)=a*x+b" << std::endl;
plotOfs << "a=1." << std::endl;
plotOfs << "b=1." << std::endl;
plotOfs << "fit f(x) \"" << scaFilepath  <<"\" using 1:2 via a,b" << std::endl;
plotOfs << "set xlabel 'input'" << std::endl;
plotOfs << "set ylabel 'ref'" << std::endl;
plotOfs << "fiteq=sprintf('f(x)=%.2f*x+%.2f' , a , b) ;" << std::endl;
plotOfs << "set label 1 fiteq at graph 0.1 , 0.9" << std::endl;
plotOfs << "#设置高宽比" << std::endl;
plotOfs << "#set size ratio 0.9" << std::endl;
plotOfs << "plot \""<< scaFilepath<<"\" using 1:2 , f(x) with lines linecolor 7" << std::endl;
plotOfs << "unset output" << std::endl;

plotOfs << "#绘制分布图" << std::endl;
plotOfs << "reset" << std::endl;
plotOfs << "set terminal pngcairo size 1000,800" << std::endl;
plotOfs << "set output \""<< fenbuPlotPng <<"\"" << std::endl;
plotOfs << "set mapping spherical" << std::endl;
plotOfs << "set angles degrees" << std::endl;
plotOfs << "set hidden3d front" << std::endl;
plotOfs << "set parametric" << std::endl;
plotOfs << "set view 90,195" << std::endl;
plotOfs << "set isosamples 30" << std::endl;//经纬度虚线格网的数量
plotOfs << "set xyplane at -1" << std::endl;
plotOfs << "set origin -0.26,-0.38" << std::endl;
plotOfs << "set size 1.4,1.75" << std::endl;
plotOfs << "unset xtics" << std::endl;
plotOfs << "unset ytics" << std::endl;
plotOfs << "unset ztics" << std::endl;
plotOfs << "#隐藏坐标轴" << std::endl;
plotOfs << "unset border " << std::endl;
plotOfs << "#设置colorbar位置" << std::endl;
plotOfs << "set colorbox vertical user origin 0.9 , .1 size .04,.8" << std::endl;
plotOfs << "set urange[0:360]" << std::endl;
plotOfs << "set vrange[-90:90]" << std::endl;
plotOfs << "set cbrange[-5:5]" << std::endl;
plotOfs << "set xrange[-0.95:0.95]" << std::endl;
plotOfs << "set yrange[-1:1]" << std::endl;
plotOfs << "set zrange[-1:1]" << std::endl;
plotOfs << "set pal maxcolors " << plotMaxColors << std::endl;
plotOfs << "set palette defined "<< plotPalette << std::endl;
plotOfs << "r = 0.99"<<std::endl ;
plotOfs << "splot \""<< lonlatbiasfile <<"\" using 1:2:(1):3 with pm3d,r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines ls 0,'"
<< inputCoastlineFilepath << "' u 1:2:(1) with lines ls 2" << std::endl;
plotOfs << "unset output" << std::endl;
*/