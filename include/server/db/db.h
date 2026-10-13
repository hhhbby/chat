#pragma once

#include <mysql/mysql.h>
#include <string>
using std::string;

class MySql {
    MySql();
public:
    static MySql* instance();
    ~MySql();
    bool connect(); 
    bool update(string sql);
    MYSQL_RES* query(string sql);
    int insert_id();

private:
    MYSQL *_conn;
};