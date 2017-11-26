#include "/usr/include/mysql/mysql.h"
#include <cstdio>
#include <iostream>
#include <string>
#include "../sharedcodes/wftools.h"  
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std ;

string programid = "fy4sstm15monitorfordb" ;
string logdir = "/root/ncc-fy4-project/extras/logs/" ;

//get count of some where clause.
//sample: wft_mysql_fetch_count( conn , "t_product" , "filepath='/root/some.hdf'" );
int wft_mysql_fetch_count( 
                MYSQL* conn  ,
                string table ,  
                string whereContent )
{
        string sql = "SELECT count(*) FROM " ;
        sql += table + " WHERE " ;
        sql += whereContent ;
        if( mysql_query(conn , sql.c_str() ) )
        {
                wft_log( logdir , 
                      programid , 
                     sql + " query failed. out." ) ;
                mysql_close(conn);
                exit(21) ;
        } 
        MYSQL_RES * res = mysql_store_result(conn) ;
        int numf = mysql_num_fields(res) ;
        if( numf != 1 )
        {
                wft_log( logdir , 
                      programid , 
                     sql + " query result field count != 1. out." ) ;
                mysql_close(conn);
                exit(22) ;
        }

        MYSQL_ROW row = mysql_fetch_row(res) ;
        int count = atof(row[0]) ;
        mysql_free_result(res) ;
        res = NULL ;
        return count ;
}

//从fy4文件命中获取时间 结果为2017-01-01 11:11:11
string wft_get_datetimestr_from_fy4sstfilename( string fy4filename )
{
        size_t pos0 = fy4filename.find_last_of("NOM_") ;
        if( pos0 == string::npos )
        {
                return "" ;
        }else
        {
                string year = fy4filename.substr( pos0+4 , 4 ) ;
                string mon = fy4filename.substr( pos0+8 , 2 ) ;
                string day = fy4filename.substr( pos0+10 , 2 ) ;
                string hh = fy4filename.substr( pos0+12 , 2 ) ;
                string mm = fy4filename.substr( pos0+14 , 2 ) ;
                string ss = fy4filename.substr( pos0+16 , 2 ) ;
                string datetimeStr = year + "-" + mon + "-" + day + " " + hh + ":"+mm+":"+ss ;
                return datetimeStr ;
        }
}

//FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_20170815000000_20170815001459_4000M_V0001.NC
string make_fy4m15_insertsql( string filepath , string thumbfile )
{
        string filename = wft_base_name(filepath) ;
        string dtstr = wft_get_datetimestr_from_fy4sstfilename(filename) ;

        string sql = "insert into t_data_sst(product_id,data_filepath,filename,start_date,end_date,thumb,big_thumb) values ("  ;
        sql += string("1") + ","
                + "'" + filepath + "'"  + ","
                + "'" + filename + "'"  + ","   
                + "'" + dtstr + "'"  + "," 
                + "'" + dtstr + "'"  + "," 
                + "'" + thumbfile + "'"  + "," 
                + "'" + "" + "'"
                + " )" ;
        //insert into xxx (xx,xx) values(xx,xx) ;
        return sql ;
}

bool wft_insert_sql( MYSQL* conn , string sql )
{
        mysql_query(conn , sql.c_str()) ;
        int id = mysql_insert_id(conn) ;
        if( id > 0 ) return true ;
        else return false ;
}





int main(int argc ,char** argv)
{
        std::cout<<"version 0.1.1a . by wangfengdev@163.com 2017-9-29. "<<std::endl ;
        if( argc == 1 )
        {
                cout<<"Error can not load startup parameters file. out."<<endl ;
                exit(20) ;
        }


        std::cout<<mysql_get_client_info()<<std::endl ;
  
       
        //connect mysql.
        string startupfile = argv[1] ;
        string host = wft_getValueFromExtraParamsFile(
                        startupfile , 
                        "#host" , 
                        true 
                        ) ;
        string user = wft_getValueFromExtraParamsFile(
                        startupfile , 
                        "#user" , 
                        true 
                        ) ;;
        string pwd =wft_getValueFromExtraParamsFile(
                        startupfile , 
                        "#pwd" , 
                        true 
                        ) ;
        string dbName =wft_getValueFromExtraParamsFile(
                        startupfile , 
                        "#db" , 
                        true 
                        ) ;
        string inDir =wft_getValueFromExtraParamsFile(
                        startupfile , 
                        "#indir" , 
                        true 
                        ) ;
        int maxError = 3 ;
        string maxErrorStr = wft_getValueFromExtraParamsFile(
                        startupfile , 
                        "#maxerror" , 
                        true 
                        ) ;
        maxError = std::max(2 , (int)atof(maxErrorStr.c_str()) ) ;

        string programid = "fy4sstm15monitorfordb" ;
        string logdir = wft_getValueFromExtraParamsFile(
                        startupfile , 
                        "#logdir" , 
                        true 
                        ) ;

        MYSQL* conn = mysql_init(NULL) ;
        if( mysql_real_connect(conn , 
                                host.c_str() , 
                                user.c_str() , 
                                pwd.c_str() , 
                                dbName.c_str() , 
                                0 , NULL , 0 )
                        == NULL )
        { 
                wft_log( logdir , 
                      programid , 
                     "error connection failed. out." ) ;
                mysql_close(conn);
                exit(11) ;
        }

        //check each file under some dir is in db. for thoes not insert into db.
        std::vector<std::string> files ;
        wft_linux_get_dir_files( inDir , files ) ;
        int errorCount = 0 ;
        for(size_t ifile = 0 ; ifile < files.size() ;++ ifile )
        {
                string filepath = files[ifile] ;
                string filename = wft_base_name(filepath) ;
                string thumbfilepath = filepath + ".png" ;
                if( wft_test_file_exists(filepath) )
                {
                        cout<<filename<<" exist."<<endl ;
                        if( wft_test_file_exists(thumbfilepath) )
                        {
                                cout<<wft_base_name(thumbfilepath)<<" exist."<<endl ;
                                string wherestr = "filename='" ;
                                wherestr += filename + "'" ;
                                if( wft_mysql_fetch_count(
                                                        conn , 
                                                        "tb_data_sst" , 
                                                        wherestr
                                                        )
                                                > 0 )
                                {
                                        cout<<"Already in db. no need insert. "<<endl ;
                                        continue ;
                                }else
                                {  
                                        string insertSql = make_fy4m15_insertsql(filepath,
                                                thumbfilepath);
                                        if( wft_insert_sql(conn , insertSql) )
                                        {
                                                wft_log( logdir , 
                                                 programid , 
                                                filename + " inserted." ) ;
                                                wft_log( logdir , 
                                                 programid , 
                                                wft_base_name(thumbfilepath)
                                                + " inserted." ) ;
                                                break ;
                                        }else{
                                                wft_log( logdir , 
                                                 programid , 
                                                filename + " insert failed." ) ;
                                                wft_log( logdir , 
                                                 programid , 
                                                wft_base_name(thumbfilepath)
                                                + " insert failed." ) ;
                                                errorCount ++ ;
                                                if( errorCount == maxError )
                                                {
                                                        wft_log( logdir , 
                                                         programid , 
                                                        "maxError reached. out." ) ;
                                                        mysql_close(conn);
                                                        exit(12) ;
                                                }
                                
                                        }
                                }
                                
                        }
                }

        }
        


        mysql_close(conn);
        
        wft_log( logdir , 
                      programid , 
                     "done." ) ;
        return 0 ;
}
