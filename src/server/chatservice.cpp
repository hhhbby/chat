#include "chatservice.h"
#include <muduo/base/Logging.h>

ChatService* ChatService::instance() 
{
    static ChatService service;
    return &service;
}
ChatService::MsgHandler ChatService::getHandler(int msgid) 
{
    if (msgid < static_cast<int>(MsgType::NUM_MSG))
        return msgHandlerMap[msgid];
    else 
        return [msgid](const TcpConnectionPtr &conn, json &js, Timestamp time){
            LOG_INFO << "msgid " << msgid << " don't have matched handler";
        };
}
ChatService::ChatService()
{
    msgHandlerMap[static_cast<int>(MsgType::LOGIN)] = bind(&ChatService::login, this, _1, _2, _3);
    msgHandlerMap[static_cast<int>(MsgType::ENROLL)] = bind(&ChatService::enroll, this, _1, _2, _3);
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "login ....";
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
