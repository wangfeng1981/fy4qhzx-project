
////////////c++ sample codes/////////////////
#include <stdio.h>
#include "/usr/include/mysql/mysql.h"
#include <iostream>
#include <string>


int main(int argc, char **argv)
{
	printf("hello world\n");
    
    printf("%s\n" , mysql_get_client_info()) ;
    
    std::string hostName = "localhost";
    std::string userId = "root" ;
    std::string pwd = "root123456" ;
    std::string dbName = "testdb" ;
    
    MYSQL* conn0 = mysql_init(NULL) ;
    if( mysql_real_connect(conn0 , 
        hostName.c_str() , 
        userId.c_str() , 
        pwd.c_str() , 
        dbName.c_str() , 
        0 , 
        NULL , 
        0
    ) == NULL )
    {
        std::cout<<"connection bad."<<std::endl;
        mysql_close(conn0);
        exit(11) ;
    }
    

    
    for(int i = 0 ; i< 10 ; ++ i )
    {
        char buff[100] , buff1[100];
        sprintf(buff , "tempfile-%d.txt" , i) ;
        sprintf(buff1 , "thumbfile-%d.png" , i ) ;
        std::string sql="insert into t_product (filepath,thumb) values(  " ;
        sql += std::string("'") + buff + "','" + buff1 + "');" ;
        
        if( mysql_query(conn0 , sql.c_str()) )
        {
            std::cout<<"insert failed."<<sql <<std::endl;
            std::cout<<mysql_error(conn0)<<std::endl ;

            exit(13) ;
        }
    }
    
    
    if (mysql_query(conn0, "SELECT * FROM t_product;")) 
    {
        std::cout<<"select failed." <<std::endl;
        exit(14) ;
    }
    MYSQL_RES *result = mysql_store_result(conn0);
    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result))) 
    { 
        for(int i = 0; i < num_fields; i++) 
        { 
            printf("%s ", row[i] ? row[i] : "NULL"); 
        } 
            printf("\n"); 
    }
    mysql_free_result(result);
    
    
    mysql_close(conn0);
    
	return 0;
}

