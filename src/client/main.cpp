#include "clientservice.hpp"

#include <iostream>
using std::cin, std::cout, std::endl;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cout << "usage: " << argv[0] << " portnumber" << endl;
        exit(-1);
    }
    int portnumber = atoi(argv[1]);
    if (portnumber == 0)
        ERR_EXIT("portnumber invalid");

    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd == -1)
        ERR_EXIT("socket failed");
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portnumber);
    if (inet_pton(AF_INET, "127.0.0.1", static_cast<void*>(&serverAddr.sin_addr)) != 1)
        ERR_EXIT("inet_pton failed");
    if (connect(clientFd, reinterpret_cast<const sockaddr*>(&serverAddr), static_cast<socklen_t>(sizeof(sockaddr_in))) != 0)
        ERR_EXIT("connect failed");

    // 在连接服务器端成功后就创建唯一一个读取线程，用于处理后面所有的读取
    thread(readHandler, clientFd).detach();
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
            exit(0);
        } else {
            cout << "invaild choice, try again" << endl;
        }
    }
}
