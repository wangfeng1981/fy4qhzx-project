// createfy4warpvrt.cpp : 定义控制台应用程序的入口点。
//创建用于对fy4全圆盘数据坐标变化的vrt文件
//王峰 2017-9-11

#include <iostream>
#include <fstream>

int main(int argc, char** argv )
{
	if (argc != 5)
	{
		std::cout << "Version 1.0.20170911" << std::endl;
		std::cout << "createfy4warpvrt.exe fy4DatasetPath fy4DatasetLut noDataValue outputFilepath" << std::endl;
		std::cout << "sample: createfy4warpvrt.exe HDF5:fy4xxxx.nc://SST 0:0,65000:65000,65001:65535,65535:65535 65535 fy4xxxx.nc.sst.vrt" << std::endl;
		return 1;
	}
	std::string fy4DatasetPath(argv[1]);
	std::string fy4DatasetLut(argv[2]);
	std::string noDataValue(argv[3]);
	std::string outputFilepath(argv[4]);

	std::ofstream  output(outputFilepath);

	output << "<VRTDataset rasterXSize = \"2748\" rasterYSize = \"2748\">"<<std::endl;
	output<<"<SRS>GEOGCS[\"WGS 84\", DATUM[\"WGS_1984\", SPHEROID[\"WGS 84\", 6378137, 298.257223563, AUTHORITY[\"EPSG\", \"7030\"]], TOWGS84[0, 0, 0, 0, 0, 0, 0], AUTHORITY[\"EPSG\", \"6326\"]], PRIMEM[\"Greenwich\", 0, AUTHORITY[\"EPSG\", \"8901\"]], UNIT[\"degree\", 0.0174532925199433, AUTHORITY[\"EPSG\", \"9108\"]], AUTHORITY[\"EPSG\", \"4326\"]]</SRS>"<<std::endl;
	output << "<Metadata domain=\"GEOLOCATION\">" << std::endl;
	output << "<MDI key=\"LINE_OFFSET\">0</MDI>" << std::endl;
	output << "<MDI key=\"LINE_STEP\">1</MDI>" << std::endl;
	output << "<MDI key=\"PIXEL_OFFSET\">0</MDI>" << std::endl;
	output << "<MDI key=\"PIXEL_STEP\">1</MDI>" << std::endl;
	output << "<MDI key=\"X_BAND\">1</MDI>" << std::endl;
	output << "<MDI key=\"X_DATASET\">fy4lon.vrt</MDI>" << std::endl;
	output << "<MDI key=\"Y_BAND\">1</MDI>" << std::endl;
	output << "<MDI key=\"Y_DATASET\">fy4lat.vrt</MDI>" << std::endl;
	output << "</Metadata>" << std::endl;
	output << "<VRTRasterBand dataType=\"Float32\" band=\"1\">" << std::endl;
	output << "<NoDataValue>" << noDataValue.c_str() << "</NoDataValue>" << std::endl;
	output << "<ComplexSource>" << std::endl;
	output << "<SourceFilename relativeToVRT=\"1\">"<< fy4DatasetPath.c_str()<<"</SourceFilename>" << std::endl;
	output << "<SourceBand>1</SourceBand>" << std::endl;
	output << "<LUT>" << fy4DatasetLut.c_str() << "</LUT>" << std::endl;
	output << "</ComplexSource>" << std::endl;
	output << "</VRTRasterBand>" << std::endl;
	output << "</VRTDataset>" << std::endl;

	output.close();

	std::cout << "done."<< std::endl;

    return 0;
}

/*
<VRTDataset rasterXSize="2748" rasterYSize="2748">
<Metadata domain="GEOLOCATION">
<MDI key="SRS">GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9108"]],AUTHORITY["EPSG","4326"]]</MDI>
<MDI key="LINE_OFFSET">0</MDI>
<MDI key="LINE_STEP">1</MDI>
<MDI key="PIXEL_OFFSET">0</MDI>
<MDI key="PIXEL_STEP">1</MDI>
<MDI key="X_BAND">1</MDI>
<MDI key="X_DATASET">fy4lon.vrt</MDI>
<MDI key="Y_BAND">1</MDI>
<MDI key="Y_DATASET">fy4lat.vrt</MDI>
</Metadata>
<VRTRasterBand dataType="Float32" band="1">
<NoDataValue>65535</NoDataValue>
<ComplexSource>
<SourceFilename relativeToVRT="1">fy4sst.tif</SourceFilename>
<SourceBand>1</SourceBand>
<LUT>0:0,65000:65000,65001:65535,65535:65535</LUT>
</ComplexSource>
</VRTRasterBand>
</VRTDataset>
*/

