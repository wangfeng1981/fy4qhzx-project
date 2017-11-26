// snowfusion.cpp : 定义控制台应用程序的入口点。
//武胜利 :
//应该是：只有整个白天所有时次均为云，才判为云，其他判识结果以无云状态下地表有雪或无雪为准，只要有一个时次有雪，就判为雪，如果除云外，晴空时次为无雪，就判为无雪。
//我们使用风4的日产品还是15min产品？
//使用combine不要使用fusion

#include "gdal_priv.h"
#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include "../../sharedcodes/wftools.h"
using namespace std;

int g_fy4CloudValue = 50; //bzf 6
int g_fy4SnowValue = 200; //bzf 10
int g_combineSnowValue = 201; //bzf 10
string g_plotvalid0 = "10"; //bzf 3 
string g_plotvalid1 = "250" ;//bzf 15
string g_dsPrefix = "HDF5:\"" ;// bzf ""
string g_dsTail = "\"://SNC" ;// bzf "" 
string g_fy4fixtail = "_4000M_ADV01.NC";//   _4000M_V0001.NC
string g_fy4fixprefix = "FY4A-_AGRI--_N_DISK_1047E_L3-_SNC-";//bzf FY4A-_AGRI--_N_DISK_0995E_L3-_SNC-


bool findfy3value(float fy4lon, float fy4lat,const int fy3size, float* fy3lonArr, float* fy3latArr, short* fy3sdArr ,
	vector<int>& latIndexVector ,
	short& resultsd , double& resultd , float& rfy3lon,float& rfy3lat )
{//采用最邻近的方法，找到fy4经纬度在fy3里面几何距离最小的一个像元，认为该像元就是覆盖这个fy4像元的Fy3二十五公里像元。
	if (fy4lon < -900.f) return false;
	if (fy4lat < -900.f) return false;
	double mind2 = 99999.0 ;
	int theIndex = -1;
	int vsize = latIndexVector.size();
	for (int i = 0; i < vsize ; ++i)
	{
		int iv = latIndexVector[i];
		if (fy3lonArr[iv] > -900.f && fy3latArr[iv] > -900.f)
		{
			double dx = fy4lon - fy3lonArr[iv];
			double dy = fy4lat - fy3latArr[iv];
			double d2 = dx*dx + dy * dy;
			if (d2 < mind2)
			{
				mind2 = d2;
				theIndex = iv;
			}
		}
	}
	if (theIndex >= 0)
	{
		resultsd = fy3sdArr[theIndex] ;
		resultd = sqrt(mind2);
		if (resultd < 100 )
		{//判断在25km像元内 2017-11-17 100 for debuging 认为fy3是连续的，那么这个地方就不进行判断了，找到最邻近那么就认为在这个像元里面。
			rfy3lon = fy3lonArr[theIndex];
			rfy3lat = fy3latArr[theIndex];
			return true;
		}
		else {
			return false;
		}
		
	}
	else
	{
		return false;
	}
}

void writecheckfile(ofstream& ofs , float f4lon, float f4lat, short f4cloud, short f4snc, float f3lon, float f3lat, double dist, short f3val, short outtype)
{
	
	//ofs << f4lon << " "
	//	<< f4lat << " "
	//	<< f4cloud << " "
	//	<< f4snc<< " "
	//	<< f3lon << " "
	//	<< f3lat << " "
	//	<< dist << " "
	//	<< f3val << " "
	//	<< outtype << " "
	//	<< endl;
}

int getLatIndex(float lat, bool north )
{
	if (north)
	{
		float arr[] = { 9 , 18.0, 27.0,
			36.0, 45.0, 54.0,
			63.0, 72.0, 81.0,
			90.5 };
		for (int i = 0; i < 10; ++i)
		{
			if (lat < arr[i]) return i;
		}
	}
	else
	{
		float arr[] = { -9.0, -18.0, -27.0,
			-36.0, -45.0, -54.0,
			-63.0, -72.0, -81.0,
			-90.5 };
		for (int i = 0; i < 10; ++i)
		{
			if (lat > arr[i]) return i;
		}
	}
	return 9;
}

