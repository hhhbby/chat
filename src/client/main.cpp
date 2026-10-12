#include <iostream>
using std::cin, std::cout, std::endl;

#include "pub.h"
#include "user.hpp"
#include "json.hpp"
using json = nlohmann::json;

#include <arpa/inet.h>
#include <unistd.h>

#define ERR_EXIT(msg) {\
    cout << msg << endl;\
    exit(-1);\
}

User _me;

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
        ERR_EXIT("send failed");
    
    char recvBuf[1024]{};
    int recvLen = recv(clientFd, recvBuf, sizeof(recvBuf), 0);
    if (recvLen <= 0)
    {
        close(clientFd);
        exit(-1);
    }
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
}
