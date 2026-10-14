#include "user.hpp"
#include "db.h"

class UserModel {
public:
    bool insert(User& user);
    User query(int id);
    bool updateState(const User &user);
    bool resetState();
private:
    MySql* mysql = MySql::instance();
};