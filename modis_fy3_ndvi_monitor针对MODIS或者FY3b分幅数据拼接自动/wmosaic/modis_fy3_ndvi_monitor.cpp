// modis_fy3_ndvi_monitor.cpp : 定义控制台应用程序的入口点。
//modis , fy3b 数据拼接->几何校正-》出图-》入库 



/*
中国区域modis分幅
 h23v03 ... ... h28v03
 ...   ...   ...  ...
 h23v07 ... ... h28v07

 中国区fy3b分幅
 5060  5070  ...  50A0  50B0
 ... ... ... ...
 2060  2070  ...  20A0  20B0


*/

#include "stdafx.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "../../sharedcodes/wftools.h"
using namespace std;

const double earthRadius = 6378137;


std::string fyVertFenFuHaoArray[18] = { "80","70","60","50","40",
"30","20","10","00",
"90","A0","B0","C0","D0",
"E0","F0","G0","H0" };

std::string fyHoriFenFuHaoArray[36] = {
	"Z0","Y0","X0","W0","V0" ,
	"U0","T0","S0","R0","Q0",
	"P0","O0","N0","M0","L0",
	"K0","J0","I0",
	"00","10","20","30","40",
	"50","60","70","80","90",
	"A0","B0","C0","D0","E0",
	"F0","G0","H0" };


int getHTileIndex(string tname)
{
	if (tname.length() == 3)
	{
		return (int)atof(tname.substr(1, 2).c_str());
	}
	else {
		for (int i = 0; i < 36; ++i)
		{
			if (tname == fyHoriFenFuHaoArray[i])
			{
				return i;
			}
		}
	}
	return -1;
}

int getVTileIndex(string tname)
{
	if (tname.length() == 3)
	{
		return (int)atof(tname.substr(1, 2).c_str());
	}
	else {
		for (int i = 0; i < 18; ++i)
		{
			if (tname == fyVertFenFuHaoArray[i])
			{
				return i;
			}
		}
	}
	return -1;
}

void getTileIndex( string filename , int hloc , int hlen , int vloc , int vlen , int& hindex , int& vindex )
{
	if (hlen == 3)
	{
		hindex = (int)atof(filename.substr(hloc + 1, 2).c_str());
		vindex = (int)atof(filename.substr(vloc + 1, 2).c_str());
	}
	else {
		string hstr = filename.substr(hloc, hlen);
		string vstr = filename.substr(vloc, vlen);
		hindex = -1;
		vindex = -1;
		for (int i = 0; i < 18; ++i)
		{
			if (vstr == fyVertFenFuHaoArray[i])
			{
				vindex = i;
				break;
			}
		}
		for (int i = 0; i < 36; ++i)
		{
			if (hstr == fyHoriFenFuHaoArray[i])
			{
				hindex = i;
				break;
			}
		}
	}
}





