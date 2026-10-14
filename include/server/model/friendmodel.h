#include "db.h"
#include <vector>
#include <string>
using std::vector;
using std::string;

class FriendModel {
public:
    bool insert(int userid, int friendid);
    vector<string> query(int userid);
private:
    MySql* mysql = MySql::instance();
};