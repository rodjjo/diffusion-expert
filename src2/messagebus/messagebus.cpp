#include <map>
#include "messagebus/messagebus.h"

namespace dfe
{

namespace 
{
    size_t current_subscriber_id = 0;
    std::map<event_id_t, std::list<Subscriber *> > subscribers;
} // namespace 


void publish_event(void *sender, event_id_t event, void *data) {
    auto lst = subscribers.find(event);
    if (lst == subscribers.end()) {
        return;
    }
    for (auto * sub: lst->second) {
        (*sub)(sender, event, data);
    }
}

Subscriber::Subscriber(const std::list<event_id_t>& events, event_handler_t handler)  {
    handler_ = handler;
    for (auto event : events) {
        auto lst = subscribers.find(event);
        if (lst == subscribers.end()) {
            subscribers[event] = std::list<Subscriber *>({ this });
        } else {
            lst->second.push_back(this);
        }
    }
};

Subscriber::~Subscriber() {
    for (auto & item : subscribers) {
        item.second.remove(this);
    }
}


    
} // namespace dfe
