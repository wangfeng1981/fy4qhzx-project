// qcfy3ndvi.cpp : 定义控制台应用程序的入口点。
//风三与NOAA质检程序。
//wangfeng1@piesat.cn 2017-10-1


 
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <cassert>
#include "../../sharedcodes/wftools.h"
using namespace std;



struct MatchData
{
	//float noaaLonc, noaaLatc, fy3lon, fy3lat;
	float noaaNdvi, fy3AverNdvi , fy3min , fy3max ;
	float bias, abias , biasm ;
	float nclon, nclat , fclon,fclat ;
	float fy3stdev;
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





int main(int argc , char** argv )
{
	std::cout << "A program to do qc work for fy3 ndvi with noaa ndvi." << std::endl;
	std::cout << "by wangfengdev@163.com 2017-10-1. " << std::endl;
	std::cout << "Version 0.1a 2017-10-1" << endl;
	std::cout << "Version 0.1.1a 2017-10-2" << endl;
	std::cout << "Version 0.1.2a 2017-10-9 , output stdev." << endl;
	std::cout << "Version 0.2a 2017-10-9 , output bias tif." << endl;
	std::cout << "Version 0.2.1a 2017-10-9 , output bias tif." << endl;
	std::cout << "Version 0.3a 2017-10-9 , add linear correction." << endl;
	std::cout << "Version 0.3.1a 2017-10-9 , add fy3 win min,max output." << endl;
	std::cout << "Version 0.4a 2017-10-10 , use three fit linear equation do corrention." << endl;
	std::cout << "Version 0.4.1a 2017-10-10 , testbias bugfix." << endl;
	std::cout << "Version 0.5a 2017-10-10 16:00 , output fy3 window aver ndvi to tif(with similar size and resolution like noaa)." << endl;
	std::cout << "Version 0.6a 2017-10-11. outwndaverfile include unmatched fy3b values." << endl;
	
	
	if (argc == 1 )
	{
		std::cout << "Sample call:" << std::endl;
		std::cout << "qcfy3ndvi -fy3 fy3.tif -noaa noaa.tif " 
			" -outroot outrootfile "
			" -plot draw.plot "
			" [-binwid 0.1] "  //直方图宽度，可选
			" [-fill -999] "  //填充值可选
			" [-maxstdev 0.1/0.05/0.01] "
			" [-lcslope 1.0 -lcinter 0.0 ] "     //默认数据使用lcslope和lcinter进行订正
			" [-lc0slope 1.0 -lc0inter 0.0 ] " //如果设置了lc0曲线,那么偏差小于-0.1时使用这个曲线
			" [-lc1slope 1.0 -lc1inter 0.0 ] " //如果设置了lc1曲线，那么偏差大于+0.1是使用这个曲线
			" [-outwndaverfile fy3wndaverndvi.tif] " //用于输出线性校正后的fy3植被指数数据，分辨率和尺寸与noaa一致

			<< std::endl ;
		std::cout << std::endl;


		std::cout << "outputs:" << std::endl;


		std::cout << "*** Error: no enough parameters. out." << std::endl;
		return 1001;
	}

	//type
	std::string fy3file , noaafile , outrootfile ;
	wft_has_param(argc, argv, "-fy3", fy3file , true);
	wft_has_param(argc, argv, "-noaa", noaafile, true);
	wft_has_param(argc, argv, "-outroot", outrootfile, true);
	double binWid = 0.1;
	string binWidStr;
	if(wft_has_param(argc, argv, "-binwid", binWidStr, false ) ) {
		binWid = atof(binWidStr.c_str());
	}
	double biasbinWid = 0.01;
	string bbinWidStr;
	if (wft_has_param(argc, argv, "-biasbinwid", bbinWidStr, false)) {
		biasbinWid = atof(bbinWidStr.c_str());
	}

	double linearCorrSlope = 1.0;
	double linearCorrInter = 0.0;
	string tempLC;
	if (wft_has_param(argc, argv, "-lcslope", tempLC, false)) {
		linearCorrSlope = atof(tempLC.c_str());
	}
	if (wft_has_param(argc, argv, "-lcinter", tempLC, false)) {
		linearCorrInter = atof(tempLC.c_str());
	}

	double lc0Slope = 1.0;
	double lc0Inter = 0.0;
	if (wft_has_param(argc, argv, "-lc0slope", tempLC, false)) {
		lc0Slope = atof(tempLC.c_str());
	}
	if (wft_has_param(argc, argv, "-lc0inter", tempLC, false)) {
		lc0Inter = atof(tempLC.c_str());
	}
	double lc1Slope = 1.0;
	double lc1Inter = 0.0;
	if (wft_has_param(argc, argv, "-lc1slope", tempLC, false)) {
		lc1Slope = atof(tempLC.c_str());
	}
	if (wft_has_param(argc, argv, "-lc1inter", tempLC, false)) {
		lc1Inter = atof(tempLC.c_str());
	}
	std::string outWndAverFile = "";
	if (wft_has_param(argc, argv, "-outwndaverfile", tempLC, false)) {
		outWndAverFile = tempLC ;
	}



	double maxStdev = 0.1;
	{
		string temp;
		if (wft_has_param(argc, argv, "-maxstdev", temp, false))
			maxStdev = atof(temp.c_str());
	}

	double fy3DataValid0 = -10000 ;
	double fy3DataValid1 =  10000 ;
	
	double fy3DataScale = 0.0001  ;
	double fy3DataOffset =  0 ;
	
	double noaaDataValid0 = -1000 ;
	double noaaDataValid1 =  10000;
	double noaaDataScale = 0.0001 ;
	double noaaDataOffset = 0;

	double refLeftLon = -180;
	double refRightLon = 180;
	double refTopLat = 90;
	double refBottomLat = -90;
	
	//filledvalue
	double inputOutFilledValue = 0 ;
	{
		std::string temp;
		if( wft_has_param(argc, argv, "-fill", temp, false ) )
			inputOutFilledValue = atof(temp.c_str());
	}

	string plotFile  ;
	wft_has_param(argc, argv, "-plot", plotFile, true);

	//输入影像分辨率，单位度
	double fy3Rx = 0.01;
	double fy3Ry = 0.01;

	std::string outputbiasfile = outrootfile + ".biasimg.tif";
	//std::string outputhistfile = inputOutputFileRoot + ".matchedpoints.txt";
	//std::string lonlatbiasfile = outrootfile + ".lonlatbias.txt";

	//std::ofstream outHistStream(outputhistfile.c_str());
	//std::ofstream outLonLatBiasStream(lonlatbiasfile.c_str());
	//打印表头
	//outHistStream << "#ibx iby inlon inlat inval refx refy match interval val00 val10 val01 val11 bias" << std::endl;
	//outHistStream << "#inlon inlat bias" << std::endl;


	GDALAllRegister();

	GDALDataset* fy3DataDs = (GDALDataset*)GDALOpen(fy3file.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* noaaDataDs = (GDALDataset*)GDALOpen(noaafile.c_str(), GDALAccess::GA_ReadOnly);


	int fy3XSize = fy3DataDs->GetRasterXSize();
	int fy3YSize = fy3DataDs->GetRasterYSize();

	int noaaXSize = noaaDataDs->GetRasterXSize();
	int noaaYSize = noaaDataDs->GetRasterYSize();

	//GDAL default transform is (0,1,0,0,0,1) 
	//Xp(lon) = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
	//Yp(lat) = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];

	double ftrans[6];
	fy3DataDs->GetGeoTransform(ftrans);

	double llvLon0 = ftrans[0];
	double llvLon1 = llvLon0 + fy3XSize * ftrans[1];
	double llvLat0 = ftrans[3];
	double llvLat1 = llvLat0 + fy3YSize * ftrans[5];


	double ntrans[6];
	int refXSize = noaaDataDs->GetRasterXSize();
	int refYSize = noaaDataDs->GetRasterYSize();
	ntrans[2] = 0;
	ntrans[1] = (refRightLon - refLeftLon) / refXSize;
	ntrans[0] = refLeftLon;
	ntrans[5] = (refBottomLat - refTopLat) / refYSize;
	ntrans[4] = 0;
	ntrans[3] = refTopLat;

	double noaaResoX = ntrans[1];
	double noaaResoY = ntrans[5];

	double wndWid = 5;
	double wndHei = 5;
	


	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputBiasDataset = driver->Create(outputbiasfile.c_str(), refXSize, refYSize, 1, GDALDataType::GDT_Float32, 0);

	double adfGeoTrans[6] = { -180 , 0.05 , 0 , 90 , 0 , -0.05 };
	//
	GDALDataset* outWndAverDataset = nullptr;
	if (outWndAverFile != "") {
		outWndAverDataset = driver->Create(outWndAverFile.c_str(), refXSize, refYSize, 1, GDALDataType::GDT_Float32, 0);

		OGRSpatialReference osrs;
		char* pszSRS_WKT = 0;
		outWndAverDataset->SetGeoTransform(adfGeoTrans);
		osrs.SetWellKnownGeogCS("EPSG:4326");
		osrs.exportToWkt(&pszSRS_WKT);
		outWndAverDataset->SetProjection(pszSRS_WKT);
		CPLFree(pszSRS_WKT);

	}

	{
		OGRSpatialReference osrs;
		char* pszSRS_WKT = 0;
		outputBiasDataset->SetGeoTransform(adfGeoTrans);
		osrs.SetWellKnownGeogCS("EPSG:4326");
		osrs.exportToWkt(&pszSRS_WKT);
		outputBiasDataset->SetProjection(pszSRS_WKT);
		CPLFree(pszSRS_WKT);
	}
	
	float * outBiasRowValues = new float[refXSize];
	float * outWndAverRowValues = new float[refXSize];

	//统计信息
	std::vector<MatchData> matchVector;
	std::vector<double> fVec, nVec, bVec;
	double averageBias = 0.0;
	double averageAbsBias = 0.0;
	double rmse = 0;
	double fy3MaxValue = -99999;
	double fy3MinValue = 99999;
	double fy3AverValue = 0;
	double biasMaxValue = -99999;
	double biasMinValue = 99999;
	double noaaMinValue = 99999;
	double noaaMaxValue = -99999;
	double noaaAverValue = 0;
	double fitxySum = 0;
	double fitxSum = 0;
	double fitySum = 0;
	double fitxxSum = 0;

	DataBlock noaaDataBlock(noaaDataDs->GetRasterBand(1) , 200 );
	DataBlock fy3DataBlock(fy3DataDs->GetRasterBand(1),  200);
	string llvFile = outrootfile + ".llv.tmp";
	ofstream llvOfs(llvFile.c_str());
	llvOfs << "#lon lat bias" << endl;
	for (int iby = 0; iby < noaaYSize; ++iby)
	{
		for (int i = 0; i < refXSize; ++i)
		{
			outBiasRowValues[i] = -9.f;
			outWndAverRowValues[i] = -9.f;
		}
		for (int ibx = 0; ibx < noaaXSize; ++ibx)
		{
			double noaaValue0 = noaaDataBlock.getValueByImagePixelXY(ibx, iby );
			double nlon = ntrans[0] + ibx * ntrans[1] + iby * ntrans[2];
			double nlat = ntrans[3] + ibx * ntrans[4] + iby * ntrans[5];
			double nclon = nlon + 2 * fy3Rx;
			double nclat = nlat + 2 * fy3Ry;
			string tempLLValueStr = "NaN";

			vector<double> vector25;
			double winAverNdvi = 0;
			double winMin = 999;
			double winMax = -999;
			int fy3x = (int)((nclon*ftrans[5] - nclat*ftrans[2] - ftrans[0] * ftrans[5] + ftrans[2] * ftrans[3]) / (ftrans[1] * ftrans[5] - ftrans[2] * ftrans[4]));
			int fy3y = (int)((nclon*ftrans[4] - nclat*ftrans[1] - ftrans[0] * ftrans[4] + ftrans[1] * ftrans[3]) / (ftrans[2] * ftrans[4] - ftrans[1] * ftrans[5]));
			{//add unmatched to outwndaverfile.
				if (fy3x > 1 && fy3x - 2 < fy3XSize && fy3y > 1 && fy3y < fy3YSize - 2)
				{
					for (int ix1 = -2; ix1 <= 2; ++ix1)
					{
						for (int iy1 = -2; iy1 <= 2; ++iy1)
						{
							int ix2 = fy3x + ix1;
							int iy2 = fy3y + iy1;
							double fvalue0 = fy3DataBlock.getValueByImagePixelXY(ix2, iy2);
							if (fy3DataValid0 <= fvalue0 && fvalue0 <= fy3DataValid1)
							{
								double fvalue = fvalue0 * fy3DataScale + fy3DataOffset;
								winAverNdvi += fvalue;
								if (fvalue > winMax) winMax = fvalue;
								if (fvalue < winMin) winMin = fvalue;
								vector25.push_back(fvalue);
							}
						}
					}
					if (vector25.size() > 0)
					{
						winAverNdvi = winAverNdvi / vector25.size();
						outWndAverRowValues[ibx] = winAverNdvi;
					}
					
				}
			}

			if (noaaDataValid0 <= noaaValue0 && noaaValue0 <= noaaDataValid1)
			{
				double noaaNdvi = noaaValue0 * noaaDataScale + noaaDataOffset;

				//Xp(lon) = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
				//Yp(lat) = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
				
				double flon = ftrans[0] + fy3x * ftrans[1] + fy3y * ftrans[2];
				double flat = ftrans[3] + fy3x * ftrans[4] + fy3y * ftrans[5];

				//use 5x5 window
				if (fy3x > 1 && fy3x - 2 < fy3XSize && fy3y > 1 && fy3y < fy3YSize - 2)
				{
					if (vector25.size() == 25)
					{// all 25 pixel values are valid then continue.
						double winStd = 0;
						for (size_t it = 0; it < vector25.size(); ++it)
						{
							winStd += (vector25[it] - winAverNdvi)* (vector25[it] - winAverNdvi);
						}
						winStd = sqrt(winStd / (vector25.size() - 1));

						if (winStd < maxStdev)
						{// stdev of window is lower than maxStdev then continue.
							double testbias = winAverNdvi - noaaNdvi;
							if (testbias <= -0.1)//bugfix
							{
								winAverNdvi = lc0Slope * winAverNdvi + lc0Inter;
							}
							else if (testbias >= 0.1)//bugfix
							{
								winAverNdvi = lc1Slope * winAverNdvi + lc1Inter;
							}
							else
							{
								winAverNdvi = linearCorrSlope * winAverNdvi + linearCorrInter;
							}
							
							noaaAverValue += noaaNdvi;
							fy3AverValue += winAverNdvi;
							wft_compare_minmax(noaaNdvi, noaaMinValue, noaaMaxValue);
							wft_compare_minmax(winAverNdvi, fy3MinValue, fy3MaxValue);
							
							MatchData md;
							md.bias = winAverNdvi - noaaNdvi ;
							md.abias = abs(md.bias);
							md.fy3AverNdvi = winAverNdvi;
							md.noaaNdvi = noaaNdvi;
							md.nclon = nclon;
							md.nclat = nclat;
							md.fclon = flon;
							md.fclat = flat;
							md.fy3stdev = winStd;
							md.fy3max = winMax;
							md.fy3min = winMin;
							md.biasm = md.fy3max - noaaNdvi;
							matchVector.push_back(md);
							fitxxSum += winAverNdvi * winAverNdvi ;
							fitxySum += winAverNdvi * noaaNdvi;
							fitxSum += winAverNdvi;
							fitySum += noaaNdvi;

							nVec.push_back(noaaNdvi);
							fVec.push_back(winAverNdvi);
							bVec.push_back(md.bias);

							tempLLValueStr = wft_double2str(md.bias);
							outBiasRowValues[ibx] = md.bias;
							outWndAverRowValues[ibx] = winAverNdvi;
						}
					}
				}

			}//end for one x
			if( (nclon-llvLon0)*(nclon-llvLon1) <=0  && (nclat-llvLat0)*(nclat-llvLat1) <=0 )
				llvOfs << nclon << " " << nclat << " " << tempLLValueStr << endl;
		}//end for all x
		outputBiasDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iby, refXSize, 1, outBiasRowValues, refXSize, 1,
			GDALDataType::GDT_Float32, 0, 0, 0);
		if (outWndAverDataset != nullptr)
		{
			outWndAverDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iby, refXSize, 1, outWndAverRowValues, refXSize, 1,
				GDALDataType::GDT_Float32, 0, 0, 0);
		}
		wft_term_progress(iby, noaaYSize);
	}
	delete[] outBiasRowValues; outBiasRowValues = nullptr;
	delete[] outWndAverRowValues; outWndAverRowValues = nullptr;
	GDALClose(outputBiasDataset);
	GDALClose(fy3DataDs);  
	GDALClose(noaaDataDs);
	if (outWndAverDataset != nullptr)
	{
		GDALClose(outWndAverDataset); outWndAverDataset = nullptr;
	}
	llvOfs.close();
	cout << "get matched points of noaa : " << matchVector.size() << endl;

