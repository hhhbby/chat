#include <chatserver.h>

int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1", 9999);
    ChatServer server(&loop, addr, "chatserver1009");

    server.start();
    loop.loop();
}