#include "WNetcdfSubDataset.h"
#include "gdal_alg.h"


WNetcdfSubDataset::WNetcdfSubDataset():m_dataSubDatasetPtr(nullptr),
m_lonSubDatasetPtr(nullptr),
m_latSubDatasetPtr(nullptr)
{
}


WNetcdfSubDataset::~WNetcdfSubDataset()
{
	this->close();
}


bool WNetcdfSubDataset::load(
	std::string filepath, std::string subDsName
)
{
	if (this->m_dataSubDatasetPtr != nullptr)
	{
		this->close();
	}
	this->m_filePath = filepath;
	this->m_subDatasetName = subDsName;

	

	{
		WNetcdfDataFile nc;
		nc.open(this->m_filePath);
		this->m_dataSubDatasetPtr = nc.extractDataset(this->m_subDatasetName);
		nc.close();
	}
	
	return 0;
}


void WNetcdfSubDataset::close()
{
	if (this->m_dataSubDatasetPtr != nullptr)
	{
		GDALClose(this->m_dataSubDatasetPtr);
		this->m_dataSubDatasetPtr = nullptr;
	}
	if (this->m_lonSubDatasetPtr != nullptr)
	{
		GDALClose(this->m_lonSubDatasetPtr);
		this->m_lonSubDatasetPtr = nullptr;
	}
	if (this->m_latSubDatasetPtr != nullptr)
	{
		GDALClose(this->m_latSubDatasetPtr);
		this->m_latSubDatasetPtr = nullptr;
	}
}


GDALDataset* createVrtDatasetByTif( std::string tifFilepath , int noDataValue )
{
	GDALDataset* tifDs = (GDALDataset*)GDALOpen(tifFilepath.c_str(), GDALAccess::GA_ReadOnly);
	GDALDriver *vrtDriver = (GDALDriver *)GDALGetDriverByName("VRT");
	std::string vrtFilePath = tifFilepath + "-temp.vrt";
	GDALDataset* vrtDs = vrtDriver->Create(vrtFilePath.c_str(), 
		tifDs->GetRasterXSize(), 
		tifDs->GetRasterYSize(), 1, GDT_Float32, NULL);

	GDALRasterBand* pBand = vrtDs->GetRasterBand(1);
	int inputBandIndex = 1;
	char xml[2048];
	int blockXSize, blockYSize;
	tifDs->GetRasterBand(inputBandIndex)->GetBlockSize(&blockXSize, &blockYSize);

	sprintf(xml,
		"<SimpleSource>"
		"<SourceFilename>%s</SourceFilename>"
		"<SourceBand>%d</SourceBand>"
		"<SourceProperties RasterXSize=\"%d\" RasterYSize=\"%d\" DataType=\"Float32\" BlockXSize=\"%d\" BlockYSize=\"%d\" />"
		"<SrcRect xOff=\"0\" yOff=\"0\" xSize=\"%d\" ySize=\"%d\" />"
		"<DstRect xOff=\"0\" yOff=\"0\" xSize=\"%d\" ySize=\"%d\" />"
		"</SimpleSource>"
		,
		tifFilepath.c_str(),
		inputBandIndex,
		tifDs->GetRasterXSize(),
		tifDs->GetRasterYSize(),
		blockXSize,
		blockYSize,
		tifDs->GetRasterXSize(),
		tifDs->GetRasterYSize(),
		tifDs->GetRasterXSize(),
		tifDs->GetRasterYSize()
		);
	pBand->SetMetadataItem("source_0", xml, "new_vrt_sources");
	vrtDs->GetRasterBand(1)->SetNoDataValue(noDataValue);
	GDALClose(tifDs);

	return vrtDs;
}


