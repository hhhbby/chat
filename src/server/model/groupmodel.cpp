#include "groupmodel.h"
#include "json.hpp"
using json = nlohmann::json;

bool GroupModel::addGroup(Group& group)
{
    char sql[1024];
    snprintf(sql, sizeof sql, "INSERT INTO group(groupname, groupdesc) VALUES(%s, %s)", 
            group.groupname.c_str(), group.groupname.c_str());
    group.id = mysql->insert_id();
    return mysql->update(sql);
}

bool GroupModel::joinGroup(int groupid, int userid, string grouprole)
{
    char sql[1024];
    snprintf(sql, sizeof sql, "INSERT INTO groupuser VALUES(%d, %d, '%s')", 
            groupid, userid, grouprole.c_str());
    return mysql->update(sql);
}

vector<string> GroupModel::queryGroup(int userid)
{
    vector<string> groups;
    char sql[1024];
    snprintf(sql, sizeof sql, "SELECT a.id, a.groupname, a.groupdesc FROM allgroup a JOIN groupuser b ON b.groupid = a.id WHERE b.userid = %d", userid);
    MYSQL_RES* res = mysql->query(sql);
    if (res)
    {
        MYSQL_ROW row = nullptr;
        while ((row = mysql_fetch_row(res)))
        {
            json groupInfo;
            groupInfo["id"] = atoi(row[0]);
            groupInfo["groupname"] = row[1];
            groupInfo["groupdesc"] = row[2];
            groups.push_back(groupInfo.dump());
        }
        mysql_free_result(res);
    }
    return groups;
}


vector<int> GroupModel::queryGroupUser(int groupid, int userid) // 查询这个用户的群友
{
    vector<int> users;
    char sql[1024];
    snprintf(sql, sizeof sql, "SELECT userid FROM groupuser WHERE groupid = %d and userid != %d",groupid, userid);
    MYSQL_RES* res = mysql->query(sql);
    if (res)
    {
        MYSQL_ROW row = nullptr;
        while ((row = mysql_fetch_row(res)))
        {
            users.push_back(atoi(row[0]));
        }
        mysql_free_result(res);
    }
    return users;
}