#pragma once

#include <mysql/mysql.h>
#include <string>
using std::string;

class MySql {
public:
    MySql();
    ~MySql();
    bool connect(); 
    bool update(string sql);
    MYSQL_RES* query(string sql);
private:
    MYSQL *_conn;
};