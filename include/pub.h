#pragma once
#include "iostream"
using std::cout, std::endl;

#define ERR_EXIT(msg) {\
    cout << msg << endl;\
    exit(-1);\
}

enum class MsgType{
    LOGIN,
    LOGIN_ACK,
    LOGOUT,
    ENROLL,
    ENROLL_ACK,
    PRIVATE_CHAT,
    ADD_FRIEND,
    ADD_GROUP,
    JOIN_GROUP,
    GROUP_CHAT,
    NUM_MSG
};


/*
设置好json有哪些字段
enroll
{
    msgid, id, name, passwd
}

login
{
    msgid, id, passwd
}

privateChat
{
    msgid, id, fromName, msg, to
}

addFriend
{
    msgid, id, friendid
}

addGroup
{
    msgid, id, groupname, groupdesc
}

joinGroup
{
    msgid, id, groupid
}

groupChat
{
    msgid, id, fromName, msg, groupid
}


*/