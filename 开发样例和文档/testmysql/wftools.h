#ifndef WFTOOLS_H
#define WFTOOLS_H

#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <algorithm>

//获取当前时间
std::string wft_get_current_time() {
  //#include <ctime>
  time_t time0;
  time(&time0);
  return std::string(asctime(localtime(&time0)));
}

//判断文件是否存在
bool wft_test_file_exists(const std::string& name) {
  std::ifstream f(name.c_str());
  return f.good();
}
//获取当前日期，格式YYYYMMDD
std::string wft_current_dateymd()
{
  time_t theTime = time(NULL);
  struct tm *aTime = localtime(&theTime);
  int day = aTime->tm_mday;
  int month = aTime->tm_mon + 1;  
  int year = aTime->tm_year + 1900; 
  char buff[10];
  sprintf(buff, "%04d%02d%02d", year, month, day);
  return std::string(buff);
}

//从路径中截取文件名
std::string wft_base_name(std::string const & path)
{
  return path.substr(path.find_last_of("/\\") + 1);
}


int wft_linux_get_dir_files(std::string dir , std::vector<std::string>& files)
{
  struct dirent * dirp ;
  DIR* dp = opendir(dir.c_str()) ;
  if( dp == NULL )
  {
    std::cout<<"Error : failed to open "<<dir<<std::endl ;
    exit(131) ;
  } 
  while( (dirp=readdir(dp)) != NULL )
  {
    files.push_back( std::string(dirp->d_name) ) ;
  }
  closedir(dp) ;
  return 0 ;
}

//写入日志
void wft_log(std::string logdir , std::string programid, std::string content)
{
  if (*logdir.rbegin() != '/') {
    logdir += "/";
  }
  std::string ymd = wft_current_dateymd();
  std::string logfile = logdir + programid + "-" + ymd + ".log" ;
  std::ofstream outfs;
  if (wft_test_file_exists(logfile))
  {
    outfs.open(logfile.c_str(), std::ofstream::app);
  }
  else {
    outfs.open(logfile.c_str());
  }
  std::string currTime = wft_get_current_time();
  outfs << currTime << " -> " << content << std::endl;
  std::cout<<content<<std::endl ;
  outfs.close();

}

//从配置文件获取参数
//read extra params file by key. note the key should like the same as text file write do not use space.
std::string wft_getValueFromExtraParamsFile(std::string extrafile, std::string key , bool musthas=false )
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
  if (find == false )
  {
    std::cout << "Error : Not found any params with key : " << key << std::endl;
    if (musthas)
    {
      std::cout << "Error : can not find params of "<<key<<", the program can not run. outing...  " << std::endl;
      exit(111);
    }
  }
  return res;
}



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

//通过标签获取命令行是否有某个tag
bool wft_has_tag(int argc, char** argv, char* key  )
{
  for (int i = 0; i < argc; ++i)
  {
    if (strcmp(argv[i], key) == 0) {
      return true ;
    }
  }
  return false;
}


//进度显示
void wft_term_progress(size_t curr, size_t total)
{
  static size_t term_progress_percent = -1;
  if (curr < total-1)
  {
    size_t newper = curr * 100 / total;
    if (newper != term_progress_percent) {
      term_progress_percent = newper;
      if (term_progress_percent % 10 == 0) {
        std::cout << term_progress_percent;
      }
      else {
        std::cout << ".";
      }
    }
  }
  else{
    if (term_progress_percent != 100) {
      term_progress_percent = 100;
      std::cout << 100 << std::endl;
    }
  }
}












#endif
