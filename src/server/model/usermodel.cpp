#include "usermodel.h"

UserModel::UserModel()
{
    mysql.connect();
}

bool UserModel::insert(User& user)
{
    char sql[1024] = {'\0'};
    // 注意sql语句的写法
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')", user.name.c_str(), user.passwd.c_str(), user.state.c_str());
    bool res = mysql.update(sql);
    if (res)
        user.id = mysql.insert_id();
    return res;
}