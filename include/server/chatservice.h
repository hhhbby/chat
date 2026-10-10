#include <unordered_map>
using std::unordered_map;

#include <functional>
using std::function;
using std::bind;
using std::placeholders::_1, std::placeholders::_2, std::placeholders::_3;

#include "json.hpp"
using json = nlohmann::json;

#include <muduo/net/TcpConnection.h>
using muduo::net::TcpConnectionPtr;
using muduo::Timestamp;

#include "pub.h"

#include "usermodel.h"

class ChatService {
    using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;
public:
    static ChatService* instance();
    MsgHandler getHandler(int msgid);
private:
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void enroll(const TcpConnectionPtr &conn, json &js, Timestamp time);

    ChatService();
    unordered_map<int, MsgHandler> msgHandlerMap;

    UserModel _userModel;
};