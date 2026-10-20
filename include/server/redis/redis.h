#include <hiredis/hiredis.h>
#include <string>
#include <functional>
using std::string;
using std::function;

class Redis {
public:
    Redis();
    ~Redis();
    bool connect(string ip, int port);
    bool publish(int channel, string msg);
    bool subscribe(int channel);
    bool unsubscribe(int channel);
    void set_notify_handler(function<void(int, string)> fn);
private:
    redisContext* _subscribe_context;
    redisContext* _publish_context;
    void observer_channel_message();
    function<void(int, string)> _notify_message_handler;
};