#include "user.hpp"
#include "db.h"

class UserModel {
public:
    bool insert(User& user);
    User queryById(int id);
    bool updateState(const User &user);
private:
    MySql* mysql = MySql::instance();
};