#include <stdlib.h>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include "simple-ui/clock.h"

namespace dfe_ui
{
    namespace clock
    {
        namespace {
            sf::Clock clock;
            int64_t editor_time = 0;

        } // namespace 

        void restart_clock() {
            clock.restart();
        }

        int64_t current_microseconds() {
            return clock.getElapsedTime().asMicroseconds();
        }

        int32_t current_miliseconds() {
            return clock.getElapsedTime().asMicroseconds();
        }

        bool editor_cursor_visible() {
            auto current_time = current_microseconds();
            if (abs((editor_time + 120000) - current_time) < 120000) {
                return true;
            } else if (abs((editor_time + 240000) - current_time) > 240000) {
                editor_time = current_time;
            }
            return false;
        }

    } // namespace clock
} // namespace dfe_ui
