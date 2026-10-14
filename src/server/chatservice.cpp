#include "chatservice.h"
#include <muduo/base/Logging.h>

ChatService* ChatService::instance() 
{
    static ChatService service;
    return &service;
}
ChatService::MsgHandler ChatService::getHandler(int msgid) 
{
    // 减少对map的查找次数
    if (msgid < static_cast<int>(MsgType::NUM_MSG) && _msgHandlerMap.find(msgid) != _msgHandlerMap.end())
        return _msgHandlerMap[msgid];
    else 
        return [msgid](const TcpConnectionPtr &conn, json &js, Timestamp time){
            LOG_INFO << "msgid " << msgid << " don't have matched handler";
        };
}
bool ChatService::reset()
{
    return _userModel.resetState();
}
ChatService::ChatService()
{
    _msgHandlerMap[static_cast<int>(MsgType::LOGIN)] = bind(&ChatService::login, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::ENROLL)] = bind(&ChatService::enroll, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::PRIVATE_CHAT)] = bind(&ChatService::privateChat, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::ADD_FRIEND)] = bind(&ChatService::addFriend, this, _1, _2, _3);
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "login ....";
    int id = js["id"];
    string passwd = js["passwd"];
    User user = _userModel.query(id);

    json response;
    response["msgid"] = MsgType::LOGIN_ACK;
    if (user.id == -1)
    {
        response["errno"] = 1;
        response["errmsg"] = "user id or password is not correct, please try again";
    } else if (passwd == user.passwd) {
        if (user.state == "online")
        {
            response["errno"] = 2;
            response["errmsg"] = "user is online, please try later";
        } else {    // 登陆成功
            response["errno"] = 0;
            response["name"] = user.name;
            user.state = "online";
            _userModel.updateState(user);
            {
                lock_guard<mutex> lock(_connMtx);
                _userConnMap.insert({id, conn});
            }
            // 读取离线消息
            vector<string> msgs = _offlineMessageModel.query(id);
            if (!msgs.empty())
            {
                response["offlinemsgs"] = msgs;
                _offlineMessageModel.remove(id);
            }
            // 读取好友列表
            vector<string> friends = _friendModel.query(id);
            if (!friends.empty())
                response["frineds"] = friends;
            // 读取群组列表
            vector<string> groups = _groupModel.queryGroup(id);
            if (!groups.empty())
                response["groups"] = groups;
        }
    }
    conn->send(response.dump());
}

void ChatService::enroll(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string passwd = js["passwd"];

    User user{-1, name, passwd};
    bool res = _userModel.insert(user);

    json response;
    response["msgid"] = MsgType::ENROLL_ACK;
    if (res)
    {
        response["errno"] = 0;
        response["id"] = user.id;
    } else {
        response["errno"] = 1;
    }
    conn->send(response.dump());
}

void ChatService::privateChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // int from = js["from"];
    int to = js["to"];
    // string fromName = js["fromName"];
    // string msg = js["msg"];

    {
        lock_guard<mutex> lock(_connMtx);
        auto it = _userConnMap.find(to);
        if (it != _userConnMap.end()) {  // 当前to用户在线，
            it->second->send(js.dump());
            return;
        }
    }
    _offlineMessageModel.insert(to, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];
    int friendid = js["friendid"];
    _friendModel.insert(id, friendid);
}

void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    Group group{-1, js["groupname"], js["groupdesc"]};
    _groupModel.addGroup(group);
    _groupModel.joinGroup(group.id, js["id"], "creator");
}

void ChatService::joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int groupid = js["groupid"];
    int userid = js["id"];
    _groupModel.joinGroup(groupid, userid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int groupid = js["groupid"];
    int userid = js["id"];
    vector<int> users = _groupModel.queryGroupUser(groupid, userid);

    vector<int> offlineUsers;
    {
        lock_guard<mutex> lock(_connMtx);
        for (int id : users)
        {
            auto it = _userConnMap.find(id);
            if (it != _userConnMap.end())
                it->second->send(js.dump());
            else
                offlineUsers.push_back(id);
        }
    }
    for (int id : offlineUsers)
        _offlineMessageModel.insert(id, js.dump());
}
