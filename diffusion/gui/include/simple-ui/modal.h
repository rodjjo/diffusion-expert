#pragma once

#include "simple-ui/component.h"

namespace dfe_ui
{

class Modal : public Component {
    public:
        Modal(Window *window);
        virtual ~Modal();
        virtual void float_on(Component *parent = NULL) override;
        void fill_color(uint32_t value);
        uint32_t fill_color();

    protected:
        bool clickable() override;
        void paint(sf::RenderTarget *render_target) override;

    protected:
        void handle_parent_resized() override;

    private:
        uint32_t m_fill_color;
};


} // namespace dfe_ui

