#include "db.h"
#include <muduo/base/Logging.h>

static string server = "127.0.0.1";
static string user = "root";
static string passwd = "123456";
static string database = "chat";

MySql::MySql()
{
    // 错误写法  mysql_init(_conn);
    _conn = mysql_init(nullptr);
}

MySql::~MySql()
{
    if (_conn)
        mysql_close(_conn);
}

bool MySql::connect()
{
    MYSQL* p = mysql_real_connect(_conn, server.c_str(), user.c_str(), passwd.c_str(), database.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        LOG_INFO << "连接成功";
        mysql_query(_conn, "set names gbk");
    } else {
        LOG_INFO << "连接失败";
    }
    return p;
}

bool MySql::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << "更新失败 " << mysql_error(_conn);
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

int MySql::insert_id()
{
    return mysql_insert_id(_conn);
}