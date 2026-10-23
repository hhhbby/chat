# Chat：C++ 网络聊天学习项目

基于 C++17、Muduo、MySQL 和 Redis 实现的命令行聊天项目，包含注册登录、私聊、群聊、好友关系及离线消息等基础流程。

项目重点是练习 Linux TCP 通信、消息分帧、线程同步、数据库访问，以及通过 Redis Pub/Sub 在多个服务进程之间转发消息。当前实现用于学习和本地演示，尚未提供并发安全和可靠消息投递的完整保证。

## 功能

- 用户注册、登录、注销，并维护在线状态。
- 添加好友，登录时加载好友列表。
- 创建群组、加入群组和群聊。
- 私聊：本进程在线用户直接转发，其他进程在线用户通过 Redis 转发，离线用户写入 MySQL。
- 登录时读取离线消息、好友和群组信息。
- TCP 使用 4 字节长度头处理半包和粘包。
- 客户端只有一个接收线程，通过 POSIX 信号量通知主线程注册、登录响应处理完成。
- 客户端循环发送以处理部分发送，对 EINTR 重试。

## 技术栈与结构

| 组件 | 用途 |
| --- | --- |
| C++17 | 核心语言 |
| Muduo | 服务端 TCP 连接、事件循环和消息回调 |
| POSIX socket / thread / semaphore | 客户端通信和线程同步 |
| MySQL C API | 用户、好友、群组和离线消息持久化 |
| Hiredis / Redis Pub/Sub | 跨服务进程消息转发 |
| nlohmann/json | 消息序列化，头文件位于 thirdparty/ |
| CMake | 构建 client 和 server |

```text
命令行客户端
    │ TCP：长度头 + JSON
    ▼
ChatServer（网络回调）
    ▼
ChatService（按 msgid 分发业务）
    ├── 本进程用户连接表 → 直接发送
    ├── Redis Pub/Sub   → 其他服务进程 → 目标客户端
    └── Model / MySQL   → 用户、好友、群组、离线消息
```

服务端配置了 4 个 Muduo 工作线程。网络回调中的数据库访问目前仍为同步操作。

```text
.
├── CMakeLists.txt
├── include/
│   ├── pub.h                    # 消息类型
│   ├── sem.hpp                  # POSIX 信号量封装
│   ├── client/
│   │   └── clientservice.hpp    # 客户端业务、收发、分帧
│   └── server/
│       ├── chatserver.h
│       ├── chatservice.h
│       ├── db/
│       ├── model/
│       └── redis/
├── src/
│   ├── client/main.cpp
│   └── server/
│       ├── main.cpp
│       ├── chatserver.cpp
│       ├── chatservice.cpp
│       ├── db/
│       ├── model/
│       └── redis/
├── thirdparty/json.hpp
└── test/                        # JSON、Muduo 等学习示例
```

## 环境准备

在 Linux 环境中准备：

- 支持 C++17 的编译器。
- CMake 3.13 或更高版本。
- Muduo 的头文件，以及 muduo_net、muduo_base 库。
- MySQL 服务和 MySQL C API 开发库 mysqlclient。
- Redis 服务和 Hiredis 开发库。
- POSIX 线程支持。

将依赖安装到编译器和链接器可以搜索到的位置；自定义安装位置时，需要补充对应的头文件、库文件搜索路径。


### 连接配置

| 配置 | 当前值 / 位置 |
| --- | --- |
| 服务端监听地址 | 127.0.0.1，src/server/main.cpp |
| 客户端连接地址 | 127.0.0.1，src/client/main.cpp |
| TCP 端口 | 启动时通过第一个命令行参数传入 |
| MySQL | 127.0.0.1:3306，数据库 chat，src/server/db/db.cpp |
| MySQL 凭据 | 当前写在 src/server/db/db.cpp，运行前按本机配置调整 |
| Redis | 127.0.0.1:6379，src/server/chatservice.cpp |

客户端和服务端默认在同一台机器运行。端口参数不会改变 IP 地址；跨机器部署需要调整地址、数据库与 Redis 连接配置。

MySQL 连接当前执行 `set names gbk`。如果要支持 UTF-8 中文和 emoji，应统一修改连接字符集、数据库及字段字符集；不能只改表的字符集。

