// fy4_l2_sst_qc_procedure.cpp : 定义控制台应用程序的入口点。
//风4 L2级产品实时海表温度（15min） 质量检验全过程调用程序。
//wangfeng1@piesat.cn 2017-9-19.

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <ctime>
#include <cerrno>


void printCurrentTime() {
	//#include <ctime>
	time_t time0;
	time(&time0);
	std::cout << asctime(localtime(&time0)) << std::endl;
}

//read extra params file by key. note the key should like the same as text file write do not use space.
std::string getValueFromExtraParamsFile(std::string extrafile, std::string key, bool musthas = false)
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
	if (find == false)
	{
		std::cout << "Error : Not found any params with key : " << key << std::endl;
		if (musthas)
		{
			std::cout << "Error : 缺少必须参数:" << key << "，程序无法运行。退出中...  " << std::endl;
			exit(111);
		}
	}
	return res;
}

//从路径中截取文件名
std::string wft_base_name(std::string const & path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}
//判断文件是否存在
bool wft_test_file_exists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}



int main(int argc , char** argv )
{
	printCurrentTime();
	std::cout << "processing ... " << std::endl;
	if (argc != 4)
	{
		std::cout << "fy4 L2 15min SST product Quality Control procedure." << std::endl;
		std::cout << "Version 0.1.1a . by wangfengdev@163.com 2017-9-19." << std::endl;
		std::cout << "Sample call:" << std::endl;
		std::cout << "\t fy4_l2_sst_qc_procedure FY4Axxxxx.NC startup_params.dat server_provide_xmlpath.xml" << std::endl;
		std::cout << "Error: no enough parameters. out." << std::endl;
		return 101;
	}




	std::string inputFy4ProductFilepath = argv[1];
	std::string startupFilepath = argv[2];
	std::string serverXmlPath = argv[3];
	std::string refType = getValueFromExtraParamsFile( startupFilepath , "#refdatatype" , true );
	std::string outputDir = getValueFromExtraParamsFile(startupFilepath, "#outputdir", true);
	std::string sysExtras = getValueFromExtraParamsFile(startupFilepath, "#extrasfilepath", true);
	std::string refDataFileDir = getValueFromExtraParamsFile(startupFilepath, "#refdatafiledir", true);

	std::string fy4lonTifFilepath =getValueFromExtraParamsFile(sysExtras, "#fy4lon") ;// "E:/coding/fy4qhzx-project/extras/fy4lon-good.tif";
	std::string fy4latTifFilepath = getValueFromExtraParamsFile(sysExtras, "#fy4lat");// "E:/coding/fy4qhzx-project/extras/fy4lat-good.tif";
	std::string coastlineFilepath = getValueFromExtraParamsFile(sysExtras, "#coastline");
	//HDF5:"E:/testdata/fy4sst15min/FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_20170815000000_20170815001459_4000M_V0001.NC"://SST
	std::string inputDspath = std::string("HDF5:\"")+ inputFy4ProductFilepath + "\"://SST";
	std::string inputFileNameOnly = wft_base_name( inputFy4ProductFilepath ) ;
	std::string outputRootPath = outputDir + "/" + inputFileNameOnly;

	if (refType == "oisst")
	{
		std::string inputRefFilepath = "";
		{//生成oisst文件路径
			size_t positionNOM = inputFileNameOnly.find("NOM");
			if (positionNOM == std::string::npos)
			{
				std::cout << "Error:输入FY4A海表温度文件名格式不正确，无法处理。程序即将退出..." << std::endl;
				exit(111);
			}
			std::string dateYmd = inputFileNameOnly.substr(positionNOM + 4, 8);
			std::string year = dateYmd.substr(0, 4);
			inputRefFilepath = refDataFileDir + "/" + year + "/avhrr-only-v2." + dateYmd + ".nc";
			std::cout << "Searching for " << inputRefFilepath << std::endl;
			bool foundRefFile = wft_test_file_exists(inputRefFilepath);
			if (foundRefFile)
			{
				std::cout << "Find it!" << inputRefFilepath << std::endl;
			}
			else {
				std::cout << "Error:没有找到参考文件，无法处理。程序即将退出..." << std::endl;
				exit(112);
			}
		}
		std::string refFilenameOnly = wft_base_name(inputRefFilepath);

		std::string refDspath = std::string("NETCDF:\"") + inputRefFilepath + "\":sst" ;
		std::string startupfile = outputRootPath + "." + refType + ".startup.dat";

		std::ofstream startupStream(startupfile.c_str());

		startupStream << "#inputfilename" << std::endl;
		startupStream << inputFileNameOnly.c_str() << std::endl;
		startupStream << "#inputdatasetpath" << std::endl;
		startupStream << inputDspath.c_str() << std::endl;
		startupStream << "#inputvalid0" << std::endl;
		startupStream << "-5" << std::endl;
		startupStream << "#inputvalid1" << std::endl;
		startupStream << "45" << std::endl;
		startupStream << "#inputdatascale" << std::endl;
		startupStream << "1" << std::endl;
		startupStream << "#inputdataoffset" << std::endl;
		startupStream << "0" << std::endl;
		startupStream << "#inputlontif" << std::endl;
		startupStream << fy4lonTifFilepath.c_str() << std::endl;
		startupStream << "#inputlattif" << std::endl;
		startupStream << fy4latTifFilepath.c_str() << std::endl;
		startupStream << "#reffilename" << std::endl;
		startupStream << refFilenameOnly.c_str() << std::endl;
		startupStream << "#refdatasetpath" << std::endl;
		startupStream << refDspath.c_str() << std::endl;
		startupStream << "#refvalid0" << std::endl;
		startupStream << "-300" << std::endl;
		startupStream << "#refvalid1" << std::endl;
		startupStream << "4500" << std::endl;
		startupStream << "#refdatascale" << std::endl;
		startupStream << "0.01" << std::endl;
		startupStream << "#refdataoffset" << std::endl;
		startupStream << "0" << std::endl;
		startupStream << "#refleftlon" << std::endl;
		startupStream << "0" << std::endl;
		startupStream << "#refrightlon" << std::endl;
		startupStream << "360" << std::endl;
		startupStream << "#reftoplat" << std::endl;
		startupStream << "90" << std::endl;
		startupStream << "#refbottomlat" << std::endl;
		startupStream << "-90" << std::endl;
		startupStream << "#outputrootpath" << std::endl;
		startupStream << outputRootPath.c_str() << std::endl;
		startupStream << "#filledvalue" << std::endl;
		startupStream << "-50" << std::endl;
		startupStream << "#nomatchedfillvalue" << std::endl;
		startupStream << "-45" << std::endl;
		startupStream << "#productcode" << std::endl;
		startupStream << "fy4sst-oisst" << std::endl;
		startupStream << "#coastline" << std::endl;
		startupStream << coastlineFilepath.c_str() << std::endl;
		startupStream << "#bechekvariety" << std::endl;
		startupStream << "XA" << std::endl;
		startupStream << "#checkVariety" << std::endl;
		startupStream << "XB" << std::endl;
		startupStream << "#dataFormat" << std::endl;
		startupStream << "XC" << std::endl;
		startupStream << "#GERO" << std::endl;
		startupStream << "XD" << std::endl;
		startupStream << "#productDate" << std::endl;
		startupStream << "201709200000" << std::endl;
		startupStream << "#productVariety" << std::endl;
		startupStream << "XF" << std::endl;
		startupStream << "#thetargetresultxmlfilepath" << std::endl;
		startupStream << serverXmlPath.c_str() << std::endl;
		startupStream << "#extrafilepath" << std::endl;
		startupStream << sysExtras.c_str() << std::endl;
		startupStream << "#plotmaxcolors" << std::endl;
		startupStream << "10" << std::endl;
		startupStream << "#plotpalette" << std::endl;
		startupStream << "(0 '0x5e4fa2', 10 '0x5e4fa2', 10 '0x3288bd', 20 '0x3288bd', 20 '0x66c2a5', 30 '0x66c2a5', 30 '0xabdda4', 40 '0xabdda4', 40 '0xe6f598', 50 '0xe6f598', 50 '0xffffbf', 60 '0xffffbf', 60 '0xfee08b', 70 '0xfee08b', 70 '0xfdae61', 80 '0xfdae61', 80 '0xf46d43', 90 '0xf46d43', 90 '0x9e0142', 100 '0x9e0142')" << std::endl;
		startupStream.close();


		std::string cmdLine = "qcbylonlatlut ";
		cmdLine = cmdLine + " " + startupfile;
		std::cout << "command line1:" << cmdLine << std::endl;
		int res = std::system(cmdLine.c_str());
		std::cout << "qcbylonlatlut exit code :" << res << std::endl;

		//输出结果文件如下
		//root.biashist.txt
		//root.plot
		//root.scatter.txt
		//root.qcresult.xml
		//root.biasimg.tif
		//root.lonlatbias.txt
		//root.matchedpoints.txt

		std::string plotfilepath = outputRootPath + ".plot";
		std::string cmdLine2 = "gnuplot ";
		cmdLine2 = cmdLine2 + plotfilepath;
		res = std::system(cmdLine2.c_str());

	}
	else {
		std::cout << "Error: 不支持的参考数据类型 " << refType << ". 程序退出中..." << std::endl;
		exit(102);
	}

	std::cout << "Procedure is done." << std::endl;
	printCurrentTime();

    return 0;
}

/*
老版命令行传参数，不再使用。
if (argc != 6)
{
std::cout << "fy4 L2 15min SST product Quality Control procedure." << std::endl;
std::cout << "Version 0.1.1a . by wangfengdev@163.com 2017-9-19." << std::endl;
std::cout << "Sample call:" << std::endl;
std::cout << "\tfy4_l2_sst_qc_procedure FY4Axxxxx.NC method[oisst] oisstxxxxx.nc outputfilepathWithOutExtensionName[/testing/fy4out] extraparamsfile" << std::endl;
std::cout << "Error: no enough parameters. out." << std::endl;
return 101;
}
*/
