//基于图像像素坐标进行差值计算，输入数据与参考数据尺寸和分辨率必须一致。

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
#include "../../sharedcodes/wftools.h"
using namespace std;

#define OPLT 0 
#define OPGT 1
#define OPLE 2 
#define OPGE 3
#define OPEQ 4
#define OPNE 5

bool ismaskok(int op, double maskvalue , double thresh )
{
	switch (op)
	{
	case OPGT :
	{
		if (maskvalue>thresh) return true;
		else return false;
	}
	case OPLT:
	{
		if (maskvalue<thresh) return true;
		else return false;
	}
	case OPGE:
	{
		if (maskvalue>=thresh) return true;
		else return false;
	}
	case OPLE:
	{
		if (maskvalue<=thresh) return true;
		else return false;
	}
	case OPEQ:
	{
		if (thresh == maskvalue) return true;
		else return false;
	}
	case OPNE:
	{
		if (thresh != maskvalue) return true;
		else return false;
	}
	default:
		return true ;
	}
}

int opstr2int(string opstr)
{
	if (opstr == ">") return OPGT;
	if (opstr == "<") return OPLT;
	if (opstr == ">=") return OPGE;
	if (opstr == "<=") return OPLE;
	if (opstr == "==") return OPEQ;
	if (opstr == "!=") return OPNE;
	return OPNE;
}