### 数据库初始化

仓库目前没有独立的数据库迁移脚本。下列 SQL 是根据 Model 代码整理的**参考初始化结构**，不是从现有服务器导出的表结构；请在新建的学习数据库中使用，不要据此覆盖已有表。

当前代码使用了 `SELECT *` 和省略列名的 INSERT，因此字段顺序需要与下表保持一致：

| 表 | 字段顺序 |
| --- | --- |
| user | id, name, password, state |
| friend | userid, friendid |
| allgroup | id, groupname, groupdesc |
| groupuser | groupid, userid, grouprole |
| offlinemessage | userid, message |

```sql
CREATE DATABASE chat CHARACTER SET gbk;
USE chat;

CREATE TABLE user (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(50) NOT NULL,
    password VARCHAR(100) NOT NULL,
    state ENUM('online', 'offline') NOT NULL DEFAULT 'offline'
);

CREATE TABLE friend (
    userid INT NOT NULL,
    friendid INT NOT NULL,
    PRIMARY KEY (userid, friendid)
);

CREATE TABLE allgroup (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    groupname VARCHAR(50) NOT NULL,
    groupdesc VARCHAR(200) NOT NULL
);

CREATE TABLE groupuser (
    groupid INT NOT NULL,
    userid INT NOT NULL,
    grouprole ENUM('creator', 'normal') NOT NULL DEFAULT 'normal',
    PRIMARY KEY (groupid, userid)
);

CREATE TABLE offlinemessage (
    userid INT NOT NULL,
    message TEXT NOT NULL,
    INDEX idx_offline_user (userid)
);
```

参考结构不包含外键、好友申请流程或双向好友对去重。群成员角色目前存储为 creator / normal，尚未形成完整权限系统。离线 message 字段保存 JSON 文本，不是普通聊天正文。

## 编译与运行

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

生成：

```text
build/bin/server
build/bin/client
```

当前 CMake 固定添加了 `-O0`，用于学习调试，不应把当前构建当作性能测试基准。

先启动并配置好 MySQL、Redis，再分别在两个终端执行：

```bash
# 终端 1
./build/bin/server 6000
```

```bash
# 终端 2
./build/bin/client 6000
```

客户端启动后选择注册或登录。注册成功会返回用户 ID，登录使用 ID 和密码。可以再开一个客户端终端测试双方聊天。

### 多进程转发演示

在同一台机器启动两个服务进程：

```bash
# 分别在两个终端运行
./build/bin/server 8001
./build/bin/server 8002
```

再分别启动：

```bash
./build/bin/client 8000
./build/bin/client 8000
```

用不同用户登录后进行私聊，可以练习 Redis 跨进程转发。两个服务进程需使用相同数据库和 Redis。此路径用于功能学习，仍受后文并发及消息可靠性限制影响。

仓库未提供 Nginx 部署配置，以上示例由客户端直接选连接Nginx，由其负责转发。

## 客户端命令

登录成功后输入：

| 命令 | 示例 | 用途 |
| --- | --- | --- |
| showinfo | showinfo | 显示当前用户、缓存的好友和群组 |
| addfriend:用户ID | addfriend:2 | 添加好友 |
| chat:用户ID:消息 | chat:2:hello | 发送私聊 |
| addgroup:群名:描述 | addgroup:cpp:study | 创建群组 |
| joingroup:群ID | joingroup:1 | 加入群组 |
| group:群ID:消息 | group:1:hello | 发送群聊 |
| logout | logout | 注销账号并回到初始菜单 |

群聊实际命令是 `group`，不是 `groupchat`。示例 ID 需要换成实际创建的用户或群组 ID。

当前命令通过 `cin >> command` 读取，不支持包含空格的完整命令。好友、建群、入群操作尚无完整业务响应；列表主要在登录时加载，操作后不会实时刷新。建群没有即时返回群 ID，可重新登录查看已加入的群组。

注销保留 TCP 连接和唯一接收线程，重新登录不会再创建读取线程。连接断开时，客户端当前直接退出进程。

## 通信协议

```text
+---------------------------+-------------------------+
| 4 字节长度头（网络字节序） | JSON 内容（N 字节）      |
+---------------------------+-------------------------+
```

