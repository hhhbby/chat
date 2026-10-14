#include "db.h"
#include <vector>
using std::vector;

class OfflineMessageModel {
public:
    bool insert(int id, string msg);
    vector<string> query(int id);
    bool remove(int id);
private:
    MySql* mysql{MySql::instance()};
};