	//
	fy3AverValue = fy3AverValue / matchVector.size();
	noaaAverValue = noaaAverValue / matchVector.size();

	//linear fit.
	double linearSlope = 0;
	double linearIntercept = 0;
	double correlation = 0;
	if (matchVector.size() > 1)
	{
		linearSlope = (matchVector.size() * fitxySum - fitxSum * fitySum) / (matchVector.size() * fitxxSum - fitxSum * fitxSum);
		linearIntercept = (fitySum - linearSlope * fitxSum) / matchVector.size();
	}
	

	double corr_0 = 0;
	double corr_1 = 0;
	double corr_2 = 0;
	int mcount = (int) matchVector.size();
	int ic = 0;

	string matchfile = outrootfile + ".mlist.tmp";
	string scatterfile = outrootfile + ".scat.tmp";
	ofstream matchOfs(matchfile.c_str());
	ofstream scatofs(scatterfile.c_str());
	matchOfs << "#fy3 noaa bias fclon nclon fclat nclat fy3stdev" << endl;
	scatofs << "#fy3 noaa " << endl;
	for (std::vector<MatchData>::iterator it = matchVector.begin(); it != matchVector.end(); ++it)
	{
		MatchData data = *it;
		double cz0 = data.fy3AverNdvi - fy3AverValue ;
		double cz1 = data.noaaNdvi - noaaAverValue ;
		corr_0 += cz0*cz1;
		corr_1 += cz0*cz0;
		corr_2 += cz1*cz1;
		wft_term_progress(++ic, mcount );
		scatofs << data.fy3AverNdvi << " " << data.noaaNdvi << endl;
		if( ic % 100 == 0 )
			matchOfs << data.fy3AverNdvi << " " << data.noaaNdvi << " " << data.bias <<" " << data.fclon 
			<< " " << data.nclon << " " << data.fclat << " " << data.nclat <<
			" "<<data.fy3min << " "<<data.fy3max <<" "<<data.biasm<<
			" " << data.fy3stdev<< endl;
	}
	correlation = corr_0 / (sqrt(corr_1)*sqrt(corr_2));
	scatofs.close();
	matchOfs.close();

