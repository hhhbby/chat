#include "redis.h"
#include <thread>
#include <iostream>

Redis::Redis():_subscribe_context(nullptr), _publish_context(nullptr)
{ }

Redis::~Redis()
{
    if (_subscribe_context)
        redisFree(_subscribe_context);
    if (_publish_context)
        redisFree(_publish_context);
}

bool Redis::connect(string ip, int port)
{
    _subscribe_context = redisConnect(ip.c_str(), port);
    _publish_context = redisConnect(ip.c_str(), port);
    if (!_subscribe_context || !_publish_context)
        return false;
    std::thread([this]()
    {
        observer_channel_message();
    }).detach();
    return true;
}

bool Redis::publish(int channel, string msg)
{
    redisReply* reply =  static_cast<redisReply*>(redisCommand(_publish_context, "PUBLISH %d %s", channel, msg.c_str()));
    if (!reply)
        return false;
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(_subscribe_context, "SUBSCRIBE %d", channel))
        return false;
    
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(_subscribe_context, &done))
            return false;
    }
    return true;
}

bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(_subscribe_context, "UNSUBSCRIBE %d", channel))
        return false;
    
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(_subscribe_context, &done))
            return false;
    }
    return true;
}

void Redis::set_notify_handler(function<void(int, string)> fn)
{
    _notify_message_handler = fn;
}

void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    while (REDIS_ERR != redisGetReply(_subscribe_context, (void**)&reply))
    {// https://redis.io/docs/latest/develop/clients/hiredis/handle-replies/
/*
消息格式
1) "message"
2) "12"
3) "hello"
*/
        if (reply != nullptr &&
            reply->type == REDIS_REPLY_ARRAY &&
            reply->elements == 3 &&
            reply->element[0]->type == REDIS_REPLY_STRING &&
            reply->element[0]->str != nullptr &&
            string(reply->element[0]->str, reply->element[0]->len) == "message" && // 这里表示是一个redis subscribe message
            reply->element[1]->type == REDIS_REPLY_STRING &&
            reply->element[1]->str != nullptr &&
            reply->element[2]->type == REDIS_REPLY_STRING &&
            reply->element[2]->str != nullptr)
        {
            int channel = std::atoi(reply->element[1]->str);

            // 按长度构造，避免消息包含 '\0' 时被截断
            string message(
                reply->element[2]->str,
                reply->element[2]->len);

            if (_notify_message_handler)
            {
                _notify_message_handler(channel, message);
            }
        }
    }
    std::cout << "observer_channel_message finished --------\n"; 
}