int WNetcdfSubDataset::warpToWgs84ByDs(GDALDataset* lonDs, GDALDataset* latDs,std::string lonFilePath,std::string latFilePath )
{
	/*
	<VRTDataset rasterXSize="2748" rasterYSize="2748">
	  <VRTRasterBand dataType="Float32" band="1">
		<NoDataValue>-999</NoDataValue>
		<ColorInterp>Gray</ColorInterp>
		<SimpleSource>
		  <SourceFilename relativeToVRT="1">fy4lon-good.tif</SourceFilename>
		  <SourceBand>1</SourceBand>
		  <SourceProperties RasterXSize="2748" RasterYSize="2748" DataType="Float32" BlockXSize="2748" BlockYSize="1" />
		  <SrcRect xOff=\"0\" yOff=\"0\" xSize=\"2748\" ySize=\"2748\" />
		  <DstRect xOff="0" yOff="0" xSize="2748" ySize="2748" />
		</SimpleSource>
	  </VRTRasterBand>
	</VRTDataset>
	*/
	//create lon vrt file

	/*
	
	
	GDALDriver *vrtDriver = (GDALDriver *)GDALGetDriverByName("VRT");
	GDALDataset* lonVrt = vrtDriver->Create("temp-lon.vrt", lonDs->GetRasterXSize()  , lonDs->GetRasterYSize() , 1, GDT_Float32, NULL);
	
	GDALRasterBand* pBand = lonVrt->GetRasterBand(1);
	int lonDsBand = 1;
	char xml[2048];
	int blockXSize, blockYSize;
	lonDs->GetRasterBand(lonDsBand)->GetBlockSize(&blockXSize, &blockYSize);

	sprintf(xml,
		"<SimpleSource>"
		"<SourceFilename>%s</SourceFilename>"
		"<SourceBand>%d</SourceBand>"
		"<SourceProperties RasterXSize=\"%d\" RasterYSize=\"%d\" DataType=\"Float32\" BlockXSize=\"%d\" BlockYSize=\"%d\" />"
		"<SrcRect xOff=\"0\" yOff=\"0\" xSize=\"%d\" ySize=\"%d\" />"
		"<DstRect xOff=\"0\" yOff=\"0\" xSize=\"%d\" ySize=\"%d\" />"
		"</SimpleSource>"
		,
		lonFilePath.c_str() ,
		lonDsBand,
		lonDs->GetRasterXSize(),
		lonDs->GetRasterYSize(),
		blockXSize , 
		blockYSize , 
		lonDs->GetRasterXSize() , 
		lonDs->GetRasterYSize() , 
		lonDs->GetRasterXSize(),
		lonDs->GetRasterYSize()
		);
	pBand->SetMetadataItem("source_0", xml , "new_vrt_sources");
	lonVrt->GetRasterBand(1)->SetNoDataValue(-999.0);
	GDALClose(lonVrt);
	*/
	


	//create lon , lat vrt file
	GDALDataset* lonVrtDs = createVrtDatasetByTif(lonFilePath, -999);
	GDALDataset* latVrtDs = createVrtDatasetByTif(latFilePath, -999);

	//create target data vrt file

	/*
	SRS: GEOGCS...
	X_DATASET: L1BGCPS:n12gac10bit.l1b
	X_BAND: 1
	Y_DATASET: L1BGCPS:n12gac10bit.l1b
	Y_BAND: 2
	PIXEL_OFFSET: 25
	LINE_OFFSET: 0
	PIXEL_STEP: 40
	LINE_STEP: 1
	*/
	GDALClose(lonVrtDs);
	GDALClose(latVrtDs);


	//char* line0 = "SRS:GEOGCS[\"WGS 84\", DATUM[\"WGS_1984\", SPHEROID[\"WGS 84\", 6378137, 298.257223563, AUTHORITY[\"EPSG\", \"7030\"]], TOWGS84[0, 0, 0, 0, 0, 0, 0], AUTHORITY[\"EPSG\", \"6326\"]], PRIMEM[\"Greenwich\", 0, AUTHORITY[\"EPSG\", \"8901\"]], UNIT[\"degree\", 0.0174532925199433, AUTHORITY[\"EPSG\", \"9108\"]], AUTHORITY[\"EPSG\", \"4326\"]]";
	char* line0 = "SRS:SRS_WKT_WGS84";
	char* line1 = "X_DATASET:E:/testdata/fy4lon-good.tif-temp.vrt";
	char* line2 = "X_BAND:1";
	char* line3 = "Y_DATASET:E:/testdata/fy4lat-good.tif-temp.vrt";
	char* line4 = "Y_BAND:1";
	char* line5 = "PIXEL_OFFSET:0";
	char* line6 = "LINE_OFFSET:0";
	char* line7 = "PIXEL_STEP:1";
	char* line8 = "LINE_STEP:1";
	char* params[] = { line0 , line1 , line2 , line3 , line4 , line5 , line6 , line7 , line8 };

	int n = this->m_dataSubDatasetPtr->GetRasterXSize();
	void* transOption = GDALCreateGeoLocTransformer(this->m_dataSubDatasetPtr, params, 0);

	把sst转成tif试一下

	//warp target data vrt file



	return 0;
}



void WNetcdfSubDataset::warpToWgs84ByTif(
	std::string lonFilePath ,
	std::string latFilePath )
{
	if (this->m_dataSubDatasetPtr == nullptr) return;

	GDALDataset* lonDs = (GDALDataset*) GDALOpen(lonFilePath.c_str(), GDALAccess::GA_ReadOnly);
	GDALDataset* latDs = (GDALDataset*)GDALOpen(latFilePath.c_str(), GDALAccess::GA_ReadOnly);
	
	this->warpToWgs84ByDs(lonDs, latDs , lonFilePath , latFilePath );

	GDALClose(lonDs);
	GDALClose(latDs);
}


void WNetcdfSubDataset::warpToWgs84ByNcHdf(
	std::string lonFilePath, std::string lonDsName,
	std::string latFilePath, std::string latDsName
	)
{
	if (this->m_dataSubDatasetPtr == nullptr) return;

	if (this->m_lonSubDatasetPtr)
	{
		GDALClose(this->m_lonSubDatasetPtr);
		this->m_lonSubDatasetPtr = nullptr;
	}
	if (this->m_latSubDatasetPtr)
	{
		GDALClose(this->m_latSubDatasetPtr);
		this->m_latSubDatasetPtr = nullptr;
	}

	this->m_lonFilePath = lonFilePath;
	this->m_lonSubDatasetName = lonDsName;
	this->m_latFilePath = latFilePath;
	this->m_latSubDatasetName = latDsName;

	{
		WNetcdfDataFile nc;
		nc.open(this->m_lonFilePath);
		this->m_lonSubDatasetPtr = nc.extractDataset(this->m_lonSubDatasetName);
		nc.close();
	}
	{
		WNetcdfDataFile nc;
		nc.open(this->m_latFilePath);
		this->m_latSubDatasetPtr = nc.extractDataset(this->m_latSubDatasetName);
		nc.close();
	}
	//create virtual data
	//create lon vrt file
	GDALDriver *vrtDriver = (GDALDriver *)GDALGetDriverByName("VRT");
	//GDALDataset* lonVrtPtr = vrtDriver->Create("", this->m_lonSubDatasetPtr , 512, 1, GDT_Float32, NULL);



	//create lat vrt file

	//create target data vrt file

	//warp target data vrt file

}