int processOne( vector<string>& infiles , string& outpath ,
	string dsPrefix , string dsTail , 
	int numHortTiles , int numVertTiles , 
	string targetYmd ,
	string displayYmdStr,
	int htloc ,int htlen , int vtloc , int vtlen ,
	int ymdloc , int ymdlen , int htile0 , int htile1 , int vtile0 , int vtile1 , double fill ,
	int tileXSize , int tileYSize , GDALDataType dataType , string lonfile , string latfile , 
	double tileWid , double tileHei , double valid0 , double valid1 ,
	string gdalwarp , string createvrt , 
	string orx , string ory , 
	string cutshpfile , 
	string image2xyz , 
	string valid0str , 
	string valid1str , 
	string scalestr , 
	string offsetstr , 
	string plot , 
	string plottem ,
	string insert , 
	string host , 
	string user , 
	string pwd , 
	string db , 
	string tb , 
	string pid , 
	double modisleft , 
	double modistop 
	)
{

	bool isfy3 = false;
	if (htlen == 2) isfy3 = true;

	if (tileXSize == 0 || tileYSize == 0) {
		std::cout << "*** Error: Invalide tileXSize or tileYSize. out.";
		return 104;
	}

	int totalXSize = tileXSize * numHortTiles;
	int totalYSize = tileYSize * numVertTiles;

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");

	string outputMos = outpath + ".mos.tif";
	GDALDataset* outDataset = driver->Create(outputMos.c_str(), totalXSize, totalYSize, 1, dataType, 0);

	if (isfy3 == false)
	{
		if (modisleft == 0 || modistop == 0)
		{
			cout << "Error : modis left or top is zero. out." << endl;
			GDALClose(outDataset);
			return 105;
		}
		//250m ->231.6563583
		//500m ->463.3127165
		//1km  ->926.6254331
		double geotrans[6] = {0,0,0,0,0,0};
		geotrans[0] = modisleft;
		geotrans[3] = modistop;
		if (tileXSize == 1200)
		{
			geotrans[1] = 926.6254331;
			geotrans[5] = -926.6254331;
		}
		else if (tileXSize == 2400)
		{
			geotrans[1] = 463.3127165;
			geotrans[5] = -463.3127165;
		}
		else {
			geotrans[1] = 231.6563583;
			geotrans[5] = -231.6563583;
		}
		outDataset->SetGeoTransform(geotrans);
		{
			OGRSpatialReference osrs;
			osrs.importFromProj4("+proj=sinu +lon_0=0 +x_0=0 +y_0=0 +a=6371007.181 +b=6371007.181 +units=m +no_defs");
			char* sinuProj = 0;
			osrs.exportToWkt(&sinuProj);
			outDataset->SetProjection(sinuProj);
			CPLFree(sinuProj);
		}
	}

	GDALDataset* londs = 0;
	GDALDataset* latds = 0;

	bool needDeleteLonFile = false;
	bool needDeleteLatFile = false;
	if (lonfile == "" && isfy3 )
	{
		needDeleteLonFile = true;
		lonfile = outpath + ".lon.tif";
		londs = driver->Create(lonfile.c_str(), totalXSize, totalYSize, 1, GDT_Float32 , 0);
	}
	if (latfile == "" && isfy3 )
	{
		needDeleteLatFile = true;
		latfile = outpath + ".lat.tif";
		latds = driver->Create(latfile.c_str(), totalXSize, totalYSize, 1, GDT_Float32, 0);
	}


	int* buff = new int[totalXSize];
	float* llbuff = new float[totalXSize];
	for (int i = 0; i < totalXSize; ++i) {
		buff[i] = fill;
		llbuff[i] = -999.f ;
	}
	cout << "filling output datafiles..." << endl;
	for (int iy = 0; iy < totalYSize; ++iy)
	{
		outDataset->GetRasterBand(1)->RasterIO(GF_Write, 0 , iy , totalXSize, 1,
			buff, totalXSize, 1, GDT_Int32, 0, 0, 0);
		if( londs ) 
			londs->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, totalXSize, 1,
			llbuff, totalXSize, 1, GDT_Float32, 0, 0, 0);
		if( latds ) 
			latds->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, totalXSize, 1,
			llbuff, totalXSize, 1, GDT_Float32, 0, 0, 0);
		wft_term_progress(iy, totalYSize);
	}
	delete[] buff; buff = 0;
	delete[] llbuff; llbuff = 0;

	double irx = tileWid / tileXSize;
	double iry = tileHei / tileYSize;
	if (isfy3)
	{
		irx = 1002.228;
		iry = 1002.228; 
	}

	cout << "Mosaicing ... " << endl;
	int* bufftile = new int[tileXSize*tileYSize];
	float* lonbuff = new float[tileXSize * tileYSize];
	float* latbuff = new float[tileXSize * tileYSize];
	for (int ifile = 0; ifile < infiles.size(); ++ifile)
	{
		string filepath = infiles[ifile];
		string filename = wft_base_name(filepath);
		string tymd = filename.substr(ymdloc, ymdlen);
		if (tymd == targetYmd)
		{
			int hi, vi;
			getTileIndex(filename, htloc, htlen, vtloc, vtlen, hi, vi);
			if (hi >= htile0 && hi <= htile1 && vi >= vtile0&& vi <= vtile1)
			{
				string dspath = dsPrefix + infiles[ifile] + dsTail;
				GDALDataset* dsone = (GDALDataset*)GDALOpen(dspath.c_str(), GDALAccess::GA_ReadOnly);
				if (dsone == 0)
				{
					cout << "Error : can not open " << dspath << " . go next." << endl;
					continue;
				}
				int xsize1 = dsone->GetRasterXSize();
				int ysize1 = dsone->GetRasterYSize();
				if (xsize1 != tileXSize || ysize1 != tileYSize)
				{
					std::cout << "*** Error: " << infiles[ifile] << " has a differenct xsize,ysize . out.";
					GDALClose(dsone);
					continue;
				}

				dsone->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, 0, tileXSize, tileYSize, bufftile, tileXSize, tileYSize,
					GDALDataType::GDT_Int32, 0, 0, 0);
				outDataset->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, (hi - htile0)*tileXSize, (vi - vtile0)*tileYSize, tileXSize, tileYSize,
					bufftile, tileXSize, tileYSize, GDALDataType::GDT_Int32, 0, 0, 0);
				GDALClose(dsone);

				if (londs || latds )
				{
					cout << "Making lon lat for tile ... " << endl;
					int allsize = tileXSize * tileYSize;
					for (int i = 0; i < allsize; ++i)
					{
						lonbuff[i] = -999.f;
						latbuff[i] = -999.f;
					}


					if (isfy3)
					{
						float ffLeftBottomX0 = ( hi - 18)*tileXSize*irx + irx / 2;
						float ffLeftBottomY0 = (8 - vi )*tileYSize*iry + iry / 2;

						for (int iy = 0; iy < tileYSize; ++iy)
						{
							for (int ix = 0; ix < tileXSize; ++ix)
							{
								int i1d = iy*tileXSize + ix;
								if (bufftile[i1d] >= valid0 && bufftile[i1d] <= valid1)
								{
									double mapx = ffLeftBottomX0 + ix * irx;
									double mapy = ffLeftBottomY0 + (tileYSize - 1 - iy)*iry;

									double mapx2 = mapx / earthRadius;
									double mapy2 = mapy / earthRadius;
									//=SQRT(1-(O2/4)*(O2/4)-(O3/2)*(O3/2))
									double tempZ = sqrt(1.0 - (mapx2 / 4)*(mapx2 / 4) - (mapy2 / 2)*(mapy2 / 2));
									//= 2 * ATAN(O5*O2 / 2 / (2 * O5*O5 - 1)) * 180 / 3.14;
									double templon = 2 * atan(tempZ*mapx2 / 2 / (2 * tempZ*tempZ - 1)) * 180 / M_PI;
									//=ASIN(O5*O3)*180/3.14
									double templat = asin(tempZ*mapy2) * 180 / M_PI;
									if (londs) {
										lonbuff[i1d] = templon;
									}
									if (latds)
									{
										latbuff[i1d]=templat;
									}
								}
							}
							wft_term_progress(iy, tileYSize);
						}

					}
					else {
						for (int iy = 0; iy < tileYSize; ++iy)
						{
							for (int ix = 0; ix < tileXSize; ++ix)
							{
								int i1d = iy*tileXSize + ix;
								if (bufftile[i1d] >= valid0 && bufftile[i1d] <= valid1)
								{
									double templat = 90.0 - ( vi * tileYSize + iy - 0.5) * iry;
									double templon = ((hi - 18) * tileXSize + ix + 1.5) * irx / cos( templat * M_PI / 180.0);
									//这个地方modis校正后的数据总是和矢量数据相比偏西边一点。

									if (londs) {
										lonbuff[i1d] = templon;
									}
									if (latds)
									{
										latbuff[i1d] = templat;
									}
								}
							}
							wft_term_progress(iy, tileYSize);
						}
					}
					if( londs ) londs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, (hi - htile0)*tileXSize, (vi - vtile0)*tileYSize, tileXSize, tileYSize,
						lonbuff, tileXSize, tileYSize, GDALDataType::GDT_Float32, 0, 0, 0);
					if( latds) latds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, (hi - htile0)*tileXSize, (vi - vtile0)*tileYSize, tileXSize, tileYSize,
						latbuff, tileXSize, tileYSize, GDALDataType::GDT_Float32, 0, 0, 0);
				}
				string filename1 = wft_base_name(infiles[ifile]);
				std::cout << "mosic " << filename1 << " done." << endl;
			}
		}
		
	}
	delete[] bufftile; bufftile = 0;
	delete[] lonbuff; lonbuff = 0;
	delete[] latbuff; latbuff = 0;

	GDALClose(outDataset);
	if( londs != 0 ) GDALClose(londs);
	if (latds != 0) GDALClose(latds);

	if (isfy3 && wft_test_file_exists(latfile)==false )
	{
		cout << "Error : lat file is not exist " << latfile << endl;
		return 201;
	}
	if (isfy3 && wft_test_file_exists(lonfile)==false )
	{
		cout << "Error : lon file is not exist " << lonfile << endl;
		return 202;
	}

	if (wft_test_file_exists(outputMos)==false )
	{
		cout << "Error : output mosaic file is not exist " << outputMos << endl;
		return 203;
	}

	//warping
	string lut = wft_int2str(valid0 - 1) + ":-9999," + wft_int2str(valid0) + ":" + wft_int2str(valid0) + ","  + wft_int2str(valid1) + ":" + wft_int2str(valid1) + "," + wft_int2str(valid1 + 1) + ":-9999";
	cout << "Warping wgs84 ... " << endl;
	if (isfy3)
	{
		string cmd1 = createvrt + " -type llfile -in " + outputMos + " -lonfile " + lonfile
			+ " -latfile " + latfile
			+ " -temproot " + outputMos
			+ " -llnodata -999 "
			+ " -pdtnodata -9999"
			+ " -pdtlut " + lut
			+ " -outtif " + outpath
			+ " -gdalwarp " + gdalwarp
			+ " -orx " + orx
			+ " -ory " + ory;

		int res1 = system(cmd1.c_str());
		cout << "fy3 warp result:" << res1 << endl;
	}
	else {
		string tr = "";
		tr = " -tr 0.01 0.01 ";
		string cmd1 = gdalwarp + " -srcnodata \"-3000\" -dstnodata \"-3000\" -t_srs EPSG:4326  " + tr + outputMos + "  " + outpath;
		int res1 = system(cmd1.c_str());
		cout << "modis warp result:" << res1 << endl;
	}
	

	if (wft_test_file_exists(outpath)==false )
	{
		cout << "Error : output  file is not exist " << outpath << endl;
		return 204;
	}

	if (needDeleteLonFile) wft_remove_file(lonfile);
	if (needDeleteLatFile) wft_remove_file(latfile);

	string usecut = "";
	if (cutshpfile != "")
	{
		usecut = " -crop_to_cutline -q -cutline " + cutshpfile;
	}
	//gdalwarp - overwrite - t_srs "+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47" - srcnodata - 9999 - dstnodata - 9999 ndvichina.tif alberschina.tif
	string outputalbers = outpath + ".albers.tif";
	string cmd2 = gdalwarp + " -overwrite "
		+ " -t_srs \"+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47\" "
		+ " -srcnodata -9999 "
		+ " -dstnodata -9999 "
		+ usecut 
		+ " -of GTiff "
		+ outpath
		+ " "
		+ outputalbers;
	int res2 = system(cmd2.c_str());
	cout << "warp albers result:" << res2 << endl;

	if (wft_test_file_exists(outputalbers)==false )
	{
		cout << "Error : output albers  file is not exist " << outputalbers << endl;
		return 205;
	}

	if (image2xyz != "" )
	{
		GDALDataset * tempds =(GDALDataset *) GDALOpen( outputalbers.c_str() , GA_ReadOnly);
		int axsize = tempds->GetRasterXSize();
		int aysize = tempds->GetRasterYSize();
		GDALClose(tempds);
		int xspace = 1;
		int yspace = 1;
		if (axsize > 2000)
		{
			xspace = axsize / 2000;
		}
		if (aysize > 2000)
		{
			yspace = aysize / 2000;
		}
		string txtfile = outputalbers + ".xyz.txt";
		string cmd3 = image2xyz + " -in " + outputalbers + " -out " + txtfile + " -valid0 " + valid0str
			+ " -valid1 " + valid1str
			+ " -scale " + scalestr
			+ " -offset " + offsetstr
			+ " -xspace " + wft_int2str(xspace)
			+ " -yspace " + wft_int2str(yspace);

		int res3 = system(cmd3.c_str());
		cout << " image2xyz result : " << res3 << endl;
		if (wft_test_file_exists(txtfile) == false)
		{
			cout << "Error : failed to make " + txtfile << endl;
			return 206;
		}

		vector<string> varVec, repVec;
		varVec.push_back("{{{OUTFILE}}}");
		varVec.push_back("{{{INFILE}}}");
		varVec.push_back("{{{TITLE}}}");
		varVec.push_back("{{{PTIME}}}");

		string pngfile = outpath + ".png";
		repVec.push_back(pngfile);
		repVec.push_back(txtfile);
		repVec.push_back(displayYmdStr);
		repVec.push_back(wft_current_datetimestr());

		string plotfile = outpath + ".plot";
		wft_create_file_by_template_with_replacement(plotfile, plottem, varVec, repVec);
		if (wft_test_file_exists(plotfile) == false)
		{
			cout << "Error : failed to make " + plotfile << endl;
			return 207;
		}

		string cmd4 = plot + " " + plotfile;
		int res4 = system(cmd4.c_str());
		cout << "gnuplot result : " << res4 << endl;
		if (wft_test_file_exists(pngfile) == false)
		{
			cout << "Error : failed to make " + pngfile << endl;
			return 208;
		}
		wft_remove_file(txtfile);


		if ( insert != "" )
		{
			//insert db.
			string cmd7 = insert + " -host " + host + " -user " + user
				+ " -pwd " + pwd + " -db " + db + " -tb " + tb
				+ " -datapath " + outpath
				+ " -dtloc 0 "  
				+ " -dtlen 0 "
				+ " -thumb " + pngfile
				+ " -pid " + pid
				+ " -startdate " + displayYmdStr 
				+ " -enddate " + displayYmdStr ;

			cout << cmd7 << endl;
			int res7 = system(cmd7.c_str());
			cout << "insertdb result:" << res7 << endl;
		}

	}


	return 100;
}



