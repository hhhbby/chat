#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>
using std::string;
using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::TcpServer;
using muduo::net::Buffer;
using muduo::net::TcpConnectionPtr;
using muduo::Timestamp;

class ChatServer
{
public:
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg);
    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time);
    TcpServer _server;
    EventLoop *_loop;
};