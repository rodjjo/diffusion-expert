#include <set>
#include "src/data/event_manager.h"

namespace dexpert
{

namespace 
{
    std::set<EventListener *> event_listeners;
} // unnamed namespace 


void add_event_listener(EventListener *listener) {
    event_listeners.insert(listener);
}

void remove_event_listener(EventListener *listener) {
    auto it = event_listeners.find(listener);
    if (it != event_listeners.end()) {
        event_listeners.erase(it);
    }
}

EventListener::EventListener() {
    add_event_listener(this);
}

EventListener::~EventListener() {
    remove_event_listener(this);
}

void trigger_event(const void *sender, int event, void *data) {
    for (auto it = event_listeners.begin(); it != event_listeners.end(); it++) {
        (*it)->event_trigged(sender, event, data);
    }
}

    
} // namespace dexpert