struct LutObj
{
	vector<int> v10[10];
	void addIndexByLat(float lat ,int index , bool north );
};

void LutObj::addIndexByLat(float lat, int index , bool north)
{
	if (north)
	{
		float arr[] = { 9, 18, 27,
			36, 45, 54,
			63, 72, 81,
			90.5 };
		for (int i = 0; i < 10; ++i)
		{
			if (lat < arr[i]) {
				v10[i].push_back(index);
				if (i - 1 >= 0) v10[i - 1].push_back(index);
				if (i + 1 < 10) v10[i + 1].push_back(index);
				break;
			}
		}
	}
	else
	{
		float arr[] = { -9, -18, -27,
			-36, -45, -54,
			-63, -72, -81,
			-90.5 };
		for (int i = 0; i < 10; ++i)
		{
			if (lat > arr[i]) {
				v10[i].push_back(index);
				if (i - 1 >= 0) v10[i - 1].push_back(index);
				if (i + 1 < 10) v10[i + 1].push_back(index);
				break;
			}
		}
	}
}


int fy4plotout(string& fy4file , string& fy4lonfile , string& fy4latfile , string& outpngfile , string& txt , string& plot , string& plottem , string& ymd )
{

	string fy4dspath = g_dsPrefix + fy4file +  g_dsTail ;

	string temp_txtfile = outpngfile + ".xyz.txt";
	string cmd1 = txt + " -in " + fy4dspath
		+ " -out " + temp_txtfile
		+ " -type llfiles "
		+ " -lon " + fy4lonfile
		+ " -lat " + fy4latfile
		+ " -valid0 " + g_plotvalid0
		+ " -valid1  " + g_plotvalid1 
		+ " -xspace 1 -yspace 1 ";
	int res1 = system(cmd1.c_str());
	cout << "totxt return code for fy4 : " << res1 << endl;
	if (wft_test_file_exists(temp_txtfile))
	{
		vector<string> v1, v2;
		v1.push_back("{{{OUTFILE}}}");
		v1.push_back("{{{INFILE}}}");
		v1.push_back("{{{DATE}}}");

		int ymdi = (int)atof(ymd.c_str());
		string ymd2 = wft_ymd_int2str(ymdi);

		v2.push_back(outpngfile);
		v2.push_back(temp_txtfile);
		v2.push_back(ymd2);

		string plotfile = outpngfile + ".plot";
		wft_create_file_by_template_with_replacement(plotfile, plottem, v1, v2);
		if (wft_test_file_exists(plotfile))
		{
			string cmd2 = plot + " " + plotfile;
			int res2 = system(cmd2.c_str());
			cout << "gnuplot return code for fy4 : " << res2 << endl;
			if (wft_test_file_exists(outpngfile))
			{
				wft_remove_file(temp_txtfile);
			}
			else {
				cout << "Error : make " << outpngfile << " failed." << endl;
				return 10;
			}
		}
		else {
			cout << "Error : make " << plotfile << " failed." << endl;
			return 10;
		}

	}
	else {
		cout << "Error : make " << temp_txtfile << " failed." << endl;
		return 10;
	}

}


