#include "offlinemessagemodel.h"
#include "db.h"
#include "json.hpp"
using json = nlohmann::json;
bool OfflineMessageModel::insert(int id, string msg)
{
    char sql[1024]{};
    snprintf(sql, sizeof sql, "insert into offlinemessage values(%d, '%s')", id, msg.c_str());
    return mysql->update(sql);
}

vector<string> OfflineMessageModel::query(int id)
{
    vector<string> msgs;
    char sql[1024]{};
    snprintf(sql, sizeof sql, "select * from offlinemessage where userid = %d", id);
    MYSQL_RES* res= mysql->query(sql);
    if (res)
    {
        MYSQL_ROW row = nullptr;
        while ((row = mysql_fetch_row(res)))
        {
            // TODO maybe more effective
            json msg;
            msg["userid"] = atoi(row[0]);
            msg["message"] = row[1];
            msgs.push_back(msg.dump());
        }
        mysql_free_result(res);
    }
    return msgs;
}

bool OfflineMessageModel::remove(int id)
{
    char sql[1024]{};
    snprintf(sql, sizeof sql, "delete from offlinemessage where userid = %d", id);
    return mysql->update(sql);
}
