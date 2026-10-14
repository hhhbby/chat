#include "db.h"
#include "group.hpp"
#include <vector>
#include <string>
using std::vector;
using std::string;

class GroupModel {
public:
    bool addGroup(Group& group);
    bool joinGroup(int groupid, int userid, string grouprole);
    vector<string> queryGroup(int userid);   // 查询这个用户的所有群
    vector<int> queryGroupUser(int groupid, int userid); // 查询这个用户的群友
private:
    MySql* mysql = MySql::instance();
};