//FY3B_VIRRX_5060_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI",
int main(int argc , char** argv )
{
	if (argc == 1)
	{
		std::cout << "*** Program description:" << std::endl;
		std::cout << "A program to mosic images." << std::endl;
		std::cout << "Version 1.0a 2017-11-23 wangfengdev@163.com" << std::endl;
		std::cout << "Version 2.0a 2017-11-25 modis warping method use geotrans and rsr.wangfengdev@163.com" << std::endl;
		std::cout << "*** Sample call: *** " << std::endl;
		std::cout << " wmosaicmonitor startup.txt " << std::endl;
		cout << "**************** example startup.txt *********************" << endl;
		cout << "#indir" << endl;
		cout << "E:/testdata/fy3bvirrndvi/2017/" << endl;
		cout << "#outdir" << endl;
		cout << "D:/" << endl;
		cout << "#outprefix" << endl;
		cout << "fy3b_ndvi_aotd." << endl;
		cout << "#outtail" << endl;
		cout << ".tif" << endl;
		cout << "#proj" << endl;
		cout << "ham" << endl;
		cout << "#htilenameloc" << endl;
		cout << "13" << endl;
		cout << "#htilenamelen" << endl;
		cout << "2" << endl;
		cout << "#vtilenameloc" << endl;
		cout << "11" << endl;
		cout << "#vtilenamelen" << endl;
		cout << "2" << endl;
		cout << "#htile0" << endl;
		cout << "50" << endl;
		cout << "#vtile0" << endl;
		cout << "60" << endl;
		cout << "#htilecnt" << endl;
		cout << "8" << endl;
		cout << "#vtilecnt" << endl;
		cout << "6" << endl;
		cout << "#dsprefix" << endl;
		cout << "HDF5:\""<<endl ;
		cout << "#dstail" << endl;
		cout << "\"://1KM_10day_NDVI"<<endl ;
		cout << "#fnfixprefix" << endl;
		cout << "FY3B_VIRRX_" << endl;
		cout << "#fnfixtail" << endl;
		cout << "AOTD_1000M_MS.HDF" << endl;
		cout << "#fnfixmidloc" << endl;
		cout << "" << endl;
		cout << "#fnfixmidstr" << endl;
		cout << "" << endl;
		cout << "#fnymdloc" << endl;
		cout << "31" << endl;
		cout << "#fnymdlen" << endl;
		cout << "8" << endl;
		cout << "#lonfile" << endl;
		cout << "" << endl;
		cout << "#latfile" << endl;
		cout << "" << endl;
		cout << "#valid0" << endl;
		cout << "-2000" << endl;
		cout << "#valid1" << endl;
		cout << "10000" << endl;
		cout << "#scale" << endl;
		cout << "0.0001" << endl;
		cout << "#offset" << endl;
		cout << "0" << endl;
		cout << "#tilewid" << endl;
		cout << "10" << endl;
		cout << "#tilehei" << endl;
		cout << "10" << endl;
		cout << "#orx" << endl;
		cout << "0.01" << endl;
		cout << "#ory" << endl;
		cout << "0.01" << endl;
		cout << "#gdalwarp" << endl;
		cout << "gdalwarp" << endl;
		cout << "#createvrt" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/createwarpvrt" << endl;
		cout << "#cutline" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/shp/cutchina.shp" << endl;
		cout << "#totxt" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/image2xyz" << endl;
		cout << "#gnuplot" << endl;
		cout << "gnuplot" << endl;
		cout << "#plottem" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/fy3b_aotd_ndvi.plot" << endl;
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
		cout << "59" << endl;
		cout << "#modisleft" << endl;
		cout << "5559752.598333" << endl;
		cout << "#modistop" << endl;
		cout << "6671703.118000" << endl;
		cout << "" << endl;
		cout << "**************** **************** *********************" << endl;
		std::cout << "*** No enough parameters, out.***" << std::endl;
		exit(101) ;
	}

	string startup = argv[1];
	string inDir, outDir, proj, htile0, vtile0, htilecntStr, vtilecntStr, dsPrefix, dsTail ,
		fnFixPrefix , fnFixTail , fnFixMidLoc , fnFixMidStr , fnYmdLoc , fnYmdLen ,
		lonfile , latfile , valid0Str , valid1Str , scaleStr , offsetStr , tileWidStr, tileHeiStr , orxStr , oryStr,
		outPrefix , outTail , htnameLocStr , htnameLenStr , vtnameLocStr , vtnameLenStr,
		gdalwarp , createvrt ;

	inDir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	outDir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);

	outPrefix = wft_getValueFromExtraParamsFile(startup, "#outprefix", true);
	outTail = wft_getValueFromExtraParamsFile(startup, "#outtail", true);

	proj = wft_getValueFromExtraParamsFile(startup, "#proj", true);

	htile0 = wft_getValueFromExtraParamsFile(startup, "#htile0", true);
	vtile0 = wft_getValueFromExtraParamsFile(startup, "#vtile0", true);

	int useHTile0 = getHTileIndex(htile0);
	int useVTile0 = getVTileIndex(vtile0);

	htilecntStr = wft_getValueFromExtraParamsFile(startup, "#htilecnt", true);
	vtilecntStr = wft_getValueFromExtraParamsFile(startup, "#vtilecnt", true);

	int nhtiles = (int)atof(htilecntStr.c_str());
	int nvtiles = (int)atof(vtilecntStr.c_str());
	int useHTile1 = useHTile0 + nhtiles - 1;
	int useVTile1 = useVTile0 + nvtiles - 1;

	dsPrefix = wft_getValueFromExtraParamsFile(startup, "#dsprefix", true);
	dsTail = wft_getValueFromExtraParamsFile(startup, "#dstail", true);

	int numHortTiles = (int)atof(htilecntStr.c_str()) ;
	int numVertTiles = (int)atof(vtilecntStr.c_str()) ;

	fnFixPrefix = wft_getValueFromExtraParamsFile(startup, "#fnfixprefix", true);
	fnFixTail = wft_getValueFromExtraParamsFile(startup, "#fnfixtail", true);

	fnFixMidLoc = wft_getValueFromExtraParamsFile(startup, "#fnfixmidloc", true);
	fnFixMidStr = wft_getValueFromExtraParamsFile(startup, "#fnfixmidstr", true);
	int filenameFixMidLoc = -1;
	if (fnFixMidStr != "")
	{
		filenameFixMidLoc = (int)atof(fnFixMidLoc.c_str());
	}

	fnYmdLoc = wft_getValueFromExtraParamsFile(startup, "#fnymdloc", true);
	fnYmdLen = wft_getValueFromExtraParamsFile(startup, "#fnymdlen", true);
	int filenameYmdLoc = (int)atof(fnYmdLoc.c_str());
	int filenameYmdLen = (int)atof(fnYmdLen.c_str());

	lonfile = wft_getValueFromExtraParamsFile(startup, "#lonfile", true);
	latfile = wft_getValueFromExtraParamsFile(startup, "#latfile", true);

	valid0Str = wft_getValueFromExtraParamsFile(startup, "#valid0", true);
	valid1Str = wft_getValueFromExtraParamsFile(startup, "#valid1", true);
	double valid0 = atof(valid0Str.c_str());
	double valid1 = atof(valid1Str.c_str());

	scaleStr = wft_getValueFromExtraParamsFile(startup, "#scale", true);
	offsetStr = wft_getValueFromExtraParamsFile(startup, "#offset", true);
	double scale = atof(scaleStr.c_str());
	double offset = atof(offsetStr.c_str());

	tileWidStr = wft_getValueFromExtraParamsFile(startup, "#tilewid", true);
	tileHeiStr = wft_getValueFromExtraParamsFile(startup, "#tilehei", true);
	double tileWid = atof(tileWidStr.c_str());
	double tileHei = atof(tileHeiStr.c_str());

	orxStr = wft_getValueFromExtraParamsFile(startup, "#orx", true);
	oryStr = wft_getValueFromExtraParamsFile(startup, "#ory", true);
	double orx = atof(orxStr.c_str());
	double ory = atof(oryStr.c_str());

	htnameLocStr = wft_getValueFromExtraParamsFile(startup, "#htilenameloc", true);
	htnameLenStr = wft_getValueFromExtraParamsFile(startup, "#htilenamelen", true);
	int htLoc = (int)atof(htnameLocStr.c_str());
	int htLen = (int)atof(htnameLenStr.c_str());

	vtnameLocStr = wft_getValueFromExtraParamsFile(startup, "#vtilenameloc", true);
	vtnameLenStr = wft_getValueFromExtraParamsFile(startup, "#vtilenamelen", true);
	int vtLoc = (int)atof(vtnameLocStr.c_str());
	int vtLen = (int)atof(vtnameLenStr.c_str());

	gdalwarp = wft_getValueFromExtraParamsFile(startup, "#gdalwarp", true);
	createvrt = wft_getValueFromExtraParamsFile(startup, "#createvrt", true);

	string cutline, image2xyz;
	cutline = wft_getValueFromExtraParamsFile(startup, "#cutline", true);
	image2xyz = wft_getValueFromExtraParamsFile(startup, "#totxt", true);

	string gnuplot, plotTemplate;
	gnuplot = wft_getValueFromExtraParamsFile(startup, "#gnuplot", true);
	plotTemplate = wft_getValueFromExtraParamsFile(startup, "#plottem", true);

	string insert, host, user, pwd, db, tb, pid;
	insert = wft_getValueFromExtraParamsFile(startup, "#insertprogram", true);
	host = wft_getValueFromExtraParamsFile(startup, "#host", true);
	user = wft_getValueFromExtraParamsFile(startup, "#user", true);
	pwd = wft_getValueFromExtraParamsFile(startup, "#pwd", true);
	db = wft_getValueFromExtraParamsFile(startup, "#db", true);
	tb = wft_getValueFromExtraParamsFile(startup, "#tb", true);
	pid = wft_getValueFromExtraParamsFile(startup, "#pid", true);

	string modisleftStr, modistopStr;
	modisleftStr = wft_getValueFromExtraParamsFile(startup, "#modisleft", true);
	modistopStr = wft_getValueFromExtraParamsFile(startup, "#modistop", true);
	double modisleft = 0;
	double modistop = 0;
	if (modisleftStr != "")
	{
		modisleft = atof(modisleftStr.c_str());
	}
	if (modistopStr != "")
	{
		modistop = atof(modistopStr.c_str());//2017-11-26
	}
	
	std::vector<std::string> selectedfiles;
	wft_get_allSelectedFiles(inDir, fnFixPrefix, fnFixTail, filenameFixMidLoc, fnFixMidStr, selectedfiles);

	int numfiles = (int)selectedfiles.size();
	if (numfiles == 0)
	{
		cout << "Warning : Selected files count is zero.out." << endl;
		exit(102);
	}

	GDALAllRegister();
	for (int i = 0; i < numfiles; ++i)
	{
		string filepath = selectedfiles[i];
		string filename = wft_base_name(filepath);
		int hi, vi;
		getTileIndex(filename, htLoc, htLen, vtLoc, vtLen, hi, vi);
		if (hi >= useHTile0 && hi <= useHTile1 && vi >= useVTile0 && vi <= useVTile1)
		{
			string ymd0 = filename.substr(filenameYmdLoc, filenameYmdLen);
			int ymd1 = (int)atof(ymd0.c_str());
			if (filenameYmdLen == 7)
			{
				int tyear = ymd1 / 1000;
				int tday = ymd1 % 1000;
				int rmon, rday;
				wft_convertdayofyear2monthday(tyear, tday, rmon, rday);
				ymd1 = tyear * 10000 + rmon * 100 + rday;
			}
			string tempoutfilepath = outDir + outPrefix + wft_int2str(ymd1) + outTail;
			if (wft_test_file_exists(tempoutfilepath) == false)
			{
				cout << "making " << tempoutfilepath << endl;
				int tileXSize, tileYSize;
				GDALDataType dataType;
				{
					string ds0path = dsPrefix + filepath + dsTail;
					GDALDataset* tempDs = (GDALDataset*)GDALOpen(ds0path.c_str(), GDALAccess::GA_ReadOnly);
					if (tempDs == 0)
					{
						cout << "Error : opening " << filepath << endl;
						continue;
					}
					tileXSize = tempDs->GetRasterXSize();
					tileYSize = tempDs->GetRasterYSize();
					dataType = tempDs->GetRasterBand(1)->GetRasterDataType();
					GDALClose(tempDs);
				}
				string displayYmd = wft_ymd_int2str(ymd1);
				int res = processOne(selectedfiles, tempoutfilepath, dsPrefix, dsTail,
					numHortTiles, numVertTiles, 
					ymd0, 
					displayYmd ,
					htLoc, htLen, vtLoc, vtLen,
					filenameYmdLoc, filenameYmdLen,
					useHTile0, useHTile1, useVTile0, useVTile1,
					-9999 , 
					tileXSize , tileYSize , dataType ,
					lonfile , latfile , tileWid , tileHei , 
					valid0 , valid1 , gdalwarp , createvrt , orxStr, oryStr ,
					cutline , image2xyz ,
					valid0Str , valid1Str , 
					scaleStr , offsetStr , 
					gnuplot , plotTemplate ,
					insert , 
					host , 
					user , 
					pwd , db , 
					tb , 
					pid , 
					modisleft , 
					modistop 
					);
				cout << " result: " << res << endl;
			}
		}
	}
	
	std::cout << "done." << std::endl;

    return 0;
}




/* modis

//HDF4_EOS:EOS_GRID:\"mod.h27v05.hdf\":MOD_Grid_monthly_1km_VI:\"1km monthly NDVI;
"HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/mod13a3_mon/MOD13A3.A2017182.h23v03.006.2017234112103.hdf\":MOD_Grid_monthly_1km_VI:1 km monthly NDVI",

"HDF5:\"E:/testdata/fy3bvirrndvi/20170831_mon/FY3B_VIRRX_20B0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF\"://1KM_Monthly_NDVI"
};*/