	string fhistfile = outrootfile + ".fhist.tmp";
	string nhistfile = outrootfile + ".nhist.tmp";
	string bhistfile = outrootfile + ".bhist.tmp";
	wft_make_histfile(-1, 1, binWid, fVec, fhistfile);
	wft_make_histfile(-1, 1, binWid, nVec, nhistfile);
	wft_make_histfile(-0.5, 0.5, biasbinWid, bVec, bhistfile);
	
	stringstream corrLabel , fa,fb ,leq ;
	corrLabel << "Corr coef:" << correlation;
	fa << linearSlope;
	fb << linearIntercept;
	leq << "fit eq:y=" << linearSlope << "*x+" << linearIntercept;


	string fpng = outrootfile + ".fhist.png";//fy3 hist
	string npng = outrootfile + ".nhist.png";//noaa hist
	string bpng = outrootfile + ".bhist.png";//bias hist
	string scatpng = outrootfile + ".scat.png";
	string heatpng = outrootfile + ".bmap.png";//bias map
	vector<string> varVec, repVec;

	varVec.push_back("{{{FY3HISTPNG}}}");
	varVec.push_back("{{{FHISTTXT}}}");
	varVec.push_back("{{{NOAAHISTPNG}}}");
	varVec.push_back("{{{NHISTTXT}}}");

