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
    // 修改连接此服务器用户的状态为离线
    vector<User> users;
    {
        lock_guard<mutex> lock(_connMtx);
        for (auto item : _userConnMap)
        {
            users.push_back(User{item.first});
        }
    }
    for (const User& u : users)
    {
        if (!_userModel.updateState(u))
            return false;
    }
    return true;
}
ChatService::ChatService()
{
    _msgHandlerMap[static_cast<int>(MsgType::LOGIN)] = bind(&ChatService::login, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::ENROLL)] = bind(&ChatService::enroll, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::PRIVATE_CHAT)] = bind(&ChatService::privateChat, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::ADD_FRIEND)] = bind(&ChatService::addFriend, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::ADD_GROUP)] = bind(&ChatService::addGroup, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::JOIN_GROUP)] = bind(&ChatService::joinGroup, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::GROUP_CHAT)] = bind(&ChatService::groupChat, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::LOGOUT)] = bind(&ChatService::logout, this, _1, _2, _3);

    if (!_redis.connect("127.0.0.1", 6379))
    {
        LOG_INFO << "redis connect failed";
    }
    _redis.set_notify_handler(bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
}

void ChatService::handleRedisSubscribeMessage(int id, string msg)
{
    lock_guard<mutex> lock(_connMtx);
    auto it = _userConnMap.find(id) ;
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
    } else {
        _offlineMessageModel.insert(id, msg);
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    int id = -1;
    {
        lock_guard<mutex> lock(_connMtx);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                id = it->first;
                it = _userConnMap.erase(it);
                break;
            }
        }
    }
    // 这个断开连接的客户端之前已登录
    if (id != -1)
    {
        _userModel.updateState(User{id});
        _redis.unsubscribe(id);
    }

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
        response["errmsg"] = "user id is not found";
    } else if (passwd == user.passwd) {
        if (user.state == "online")
        {
            response["errno"] = 2;
            response["errmsg"] = "user is online, please try later";
        } else {    // 登陆成功
            response["errno"] = 0;
            response["name"] = user.name;
            user.state = "online";
            _redis.subscribe(id);
            _userModel.updateState(user);
            LOG_INFO << "login success";
            {
                lock_guard<mutex> lock(_connMtx);
                _userConnMap.insert({id, conn});
            }
            // LOG_INFO << "loading offline";
            // 读取离线消息
            vector<string> msgs = _offlineMessageModel.query(id);
            if (!msgs.empty())
            {
                response["offlinemsgs"] = msgs;
                _offlineMessageModel.remove(id);
            }
            // LOG_INFO << "loading friend";
            // 读取好友列表
            vector<string> friends = _friendModel.query(id);
            if (!friends.empty())
                response["friends"] = friends;
                // 读取群组列表
            // LOG_INFO << "loading group";
            vector<string> groups = _groupModel.queryGroup(id);
            if (!groups.empty())
                response["groups"] = groups;
        }
    } else {
        response["errno"] = 2;
        response["errmsg"] = "user id or password is not correct";
    }
    conn->send(response.dump());
}

void ChatService::logout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];
    {
        lock_guard<mutex> lock(_connMtx);
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }
    _redis.unsubscribe(id);
    _userModel.updateState(User{id});
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
    // int from = js["id"];
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
    User user = _userModel.query(to);
    if (user.state == "online")
    {
        _redis.publish(to, js.dump());
    } else {
        _offlineMessageModel.insert(to, js.dump());
    }
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
    {
        User user = _userModel.query(id);
        if (user.state == "online")
        {
            _redis.publish(id, js.dump());
        } else {
            _offlineMessageModel.insert(id, js.dump());
        }
    }
}
