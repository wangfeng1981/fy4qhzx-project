// qcbylonlatlut.cpp : 定义控制台应用程序的入口点。
//待检验数据使用经纬度查找表，检验源数据为等经纬度坐标系并使用双线性差值计算参考值。
//wangfeng1@piesat.cn 2017-9-15

#include "stdafx.h"
#include "gdal_priv.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <algorithm>

struct WTriple
{
	double val0, val1, val2;
	inline WTriple(double v0, double v1, double v2) :val0(v0), val1(v1), val2(v2) {  };
};


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




int main(int argc , char** argv )
{
	if (argc != 2 )
	{
		std::cout << "*** Program description:" << std::endl;
		std::cout << "A program to do baseimage(Lon Lat Lut) subtract refimage(Equal lon,lat coordinate) and compute related bias et al." << std::endl;
		std::cout << "Version 0.1.3a 2017-9-15 wangfengdev@163.com" << std::endl;
		std::cout << "*** Sample call: ***" << std::endl;
		std::cout << "qcbylonlatlut start_up_params.txt"<< std::endl;
		std::cout << "*** Error: no enough parameters. out." << std::endl;
		return 1001;
	}

	const std::string inputStartupParamsFile = argv[1];

	//inputfilename
	std::string inputInputFilename = getValueFromExtraParamsFile(inputStartupParamsFile, "#inputfilename", true);
	//inputdatasetpath
	const std::string inputbaseDataPath = getValueFromExtraParamsFile(inputStartupParamsFile, "#inputdatasetpath", true);
	//inputvalid0
	const double inputbaseDataValid0 = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#inputvalid0", true).c_str());
	//inputvalid1
	const double inputbaseDataValid1 = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#inputvalid1", true).c_str());
	//inputdatascale
	const double inputbaseDataScale = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#inputdatascale", true).c_str());
	//inputdataoffset
	const double inputbaseDataOffset = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#inputdataoffset", true).c_str());
	//inputlontif
	const std::string inputbaseLonPath = getValueFromExtraParamsFile(inputStartupParamsFile, "#inputlontif", true);
	//inputlattif
	const std::string inputbaseLatPath = getValueFromExtraParamsFile(inputStartupParamsFile, "#inputlattif", true);
	//reffilename
	std::string inputRefFilename = getValueFromExtraParamsFile(inputStartupParamsFile, "#reffilename", true);
	//refdatasetpath
	const std::string inputrefDataPath = getValueFromExtraParamsFile(inputStartupParamsFile, "#refdatasetpath", true);
	//refvalid0
	const double inputrefDataValid0 = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#refvalid0", true).c_str());
	//refvalid1
	const double inputrefDataValid1 = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#refvalid1", true).c_str());
	//refdatascale
	const double inputrefDataScale = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#refdatascale", true).c_str());
	//refdataoffset
	const double inputrefDataOffset = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#refdataoffset", true).c_str());
	//refleftlon
	double refLeftLon = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#refleftlon", true).c_str());
	//refrightlon
	double refRightLon = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#refrightlon", true).c_str());
	//reftoplat
	double refTopLat = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#reftoplat", true).c_str());
	//refbottomlat
	double refBottomLat = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#refbottomlat", true).c_str());
	//outputrootpath
	const std::string inputOutputFileRoot = getValueFromExtraParamsFile(inputStartupParamsFile, "#outputrootpath", true);
	//filledvalue
	const double inputOutFilledValue = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#filledvalue", true).c_str());
	//nomatchedfillvalue
	const double inputOutNotMatchedValue = atof(getValueFromExtraParamsFile(inputStartupParamsFile, "#nomatchedfillvalue", true).c_str());
	//productcode
	const std::string inputProductType = getValueFromExtraParamsFile(inputStartupParamsFile, "#productcode", true);
	//coastline
	const std::string inputCoastlineFilepath = getValueFromExtraParamsFile(inputStartupParamsFile, "#coastline", true);
	//bechekvariety
	const std::string xp1 = getValueFromExtraParamsFile(inputStartupParamsFile, "#bechekvariety", true);
	//checkVariety
	const std::string xp2 = getValueFromExtraParamsFile(inputStartupParamsFile, "#checkVariety", true);
	//dataFormat
	const std::string xp3 = getValueFromExtraParamsFile(inputStartupParamsFile, "#dataFormat", true);
	//GERO
	const std::string xp4 = getValueFromExtraParamsFile(inputStartupParamsFile, "#GERO", true);
	//productDate
	const std::string xp5 = getValueFromExtraParamsFile(inputStartupParamsFile, "#productDate", true);
	//productVariety
	const std::string xp6 = getValueFromExtraParamsFile(inputStartupParamsFile, "#productVariety", true);
	//thetargetresultxmlfilepath
	const std::string serverProvideXmlPath = getValueFromExtraParamsFile(inputStartupParamsFile, "#thetargetresultxmlfilepath", true);
	//extrafilepath
	const std::string extraFilePath = getValueFromExtraParamsFile(inputStartupParamsFile, "#extrafilepath", true);

	//plotmaxcolors
	const std::string plotMaxColors = getValueFromExtraParamsFile(inputStartupParamsFile, "#plotmaxcolors", true);
	//plotpalette
	const std::string plotPalette = getValueFromExtraParamsFile(inputStartupParamsFile, "#plotpalette", true);
	

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

	double refResoX = (refRightLon - refLeftLon) / refXSize ;
	double refResoY = (refBottomLat - refTopLat) / refYSize;

	double refLon0 = refLeftLon + refResoX / 2;
	double refLat0 = refTopLat + refResoY / 2;

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputBiasDataset = driver->Create(outputbiasfile.c_str(), baseXSize, baseYSize , 1, GDALDataType::GDT_Float32, nullptr);

	float * latValues = new float[baseXSize];
	float * lonValues = new float[baseXSize];
	float * dataValues = new float[baseXSize];
	float * outBiasRowValues = new float[baseXSize];

	float * refDataBuffer = new float[refXSize * 200];
	int refBufferY0Index = -1;
	int refBufferY1Index = 0;

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

	for (int iby = 0; iby < baseYSize; ++iby)
	{
		std::stringstream ssbuffer;
		std::stringstream lonLatBiasBuffer;
		baseDataDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iby, baseXSize, 1,
			dataValues, baseXSize, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
		baseLonDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iby, baseXSize, 1,
			lonValues, baseXSize, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
		baseLatDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iby, baseXSize, 1,
			latValues, baseXSize, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);

		for (int ibx = 0; ibx < baseXSize; ++ibx)
		{
			//初始化输出bias数组
			outBiasRowValues[ibx] = inputOutFilledValue ;

			double baseValue = dataValues[ibx];
			double blonValue = lonValues[ibx];
			double blatValue = latValues[ibx];
			int matchRefType = 0;//匹配类型 0-未匹配，1-匹配一个值，2-匹配X轴两个值，3-匹配Y轴两个值，4-匹配四个值
			if (baseValue >= inputbaseDataValid0 && baseValue <= inputbaseDataValid1 && blonValue>-998 && blatValue>-998 )
			{
				outBiasRowValues[ibx] = inputOutNotMatchedValue;

				int targetRefX = (int)( (blonValue - refLon0) / refResoX );
				int targetRefY = (int)( (blatValue - refLat0) / refResoY );
				double tempInputValue = baseValue * inputbaseDataScale + inputbaseDataOffset ;
				double refMatchedData[5] = { 
					-9999,-9999,-9999,-9999,-9999
				};//0 inter value, x0y0 , x1y0 , x0y1, x1y1
				double bias = 0.0;

				//缓存200行数据 缓存版本反而比不缓存满了10秒，说明GDAL rasterIO内部已经缓存数据了。
				//if (targetRefY >= 0 && targetRefY < refYSize)
				//{
				//	if (targetRefY < refBufferY0Index || targetRefY>refBufferY1Index - 1 || (targetRefY == refBufferY1Index &&  refBufferY1Index != refYSize - 1))
				//	{
				//		int copyedLines = MIN(200, refYSize - targetRefY);
				//		refBufferY0Index = targetRefY;
				//		refBufferY1Index = targetRefY + copyedLines - 1;
				//		refDataDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, targetRefY, refXSize, copyedLines,
				//			refDataBuffer, refXSize, copyedLines, GDALDataType::GDT_Float32, 0, 0, nullptr);
				//		std::cout << "buffer trx try by0 by1:" << targetRefX << " " << targetRefY << " " << refBufferY0Index << " " << refBufferY1Index << std::endl;
				//	}
				//}
				
				if (targetRefX >= 0 && targetRefX < refXSize-1 && targetRefY >= 0 && targetRefY < refYSize-1 )
				{//4

					float tempRefValueArr[4] ;
					refDataDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, targetRefX, targetRefY,2,2,
						tempRefValueArr, 2, 2, GDALDataType::GDT_Float32, 0, 0, nullptr);
					//tempRefValueArr[0] = refDataBuffer[(targetRefY - refBufferY0Index)*refXSize + targetRefX]; //缓存版本
					//tempRefValueArr[1] = refDataBuffer[(targetRefY - refBufferY0Index)*refXSize + targetRefX + 1];//缓存版本
					//tempRefValueArr[2] = refDataBuffer[(targetRefY - refBufferY0Index + 1)*refXSize + targetRefX];//缓存版本
					//tempRefValueArr[3] = refDataBuffer[(targetRefY - refBufferY0Index + 1)*refXSize + targetRefX + 1];//缓存版本

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
							
							double tval0 = tempRefValueArr[0] + (blonValue - (targetRefX * refResoX + refLon0)) / refResoX*(tempRefValueArr[1] - tempRefValueArr[0]);
							double tval1 = tempRefValueArr[2] + (blonValue - (targetRefX * refResoX + refLon0)) / refResoX*(tempRefValueArr[3] - tempRefValueArr[2]);
							double tval = tval0 + (blatValue - (targetRefY*refResoY + refLat0)) / refResoY * (tval1 - tval0);
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
					refDataDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, targetRefX, targetRefY, 1, 2,
						tempRefValueArr, 1, 2, GDALDataType::GDT_Float32, 0, 0, nullptr);
					//tempRefValueArr[0] = refDataBuffer[(targetRefY - refBufferY0Index)*refXSize + targetRefX];//缓存版本
					//tempRefValueArr[1] = refDataBuffer[(targetRefY - refBufferY0Index + 1)*refXSize + targetRefX];//缓存版本

					if (inputrefDataValid0 <= tempRefValueArr[0] && tempRefValueArr[0] <= inputrefDataValid1)
					{
						matchRefType = 3;
						refMatchedData[0] = tempRefValueArr[0];
						refMatchedData[1] = tempRefValueArr[0];
						refMatchedData[3] = tempRefValueArr[1];
						if (inputrefDataValid0 <= tempRefValueArr[1] && tempRefValueArr[1] <= inputrefDataValid1)
						{
							double tval0 = tempRefValueArr[0] + (blatValue - (targetRefY * refResoY + refLat0)) / refResoY * (tempRefValueArr[1] - tempRefValueArr[0]);
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
					refDataDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, targetRefX, targetRefY, 2, 1,
						tempRefValueArr, 2, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
					//tempRefValueArr[0] = refDataBuffer[(targetRefY - refBufferY0Index)*refXSize + targetRefX];//缓存版本
					//tempRefValueArr[1] = refDataBuffer[(targetRefY - refBufferY0Index)*refXSize + targetRefX+1];//缓存版本
					if (inputrefDataValid0 <= tempRefValueArr[0] && tempRefValueArr[0] <= inputrefDataValid1)
					{
						matchRefType = 2;
						refMatchedData[0] = tempRefValueArr[0];
						refMatchedData[1] = tempRefValueArr[0];
						refMatchedData[2] = tempRefValueArr[1];
						if (inputrefDataValid0 <= tempRefValueArr[1] && tempRefValueArr[1] <= inputrefDataValid1)
						{
							double tval0 = tempRefValueArr[0] + (blonValue - (targetRefX * refResoX + refLon0)) / refResoX * (tempRefValueArr[1] - tempRefValueArr[0]);
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
					float tempRefValue = 0.0;
					refDataDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, targetRefX, targetRefY, 1, 1,
						&tempRefValue, 1, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
					/*tempRefValue = refDataBuffer[(targetRefY - refBufferY0Index)*refXSize + targetRefX];*/  //缓存版本
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
			if (matchRefType == 0)
			{
				if (blonValue>-998 && blatValue>-998 && matchRefType == 0) {
					lonLatBiasBuffer << blonValue << " " << blatValue << " NaN" << std::endl;
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
				outBiasRowValues , baseXSize , 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
		//progress
		{
			static int lastprocess = 0;
			int percent = (int)(iby * 100.f / baseYSize);
			if (lastprocess != percent && percent%10 == 0 ) {
				lastprocess = percent;
				std::cout << percent << " ";
			}
		}
	}

	std::cout << " 100 " << std::endl;
	delete [] latValues  ;
	delete[] lonValues  ;
	delete[] dataValues  ;
	delete[] refDataBuffer;


	GDALClose(baseDataDs);  
	GDALClose(baseLonDs);
	GDALClose(baseLatDs);

	GDALClose(refDataDs);
	outHistStream.close();
	outLonLatBiasStream.close();
	GDALClose(outputBiasDataset);

	std::cout << "calculating quality parameters..." << std::endl;
	//计算指标
	double corr = 0;
	double skew = 0;
	double kurt = 0;
	double middle = 0;
	double stddev = 0;
	if (validPixelCount > 1)
	{
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
	//输出直方图
	{
		
		float histStride = 0.5;
		int histBinCount = (biasMaxValue - biasMinValue+1) / histStride;
		float * histBinArray = new float[histBinCount] ;
		for (int ih = 0; ih < histBinCount; ++ih) histBinArray[ih] = 0.f;
		std::cout << "saving bias histogram values..." << std::endl;
		
		std::ofstream biasdistOfs(biasDistFilepath.c_str());
		std::ofstream scaOfs(scaFilepath.c_str());
		biasdistOfs << "#x bias" << std::endl;
		scaOfs << "#invalue  refvalue" << std::endl;
		size_t pxCount = biasVector.size();
		for (size_t it = 0; it < pxCount; ++it)
		{
			// biasdistOfs << biasVector[it] << std::endl;
			int ihist = (biasVector[it] - biasMinValue) / histStride;
			++ histBinArray[ihist];
			scaOfs << dataTripleVector[it].val0 << "  " << dataTripleVector[it].val1 << std::endl;
		}

		for (int ih = 0; ih < histBinCount; ++ih)
		{
			biasdistOfs << biasMinValue + (ih+0.5)* histStride  << " " << histBinArray[ih] / validPixelCount << std::endl;
		}

		delete[] histBinArray;

		biasdistOfs.close();
		scaOfs.close();
	}

	std::string histPlotPng = inputOutputFileRoot + ".hist.png";
	std::string scatterPlotPng = inputOutputFileRoot + ".scatter.png";
	std::string fenbuPlotPng = inputOutputFileRoot + ".heatmap.png";
	//生成绘图gnuplot文件
	{
		std::cout << "creating plots script files...." << std::endl;

		//color legend from extras
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

		plotOfs.close();
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
			<<",MIN:"<<biasMinValue<<",RMSE:"<<rmse<<",SKEW:"<<skew<< "}\"/>";

		xmlStream << "<OutputFilename>" << outputhistfile.c_str() << "</OutputFilename>" << std::endl;//matched point
		xmlStream << "<OutputFilename>" << lonlatbiasfile.c_str() << "</OutputFilename>" << std::endl; //绘制地球的lonlatbias数据
		xmlStream << "<OutputFilename>" << biasDistFilepath.c_str() << "</OutputFilename>" << std::endl;//直方图数据
		xmlStream << "<OutputFilename>" << scaFilepath.c_str() << "</OutputFilename>" << std::endl;//散点图数据

		xmlStream << "<matched_count>" << biasVector.size() << "</matched_count>" << std::endl;
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
		xmlStream << "<OutputFile>" << std::endl;
		xmlStream << "<OutputFiles>" << std::endl;
		xmlStream << "</XML>" << std::endl;
		xmlStream.close();

	}

	std::cout << "done." << std::endl;

    return 0;
}

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