	varVec.push_back("{{{BIASHISTPNG}}}");
	varVec.push_back("{{{BHISTTXT}}}");
	varVec.push_back("{{{SCATPNG}}}");
	varVec.push_back("{{{SCATTXT}}}");

	varVec.push_back("{{{FXA}}}");
	varVec.push_back("{{{FXB}}}");
	varVec.push_back("{{{LEQ}}}");
	varVec.push_back("{{{CORR}}}");

	varVec.push_back("{{{HEATPNG}}}");
	varVec.push_back("{{{TITLE1}}}");
	varVec.push_back("{{{TITLE2}}}");
	varVec.push_back("{{{LLVFILE}}}");

	repVec.push_back(fpng);
	repVec.push_back(fhistfile);
	repVec.push_back(npng);
	repVec.push_back(nhistfile);

	repVec.push_back(bpng);
	repVec.push_back(bhistfile);
	repVec.push_back(scatpng);
	repVec.push_back(scatterfile);

	repVec.push_back(fa.str());
	repVec.push_back(fb.str());
	repVec.push_back(leq.str());
	repVec.push_back(corrLabel.str());

	repVec.push_back(heatpng);
	repVec.push_back("FY3B-NDVI vs NOAA-NDVI");
	repVec.push_back("Bias Map");
	repVec.push_back(llvFile);

	string newPlotFile = outrootfile + ".plot";
	wft_create_file_by_template_with_replacement(newPlotFile, plotFile, varVec, repVec);
	string cmd1 = "gnuplot ";
	cmd1 = cmd1 + newPlotFile;
	int ret1 = system(cmd1.c_str());
	cout << "plot return code : " << ret1 << endl;
	std::cout << "done." << std::endl;

    return 0;
}


