#include "factory.h"

int mysqlConnect(MYSQL **connect)
{
    MYSQL *conn;
    conn = mysql_init(conn);

    const char* server = "localhost";
    const char* user = "root";
    const char* password = "lvda123456";
    const char* database = "netdisk";    //要访问的数据库名称
    if(!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("Error connecting to database:%s\n", mysql_error(conn));
        return -1;
    }else{
        printf("Connected ...\n");
    }
    *connect = conn;
    return 0;
}

