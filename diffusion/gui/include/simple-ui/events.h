#pragma once

#include <functional>


namespace dfe_ui
{
    class Component;

    typedef std::function<void(Component* self)> component_event_t;
    typedef std::function<void(Component* self, Component* other)> component_event2_t;
    typedef std::function<bool(Component* self)> component_ask_event_t;
    typedef std::function<bool(Component* self, Component* other)> component_ask_event2_t;
    
} // namespace dfe_ui
