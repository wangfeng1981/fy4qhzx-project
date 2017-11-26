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
//2.0a 增加基于Geotiff GeoTrans的比较，去掉QCS多余参数的输出。2017-10-26


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

-type geotrans -inf E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.mosic.tif -reff E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.modvrt.wgs84.tif -inds E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.mosic.tif -refds E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.modvrt.wgs84.tif -inlon E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.fy3lon.tif -inlat E:/testdata/testdatandvi/22/FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF.fy3lat.tif -outroot E:/testdata/testdatandvi/23new/testbil -iv0 -10000 -iv1 10000 -rv0 -2000 -rv1 10000 -isca 0.0001 -iofs 0 -rsca 0.0001 -rofs 0  -fill -99 -nomat -88 -pcode f3mndvi -coast null  -xbk x1 -xcv x2 -xdf x3 -xge x4 -xpd x5 -xpv x6 -outxml cdxml-nn.xml  -plot E:/coding/fy4qhzx-project/extras/ndvi.plot -matchmethod bil
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
using namespace std;

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
	bool tryGetValueByImagePixelXY(int imgx, int imgy,double& val);
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
bool DataBlock::tryGetValueByImagePixelXY(int imgx, int imgy, double& val)
{
	if (isInsideImage(imgx, imgy)) {
		val = getValueByImagePixelXY(imgx, imgy);
		return true;
	}
	else {
		return false;
	}
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


///
///v00 左上角值，v10 x坐标+1 ， v01 y坐标+1 ， v11 右下角坐标值
///
/// v00 v10
/// v01 v11
///
double wft_bilinear_interpol(double xx, double yy, double x0, double y0, double rx, double ry,
	double v00, double v10, double v01, double v11)
{
	double tval0 = v00 + (xx - x0) / rx * (v10 - v00);
	double tval1 = v01 + (xx - x0) / rx * (v11 - v01);
	double tval = tval0 + (yy - y0) / ry  * (tval1 - tval0);
	return tval;
}



void wft_make_histfile(double vmin, double vmax, double binwid, std::vector<float> vec, std::string outfile)
{
	double tmin = ((int)(vmin / binwid))*binwid;
	double tmax = (1 + (int)(vmax / binwid))*binwid;
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
		histArray[i] = histArray[i] / vsize * 100 ;
		histOfs << (tmin + i*binwid) << " " << histArray[i] << std::endl;
	}
	histOfs.close();
	delete[] histArray;
}




