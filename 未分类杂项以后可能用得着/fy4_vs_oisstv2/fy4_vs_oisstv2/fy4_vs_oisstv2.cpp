// fy4_vs_oisstv2.cpp : 定义控制台应用程序的入口点。
//fy4海表温度0.25度的tif数据与OISSTv2 0.25度数据进行质量检验运算
//王峰 2017-9-11


#include "stdafx.h"
#include "gdal_priv.h"
#include "WNetcdfDataFile.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

int main()
{
	GDALAllRegister();

	//输入文件路径和波段信息、数据集信息
	std::string fy4Filepath = "E:/testdata/testdata910/fy4sst0807.nc.sst.vrt.wgs84.tif";
	int fy4DsBandIndex = 1;//tif文件需要提供应用的波段号，从1开始计。
	std::string yzFilepath = "E:/testdata/testdata910/avhrr-only-v2.20170807.nc";
	std::string yzSubDsName = "SST";//NC或者hdf文件需要提供数据集的名字。
	int yzBandIndex = 1;//验证波段号.
	std::string zhijianOutputFilesRoot = fy4Filepath;

	//结果无效时填充的值
	double resultNoDataValue = -999;

	//fy4sst有效值范围
	WValueRange fy4ValidRange(-100,100);
	double fy4Scale = 1;
	double fy4Offset = 0;


	//OISSTv2有效值范围与scale和offset
	WValueRange yzValidRange(-300, 4500);
	double yzScale = 0.01;
	double yzOffset = 0.0;


	//打开数据集
	GDALDataset* fy4Dataset = (GDALDataset*)GDALOpen(fy4Filepath.c_str(), GDALAccess::GA_ReadOnly);
	WNetcdfDataFile ncFile;
	ncFile.open(yzFilepath);
	GDALDataset* yzDataset = ncFile.extractDataset(yzSubDsName);

	///
	///以fy4sst为基准做逐项元运算
	///
	int fy4XSize = fy4Dataset->GetRasterXSize();
	int fy4YSize = fy4Dataset->GetRasterYSize();
	int yzXSize = yzDataset->GetRasterXSize();
	int yzYSize = yzDataset->GetRasterYSize();


	//创建差值输出数据集tif
	std::string zhijianOutputBiasImageFilepath = zhijianOutputFilesRoot + ".biasimage.tif";
	GDALDriver* tifDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (tifDriver == nullptr)
	{
		std::cout << "Get geotiff driver failed. out." << std::endl;
		return 2;
	}
	GDALDataset* outputDataset = tifDriver->CreateCopy(zhijianOutputBiasImageFilepath.c_str() , 
		fy4Dataset , false , nullptr , GDALTermProgress, nullptr);
	//(zhijianOutputBiasImageFilepath.c_str(), fy4XSize, fy4YSize, 1, GDALDataType::GDT_Float32, nullptr);
	GDALRasterBand* outputBand = outputDataset->GetRasterBand(1);


	//Fetches the coefficients for transforming between pixel / line(P, L) raster space, and projection coordinates(Xp, Yp) space。
	//Xp = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
	//Yp = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
	//风四wgs84校正后放射变换系数
	double fy4Transform[6];
	fy4Dataset->GetGeoTransform(fy4Transform);

	//OISSTv2 0.25度放射变换系数
	double yzTransform[6];
	yzTransform[0] = 0 ;
	yzTransform[1] = 0.25 ;
	yzTransform[2] = 0;
	yzTransform[3] = 90 ;//每个产品这个值需要核实一下
	yzTransform[4] = 0;
	yzTransform[5] = 0.25 ;


	GDALRasterBand* fy4Band = fy4Dataset->GetRasterBand(fy4DsBandIndex);
	GDALRasterBand* yzBand = yzDataset->GetRasterBand(yzBandIndex);


	std::string outputBiasImageFilepath = zhijianOutputFilesRoot + ".lonlatbias.txt";
	std::ofstream biasImageOfs(outputBiasImageFilepath);
	biasImageOfs << "#lon lat fy yz bias" << std::endl;

	std::vector<WDataTripple> dataPairVector;
	std::vector<double> biasVector;
	double validPixelCount = 0;
	double averageFy4 = 0;
	double averageYz = 0;
	double averageBias = 0.0;
	double averageAbsBias = 0.0;
	double rmse = 0;
	double fymaxvalue = -99999;
	double fyminvalue = 99999;
	for (int iy = 0; iy < fy4YSize; ++iy)
	{
		for (int ix = 0; ix < fy4XSize; ++ix )
		{
			bool hasMatched = false;
			//风云像素值
			double fy4PixelLon = fy4Transform[0] + ix * fy4Transform[1] + iy * fy4Transform[2];
			double fy4PixelLat = fy4Transform[3] + ix * fy4Transform[4] + iy * fy4Transform[5];

			double fy4PixelValue = 0;
			double yzPixelValue = 0;
			double bias = 0;
			fy4Band->RasterIO(GDALRWFlag::GF_Read,ix, iy,
				1, 1, &fy4PixelValue, 1, 1, GDALDataType::GDT_Float64, 0, 0);
			double outputValue = resultNoDataValue;
			if (fy4ValidRange.isInside(fy4PixelValue))
			{
				fy4PixelValue = fy4PixelValue*fy4Scale + fy4Offset;

				//验证数据像素值
				int yzPixelX = (fy4PixelLon - yzTransform[0]) / yzTransform[1];
				int yzPixelY = (yzTransform[3] - fy4PixelLat) / yzTransform[5];

				if (yzPixelX >= 0 && yzPixelX < yzXSize&& yzPixelY >= 0 && yzPixelY < yzYSize)
				{
					
					yzBand->RasterIO(GDALRWFlag::GF_Read, yzPixelX, yzPixelY ,
						1, 1, &yzPixelValue, 1, 1, GDALDataType::GDT_Float64, 0, 0);
					if (yzValidRange.isInside(yzPixelValue))
					{
						hasMatched = true;
						yzPixelValue = yzPixelValue*yzScale + yzOffset;
						
						bias = fy4PixelValue - yzPixelValue;
						outputValue = bias;						

						//指标计算
						dataPairVector.push_back(WDataTripple(fy4PixelValue, yzPixelValue , bias));
						biasVector.push_back(bias);
						validPixelCount += 1;
						averageFy4 += fy4PixelValue;
						averageYz += yzPixelValue;
						averageAbsBias += fabs(bias);
						averageBias += bias;
						rmse += bias * bias;
						if (fy4PixelValue > fymaxvalue) {
							fymaxvalue = fy4PixelValue;
						}
						if (fy4PixelValue < fyminvalue) {
							fyminvalue = fy4PixelValue;
						}
					}
				}
			}
			outputBand->RasterIO(GDALRWFlag::GF_Write, ix, iy, 1, 1, &outputValue, 1, 1, GDALDataType::GDT_Float64, 0, 0, nullptr);
			if (hasMatched) {
				biasImageOfs << fy4PixelLon << " " << fy4PixelLat << " " << fy4PixelValue << " "
					<< yzPixelValue << " " << bias << std::endl;
			}
			else {
				biasImageOfs << fy4PixelLon << " " << fy4PixelLat << " " << "NaN" << " "
					<< "NaN" << " " << "NaN" << std::endl;
			}
		}
		biasImageOfs << "" << std::endl;
	}
	GDALClose(outputDataset);
	GDALClose(fy4Dataset);
	GDALClose(yzDataset);
	ncFile.close();
	biasImageOfs.close();

	//计算指标
	double corr = 0;
	double skew = 0;
	double kurt = 0;
	double middle = 0;
	double stddev = 0;
	if (validPixelCount > 1)
	{
		averageFy4 = averageFy4 / validPixelCount;
		averageYz = averageYz / validPixelCount;
		averageBias = averageBias / validPixelCount;
		averageAbsBias = averageAbsBias / validPixelCount;
		rmse = sqrt(rmse / validPixelCount);
		size_t numPixels = dataPairVector.size();
		for (std::vector<WDataTripple>::iterator it = dataPairVector.begin(); it != dataPairVector.end(); ++it)
		{
			WDataTripple data = *it;
			double cz = data.mz - averageBias;
			stddev += cz*cz ;
		}
		stddev = sqrt(stddev / validPixelCount);

		double corr_0 = 0;
		double corr_1 = 0;
		double corr_2 = 0;
		for (std::vector<WDataTripple>::iterator it = dataPairVector.begin(); it != dataPairVector.end(); ++it)
		{
			WDataTripple data = *it;
			double cz0 = data.mx - averageFy4;
			double cz1 = data.my - averageYz;
			double czbias = data.mz - averageBias;
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
			middle = (biasVector[middleIndex0] + biasVector[middleIndex1] ) / 2.0;
		}
	}
	else {
		averageFy4 = -999;
		averageYz = -999;
		averageBias = -999;
		averageAbsBias = -999;
		rmse = -999;
		corr = -999;
		skew = -999;
		kurt = -999;
		middle = -999;
		stddev = -999;
	}
	
	std::string outputZhiBiaoFilepath = zhijianOutputFilesRoot + ".zhibiao.xml";
	std::ofstream xmlStream(outputZhiBiaoFilepath);
	xmlStream << "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>" << std::endl;
	xmlStream << "<results>" << std::endl;
	xmlStream << "<input_fy>" << fy4Filepath.c_str() << "</input_fy>" << std::endl;
	xmlStream << "<input_ref>" << yzFilepath.c_str() << "</input_ref>" << std::endl;
	xmlStream << "<matched_count>" << dataPairVector.size() << "</matched_count>" << std::endl;
	xmlStream << "<min_fy>" << fyminvalue << "</min_fy>" << std::endl;
	xmlStream << "<max_fy>" << fymaxvalue << "</max_fy>" << std::endl;
	xmlStream << "<average_fy>" << averageFy4 << "</average_fy>" << std::endl;
	xmlStream << "<average_ref>" << averageYz << "</average_ref>" << std::endl;
	xmlStream << "<average_bias>" << averageBias << "</average_bias>" << std::endl;
	xmlStream << "<average_abs_bias>" << averageAbsBias << "</average_abs_bias>" << std::endl;
	xmlStream << "<rmse>" << rmse << "</rmse>" << std::endl;
	xmlStream << "<corr>" << corr << "</corr>" << std::endl;
	xmlStream << "<skew>" << skew << "</skew>" << std::endl;
	xmlStream << "<kurt>" << kurt << "</kurt>" << std::endl;
	xmlStream << "<middle>" << middle << "</middle>" << std::endl;
	xmlStream << "<stddev>" << stddev << "</stddev>" << std::endl;
	xmlStream << "</results>" << std::endl;
	xmlStream.close();

	{
		std::cout << "write bias distributions..." << std::endl;
		std::string biasDistFilepath = zhijianOutputFilesRoot + ".biasdist.txt";
		std::string scaFilepath = zhijianOutputFilesRoot + ".sca.txt";
		std::ofstream biasdistOfs(biasDistFilepath);
		std::ofstream scaOfs(scaFilepath);
		biasdistOfs << "#bias" << std::endl;
		scaOfs << "#fyvalue  yzvalue" << std::endl;
		size_t pxCount = biasVector.size();
		for (size_t it = 0; it < pxCount ; ++it)
		{
			biasdistOfs << biasVector[it] << std::endl;
			scaOfs << dataPairVector[it].mx << "  " << dataPairVector[it].my << std::endl;
		}
		biasdistOfs.close();
		scaOfs.close();
	}
	

	std::cout << "done." << std::endl;

    return 0;
}

