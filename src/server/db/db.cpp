#include "db.h"
#include <muduo/base/Logging.h>

static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string database = "chat";

MySql::MySql()
{
    mysql_init(_conn);
}

MySql::~MySql()
{
    mysql_close(_conn);
}

bool MySql::connect()
{
    MYSQL* p = mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        LOG_INFO << "连接成功";
        mysql_query(_conn, "set names gbk");
    }
    return p;
}

bool MySql::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << "更新失败";
        return false;
    }
    return true;
}

MYSQL_RES* MySql::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << "查询失败";
        return nullptr;
    }
    return mysql_use_result(_conn);
}