int main(int argc , char** argv )
{
	std::cout << "Version 0.6a " << std::endl;
	std::cout << "by wangfengdev@163.com 2017 - 9 - 21. " << std::endl;
	cout << "Version 2.0 . add support of GeoTrans match, remove qcs staff. 2017-10-26." << endl;
	cout << "Version 2.0.1. add correlation output. 2017-10-26." << endl;
	cout << "Version 2.0.2. 2017-10-30. bugfixed for reftype ingeotrans." << endl;
	if (argc == 1 )
	{
		std::cout << "Sample call:" << std::endl;
		std::cout << "qcbylonlatlut  -in inds.tif -ref refds.tif " <<endl<<
			" -intype llrange/llfiles/geotrans  " << endl <<
			" -reftype llrange/geotrans " <<endl << 
			"[ -inlon inlon.tif -inlat inlat.tif ] *only for -intype=llfiles " <<endl <<
			"[ -inleft -180 -inright 180 -intop 90 -inbottom -90 ] *only for -intype=llrange  " << endl <<
			"[ -refleft -180 -refright 180 -reftop 90 -refbottom -90 ] *only for -reftype=llrange " << endl <<
			" -outroot outroot.tif "<<endl<<
			" -iv0 -10000 -iv1 10000 -rv0 -2000 -rv1 10000  "
			" -isca 0.0001 -iofs 0 -rsca 0.0001 -rofs 0 "
			" -fill -999  "<<endl<<
			" [-matchmethod nn/bil/aver] "<<endl<<
			" [-irx 0.04 -iry 0.04] *only for -matchmethod=aver "<<endl<< //输入数据分辨率单位度，可选，仅在匹配模式为aver时有用，如果没有这两个值默认使用3x3像元平均
			" [-biasbinwid 0.1] "  //直方图宽度，可选
			" [-databinwid 0.1] "  //数据直方图宽度
			<< std::endl ;
		std::cout << std::endl;
	
		std::cout << "outputs:" << std::endl;
		std::cout << "root.biasimg.tif \t root.matchedpoints.txt \t root.bias.llv.txt " << std::endl;
		std::cout << "root.bhist.txt \t root.rhist.txt \t root.ihist.txt " << std::endl;
		std::cout << "root.scatter.txt \t root.linear.txt " << std::endl;

		std::cout << "*** Error: no enough parameters. out." << std::endl;
		return 1001;
	}


	std::string inFile , refFile ;
	wft_has_param(argc, argv, "-in", inFile, true);
	wft_has_param(argc, argv, "-ref", refFile, true);

	//type
	std::string intype, reftype;
	wft_has_param(argc, argv, "-intype", intype, true);
	//wft_has_param(argc, argv, "-reftype", intype, true);//bug 2017-10-30
	wft_has_param(argc, argv, "-reftype", reftype, true);//bugfixed 2017-10-30
	double inleft, inright, intop, inbottom;
	string inlonfile, inlatfile;
	int inlltype = 0;
	if (intype == "llrange")
	{
		inlltype = 0;
		std::string temp;
		wft_has_param(argc, argv, "-inleft", temp, true);
		inleft = atof(temp.c_str());
		wft_has_param(argc, argv, "-inright", temp, true);
		inright = atof(temp.c_str());
		wft_has_param(argc, argv, "-intop", temp, true);
		intop = atof(temp.c_str());
		wft_has_param(argc, argv, "-inbottom", temp, true);
		inbottom = atof(temp.c_str());
	}
	else if (intype == "llfiles")
	{
		inlltype = 1;
		wft_has_param(argc, argv, "-inlon", inlonfile , true);
		wft_has_param(argc, argv, "-inlat", inlatfile , true);
	}
	else
	{
		inlltype = 2;

	}

	double refleft, refright, reftop, refbottom;
	int reflltype = 0;
	if (reftype == "llrange")
	{
		reflltype = 0;
		std::string temp;
		wft_has_param(argc, argv, "-refleft", temp, true);
		refleft = atof(temp.c_str());
		wft_has_param(argc, argv, "-refright", temp, true);
		refright = atof(temp.c_str());
		wft_has_param(argc, argv, "-reftop", temp, true);
		reftop = atof(temp.c_str());
		wft_has_param(argc, argv, "-refbottom", temp, true);
		refbottom = atof(temp.c_str());
	}
	else
	{
		reflltype = 2;

	}



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

	 //refdatascale
	 double biasHistWid = 0.1;
	 double dataHistWid = 0.1;
	 {
		 std::string temp;
		 if( wft_has_param(argc, argv, "-biasbinwid", temp, false ) )
			 biasHistWid = atof(temp.c_str());
		 if( wft_has_param(argc, argv, "-databinwid", temp, false) )
			 dataHistWid = atof(temp.c_str());
	 }


	//refdataoffset
	 double inputrefDataOffset = 0;
	 {
		 std::string temp;
		 wft_has_param(argc, argv, "-rofs", temp, true);
		 inputrefDataOffset = atof(temp.c_str());
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

	 //参考数据匹配方法 matched methods.
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
	 //输入影像分辨率，单位度
	 double inputResoX = 0.01;
	 double inputResoY = 0.01;
	 {
		 std::string irx, iry;
		 bool hasIRX = wft_has_param(argc, argv, "-irx", irx, false);
		 bool hasIRY = wft_has_param(argc, argv, "-iry", iry, false);
		 if (hasIRX) inputResoX = atof(irx.c_str());
		 if (hasIRY) inputResoY = atof(iry.c_str());
	 }

	std::string outputbiasfile = inputOutputFileRoot + ".biasimg.tif";
	std::string outputhistfile = inputOutputFileRoot + ".matchedpoints.txt";
	std::string lonlatbiasfile = inputOutputFileRoot + ".bias.llv.txt";

	std::ofstream outHistStream(outputhistfile.c_str());
	std::ofstream outLonLatBiasStream(lonlatbiasfile.c_str());
	//打印表头
	outHistStream << "#ibx iby inlon inlat inval refx refy match interval val00 val10 val01 val11 bias" << std::endl;
	outHistStream << "#inlon inlat bias" << std::endl;


	GDALAllRegister();

	GDALDataset* baseDataDs = (GDALDataset*)GDALOpen(inFile.c_str(), GDALAccess::GA_ReadOnly);
	int baseXSize = baseDataDs->GetRasterXSize();
	int baseYSize = baseDataDs->GetRasterYSize();
	double inGeoTrans[6];
	GDALDataset* inLonDs = 0; 
	GDALDataset* inLatDs = 0; 
	if (inlltype == 0)
	{
		inGeoTrans[2] = 0;
		inGeoTrans[1] = (inright - inleft) / baseXSize;
		//fix referenceGeoTransform[0] = refLeftLon + referenceGeoTransform[1] / 2;//fix 原点坐标使用边缘坐标，不再使用中心坐标
		// inGeoTrans[0] = refleft; bug 2017-10-30
		inGeoTrans[0] = inleft ; // bugfixed 2017 - 10 - 30

		inGeoTrans[5] = (inbottom - intop) / baseYSize;
		inGeoTrans[4] = 0;
		//fix referenceGeoTransform[3] = refTopLat + referenceGeoTransform[5] / 2;//fix 原点坐标使用边缘坐标，不再使用中心坐标
		inGeoTrans[3] = intop;
	}
	else if (inlltype == 1)
	{
		inLonDs = (GDALDataset*)GDALOpen(inlonfile.c_str(), GDALAccess::GA_ReadOnly);
		inLatDs = (GDALDataset*)GDALOpen(inlatfile.c_str(), GDALAccess::GA_ReadOnly);
		{
			int xs = inLonDs->GetRasterXSize();
			int ys = inLonDs->GetRasterYSize();
			int x1 = inLatDs->GetRasterXSize();
			int y1 = inLatDs->GetRasterYSize();
			if (baseXSize != xs || baseXSize != x1 || baseYSize != ys || baseYSize != y1)
			{
				std::cout << "*** Error: base data size is not equal with lon or lat data size . out." << std::endl;
				return 1002;
			}
		}
	}
	else
	{
		baseDataDs->GetGeoTransform(inGeoTrans);
	}

	GDALDataset* refDataDs = (GDALDataset*)GDALOpen(refFile.c_str(), GDALAccess::GA_ReadOnly);
	int refXSize = refDataDs->GetRasterXSize();
	int refYSize = refDataDs->GetRasterYSize();

	double referenceGeoTransform[6];
	//GDAL default transform is (0,1,0,0,0,1) 
	//Xp(lon) = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
	//Yp(lat) = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];

	if( reflltype ==  0 )
	{
		referenceGeoTransform[2] = 0;
		referenceGeoTransform[1] = (refright - refleft) / refXSize;
		//fix referenceGeoTransform[0] = refLeftLon + referenceGeoTransform[1] / 2;//fix 原点坐标使用边缘坐标，不再使用中心坐标
		referenceGeoTransform[0] = refleft;

		referenceGeoTransform[5] = (refbottom - reftop) / refYSize ;
		referenceGeoTransform[4] = 0 ;
		//fix referenceGeoTransform[3] = refTopLat + referenceGeoTransform[5] / 2;//fix 原点坐标使用边缘坐标，不再使用中心坐标
		referenceGeoTransform[3] = reftop;
	}
	else if ( reflltype == 2 )
	{
		refDataDs->GetGeoTransform(referenceGeoTransform);
		//fix 原点坐标使用边缘坐标，不再使用中心坐标 referenceGeoTransform[0] = referenceGeoTransform[0] + referenceGeoTransform[1] / 2;
		//fix 原点坐标使用边缘坐标，不再使用中心坐标 referenceGeoTransform[3] = referenceGeoTransform[3] + referenceGeoTransform[5] / 2;
	}
	else {
		std::cout << "Error : Unsupported type " << intype << std::endl;
		exit(91);
	}
	double refResoX = referenceGeoTransform[1];
	double refResoY = referenceGeoTransform[5];
	int averWndWid = fabs(inputResoX / refResoX);
	int averWndHei = fabs(inputResoY / refResoY);
	//计算参考数据平均值窗口大小
	if (averWndWid % 2 == 0) averWndWid += 1;
	if (averWndHei % 2 == 0) averWndHei += 1;
	int halfAverWndWid = averWndWid / 2;
	int halfAverWndHei = averWndHei / 2;


	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputBiasDataset = driver->Create(outputbiasfile.c_str(), baseXSize, baseYSize , 1, GDALDataType::GDT_Float32, 0);

	float * outBiasRowValues = new float[baseXSize];

	//统计信息
	//std::vector<WTriple> dataTripleVector;
	std::vector<float> biasVector;
	std::vector<float> inDataVec, refDataVec;
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
	DataBlock* ptrLonDataBlock = 0;
	DataBlock* ptrLatDataBlock = 0;
	if (inlltype == 1)
	{
		ptrLonDataBlock = new DataBlock(inLonDs->GetRasterBand(1), 200);
		ptrLatDataBlock = new DataBlock(inLatDs->GetRasterBand(1), 200);
	}
	

	int lonlatbiasStridex = 1;
	int lonlatbiasStridey = 1;
	if (baseXSize > 1000 ) {
		lonlatbiasStridex = baseXSize / 1000;
	}
	if (baseYSize > 1000 ) {
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
			double blonValue = 0;
			double blatValue = 0;
			if (inlltype == 1)
			{
				blonValue = ptrLonDataBlock->getValueByImagePixelXY(ibx, iby);//中心坐标
				blatValue = ptrLatDataBlock->getValueByImagePixelXY(ibx, iby);//中心坐标
			}
			else
			{
				blonValue = inGeoTrans[0] + ibx * inGeoTrans[1] + iby * inGeoTrans[2];
				blatValue = inGeoTrans[3] + ibx * inGeoTrans[4] + iby * inGeoTrans[5];
				//Xp(lon) = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
				//Yp(lat) = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
			}

			bool isRefMatched = false;
			if (baseValue >= inputbaseDataValid0 && baseValue <= inputbaseDataValid1 && blonValue>-998 && blatValue>-998 )
			{
				double tempInputValue = baseValue * inputbaseDataScale + inputbaseDataOffset;
				outBiasRowValues[ibx] = inputOutFilledValue ;
				const double* t = referenceGeoTransform;
				int targetRefX = (int)round((blonValue*t[5] - blatValue*t[2] - t[0] * t[5] + t[2] * t[3]) / (t[1] * t[5] - t[2] * t[4]));
				int targetRefY = (int)round((blonValue*t[4] - blatValue*t[1] - t[0] * t[4] + t[1] * t[3]) / (t[2] * t[4] - t[1] * t[5]));

				double refMatchedValue = 0;
				if ( 
						(targetRefX >= 0 && targetRefX < refXSize  && targetRefY >= 0 && targetRefY < refYSize) 
						&&
						( inputrefDataValid0<=refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY)
							&& refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY) <= inputrefDataValid1 
						)
					)
				{//参考数据最邻近像元必须有效才能继续后续运算 the reference pixel must available then move next.
					isRefMatched = true;
					if (matchMethod == 0)
					{//最邻近法
						refMatchedValue = refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY);
					}
					else if (matchMethod == 1)
					{//双线性法
						refMatchedValue = refDataBlock.getValueByImagePixelXY(targetRefX, targetRefY);
						double refnnCenterX = targetRefX * refResoX + refResoX/2 + t[0] ;
						double refnnCenterY = targetRefY * refResoY + refResoY / 2 + t[3] ;
						int nget = 0;
						double v00, v10, v01, v11;
						double refwndlon0 = 0;
						double refwndlat0 = 0;
						if (blonValue <= refnnCenterX && blatValue <= refnnCenterY)
						{//Quater1 left top
							refwndlon0 = refnnCenterX - refResoX;
							refwndlat0 = refnnCenterY - refResoY;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX - 1, targetRefY - 1, v00) && (inputrefDataValid0 <= v00 && v00 <= inputrefDataValid1) ) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX    , targetRefY - 1, v10) && (inputrefDataValid0 <= v10 && v10 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX - 1, targetRefY    , v01) && (inputrefDataValid0 <= v01 && v01 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX    , targetRefY    , v11) && (inputrefDataValid0 <= v11 && v11 <= inputrefDataValid1)) ++nget;
						}else if (blonValue > refnnCenterX && blatValue <= refnnCenterY)
						{//Quater2 right top
							refwndlon0 = refnnCenterX  ;
							refwndlat0 = refnnCenterY - refResoY;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX    , targetRefY - 1, v00) && (inputrefDataValid0 <= v00 && v00 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX + 1, targetRefY - 1, v10) && (inputrefDataValid0 <= v10 && v10 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX    , targetRefY, v01) && (inputrefDataValid0 <= v01 && v01 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX + 1, targetRefY, v11) && (inputrefDataValid0 <= v11 && v11 <= inputrefDataValid1)) ++nget;
						}
						else if (blonValue <= refnnCenterX && blatValue > refnnCenterY)
						{//Quater3 left bottom
							refwndlon0 = refnnCenterX - refResoX;
							refwndlat0 = refnnCenterY ;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX - 1, targetRefY , v00) && (inputrefDataValid0 <= v00 && v00 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX    , targetRefY , v10) && (inputrefDataValid0 <= v10 && v10 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX - 1, targetRefY+1, v01) && (inputrefDataValid0 <= v01 && v01 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX    , targetRefY+1, v11) && (inputrefDataValid0 <= v11 && v11 <= inputrefDataValid1)) ++nget;
						}
						else {
							//Quater4 right bottom
							refwndlon0 = refnnCenterX;
							refwndlat0 = refnnCenterY;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX    , targetRefY, v00) && (inputrefDataValid0 <= v00 && v00 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX + 1, targetRefY, v10) && (inputrefDataValid0 <= v10 && v10 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX    , targetRefY + 1, v01) && (inputrefDataValid0 <= v01 && v01 <= inputrefDataValid1)) ++nget;
							if (refDataBlock.tryGetValueByImagePixelXY(targetRefX + 1, targetRefY + 1, v11) && (inputrefDataValid0 <= v11 && v11 <= inputrefDataValid1)) ++nget;
						}
						if (nget == 4)
						{
							refMatchedValue = wft_bilinear_interpol(blonValue,blatValue,refwndlon0,refwndlat0,refResoX,refResoY,v00,v10,v01,v11);
						}
					}
					else {
						//平均值法
						int nget = 0;
						for (int wy = -halfAverWndHei; wy <= halfAverWndHei; ++wy)
						{
							for (int wx = -halfAverWndWid; wx <= halfAverWndWid; ++wx)
							{
								double tempval = 0;
								int iwx = targetRefX + wx;
								int iwy = targetRefY + wy;
								if (refDataBlock.tryGetValueByImagePixelXY(iwx, iwy, tempval)
									&& (inputrefDataValid0 <= tempval && tempval <= inputrefDataValid1))
								{
									++nget;
									refMatchedValue += tempval;
								}
							}
						}
						refMatchedValue = refMatchedValue/nget ;
					}
				}
				
				if (isRefMatched > 0)
				{//找到匹配值
					
					double tempRefValue = refMatchedValue * inputrefDataScale + inputrefDataOffset;
					double bias = tempInputValue - tempRefValue;

					//统计信息
					++validPixelCount;
					fitxySum += tempInputValue * tempRefValue;
					fitxSum += tempInputValue;
					fitySum += tempRefValue;
					fitxxSum += tempInputValue*tempInputValue ;

					biasVector.push_back(bias);
					inDataVec.push_back(tempInputValue);
					refDataVec.push_back(tempRefValue);

					averageInputValue += tempInputValue;
					averageRefValue += tempRefValue;
					averageBias += bias;
					averageAbsBias += ABS(bias);
					rmse += bias * bias;
					if (tempInputValue > inputMaxValue) {
						inputMaxValue = tempInputValue;
					}
					if (tempInputValue < inputMinValue) {
						inputMinValue = tempInputValue;
					}
					if (tempRefValue< refMinValue)
					{
						refMinValue = tempRefValue;
					}
					if (tempRefValue > refMaxValue)
					{
						refMaxValue = tempRefValue;
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
							<< tempRefValue << " "
							<< bias << std::endl;
						lonLatBiasBuffer << blonValue << " " << blatValue << " " << bias << std::endl;
					}
				}
				
			}
			if (isRefMatched == false )
			{
				if (blonValue>-998 && blatValue>-998 ) {
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

	GDALClose(baseDataDs);  
	delete ptrLatDataBlock;
	delete ptrLonDataBlock;
	GDALClose(inLonDs);
	GDALClose(inLatDs);

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
		size_t numPixels = inDataVec.size();
		for (int i = 0; i < numPixels; ++ i )
		{
			double cz = biasVector[i] - averageBias;
			stddev += cz*cz;
		}
		stddev = sqrt(stddev / validPixelCount);

		double corr_0 = 0;
		double corr_1 = 0;
		double corr_2 = 0;
		size_t curr = 0;
		for (int i = 0; i < numPixels; ++i)
		{
			double cz0 = inDataVec[i] - averageInputValue;
			double cz1 = refDataVec[i] - averageRefValue;
			double czbias = biasVector[i] - averageBias;
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
		std::cout << "sorting bias data for median value ..." << std::endl;
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

	std::string biashistfile  = inputOutputFileRoot + ".bhist.txt";
	std::string inhistfile = inputOutputFileRoot + ".ihist.txt";
	std::string refhistfile = inputOutputFileRoot + ".rhist.txt";
	std::string scaFilepath = inputOutputFileRoot + ".scatter.txt";
	std::string linearEquationFile = inputOutputFileRoot + ".linear.txt";
	//输出直方图
	{
		//线性公式
		std::ofstream linearStream(linearEquationFile.c_str());
		linearStream << "#slope" << std::endl;
		linearStream << linearSlope << std::endl;
		linearStream << "#inter" << std::endl;
		linearStream << linearIntercept << std::endl;
		linearStream << "#corr" << endl;
		linearStream << corr << endl;
		linearStream.close();

		//散点图
		std::ofstream scaOfs(scaFilepath.c_str());
		scaOfs << "#invalue  refvalue" << std::endl;
		size_t pxCount = biasVector.size();
		int scaStride = pxCount / 200000;
		for (size_t it = 0; it < pxCount; ++it)
		{
			if (it % scaStride == 0) {
				scaOfs << inDataVec[it] << " " << refDataVec[it] << std::endl;
			}
			wft_term_progress(it, pxCount);
		}
		scaOfs.close();

		wft_make_histfile(biasMinValue, biasMaxValue, biasHistWid, biasVector, biashistfile);
		wft_make_histfile(inputMinValue, inputMaxValue, dataHistWid, inDataVec, inhistfile);
		wft_make_histfile(refMinValue, refMaxValue, dataHistWid, refDataVec , refhistfile);
		
	}

	std::cout << "done." << std::endl;

    return 0;
}