int processOneFile( string& fy4file , string& fy4lonfile , string& fy4latfile , 
	string& outputfile , 
	string& fy3file , 
	string& fy3northlonfile,
	string& fy3northlatfile ,
	string& fy3southlonfile ,
	string& fy3southlatfile , 
	string& txt ,
	string& gnuplot , 
	string& plottemplate , 
	string& insert , 
	string& host , 
	string& user , 
	string& pwd , 
	string& db , 
	string& tb , 
	string& pid ,
	string& ymd 
	)
{
	//fy4 data
	string fy4dspath =  g_dsPrefix + fy4file +  g_dsTail ;
	GDALDataset* fy4ds = (GDALDataset*)GDALOpen(fy4dspath.c_str(), GDALAccess::GA_ReadOnly);
	//GDALDataset* fy4cloudds = (GDALDataset*)GDALOpen(fy4cloudfile.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* fy4londs = (GDALDataset*)GDALOpen(fy4lonfile.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* fy4latds = (GDALDataset*)GDALOpen(fy4latfile.c_str(), GDALAccess::GA_ReadOnly);
	const int fy4XSize = fy4ds->GetRasterXSize();
	const int fy4YSize = fy4ds->GetRasterYSize();

	//out data
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outputfile.c_str(), fy4XSize, fy4YSize, 1, GDT_Byte, nullptr);


	//fy4 original output png
	string fy4pngfile = outputfile + ".fy4.png";
	fy4plotout(fy4file, fy4lonfile, fy4latfile, fy4pngfile , txt , gnuplot ,  plottemplate , ymd );

	//fy3 snow depth data
	const int fy3size = 721 * 721;
	short* fy3northArr = new short[fy3size];
	short* fy3southArr = new short[fy3size];
	{
		short* fy3north0 = new short[fy3size];
		short* fy3north1 = new short[fy3size];
		short* fy3south0 = new short[fy3size];
		short* fy3south1 = new short[fy3size];
		string fy3northfile = "HDF5:\"" + fy3file + "\"://SD_Northern_Daily";
		string fy3southfile = "HDF5:\"" + fy3file + "\"://SD_Southern_Daily";
		GDALDataset* fy3nds = (GDALDataset*)GDALOpen(fy3northfile.c_str(), GDALAccess::GA_ReadOnly);
		GDALDataset* fy3sds = (GDALDataset*)GDALOpen(fy3southfile.c_str(), GDALAccess::GA_ReadOnly);
		fy3nds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read,
			0, 0, 721, 721, fy3north0, 721, 721, GDALDataType::GDT_Int16, 0, 0, 0);
		fy3nds->GetRasterBand(2)->RasterIO(GDALRWFlag::GF_Read,
			0, 0, 721, 721, fy3north1, 721, 721, GDALDataType::GDT_Int16, 0, 0, 0);

		fy3sds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read,
			0, 0, 721, 721, fy3south0, 721, 721, GDALDataType::GDT_Int16, 0, 0, 0);
		fy3sds->GetRasterBand(2)->RasterIO(GDALRWFlag::GF_Read,
			0, 0, 721, 721, fy3south1, 721, 721, GDALDataType::GDT_Int16, 0, 0, 0);
		GDALClose(fy3nds);
		GDALClose(fy3sds);
		//计算上轨和下轨的平均值
		for (int i = 0; i < fy3size; ++i)
		{
			if (0 <= fy3north0[i] && fy3north0[i] <= 1000 &&
				0 <= fy3north0[i] && fy3north0[i] <= 1000
				)
			{
				fy3northArr[i] = (short)round(fy3north0[i] / 2.0 + fy3north1[i] / 2.0);
			}
			else if (0 <= fy3north0[i] && fy3north0[i] <= 1000) {
				fy3northArr[i] = fy3north0[i];
			}
			else {
				fy3northArr[i] = fy3north1[i];
			}

			if (0 <= fy3south0[i] && fy3south0[i] <= 1000 &&
				0 <= fy3south1[i] && fy3south1[i] <= 1000
				)
			{
				fy3southArr[i] = (short)round(fy3south0[i] / 2.0 + fy3south1[i] / 2.0);
			}
			else if (0 <= fy3south0[i] && fy3south0[i] <= 1000) {
				fy3southArr[i] = fy3south0[i];
			}
			else {
				fy3southArr[i] = fy3south1[i];
			}
		}

		delete[] fy3north0;
		delete[] fy3north1;
		delete[] fy3south0;
		delete[] fy3south1;
	}


	float* fy3nlonArr = new float[fy3size];
	float* fy3nlatArr = new float[fy3size];
	float* fy3slonArr = new float[fy3size];
	float* fy3slatArr = new float[fy3size];

	LutObj northLut[360];
	LutObj southLut[360];
	//fy3 lon lat
	{
		GDALDataset* fy3nlonds = (GDALDataset*)GDALOpen(fy3northlonfile.c_str(), GDALAccess::GA_ReadOnly);
		GDALDataset* fy3nlatds = (GDALDataset*)GDALOpen(fy3northlatfile.c_str(), GDALAccess::GA_ReadOnly);
		GDALDataset* fy3slonds = (GDALDataset*)GDALOpen(fy3southlonfile.c_str(), GDALAccess::GA_ReadOnly);
		GDALDataset* fy3slatds = (GDALDataset*)GDALOpen(fy3southlatfile.c_str(), GDALAccess::GA_ReadOnly);
		fy3nlonds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read,
			0, 0, 721, 721, fy3nlonArr, 721, 721, GDALDataType::GDT_Float32, 0, 0, 0);
		fy3nlatds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read,
			0, 0, 721, 721, fy3nlatArr, 721, 721, GDALDataType::GDT_Float32, 0, 0, 0);
		fy3slonds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read,
			0, 0, 721, 721, fy3slonArr, 721, 721, GDALDataType::GDT_Float32, 0, 0, 0);
		fy3slatds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read,
			0, 0, 721, 721, fy3slatArr, 721, 721, GDALDataType::GDT_Float32, 0, 0, 0);
		GDALClose(fy3nlonds);
		GDALClose(fy3nlatds);
		GDALClose(fy3slonds);
		GDALClose(fy3slatds);
		std::cout << "Indexing latitude and longitude ..." << endl;
		for (int i = 0; i < fy3size; ++i)
		{
			float northlon = fy3nlonArr[i];
			if (northlon > -900.f)
			{
				int lonindex = (int)northlon + 180;
				if (lonindex == 360) lonindex = 359;
				northLut[lonindex].addIndexByLat(fy3nlatArr[i], i, true);
				if (lonindex == 0)
				{
					northLut[359].addIndexByLat(fy3nlatArr[i], i, true);
					northLut[1].addIndexByLat(fy3nlatArr[i], i, true);
				}
				else if (lonindex == 359)
				{
					northLut[358].addIndexByLat(fy3nlatArr[i], i, true);
					northLut[0].addIndexByLat(fy3nlatArr[i], i, true);
				}
				else
				{
					northLut[lonindex - 1].addIndexByLat(fy3nlatArr[i], i, true);
					northLut[lonindex + 1].addIndexByLat(fy3nlatArr[i], i, true);
				}
			}
			float southlon = fy3slonArr[i];
			if (southlon > -900.f)
			{
				int lonindex = (int)northlon + 180;
				if (lonindex == 360) lonindex = 359;
				southLut[lonindex].addIndexByLat(fy3slatArr[i], i, false);
				if (lonindex == 0)
				{
					southLut[359].addIndexByLat(fy3slatArr[i], i, false);
					southLut[1].addIndexByLat(fy3slatArr[i], i, false);
				}
				else if (lonindex == 359)
				{
					southLut[358].addIndexByLat(fy3slatArr[i], i, false);
					southLut[0].addIndexByLat(fy3slatArr[i], i, false);
				}
				else
				{
					southLut[lonindex - 1].addIndexByLat(fy3slatArr[i], i, false);
					southLut[lonindex + 1].addIndexByLat(fy3slatArr[i], i, false);
				}
			}
		}
		std::cout << "Indexing done." << endl;
		std::cout << "ilat north south" << endl;
		ofstream tofs("temp.txt");
		for (int il = 0; il < 360; ++il)
		{
			for (int i = 0; i < 10; ++i)
			{
				std::cout << i << " " << northLut[il].v10[i].size() << " " << southLut[il].v10[i].size() << endl;
				tofs << i << " " << northLut[il].v10[i].size() << " " << southLut[il].v10[i].size() << endl;
			}
		}
		tofs.close();
	}

	short* fy4buffer = new short[fy4XSize];
	short* fy4cloudbuffer = new short[fy4XSize];
	short* fy4outbuffer = new short[fy4XSize];
	float* fy4lonbuffer = new float[fy4XSize];
	float* fy4latbuffer = new float[fy4XSize];
	for (int iy = 0; iy < fy4YSize; ++iy)
	{
		fy4ds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, fy4XSize, 1,
			fy4buffer, fy4XSize, 1, GDALDataType::GDT_Int16, 0, 0, nullptr);
		fy4londs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, fy4XSize, 1,
			fy4lonbuffer, fy4XSize, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
		fy4latds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, iy, fy4XSize, 1,
			fy4latbuffer, fy4XSize, 1, GDALDataType::GDT_Float32, 0, 0, nullptr);
		for (int ix = 0; ix < fy4XSize; ++ix)
		{
			fy4outbuffer[ix] = fy4buffer[ix];//先赋给fy4原始像元值
											 //if (ix % 50 != 0 || iy % 50 != 0) continue;//here for debuging.
			float f4lon = fy4lonbuffer[ix];
			float f4lat = fy4latbuffer[ix];

			if (f4lon > 180.f)
			{
				f4lon = f4lon - 360.f;//bugfixed.
			}

			//FY4为雪的时候 snow : fy4 default 200, baizhaofeng 10
			if (fy4buffer[ix] == g_fy4SnowValue )//2017-11-20
			{//
				fy4outbuffer[ix] = fy4buffer[ix];
			}
			else if (fy4buffer[ix] == g_fy4CloudValue )// cloud : fy4 default 50 , baizhaofeng 6. 2017-11-20
			{//fy4-snc云像元50，查找fy3-swe数据。

				if (f4lon > -900.f && f4lat > -900.f)
				{
					if (f4lat >= 0)
					{//北半球
						short fy3val;
						double dist;
						float tfy3lon, tfy3lat;
						int lonindex = (int)f4lon + 180;
						if (lonindex == 360) lonindex = 359;
						int latindex = getLatIndex(f4lat, true);
						bool find = findfy3value(f4lon, f4lat, fy3size, fy3nlonArr, fy3nlatArr, fy3northArr,
							northLut[lonindex].v10[latindex],
							fy3val, dist, tfy3lon, tfy3lat);
						if (find)
						{
							fy4outbuffer[ix] = (fy3val <= 10 || fy3val > 1000) ? fy4buffer[ix] : g_combineSnowValue ;// 2017-11-20
						}
						else {
							//FY4有云，但是FY3没有定位到有效像元
							fy4outbuffer[ix] = fy4buffer[ix];
						}
					}
					else
					{//南半球
						short fy3val;
						double dist;
						float tfy3lon, tfy3lat;
						int lonindex = (int)f4lon + 180;
						if (lonindex == 360) lonindex = 359;
						int latindex = getLatIndex(f4lat, false);
						bool find = findfy3value(f4lon, f4lat, fy3size, fy3slonArr, fy3slatArr, fy3southArr,
							southLut[lonindex].v10[latindex],
							fy3val, dist, tfy3lon, tfy3lat);
						if (find)
						{
							fy4outbuffer[ix] = (fy3val <= 10 || fy3val > 1000) ? fy4buffer[ix] : g_combineSnowValue ;//2017-11-20
						}
						else {
							//FY4有云，但是FY3没有定位到有效像元
							fy4outbuffer[ix] = fy4buffer[ix];
						}
					}
				}
				else
				{
					//lon lat is fill value.
				}
			}
			else
			{
				//fill or space value in cloud.
			}
		}
		outds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, fy4XSize, 1,
			fy4outbuffer, fy4XSize, 1, GDALDataType::GDT_Int16, 0, 0, nullptr);
		wft_term_progress(iy, fy4YSize);
	}

	delete[] fy4buffer; fy4buffer = 0;
	delete[] fy4cloudbuffer; fy4cloudbuffer = 0;
	delete[] fy4outbuffer; fy4outbuffer = 0;
	delete[] fy4lonbuffer; fy4lonbuffer = 0;
	delete[] fy4latbuffer; fy4latbuffer = 0;


	delete[] fy3nlonArr; fy3nlonArr = 0;
	delete[] fy3nlatArr; fy3nlatArr = 0;
	delete[] fy3slonArr; fy3slonArr = 0;
	delete[] fy3slatArr; fy3slatArr = 0;

	delete[] fy3northArr; fy3northArr = 0;
	delete[] fy3southArr; fy3southArr = 0;

	GDALClose(fy4ds);
	GDALClose(outds);
	GDALClose(fy4londs);
	GDALClose(fy4latds);
	//E:/coding/fy4qhzx-project/extras/fy4totxt -in snowfusion_out.tif -out snow-sm.txt -type llfiles -lon E:/coding/fy4qhzx-project/extras/fy4lon.tif 
	//-lat E:/coding/fy4qhzx-project/extras/fy4lat.tif  -valid0 10 -valid1 250 -xspace 5 -yspace 5

	if (wft_test_file_exists(outputfile))
	{
		string temp_txtfile = outputfile + ".xyz.txt";
		string cmd1 = txt + " -in " + outputfile
			+ " -out " + temp_txtfile
			+ " -type llfiles "
			+ " -lon " + fy4lonfile
			+ " -lat " + fy4latfile
			+ " -valid0  " + g_plotvalid0 
			+ " -valid1 " + g_plotvalid1
			+ " -xspace 1 -yspace 1 ";
		int res1 = system(cmd1.c_str());
		cout << "totxt return code : " << res1 << endl;
		if (wft_test_file_exists(temp_txtfile))
		{
			string outpngfile = outputfile + ".png";
			vector<string> v1,v2;
			v1.push_back("{{{OUTFILE}}}");
			v1.push_back("{{{INFILE}}}");
			v1.push_back("{{{DATE}}}");

			int ymdi = (int)atof(ymd.c_str());
			string ymd2 = wft_ymd_int2str(ymdi);

			v2.push_back(outpngfile);
			v2.push_back(temp_txtfile);
			v2.push_back(ymd2);

			string plotfile = outputfile + ".plot";
			wft_create_file_by_template_with_replacement(plotfile, plottemplate, v1, v2);
			if (wft_test_file_exists(plotfile))
			{
				string cmd2 = gnuplot + " " + plotfile;
				int res2 = system(cmd2.c_str());
				cout << "gnuplot return code : " << res2 << endl;
				if (wft_test_file_exists(outpngfile))
				{
					wft_remove_file(temp_txtfile);

					cout << "Warning in windows no insert db " << endl;
					return 0;
					string cmd7 = insert + " -host " + host + " -user " + user
						+ " -pwd " + pwd + " -db " + db + " -tb " + tb
						+ " -datapath " + outputfile
						+ " -dtloc 0 "
						+ " -dtlen 0 "
						+ " -thumb " + outpngfile
						+ " -pid " + pid
						+ " -startdate " + ymd2  
						+" -enddate " + ymd2 ;

					cout << cmd7 << endl;
					int res7 = system(cmd7.c_str());
					cout << "insertdb result:" << res7 << endl;

				}
				else {
					cout << "Error : make " << outpngfile << " failed." << endl;
					return 10;
				}
			}
			else {
				cout << "Error : make " << plotfile << " failed." << endl;
				return 10;
			}

		}
		else {
			cout << "Error : make " << temp_txtfile << " failed." << endl;
			return 10;
		}
	}
	else {
		cout << "Error : make " << outputfile << " failed." << endl;
		return 10;
	}
	return 100;
}

