#pragma once

enum class MsgType{
    LOGIN,
    LOGIN_ACK,
    ENROLL,
    ENROLL_ACK,
    PRIVATE_CHAT,
    ADD_FRIEND,
    ADD_GROUP,
    JOIN_GROUP,
    GROUP_CHAT,
    NUM_MSG
};