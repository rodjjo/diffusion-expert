#pragma once

#include <list>
#include <functional>

namespace dfe
{

/*
    This message buss is not thread safe.
    Should only be called from the gui thread
*/

typedef enum {
    event_none = 0,
    event_main_menu_clicked,
    event_main_menu_file_new_art,
    event_main_menu_file_open,
    event_main_menu_file_open_layer,
    event_main_menu_file_close,
    event_main_menu_edit_settings,
    event_main_menu_exit,
    event_layer_count_changed,
} event_id_t;

typedef std::function<void(void *sender, event_id_t event, void *data)> event_handler_t;

class Subscriber {
    event_handler_t handler_;
public:
    Subscriber(const std::list<event_id_t>& events, event_handler_t handler);
    virtual ~Subscriber();
    void operator() (void *sender, event_id_t event, void *data) {
        handler_(sender, event, data);
    }
};

class SubscriberThis: public Subscriber {
    public:
        SubscriberThis(const std::list<event_id_t>& events) : Subscriber(
            events, 
            [this] (void *sender, event_id_t event, void *data) {
                this->dfe_handle_event(sender, event, data);
            }) {

        }
        virtual void dfe_handle_event(void *sender, event_id_t event, void *data) = 0;
    
};

void publish_event(void *sender, event_id_t event, void *data);

} // namespace dfe
