// extractncepwind.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include "../../sharedcodes/wftools.h"
using namespace std;

int main()
{
	GDALAllRegister();

	string infile = "HDF4_EOS:EOS_GRID:\"E:/testdata/modisndvi/testproj/demo.hdf\":MODIS_Grid_16DAY_250m_500m_VI:250m 16 days NDVI";
	string outfile = "E:/testdata/modisndvi/testproj/demo.sinu-2.tif";


	GDALDataset* inds = (GDALDataset*)GDALOpen(infile.c_str(), GDALAccess::GA_ReadOnly);
	int xsize = inds->GetRasterXSize();
	int ysize = inds->GetRasterYSize();
	GDALDataType datatype = inds->GetRasterBand(1)->GetRasterDataType();

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outfile.c_str(), xsize, ysize , 1, datatype , 0);


	double trans[6] = { 5559752.5983 , 231.65635826 , 0 , 7783653.6377 , 0 ,  -231.65635826  };
	outds->SetGeoTransform(trans);

	//outds->SetProjection("+proj=sinu +R=6371007.181 +nadgrids=@null +wktext");
	//outds->SetProjection("+proj=sinu +a=6371007.181 +b=6371007.181 +units=m +no_defs");

	ofstream ofs(outfile + ".info.txt");

	const char* inproj = inds->GetProjectionRef();
	ofs << inproj << endl;
	{
		OGRSpatialReference osrs;
		osrs.importFromProj4("+proj=sinu +lon_0=0 +x_0=0 +y_0=0 +a=6371007.181 +b=6371007.181 +units=m +no_defs");
		char* outproj = 0;
		osrs.exportToWkt(&outproj);
		ofs << "wkt:" << endl;
		ofs << outproj << endl;
		outds->SetProjection(outproj);
		CPLFree(outproj);
	}
	{
		char projstr2[] = "PROJCS[\"unnamed\",GEOGCS[\"Unknown datum based upon the custom spheroid\",DATUM[\"Not_specified_based_on_custom_spheroid\",SPHEROID[\"Custom spheroid\",6371007.181,0]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Sinusoidal\"],PARAMETER[\"longitude_of_center\",0],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]]]";

		//outds->SetProjection(projstr2);

		//char* wkt2 = new char[1000] ;
		//strcpy(wkt2, inproj);
		//OGRSpatialReference osrs;
		//osrs.importFromWkt(&wkt2);

		//char* proj2 = 0;
		//osrs.exportToProj4(&proj2);
		//ofs << "proj2:" << endl;
		//ofs << proj2 << endl;


		//delete[] wkt2;

	}



	int* buff = new int[xsize * ysize];

	inds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, 0, xsize, ysize, buff, xsize, ysize, GDALDataType::GDT_Int32, 0, 0, 0);
	outds->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, 0, xsize, ysize, buff, xsize, ysize, GDALDataType::GDT_Int32, 0, 0, 0);

	delete[] buff;


	GDALClose(outds);
	GDALClose(inds);

	getchar();
	getchar();

    return 0;
}