//bzf FY4A-_AGRI--_N_DISK_0995E_L3-_SNC-_MULT_NOM_20170401000000_2017401235959_4000M_V0001
//FY4A-_AGRI--_N_DISK_1047E_L3-_SNC-_MULT_NOM_20171029000000_20171029235959_4000M_ADV01.NC
//FY3C_MWRIX_GBAL_L2_SWE_MLT_ESD_20170204_POAD_025KM_MS.HDF
bool isValidFy4SncL3File(string& filename)
{
	int pos0 = filename.find(g_fy4fixprefix);
	int pos1 = filename.find(g_fy4fixtail);
	if (pos0 != string::npos && pos1 != string::npos && pos1 == filename.length() - 15)
	{
		return true;
	}
	else {
		return false;
	}
}



int main(int argc , char** argv )
{
	std::cout << "fy4 and fy3 snow cover fusion program." << std::endl;
	std::cout << "Version 0.1a  by wangfeng1@piesat.cn 2017-11-06." << std::endl;
	std::cout << "Version 0.2a  cloud in snc is the same with cloud in clm product over land, so no need cld product." << std::endl;
	std::cout << "Version 0.3a  monitor functionbility 2017-11-16." << std::endl;
	std::cout << "Version 0.4a  confirm fy3 is continuous. 2017-11-1." << std::endl;
	std::cout << "Version 0.5a  add baizhaofeng combine data support. 2017-11-20." << std::endl;
	if (argc == 1)
	{
		std::cout << "Sample call:" << std::endl;
		std::cout << "snowfusion startup.txt" << std::endl;

		cout << "**************  Sample startup.txt  *****************" << endl;

		cout << "#fy4dir" << endl;
		cout << "/fy4snc/" << endl;
		cout << "#fy3dir" << endl;
		cout << "/fy3swe/" << endl;
		cout << "#outdir" << endl;
		cout << "/snow-out/" << endl;
		cout << "#outprefix" << endl;
		cout << "fy4a_fy3c_snowfusion." << endl;
		cout << "#fy4lon" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy4lon.tif" << endl;
		cout << "#fy4lat" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy4lat.tif" << endl;
		cout << "#fy3northlon" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy3easegrid_northern_lon.tif" << endl;
		cout << "#fy3northlat" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy3easegrid_northern_lat.tif" << endl;
		cout << "#fy3southlon" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy3easegrid_southern_lon.tif" << endl;
		cout << "#fy3southlat" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy3easegrid_southern_lat.tif" << endl;
		cout << "#totxt" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy4totxt" << endl;
		cout << "#plot" << endl;
		cout << "gnuplot" << endl;
		cout << "#plottem" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fsnow2017-11-16.plot" << endl;
		cout << "#insertprogram" << endl;
		cout << "/QHZX_DATA/produce_codes/insertdb/insertdb" << endl;
		cout << "#host" << endl;
		cout << "localhost" << endl;
		cout << "#user" << endl;
		cout << "htht" << endl;
		cout << "#pwd" << endl;
		cout << "htht123456" << endl;
		cout << "#db" << endl;
		cout << "qhzx_uus" << endl;
		cout << "#tb" << endl;
		cout << "tb_product_data" << endl;
		cout << "#pid" << endl;
		cout << "53" << endl;
		cout << "#bzf" << endl;
		cout << "1" << endl;

		cout << "****************************************" << endl;

		exit(101);
	}

	string startupfile = argv[1];

	string fy4dir = wft_getValueFromExtraParamsFile( startupfile , "#fy4dir" , true ) ;
	string fy3dir = wft_getValueFromExtraParamsFile(startupfile, "#fy3dir", true);
	string outdir = wft_getValueFromExtraParamsFile(startupfile, "#outdir", true);
	string outprefix = wft_getValueFromExtraParamsFile(startupfile, "#outprefix", true);
	string fy4lon = wft_getValueFromExtraParamsFile(startupfile, "#fy4lon", true);
	string fy4lat = wft_getValueFromExtraParamsFile(startupfile, "#fy4lat", true);
	string fy3lon1 = wft_getValueFromExtraParamsFile(startupfile, "#fy3northlon", true);
	string fy3lat1 = wft_getValueFromExtraParamsFile(startupfile, "#fy3northlat", true);
	string fy3lon2 = wft_getValueFromExtraParamsFile(startupfile, "#fy3southlon", true);
	string fy3lat2 = wft_getValueFromExtraParamsFile(startupfile, "#fy3southlat", true);

	string txtProgram = wft_getValueFromExtraParamsFile(startupfile, "#totxt", true);
	string gnuplot = wft_getValueFromExtraParamsFile(startupfile, "#plot", true);
	string plotTemplate = wft_getValueFromExtraParamsFile(startupfile, "#plottem", true);
	string insertdb = wft_getValueFromExtraParamsFile(startupfile, "#insertprogram", true);
	string host = wft_getValueFromExtraParamsFile(startupfile, "#host", true);
	string user = wft_getValueFromExtraParamsFile(startupfile, "#user", true);
	string pwd = wft_getValueFromExtraParamsFile(startupfile, "#pwd", true);
	string db = wft_getValueFromExtraParamsFile(startupfile, "#db", true);
	string tb = wft_getValueFromExtraParamsFile(startupfile, "#tb", true);
	string pid = wft_getValueFromExtraParamsFile(startupfile, "#pid", true);


	string bzf = wft_getValueFromExtraParamsFile(startupfile, "#bzf", false);//2017-11-20.
	if (bzf == "1")
	{
		g_fy4CloudValue = 6;
		g_fy4SnowValue = 10;
		g_combineSnowValue = 10;
		g_plotvalid0 = "3";
		g_plotvalid1 = "15";
		g_dsPrefix = "";
		g_dsTail = "";
		g_fy4fixtail = "_4000M_V0001.NC";
		g_fy4fixprefix = "FY4A-_AGRI--_N_DISK_";//bzf FY4A-_AGRI--_N_DISK_0995E_L3-_SNC-
	}
	else {
		g_fy4CloudValue = 50; //bzf 6
		g_fy4SnowValue = 200; //bzf 10
		g_combineSnowValue = 201; //bzf 10
		g_plotvalid0 = "10"; //bzf 3 
		g_plotvalid1 = "250";//bzf 15
		g_dsPrefix = "HDF5:\"";// bzf ""
		g_dsTail = "\"://SNC";// bzf "" 
		g_fy4fixtail = "_4000M_ADV01.NC";//   _4000M_V0001.NC
		g_fy4fixprefix = "FY4A-_AGRI--_N_DISK_"; 
	}

	
	GDALAllRegister();
	
	vector<string> allfy4files;
	vector<string> allfy3files;

	wft_get_allfiles(fy4dir, allfy4files);
	wft_get_allfiles(fy3dir, allfy3files);
	
	for (int i = 0; i < allfy4files.size(); ++i)
	{
		string filepath = allfy4files[i];
		string filename = wft_base_name(filepath);
		if (isValidFy4SncL3File(filename))
		{
			//FY3C_MWRIX_GBAL_L2_SWE_MLT_ESD_20170204_POAD_025KM_MS.HDF
			string ymd = filename.substr(44, 8);
			string fy3name = string("FY3C_MWRIX_GBAL_L2_SWE_MLT_ESD_") + ymd + "_POAD_025KM_MS.HDF";
			string fy3path = fy3dir + fy3name;
			if (wft_test_file_exists(fy3path))
			{
				string outfilename = outprefix + ymd + ".tif";
				string outpath = outdir + outfilename;
				if (wft_test_file_exists(outpath) == false )
				{
					cout << "find pair : " << filepath << " , " << fy3path << endl;
					int res = processOneFile(filepath, fy4lon, fy4lat, outpath, fy3path, fy3lon1, fy3lat1, fy3lon2, fy3lat2 , 
						txtProgram , 
						gnuplot , 
						plotTemplate , 
						insertdb , 
						host , 
						user , 
						pwd , 
						db , 
						tb ,
						pid , 
						ymd );
					if (res == 100)
					{
						break;
					}
				}
			}
		}
	}
	

	cout << "done." << endl;
	
    return 0;
}

//int main(int argc, char** argv)
//{
//	doit(argc, argv);
//	return 1;
//}