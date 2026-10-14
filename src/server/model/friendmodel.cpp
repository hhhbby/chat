#include "friendmodel.h"
#include "json.hpp"
using json = nlohmann::json;

bool FriendModel::insert(int userid, int friendid)
{
    char sql[1024];
    snprintf(sql, sizeof sql, "INSERT INTO friend VALUES(%d, %d)", userid, friendid);
    return mysql->update(sql);
}

vector<string> FriendModel::query(int userid)
{
    vector<string> friends;
    char sql[1024];
    snprintf(sql, sizeof sql, "SELECT a.friendid, b.name, b.state FROM friend a JOIN user b ON a.friendid = b.id WHERE a.userid = %d", userid);
    MYSQL_RES* res = mysql->query(sql);
    if (res)
    {
        MYSQL_ROW row = nullptr;
        while ((row = mysql_fetch_row(res)))
        {
            json friendInfo;
            friendInfo["id"] = atoi(row[0]);
            friendInfo["name"] = row[1];
            friendInfo["state"] = row[2];
            friends.push_back(friendInfo.dump());
        }
        mysql_free_result(res);
    }
    return friends;
}