- 长度头表示 JSON 序列化后的字节数，不包含头本身或字符串结尾的零字符。
- 客户端使用 htonl / ntohl，服务端使用 Muduo Buffer 的整数接口处理字节序。
- 当前工作区中双方接收正文长度限定为 1 至 1 MiB。
- 客户端使用 pending 累积缓冲区，服务端使用 Muduo Buffer；完整一帧到齐后才解析 JSON，并循环处理多帧。
- 发送端尚未检查同样的长度上限，超限响应会被接收端拒绝。
- Redis 和数据库中继续传递或保存 JSON 内容，只有 TCP 收发边界添加或移除长度头。

消息类型定义于 include/pub.h：

| msgid | 类型 |
| --- | --- |
| 0 | LOGIN |
| 1 | LOGIN_ACK |
| 2 | LOGOUT |
| 3 | ENROLL |
| 4 | ENROLL_ACK |
| 5 | PRIVATE_CHAT |
| 6 | ADD_FRIEND |
| 7 | ADD_GROUP |
| 8 | JOIN_GROUP |
| 9 | GROUP_CHAT |

登录正文示例：

```json
{"msgid":0,"id":1,"passwd":"example-password"}
```

私聊正文示例：

```json
{"msgid":5,"id":1,"fromName":"alice","to":2,"msg":"hello"}
```

这些示例仅展示正文。直接通过 telnet 或 nc 发送纯 JSON 不符合当前协议，需要同时发送二进制长度头。useful.txt 中的历史示例可能与当前消息编号不一致，以 include/pub.h 和实际收发代码为准。

## 已知限制

本项目保留了一些未完成的工程问题：

1. **并发访问**：多个线程共享同一个 MySQL 连接和 Redis 上下文，尚未完善连接独占、锁范围或连接池。
2. **SQL 与字符处理**：SQL 直接拼接，固定缓冲区可能截断长文本，单引号可能导致 SQL 失败；尚未使用参数绑定。
3. **消息可靠性**：离线消息在客户端确认前删除；Redis Pub/Sub 转发与本地发送没有端到端 ACK、重试或去重。
4. **会话与权限**：请求中的用户身份尚未与连接严格绑定，群权限校验不完整；并发登录和异常退出后的在线状态恢复仍需完善。
5. **安全与输入校验**：密码按明文保存与比较，连接凭据硬编码，没有 TLS，JSON 异常处理和输入校验尚不完整。
6. **错误处理**：数据库和 Redis 连接错误及业务返回值未完整处理；建群调用处仍未检查建群成功后再添加创建者。
7. **线程退出**：接收线程使用 detach，缺少统一的停止、唤醒、join 和资源回收流程；SIGINT 中的清理流程也需完善。
8. **客户端边界情况**：send 尚未抑制 SIGPIPE，数字输入失败、EOF 和命令空格处理不完善。
9. **功能体验**：缺少业务操作确认、列表实时刷新、历史消息分页和离线群消息的完整展示。

因此，多线程网络框架不代表本项目已实现高并发安全，Redis 转发也不代表消息保证送达。建议优先完善 MySQL 和 Redis 的并发访问，再逐步扩展业务。

## 验证建议

test/ 中主要是库的学习示例，尚无完整自动化业务回归测试。可以按以下顺序手动检查：

- 注册、正确密码登录、错误密码登录。
- 同一客户端反复注销、重新登录。
- 两个客户端私聊，以及接收者离线后重新登录查看消息。
- 创建群、加入群、群消息转发。
- 两个服务进程间的消息转发。
- 使用遵循协议的测试客户端拆开发送长度头、拆开发送正文、合并发送多帧。
- 测试大于 1024 字节的完整响应、零长度头和超过上限的头。

长文本的数据库存储仍受 SQL 缓冲区限制，测试分帧时应与数据库写入问题区分。这些是建议执行的验证项，不代表已经全部通过。

## 学习内容

- TCP 字节流、消息边界、半包粘包与长度头协议。
- 网络字节序、部分发送、EINTR、FIN/RST 和 SIGPIPE。
- Muduo 回调与事件循环。
- 单接收线程、信号量同步及线程生命周期。
- MySQL C API、结果集生命周期和连接共享问题。
- Redis Pub/Sub 与跨服务进程通信。
