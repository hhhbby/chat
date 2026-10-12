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
ChatService::ChatService()
{
    _msgHandlerMap[static_cast<int>(MsgType::LOGIN)] = bind(&ChatService::login, this, _1, _2, _3);
    _msgHandlerMap[static_cast<int>(MsgType::ENROLL)] = bind(&ChatService::enroll, this, _1, _2, _3);
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "login ....";
    int id = js["id"];
    string passwd = js["passwd"];
    User user = _userModel.queryById(id);

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
        } else {
            response["errno"] = 0;
            response["name"] = user.name;
            user.state = "online";
            _userModel.updateState(user);
            {
                lock_guard<mutex> lock(_connMtx);
                _userConnMap.insert({id, conn});
            }
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
