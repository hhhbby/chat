#include <unordered_map>
#include <mutex>
using std::unordered_map;
using std::mutex;
using std::lock_guard;

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

#include "redis.h"

#include "usermodel.h"
#include "offlinemessagemodel.h"
#include "friendmodel.h"
#include "groupmodel.h"

class ChatService {
    using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;
public:
    static ChatService* instance();
    MsgHandler getHandler(int msgid);
    void clientCloseException(const TcpConnectionPtr &conn);
    bool reset();
private:
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void logout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void enroll(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void privateChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    ChatService();
    unordered_map<int, MsgHandler> _msgHandlerMap;
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    mutex _connMtx;

    UserModel _userModel;
    OfflineMessageModel _offlineMessageModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    Redis _redis;
    void handleRedisSubscribeMessage(int id, string msg);
};