#include <iostream>
#include <vector>
#include <string>
using std::cin, std::cout, std::endl;
using std::vector, std::string;

#include "pub.h"
#include "user.hpp"
#include "group.hpp"
#include "json.hpp"
using json = nlohmann::json;

#include <arpa/inet.h>
#include <unistd.h>

#define ERR_EXIT(msg) {\
    cout << msg << endl;\
    exit(-1);\
}

User g_me;
vector<User> g_currentUserFriendList;
vector<Group> g_currentUserGroupList;

void showLoginInfo();
void enroll(int clientFd);
void login(int clientFd);

int main()
{
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd == -1)
        ERR_EXIT("socket failed");
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9999);
    if (inet_pton(AF_INET, "127.0.0.1", static_cast<void*>(&serverAddr.sin_addr)) != 1)
        ERR_EXIT("inet_pton failed");
    if (connect(clientFd, reinterpret_cast<const sockaddr*>(&serverAddr), static_cast<socklen_t>(sizeof(sockaddr_in))) != 0)
        ERR_EXIT("connect failed");

    while(true)
    {
        cout << "client choices" << endl;
        cout << "1.enroll" << endl;
        cout << "2.login" << endl;
        cout << "3.exit" << endl;
        cout << "input your choice:";
        int choice = -1;
        cin >> choice;
        cin.get();      // 去除\n，也可以使用cin.ignore
        if (choice == 1) {
            enroll(clientFd);            
        } else if (choice == 2) {
            login(clientFd);
        } else if (choice == 3) {
            exit(-1);
        } else {
            cout << "invaild choice, try again" << endl;
        }
    }
}

void enroll(int clientFd)
{
    string name;
    string passwd;
    cout << "input your name" << endl;
    getline(cin, name);
    cout << "input your passwd" << endl;
    getline(cin, passwd);
    
    json sendJs;
    sendJs["msgid"] = MsgType::ENROLL;
    sendJs["name"] = name;
    sendJs["passwd"] = passwd;
    string sendBuf = sendJs.dump();

    int sendLen = send(clientFd, sendBuf.c_str(), sendBuf.size(), 0);
    if (sendLen == -1)
        ERR_EXIT("send failed in enroll");        
    char recvBuf[1024]{};
    int recvLen = recv(clientFd, recvBuf, sizeof(recvBuf), 0);
    if (recvLen <= 0)
        ERR_EXIT("recv failed in enroll");
    json recvJs = json::parse(recvBuf);
    int errnum = recvJs["errno"];
    if (errnum == 0)
    {
        int id = recvJs["id"];
        cout << "enroll succedd, your id is " << id << endl;
    } else {
        cout << "enroll failed" << endl;
    }
    // MsgType type = recvJs["msgid"];
}

void login(int clientFd)
{
    int id;
    string passwd;
    cout << "input your id:";
    cin >> id;
    cin.get();
    cout << "input your passwd:";
    getline(cin, passwd);
    json js;
    js["msgid"] = MsgType::LOGIN;
    js["id"] = id;
    js["passwd"] = passwd;
    string sendBuf = js.dump();
    int sendLen = send(clientFd, sendBuf.c_str(), sendBuf.size(), 0);
    if (sendLen == -1)
        ERR_EXIT("send failed in login");
    char recvBuf[1024]{};
    int recvLen = recv(clientFd, recvBuf, sizeof(recvBuf), 0);
    if (recvLen <= 0)
        ERR_EXIT("recv failed in login");
    json recvJs = json::parse(recvBuf);
    int errnum = recvJs["errno"];
    if (errnum == 0)
    {
        string name = recvJs["name"];
        cout << "login succedd, you are " << name << endl;
        g_me.id = id;
        g_me.name = name;
        g_me.state = "online";
        g_currentUserFriendList.clear();
        if (recvJs.contains("friends"))
        {
            vector<string> friends = recvJs["friends"];
            for (const string& friendStr : friends)
            {
                json js = json::parse(friendStr);
                User user{js["id"], js["name"], "", js["state"]};
                g_currentUserFriendList.push_back(user);
            }
        }
        g_currentUserGroupList.clear();
        if (recvJs.contains("groups"))
        {
            vector<string> groups = recvJs["groups"];
            for (const string& groupStr : groups)
            {
                json js = json::parse(groupStr);
                Group group{js["id"], js["groupname"], js["groupdesc"]};
                g_currentUserGroupList.push_back(group);
            }
        }

        showLoginInfo();

        if (recvJs.contains("offlinemsgs"))
        {
            vector<string> offlinemsgs = recvJs["offlinemsgs"];
            for (const string& msgStr : offlinemsgs)
            {
                json js = json::parse(msgStr);
                json message = json::parse(js["message"].get<string>());
                string fromName = message["fromName"];
                string content = js["msg"];
                cout << fromName << " say : " << content << endl;
            }
        }

    } else {
        cout << "login failed:" << recvJs["errmsg"] << endl;
    }
}

void showLoginInfo()
{
    cout << "------ friends ------" << endl;
    for (const User& u : g_currentUserFriendList)
    {
        cout << "id: " << u.id << ", name: " << u.name << ", state: " << u.state << endl;
    }

    cout << "------ groups ------" << endl;
    for (const Group& g : g_currentUserGroupList)
    {
        cout << "id: " << g.id << ", name: " << g.groupname << ", desc: " << g.groupdesc << endl;
    }
}
