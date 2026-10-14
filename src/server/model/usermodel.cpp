#include "usermodel.h"

bool UserModel::insert(User& user)
{
    char sql[1024] = {'\0'};
    // 注意sql语句的写法
    snprintf(sql, sizeof sql, "insert into user(name, password, state) values('%s', '%s', '%s')", user.name.c_str(), user.passwd.c_str(), user.state.c_str());
    bool res = mysql->update(sql);
    if (res)
        user.id = mysql->insert_id();
    return res;
}

User UserModel::query(int id)
{
    User user;
    char sql[1024] = {'\0'};
    snprintf(sql, sizeof sql, "select * from user where id = %d", id);
    MYSQL_RES* res = mysql->query(sql);
    if (res)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row)
        {
            // 可能查询成功，但是符合条件的row为空
            user.id = atoi(row[0]);
            user.name = row[1];
            user.passwd = row[2];
            user.state = row[3];
        }
        mysql_free_result(res);
    }
    return user;
}

bool UserModel::updateState(const User &user)
{
    char sql[1024] = {'\0'};
    snprintf(sql, sizeof sql, "update user set state = '%s' where id = %d LIMIT 1", user.state.c_str(), user.id);
    bool res = mysql->update(sql);
    return res;
}