int main(int argc , char** argv )
{
	std::cout << "Version 0.1a. by wangfengdev@163.com 2017-10-10.  " << std::endl;
	std::cout << "Version 0.2a. hist value use percent (%) .  " << std::endl;
	std::cout << "Version 0.3a. add two mask and add some output parameters  .2017-11-17.  " << std::endl;
	if (argc == 1 )
	{
		std::cout << "Sample call:" << std::endl;
		std::cout << "qcbypixels -in input.tif -ref reference.tif " 
			" -outtif output.tif "
			" -iv0 -10000 -iv1 10000 -rv0 -2000 -rv1 10000  "
			" [-isca 1 -iofs 0 -rsca 1 -rofs 0] "
			" [-fill -999] "
			" [-biasbinwid 0.1] "  //偏差直方图宽度，可选
			" [-binwid 0.2] "      //数值直方图宽度，可选
			" [-x0 0 -x1 100 -y0 0 -y1 100] " //出图范围
			" [-mask0file mask0.tif -mask0operator >,<,==,<=,>=,<> -mask0thresh 1] "
			" [-mask1file mask1.tif -mask1operator >,<,==,<=,>=,<> -mask1thresh 0.5] "
			" [-plottem template.plot] "
			" [-tltprefix TitlePrefix] "
			" [-tlttail TitleTail] "
			<< std::endl ;

		std::cout << "*** Error: no enough parameters. out." << std::endl;
		return 101;
	}

	std::string inputFilepath = "";
	wft_has_param(argc, argv, "-in", inputFilepath, true);
	std::string referenceFilepath = "";
	wft_has_param(argc, argv, "-ref", referenceFilepath, true);
	std::string outputTif = "";
	wft_has_param(argc, argv, "-outtif", outputTif, true);

	bool usemask0 = false;
	bool usemask1 = false;
	string mask0file, mask1file, mask0op, mask1op, mask0thstr, mask1thstr;
	{
		wft_has_param(argc, argv, "-mask0file", mask0file, false );
		wft_has_param(argc, argv, "-mask0operator", mask0op, false);
		wft_has_param(argc, argv, "-mask0thresh", mask0thstr, false);

		wft_has_param(argc, argv, "-mask1file", mask1file, false);
		wft_has_param(argc, argv, "-mask1operator", mask1op, false);
		wft_has_param(argc, argv, "-mask1thresh", mask1thstr, false);

		if (mask0file.length() > 0 && mask0op.length() > 0 && mask0thstr.length() > 0) usemask0 = true;
		if (mask1file.length() > 0 && mask1op.length() > 0 && mask1thstr.length() > 0) usemask1 = true;

	}


	double inValid0, inValid1, refValid0, refValid1;
	{
		std::string temp;
		wft_has_param(argc, argv, "-iv0", temp, true); 
		inValid0 = atof(temp.c_str());
		wft_has_param(argc, argv, "-iv1", temp, true);
		inValid1 = atof(temp.c_str());
		wft_has_param(argc, argv, "-rv0", temp, true); 
		refValid0 = atof(temp.c_str());
		wft_has_param(argc, argv, "-rv1", temp, true);
		refValid1 = atof(temp.c_str());
	}

	double inScale(1), inOffset(0), refScale(1), refOffset(0) , fillValue(0) , biasBinWid(0.1) , binWid(1) ;
	{
		std::string temp;
		if (wft_has_param(argc, argv, "-isca", temp, false))
		{
			inScale = atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-iofs", temp, false))
		{
			inOffset = atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-rsca", temp, false))
		{
			refScale = atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-rofs", temp, false))
		{
			refOffset = atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-fill", temp, false))
		{
			fillValue = atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-biasbinwid", temp, false))
		{
			biasBinWid = atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-binwid", temp, false))
		{
			binWid = atof(temp.c_str());
		}

	}
	std::string outbiashist = outputTif + ".bhist.tmp";
	std::string outInhist = outputTif + ".ihist.tmp";
	std::string outRefhist = outputTif + ".rhist.tmp";
	std::string outMatched = outputTif + ".mlist.tmp";
	std::string outllvList = outputTif + ".llv.tmp";
	std::string outinfofile = outputTif + ".info.txt";

	string plotTemplate , titlePrefix , titleTail ;
	wft_has_param(argc, argv, "-plottem", plotTemplate , false);
	wft_has_param(argc, argv, "-tltprefix", titlePrefix, false);
	wft_has_param(argc, argv, "-tlttail", titleTail, false);

	
	std::ofstream matchStream(outMatched.c_str());
	std::ofstream llvStream(outllvList.c_str());



	matchStream << "#ix iy iv rv bias" << std::endl;
	llvStream << "#lon lat bias" << std::endl;

	GDALAllRegister();

	GDALDataset* inDs = (GDALDataset*)GDALOpen(inputFilepath.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* refDs = (GDALDataset*)GDALOpen(referenceFilepath.c_str(), GDALAccess::GA_ReadOnly);

	int inXSize = inDs->GetRasterXSize();
	int inYSize = inDs->GetRasterYSize();

	int useX0(0), useX1(inXSize-1), useY0(0), useY1(inYSize - 1);
	{
		std::string temp;
		if (wft_has_param(argc, argv, "-x0", temp, false))
		{
			useX0 = (int)atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-x1", temp, false))
		{
			useX1 = (int)atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-y0", temp, false))
		{
			useY0 = (int)atof(temp.c_str());
		}
		if (wft_has_param(argc, argv, "-y1", temp, false))
		{
			useY1 = (int)atof(temp.c_str());
		}
	}

	int refXSize = refDs->GetRasterXSize();
	int refYSize = refDs->GetRasterYSize();
	if (inXSize != refXSize || inYSize != refYSize)
	{
		std::cout << "input image size " << inXSize << " " << inYSize << 
			" is not equal with reference file image size " << refXSize << " " << refYSize << " .out." << std::endl;
		exit(102);
	}

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputBiasDataset = driver->Create(outputTif.c_str(), inXSize, inYSize, 1, GDALDataType::GDT_Float32, 0);
	double trans[6];
	inDs->GetGeoTransform(trans);
	{
		outputBiasDataset->SetGeoTransform(trans);
		const char* proj = inDs->GetProjectionRef();
		outputBiasDataset->SetProjection(proj);
	}

	double * inRowValues = new double[inXSize];
	double * refRowValues = new double[inXSize];
	double * outBiasRowValues = new double[inXSize];

	double* maskbuffer0 = new double[inXSize];
	double* maskbuffer1 = new double[inXSize];

	GDALDataset* mask0ds = 0;
	GDALDataset* mask1ds = 0;
	double mask0thresh, mask1thresh;
	int mask0operator, mask1operator;
	if (usemask0)
	{
		mask0ds = (GDALDataset*)GDALOpen(mask0file.c_str(), GDALAccess::GA_ReadOnly);
		mask0thresh = atof(mask0thstr.c_str());
		mask0operator = opstr2int(mask0op);
	}

	if (usemask1)
	{
		mask1ds = (GDALDataset*)GDALOpen(mask1file.c_str(), GDALAccess::GA_ReadOnly);
		mask1thresh = atof(mask0thstr.c_str());
		mask1operator = opstr2int(mask1op);
	}

	double averageBiasSum = 0;
	double averageRBiasSum = 0;
	double averageAbsBiasSum = 0;
	double averageAbsRBiasSum = 0;

	double fitxySum = 0;
	double fitxSum = 0;
	double fitySum = 0;
	double fitxxSum = 0;

	double insum = 0;
	double refsum = 0;

	double bias2Sum = 0;

	double biasMin(9999), biasMax(-9999);
	double inMin(9999), inMax(-9999);
	double refMin(9999), refMax(-9999);

	std::vector<double> biasVector, inVector, refVector;

	for (int iby = 0; iby < inYSize; ++iby)
	{
		inDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iby, inXSize, 1,
			inRowValues, inXSize, 1, GDALDataType::GDT_Float64, 0, 0, 0);
		refDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iby, inXSize, 1,
			refRowValues, inXSize, 1, GDALDataType::GDT_Float64, 0, 0, 0);
		if (mask0ds)
		{
			mask0ds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iby, inXSize, 1,
				maskbuffer0, inXSize, 1, GDALDataType::GDT_Float64, 0, 0, 0);
		}
		if (mask1ds)
		{
			mask1ds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iby, inXSize, 1,
				maskbuffer1, inXSize, 1, GDALDataType::GDT_Float64, 0, 0, 0);
		}

		for (int ibx = 0; ibx < inXSize; ++ibx)
		{
			//初始化输出bias数组
			double inval = inRowValues[ibx];
			double refval = refRowValues[ibx];
			outBiasRowValues[ibx] = fillValue ;
			
			if (useX0 <= ibx && ibx <= useX1 && useY0 <= iby && iby <= useY1)
			{
				bool passmask0 = true;
				bool passmask1 = true;
				if (usemask0)
				{
					passmask0 = ismaskok(mask0operator,  maskbuffer0[ibx] , mask0thresh );
				}
				if (usemask1)
				{
					passmask1 = ismaskok(mask1operator, maskbuffer1[ibx] , mask1thresh );
				}

				bool isMatched = false;
				double bias = 0;
				if (inValid0 <= inval && inval <= inValid1 && refValid0 <= refval && refval <= refValid1 && passmask0 && passmask1 )
				{
					isMatched = true;
					inval = inScale * inval + inOffset;
					refval = refScale * refval + refOffset;
					bias = inval - refval;
					outBiasRowValues[ibx] = bias;

					//info
					averageBiasSum += bias;
					averageRBiasSum += (bias / refval);
					averageAbsBiasSum += abs(bias);
					averageAbsRBiasSum += abs(bias / refval);

					fitxySum += inval * refval;
					fitxSum += inval ;
					fitySum += refval ;
					fitxxSum += inval*inval;

					insum += inval;
					refsum += refval;

					bias2Sum += bias*bias;

					inVector.push_back(inval);
					refVector.push_back(refval);
					biasVector.push_back(bias);
					wft_compare_minmax(bias, biasMin, biasMax);
					wft_compare_minmax(inval, inMin, inMax);
					wft_compare_minmax(refval, refMin, refMax);

				}

				double cx = trans[0] + trans[1] * ibx + trans[2] * iby;
				double cy = trans[3] + trans[4] * ibx + trans[5] * iby;
				if (isMatched == false)
				{
					llvStream << cx << " " << cy << " NaN" << std::endl;
				}
				else {
					llvStream << cx << " " << cy << " " << bias << std::endl;
					matchStream << ibx << " " << iby << " " << inval << " " << refval << " " << bias << std::endl;
					
					
				}
			}
			
		}//一行处理结束
		outputBiasDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iby, inXSize, 1,
			outBiasRowValues, inXSize, 1, GDALDataType::GDT_Float64, 0, 0, 0);
		wft_term_progress(iby, inYSize );
	}

	delete[] inRowValues; inRowValues = 0;
	delete[] refRowValues; refRowValues = 0;
	delete[] outBiasRowValues; outBiasRowValues = 0;
	delete[] maskbuffer0;
	delete[] maskbuffer1;

	if( mask0ds ) 	GDALClose(mask0ds);
	if( mask1ds ) 	GDALClose(mask1ds);

	GDALClose(inDs);  
	GDALClose(refDs);
	GDALClose(outputBiasDataset);

	matchStream.close();
	llvStream.close();

	std::cout << "making hist files..." << std::endl;
	wft_make_histfile(biasMin, biasMax, biasBinWid, biasVector, outbiashist);
	wft_make_histfile(inMin, inMax, binWid, inVector, outInhist);
	wft_make_histfile(refMin, refMax, binWid, refVector, outRefhist);

	cout << "calculating bias info...." << endl;

	int n1 = biasVector.size();
	if (n1 == 0) cout << "Warning : not matched points." << endl;
	double averageBias = averageBiasSum / n1;
	double averageRBias = averageRBiasSum / n1;
	double averageAbsBias = averageAbsBiasSum / n1;
	double averageAbsRBias = averageAbsRBiasSum / n1;
	double rmseSum = 0;
	double inaver = insum / n1;
	double refaver = refsum / n1;
	double corr_0 = 0;
	double corr_1 = 0;
	double corr_2 = 0;
	for (int i = 0; i < n1; ++i)
	{
		rmseSum += (biasVector[i] - averageBias)*(biasVector[i] - averageBias);

		double cz0 = inVector[i] - inaver;
		double cz1 = refVector[i] - refaver;
		double czbias = biasVector[i] - averageBias;
		corr_0 += cz0*cz1;
		corr_1 += cz0*cz0;
		corr_2 += cz1*cz1;
	}
	double rmse = sqrt(rmseSum / n1);
	double corr = corr_0 / (sqrt(corr_1) * sqrt(corr_2));
	double rmse2 = sqrt(bias2Sum / n1);

	double linearSlope = (n1 * fitxySum - fitxSum * fitySum) / (n1 * fitxxSum - fitxSum * fitxSum);
	double linearIntercept = (fitySum - linearSlope * fitxSum) / n1;

	ofstream info(outinfofile.c_str());
	info << "count: " << n1 << endl;
	info << "average-bias: " << averageBias <<  endl;
	info << "average-rbias: " << averageRBias << endl;
	info << "average-abs-bias: " << averageAbsBias << endl;
	info << "average-abs-rbias: " << averageAbsRBias << endl;
	info << "rmse1[sqrt(sum(bias-averbias))]: " << rmse << endl;
	info << "rmse2[sqrt(sum(bias*bias))]: " << rmse2 << endl;
	info << "corr:" << corr << endl;
	info << "linearslope:" << linearSlope << endl;
	info << "linearintercept:" << linearIntercept << endl;
	info.close();

	if (plotTemplate.length() > 1 )
	{
		string plotfile = outputTif + ".plot";
		vector<string> varVec, repVec;
		varVec.push_back("{{{TITLE1}}}");

		varVec.push_back("{{{IHISTOUT}}}");
		varVec.push_back("{{{IHISTIN}}}");
		varVec.push_back("{{{RHISTOUT}}}");
		varVec.push_back("{{{RHISTIN}}}");
		varVec.push_back("{{{BHISTOUT}}}");
		varVec.push_back("{{{BHISTIN}}}");
		varVec.push_back("{{{SCAOUT}}}");
		varVec.push_back("{{{SCATIN}}}");

		varVec.push_back("{{{CORR}}}");
		varVec.push_back("{{{AABIAS}}}");
		varVec.push_back("{{{RMSE}}}");
		varVec.push_back("{{{SLOPE}}}");
		varVec.push_back("{{{INTER}}}");

		varVec.push_back("{{{MAPOUT}}}");
		varVec.push_back("{{{MAPIN}}}");


		repVec.push_back(titlePrefix + titleTail);

		repVec.push_back(outInhist + ".png" );
		repVec.push_back(outInhist);
		repVec.push_back(outRefhist + ".png");
		repVec.push_back(outRefhist);
		repVec.push_back(outbiashist + ".png");
		repVec.push_back(outbiashist);

		repVec.push_back(outMatched + ".png");
		repVec.push_back(outMatched);

		repVec.push_back(wft_float2str(corr));
		repVec.push_back(wft_float2str(averageAbsBias));
		repVec.push_back(wft_float2str(rmse));
		repVec.push_back(wft_float2str(linearSlope));
		repVec.push_back(wft_float2str(linearIntercept));

		repVec.push_back(outllvList + ".png");
		repVec.push_back(outllvList);

		wft_create_file_by_template_with_replacement(plotfile, plotTemplate, varVec, repVec);

		cout << "ploting... ... " << endl;
		string cmd1 = "gnuplot " + plotfile;
		int res1 = system(cmd1.c_str());
		cout << "plot return code : " << res1 << endl;
	}



	std::cout << "done." << std::endl;

    return 0;
}


