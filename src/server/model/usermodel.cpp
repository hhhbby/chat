#include "usermodel.h"

UserModel::UserModel()
{
    mysql.connect();
}

bool UserModel::insert(User& user)
{
    char sql[1024] = {'\0'};
    sprintf(sql, "insert into user(name, passwd, state) values(%s, %s, %s)", user.name, user.passwd, user.state);
    bool res = mysql.update(sql);
    if (res)
        user.id = mysql.insert_id();
    return mysql.update(sql); 
}