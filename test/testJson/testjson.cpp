#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
using namespace std;

#include <vector>
#include <map>

void func1()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "xing";
    js["to"] = "zhang san";
    js["msg"] = "hello, I am a newer";
    cout << "func1" << endl;
    cout << js << endl;
}

void func2()
{
    json js;
    js["msg_type"] = 2;
    js["to"] = {1, 2, 3, 4, 5};

    js["msg"]["1"] = "msg1";
    js["msg"]["2"] = "msg2";

    js["msg"] = {{"1", "msg1"}, {"2", "msg-2"}};
    cout << "func2" << endl;
    cout << js << endl;
}

void func3()
{
    json js;
    vector<int> v1 = {1, 2, 3, 4};
    js["list"] = v1;
    map<int, string> m;
    m[1] = "xxx";
    m[2] = "yyy";
    js["map"] = m;

    map<int, string> m1;
    m1.insert({1, "黄山"});
    m1.insert({2, "华山"});
    m1.insert({3, "泰山"});
    js["path"] = m1;
    cout << "func3" << endl;
    cout << js << endl;
}

string func4()
{
    json js;
    js["msg_type"] = 2;
    js["to"] = {1, 2, 3, 4, 5};

    js["msg"]["1"] = "msg1";
    js["msg"]["2"] = "msg2";

    return js.dump();
}

int main()
{
    func1();
    func2();
    func3();

    string recvBuf = func4();
    json jsbuf = json::parse(recvBuf);
    cout << jsbuf["to"] << endl;
    auto arr = jsbuf["to"];
    cout << arr[2] << endl;
    auto msg = jsbuf["msg"];
    cout << msg["1"] << endl;
    cout << msg["2"] << endl;
    return 0;
}