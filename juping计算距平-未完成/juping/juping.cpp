// juping.cpp : 定义控制台应用程序的入口点。
//计算距平并出图
//wangfengdev@163.com 2017-9-25


#include "gdal_priv.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>



int main(int argc , char** argv )
{
	std::cout << "Version 0.1a " << std::endl;
	std::cout << "processing... " << std::endl;
	if (argc == 1)
	{
		std::cout << "A program to compute anomaly. " << std::endl;
		std::cout << "by wangfengdev@163.com 2017-9-25. " << std::endl;
		std::cout << "Sample call: " << std::endl;
		std::cout << "  juping -in infile.tif -iv0 -10 -iv1 10 -isca 0.1 -iofs 0.0 "
			" -ref reffile.tif -rv0 -10 -rv1 10 -rsca 0.1 -rofs 0.0 "
			" -out output.tif -fill -999 "
			<< std::endl;
		std::cout << "No enough parameters. out. " << std::endl;
		exit(101);

	}




	std::cout << "done." << std::endl;
    return 0;
}

