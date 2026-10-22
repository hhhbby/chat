#include "json.hpp"
using json = nlohmann::json;
#include "pub.h"
#include "user.hpp"
#include "group.hpp"

#include "sem.hpp"

#include <unistd.h>     // for close()
#include <arpa/inet.h>

#include <thread>
using std::thread;
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
using std::cin, std::cout, std::endl;
using std::vector, std::string, std::unordered_map, std::function;

Sem sem(0);
User g_me;
vector<User> g_currentUserFriendList;
vector<Group> g_currentUserGroupList;
bool g_loggedin = false;

void chatMenu(int clientFd);

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
    
    while (sem.wait() == -1)
    {
        if (errno != EINTR)
            exit(-3);
    }
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

    while (sem.wait() == -1)
    {
        if (errno != EINTR)
            exit(-3);
    }
    if (g_loggedin)
    {
        chatMenu(clientFd);
    }
}

void help()
{
    cout << "choice\t\tcommand" << endl << endl;
    cout << "showinfo\tshowinfo" << endl;
    cout << "addfriend\taddfriend:friendid" << endl;
    cout << "chat\t\tchat:friendid:msg" << endl;
    cout << "addgroup\taddgroup:groupname:groupdesc" << endl;
    cout << "joingroup\tjoingroup:groupid" << endl;
    cout << "groupchat\tgroup:groupid:msg" << endl;
    cout << "logout\t\tlogout" << endl;
}

void showinfo();
bool addfriend(int clientFd, string cmd);
bool chat(int clientFd, string cmd);
bool addgroup(int clientFd, string cmd);
bool joingroup(int clientFd, string cmd);
bool groupchat(int clientFd, string cmd);
void logout(int clientFd);

unordered_map<string, function<bool(int, string)>> g_cmdMap {
    {"addfriend", addfriend},
    {"chat", chat},
    {"addgroup", addgroup},
    {"joingroup", joingroup},
    {"group", groupchat}
};

bool dipatch(int clientFd, string command)
{
    size_t idx = command.find(":");
    if (idx == string::npos)
    {
        if (command == "showinfo")
        {
            showinfo();
            return true;
        } else if(command == "logout") {
            logout(clientFd);
            return true;
        }
    } else {
        auto it = g_cmdMap.find(command.substr(0, idx));
        if (it != g_cmdMap.end())
        {
            return it->second(clientFd, command.substr(idx + 1));
        }
    }
    return false;
}

void chatMenu(int clientFd)
{
    while (g_loggedin)
    {
        cout << endl << "input your command:";
        string command;
        cin >> command;
        if (!dipatch(clientFd, command))
        {
            cout << "invalid command" << endl;
        }
    }
}

void doLogin(json &recvJs)
{
    int errnum = recvJs["errno"];
    if (errnum == 0)
    {
        string name = recvJs["name"];
        g_me.id = recvJs["id"];
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
        showinfo();
        if (recvJs.contains("offlinemsgs"))
        {
            vector<string> offlinemsgs = recvJs["offlinemsgs"];
            for (const string& msgStr : offlinemsgs)
            {
                json js = json::parse(msgStr);
                json message = json::parse(js["message"].get<string>());
                string fromName = message["fromName"];
                int fromId = message["id"];
                string msg = message["msg"];
                cout << fromName << "[" << fromId << "] say : " << msg << endl;
            }
        }
        g_loggedin = true;
    } else {
        cout << "login failed:" << recvJs["errmsg"] << endl;
        g_loggedin = false;
    }
    sem.post();
}

void doEnroll(json &recvJs)
{
    int errnum = recvJs["errno"];
    if (errnum == 0)
    {
        int id = recvJs["id"];
        cout << "enroll succedd, your id is " << id << endl;
    } else {
        cout << "enroll failed" << endl;
    }
    sem.post();
}

