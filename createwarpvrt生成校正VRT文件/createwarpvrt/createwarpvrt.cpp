// createwarpvrt.cpp : 定义控制台应用程序的入口点。
//创建需要几何校正数据的vrt文件
//wangfeng1@piesat.cn
//按照范围将经纬度数值写入tif文件时，如果经度超过180度，那么使用lon-360的值代替，比如190度时写入经度为-170度
//程序正常运行后会在temproot路径下生成三个文件 temproot.lon.vrt temproot.lat.vrt temproot.warp.vrt
//如果输入-type参数为llrange则会生成temproot.lon.tif temproot.lat.tif两个文件 
//
//2017-9-28 v1.3a 增加设置输出路径的功能,去掉datatype参数采用自动从数据集获取。


#include "gdal_priv.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cassert>


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





int main(int argc , char** argv )
{
	std::cout << "version 1.3.1a by wangfengdev@163.com 2017-9-21." << std::endl;
	std::cout << "version 1.4a by wangfengdev@163.com 2017-10-10. add gdalwarp call." << std::endl;
	if (argc == 1 )
	{
		std::cout << "Sample call:" << std::endl;
		std::cout << "createwarpvrt -type llfile -in hdf5:xxxxx.hdf://product "
					 "   -lonfile lon.tif -latfile lat.tif "
					 "   [-temproot /outputdir/xxxx.hdf]   "
					 "   [-llnodata -999] [-pdtnodata 65535] " 
					 "   [-pdtlut -101:65535,-100:-100,100:100,101:65535,65535:65535] "
			         "   [-outvrt output.warp.vrt] "
					 "   [-outtif output.wgs84.tif] "  //输出校正后的tif文件
					 "   [-gdalwarp gdalwarp] "            //生成vrt后自动调用gdalwarp进行校正
					 "   [-orx 0.05 -ory 0.05] " //输出分辨率

			<< std::endl;
		std::cout << "createwarpvrt -type llrange -in hdf5:xxxxx.hdf://product "
			"   -lon0 0.5 -lat0 89.5 -lonstride 1 -latstride -1 "
			"   [-temproot /outputdir/xxxx.hdf]   "
			"   [-llnodata -999] [-pdtnodata 65535] "
			"   [-pdtlut -101:65535,-100:-100,100:100,101:65535,65535:65535] "
			"   [-outvrt output.warp.vrt] "
			"   [-outtif output.wgs84.tif] "  //输出校正后的tif文件
			"   [-gdalwarp gdalwarp] "            //生成vrt后自动调用gdalwarp进行校正
			"   [-orx 0.05 -ory 0.05] " //输出分辨率
			<< std::endl;

		//std::cout << "createwarpvrt -llfile hdf5:xxxxx.hdf://product tempfileroot lon.tif lat.tif llNoData[-999] pdtNoData[65535] pdtLut[-101:65535,-100:-100,100:100,101:65535,65535:65535] pdtOutType[Float32]" << std::endl;
		//std::cout << "createwarpvrt -llrange hdf5:xxxxx.hdf://product tempfileroot originLon originLat strideLon strideLat llNoData[-999] pdtNoData[65535] ptdLut[-101:65535,-100:-100,100:100,101:65535,65535:65535] pdtOutType[Float32]" << std::endl;
		exit(101);
	}

	std::string appModeStr = "" ;
	wft_has_param(argc, argv, "-type", appModeStr , true );
	std::string productfile = "";
	wft_has_param(argc, argv, "-in", productfile, true);
	std::string tempfileroot = "";
	wft_has_param(argc, argv, "-temproot", tempfileroot, false);
	std::string llNodataStr = "-999";
	wft_has_param(argc, argv, "-llnodata", llNodataStr, false);
	if (llNodataStr == "") llNodataStr = "-999";

	std::string productNodataStr = "";
	wft_has_param(argc, argv, "-pdtnodata", productNodataStr, false);
	std::string productLutStr = "";
	wft_has_param(argc, argv, "-pdtlut", productLutStr, false);
	std::string pdtOutType = "Byte";

	std::string outputWarpVrtFile = "";
	wft_has_param(argc, argv, "-outvrt", outputWarpVrtFile, false);
	std::string productWarpFilepath = tempfileroot + ".warp.vrt";
	if (outputWarpVrtFile != "")
	{
		tempfileroot = outputWarpVrtFile;
		productWarpFilepath = outputWarpVrtFile;
	}
	else if (tempfileroot == "")
	{
		tempfileroot = "temp";
		productWarpFilepath = tempfileroot + ".warp.vrt";
	}

	//version 1.4a
	//"   [-outtif output.wgs84.tif] "  //输出校正后的tif文件
	//	"   [-gdalwarp gdalwarp] "            //生成vrt后自动调用gdalwarp进行校正
	//	"   [-orx 0.05 -ory 0.05] " //输出分辨率
	bool doUseGdalWarp = false;
	std::string tempStr;
	std::string gdalWarp;
	std::string outputTif = tempfileroot + ".wgs84.tif";
	if (wft_has_param(argc, argv, "-gdalwarp", gdalWarp, false))
	{
		doUseGdalWarp = true ;
	}
	std::string outputResoX, outputResoY;
	if (doUseGdalWarp)
	{
		if (wft_has_param(argc, argv, "-outtif", tempStr, false))
		{
			outputTif = tempStr;
		}
		wft_has_param(argc, argv, "-orx", outputResoX, true);
		wft_has_param(argc, argv, "-ory", outputResoY, true);
	}


	std::string lonfile = "";
	std::string latfile = "";
	
	double lon0 = 0;
	double lat0 = 0;
	double stridelon = 0;
	double stridelat = 0;
	int lonBlockX(0), lonBlockY(0);
	int latBlockX(0), latBlockY(0);
	int appMode = 0;
	if (appModeStr == std::string("llfile")) {
		appMode = 0;
		wft_has_param(argc, argv, "-lonfile", lonfile, true);
		wft_has_param(argc, argv, "-latfile", latfile, true);
	}
	else if (appModeStr == std::string("llrange")) {
		appMode = 1;
		std::string s1, s2, s3, s4;
		wft_has_param(argc, argv, "-lon0", s1, true);
		wft_has_param(argc, argv, "-lat0", s2, true);
		wft_has_param(argc, argv, "-lonstride", s3, true);
		wft_has_param(argc, argv, "-latstride", s4, true);
		lon0 = atof(s1.c_str());
		lat0 = atof(s2.c_str());
		stridelon = atof(s3.c_str());
		stridelat = atof(s4.c_str());
	}
	else {
		std::cout << "Error : invalid parameters. out." << std::endl;
		exit(102);
	}
	GDALAllRegister();
	int rasterXSize, rasterYSize;
	GDALDataset* productDataset = (GDALDataset*)GDALOpen(productfile.c_str(), GDALAccess::GA_ReadOnly);
	rasterXSize = productDataset->GetRasterXSize();
	rasterYSize = productDataset->GetRasterYSize();
	{//output datatype
		GDALDataType dtype = productDataset->GetRasterBand(1)->GetRasterDataType();
		if (dtype == GDT_Byte) pdtOutType = "Byte";
		else if (dtype == GDT_CFloat32) pdtOutType = "CFloat32";
		else if (dtype == GDT_CFloat64) pdtOutType = "CFloat64";
		else if (dtype == GDT_CInt16) pdtOutType = "CInt16";
		else if (dtype == GDT_CInt32) pdtOutType = "CInt32";
		else if (dtype == GDT_Float32) pdtOutType = "Float32";
		else if (dtype == GDT_Float64) pdtOutType = "Float64";
		else if (dtype == GDT_Int16) pdtOutType = "Int16";
		else if (dtype == GDT_Int32) pdtOutType = "Int32";
		else if (dtype == GDT_UInt16) pdtOutType = "UInt16";
		else if (dtype == GDT_UInt32) pdtOutType = "UInt32";
		else {
			std::cout << "Error : product dataset has unsupported data type:"<<dtype<< ". out." << std::endl;
			exit(110);
		}
	}
	GDALClose(productDataset);

	if (appMode == 0)
	{
		GDALDataset* lonDataset = (GDALDataset*)GDALOpen(lonfile.c_str(), GDALAccess::GA_ReadOnly);
		GDALDataset* latDataset = (GDALDataset*)GDALOpen(latfile.c_str(), GDALAccess::GA_ReadOnly);
		int lonXSize, lonYSize, latXSize, latYSize;
		lonXSize = lonDataset->GetRasterXSize();
		lonYSize = lonDataset->GetRasterYSize();
		latXSize = latDataset->GetRasterXSize();
		latYSize = latDataset->GetRasterYSize();

		if (rasterXSize != lonXSize || rasterXSize != latXSize || rasterYSize != lonYSize || rasterYSize != latYSize ) {
			std::cout << "Error : 文件尺寸不同，不能处理!" << std::endl;
			GDALClose(lonDataset);
			GDALClose(latDataset);
			exit(103);
		}

		lonDataset->GetRasterBand(1)->GetBlockSize(&lonBlockX, &lonBlockY);
		latDataset->GetRasterBand(1)->GetBlockSize(&latBlockX, &latBlockY);

		GDALClose(lonDataset);
		GDALClose(latDataset);
	}
	else {
		lonfile = tempfileroot + ".lon.tif";
		latfile = tempfileroot + ".lat.tif";
		GDALDriver* driverPtr = GetGDALDriverManager()->GetDriverByName("GTiff");
		GDALDataset* lonDataset = driverPtr->Create(lonfile.c_str(), rasterXSize, rasterYSize, 1, GDT_Float32,0);
		GDALDataset* latDataset = driverPtr->Create(latfile.c_str(), rasterXSize, rasterYSize, 1, GDT_Float32,0);
		
		GDALRasterBand* lonBand = lonDataset->GetRasterBand(1);
		GDALRasterBand* latBand = latDataset->GetRasterBand(1);
		
		std::cout << "start writing temp lon,lat tif files..." << std::endl;
		float* lonValuesRow = new float[rasterXSize];
		float* latValuesRow = new float[rasterXSize];
		for (int iy = 0; iy < rasterYSize; ++iy)
		{
			for (int ix = 0; ix < rasterXSize; ++ix)
			{
				lonValuesRow[ix] = lon0 + stridelon * ix;
				if (lonValuesRow[ix] > 180) lonValuesRow[ix] -= 360;//wf 2017-9-21
				latValuesRow[ix] = lat0 + stridelat * iy;
			}
			lonBand->RasterIO(GDALRWFlag::GF_Write, 0, iy, rasterXSize, 1, lonValuesRow, rasterXSize, 1, GDALDataType::GDT_Float32, 0, 0);
			latBand->RasterIO(GDALRWFlag::GF_Write, 0, iy, rasterXSize, 1, latValuesRow, rasterXSize, 1, GDALDataType::GDT_Float32, 0, 0);
		}
		delete[] lonValuesRow;
		delete[] latValuesRow;

		lonDataset->GetRasterBand(1)->GetBlockSize(&lonBlockX, &lonBlockY);
		latDataset->GetRasterBand(1)->GetBlockSize(&latBlockX, &latBlockY);

		GDALClose(lonDataset);
		GDALClose(latDataset);
		std::cout << "temp lon,lat tif files are done." << std::endl;
	}

	
	char lonvrtfileText[5000];
	sprintf(lonvrtfileText,
		"<VRTDataset rasterXSize = \"%d\" rasterYSize=\"%d\">\n"
		"<SRS>GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AUTHORITY[\"EPSG\",\"4326\"]]</SRS>\n"
		"<VRTRasterBand dataType = \"Float32\" band=\"1\">\n"
		"<NoDataValue>%s</NoDataValue>\n"
		"<ColorInterp>Gray</ColorInterp>\n"
		"<SimpleSource>\n"
		"<SourceFilename relativeToVRT = \"0\">%s</SourceFilename>\n"
		"<SourceBand>1</SourceBand>\n"
		"<SourceProperties RasterXSize = \"%d\" RasterYSize=\"%d\" DataType=\"Float32\" BlockXSize=\"%d\" BlockYSize=\"%d\" />\n"
		"<SrcRect xOff = \"0\" yOff=\"0\" xSize=\"%d\" ySize=\"%d\" />\n"
		"<DstRect xOff = \"0\" yOff=\"0\" xSize=\"%d\" ySize=\"%d\" />\n"
		"</SimpleSource>\n"
		"</VRTRasterBand>\n"
		"</VRTDataset>" ,
		rasterXSize , rasterYSize , 
		llNodataStr.c_str() , 
		lonfile.c_str() , 
		rasterXSize , rasterYSize , lonBlockX , lonBlockY ,
		rasterXSize , rasterYSize , 
		rasterXSize , rasterYSize 
		);
	std::string lonVrtFilepath = tempfileroot + ".lon.vrt";
	std::ofstream lonOfs(lonVrtFilepath.c_str());
	lonOfs << lonvrtfileText << std::endl;
	lonOfs.close();

	
	char latvrtfileText[5000];
	sprintf(latvrtfileText,
		"<VRTDataset rasterXSize = \"%d\" rasterYSize=\"%d\">\n"
		"<SRS>GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AUTHORITY[\"EPSG\",\"4326\"]]</SRS>\n"
		"<VRTRasterBand dataType = \"Float32\" band=\"1\">\n"
		"<NoDataValue>%s</NoDataValue>\n"
		"<ColorInterp>Gray</ColorInterp>\n"
		"<SimpleSource>\n"
		"<SourceFilename relativeToVRT = \"0\">%s</SourceFilename>\n"
		"<SourceBand>1</SourceBand>\n"
		"<SourceProperties RasterXSize = \"%d\" RasterYSize=\"%d\" DataType=\"Float32\" BlockXSize=\"%d\" BlockYSize=\"%d\" />\n"
		"<SrcRect xOff = \"0\" yOff=\"0\" xSize=\"%d\" ySize=\"%d\" />\n"
		"<DstRect xOff = \"0\" yOff=\"0\" xSize=\"%d\" ySize=\"%d\" />\n"
		"</SimpleSource>\n"
		"</VRTRasterBand>\n"
		"</VRTDataset>",
		rasterXSize, rasterYSize,
		llNodataStr.c_str(),
		latfile.c_str(),
		rasterXSize, rasterYSize, latBlockX, latBlockY,
		rasterXSize, rasterYSize,
		rasterXSize, rasterYSize
		);
	std::string latVrtFilepath = tempfileroot + ".lat.vrt";
	std::ofstream latOfs(latVrtFilepath.c_str());
	latOfs << latvrtfileText << std::endl;
	latOfs.close();

	char productVrtFileText[4096];
	sprintf(productVrtFileText,
		"<VRTDataset rasterXSize = \"%d\" rasterYSize=\"%d\">\n"
		"<SRS>GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AUTHORITY[\"EPSG\",\"4326\"]]</SRS>\n"
		"<Metadata domain = \"GEOLOCATION\">\n"
		"<MDI key = \"LINE_OFFSET\">0</MDI>\n"
		"<MDI key = \"LINE_STEP\">1</MDI>\n"
		"<MDI key = \"PIXEL_OFFSET\">0</MDI>\n"
		"<MDI key = \"PIXEL_STEP\">1</MDI>\n"
		"<MDI key = \"X_BAND\">1</MDI>\n"
		"<MDI key = \"X_DATASET\">%s</MDI>\n"
		"<MDI key = \"Y_BAND\">1</MDI>\n"
		"<MDI key = \"Y_DATASET\">%s</MDI>\n"
		"</Metadata>\n"
		"<VRTRasterBand dataType = \"%s\" band=\"1\">\n"
		"<NoDataValue>%s</NoDataValue>\n"
		"<ComplexSource>\n"
		"<SourceFilename relativeToVRT = \"0\">%s</SourceFilename>\n"
		"<SourceBand>1</SourceBand>\n"
		"<LUT>%s</LUT>\n"
		"</ComplexSource>\n"
		"</VRTRasterBand>\n"
		"</VRTDataset>",
		rasterXSize, rasterYSize,
		lonVrtFilepath.c_str() , 
		latVrtFilepath.c_str() , 
		pdtOutType.c_str(),
		productNodataStr.c_str() , 
		productfile.c_str() , 
		productLutStr.c_str()
		);
	
	std::ofstream pdtOfs(productWarpFilepath.c_str());
	pdtOfs << productVrtFileText << std::endl;
	pdtOfs.close();

	std::cout << "createwarpvrt done." << std::endl;

	if (doUseGdalWarp)
	{
		std::cout << "call gdalwarp ..." << std::endl;
		std::string cmd = gdalWarp + " -geoloc -t_srs \"+proj=longlat +ellps=WGS84\"  " +
			" -tr " + outputResoX + " " + outputResoY +
			" -r bilinear -overwrite " + productWarpFilepath + " " + outputTif;
		std::cout << cmd << std::endl;
		int res = std::system(cmd.c_str());
		//gdalwarp -geoloc -t_srs "+proj=longlat +ellps=WGS84" -tr 0.05 0.05 -r bilinear -overwrite demo.warp.vrt demo.wgs84.tif
		std::cout << "gdalwarp return code :" << res << std::endl;
	}

	std::cout << "done." << std::endl;
    return 0;
}

