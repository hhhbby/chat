#include "user.hpp"
#include "db.h"

class UserModel {
public:
    UserModel();
    bool insert(User& user);
private:
    MySql mysql;
};