// for read thread to recv all msg from server
void readHandler(int clientFd)
{
    while (true)
    {
        char buf[1024]{};
        if (recv(clientFd, buf, sizeof buf, 0) <= 0)
        {
            close(clientFd);
            cout << "server error" << endl;
            exit(-1);
        }
        json js = json::parse(buf);
        MsgType msgid = js["msgid"];
        switch (msgid)
        {
        case MsgType::PRIVATE_CHAT:
            {
                string fromName = js["fromName"];
                int fromid = js["id"];
                string msg = js["msg"];
                cout  << endl << fromName << "[" << fromid << "] say : " << msg  << endl;
            }
            break;
        case MsgType::GROUP_CHAT:
            {
                string fromName = js["fromName"];
                int fromid = js["id"];
                string msg = js["msg"];
                int groupid = js["groupid"];
                cout  << endl << fromName << "[" << fromid << "][group " << groupid << "] say : " << msg << endl;
            }
            break;
        case MsgType::LOGIN_ACK:
            doLogin(js);
            break;
        case MsgType::ENROLL_ACK:
            doEnroll(js);
            break;
        default:
            break;
        }
        if (msgid != MsgType::LOGIN_ACK && msgid != MsgType::ENROLL_ACK)
            cout << endl << "input your command:" << std::flush;
    }
}

// all the functions behind this line are services
void showinfo()
{
    cout << "Hi," << g_me.name << "(id:" << g_me.id << ")" << endl;
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

    help();
}

bool addfriend(int clientFd, string cmd)
{
    int friendid = atoi(cmd.c_str());
    if (friendid <= 0)
        return false;
    json js;
    js["msgid"] = MsgType::ADD_FRIEND;
    js["id"] = g_me.id;
    js["friendid"] = friendid;
    string sendBuf = js.dump();
    return send(clientFd, sendBuf.c_str(), sendBuf.size(), 0) > 0;
}

bool chat(int clientFd, string cmd)
{
    size_t idx = cmd.find(":");
    if (idx == string::npos)
        return false;
    int friendid = atoi(cmd.substr(0, idx).c_str());
    if (friendid <= 0)
        return false;
    json js;
    js["msgid"] = MsgType::PRIVATE_CHAT;
    js["id"] = g_me.id;
    js["fromName"] = g_me.name;
    js["to"] = friendid;
    js["msg"] = cmd.substr(idx + 1);
    string sendBuf = js.dump();
    return send(clientFd, sendBuf.c_str(), sendBuf.size(), 0) > 0;
}

bool addgroup(int clientFd, string cmd)
{
    size_t idx = cmd.find(":");
    if (idx == string::npos)
        return false;
    json js;
    js["msgid"] = MsgType::ADD_GROUP;
    js["id"] = g_me.id;
    js["groupname"] = cmd.substr(0, idx);
    js["groupdesc"] = cmd.substr(idx + 1);
    string sendBuf = js.dump();
    return send(clientFd, sendBuf.c_str(), sendBuf.size(), 0) > 0;
}

bool joingroup(int clientFd, string cmd)
{
    int groupid = atoi(cmd.c_str());
    if (groupid == 0)
        return false;
    json js;
    js["msgid"] = MsgType::JOIN_GROUP;
    js["id"] = g_me.id;
    js["groupid"] = groupid;
    string sendBuf = js.dump();
    return send(clientFd, sendBuf.c_str(), sendBuf.size(), 0) > 0;
}

bool groupchat(int clientFd, string cmd)
{
    size_t idx = cmd.find(":");
    if (idx == string::npos)
        return false;
    int groupid = atoi(cmd.substr(0, idx).c_str());
    if (groupid <= 0)
        return false;
    json js;
    js["msgid"] = MsgType::GROUP_CHAT;
    js["id"] = g_me.id;
    js["fromName"] = g_me.name;
    js["groupid"] = groupid;
    js["msg"] = cmd.substr(idx + 1);
    string sendBuf = js.dump();
    return send(clientFd, sendBuf.c_str(), sendBuf.size(), 0) > 0;
}

void logout(int clientFd)
{
    json js;
    js["msgid"] = MsgType::LOGOUT;
    js["id"] = g_me.id;
    string sendBuf = js.dump();
    if (send(clientFd, sendBuf.c_str(), sendBuf.size(), 0) < 0)
    {
        cout << "send loginout msg error -> " << sendBuf << endl;
    } else {
        g_loggedin = false;
    }
}