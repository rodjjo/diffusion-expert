#ifndef SRC_EVENT_MANAGER_H
#define SRC_EVENT_MANAGER_H

namespace dexpert
{

class EventListener {
    public:
        EventListener();
        virtual ~EventListener();
    protected:
        friend void trigger_event(const void *, int, void *);
        virtual void event_trigged(const void *sender, int event, void *data) {};
};

void trigger_event(const void *sender, int event = 0, void *data = (void *)0);


} // namespace dexpert


#endif // SRC_EVENT_MANAGER_H