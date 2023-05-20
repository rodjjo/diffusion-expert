/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_PREVIEW_PANEL_H_
#define SRC_PANELS_PREVIEW_PANEL_H_

#include <memory>
#include <FL/Fl_Group.H>

#include "src/controls/image_panel.h"
#include "src/panels/miniature.h"
#include "src/controls/button.h"

namespace dexpert
{
    typedef enum {
        preview_event_next,
        preview_event_previous,
        preview_event_next_var,
        preview_event_previous_var,
        preview_event_view,
        preview_event_use,
        preview_event_remove,
    } preview_event_t;

    class PreviewPanel: public Fl_Group
    {
    public:
        PreviewPanel(int x, int y, int w, int h, bool main_preview);
        virtual ~PreviewPanel();
        void setCol(size_t value);
        size_t getCol();
        void setRow(size_t value);
        size_t getRow();
        void resize(int x, int y, int w, int h) override;
        void updateImage();
    private:
        void alignComponents();
    private:
        size_t row_ = 0;
        size_t col_ = 0;
        bool main_preview_ = false;
        ImagePanel *miniature_;
        std::unique_ptr<Button> btnUse_;
        std::unique_ptr<Button> btnPrevious_;
        std::unique_ptr<Button> btnNext_;
        std::unique_ptr<Button> btnPreviousVar_;
        std::unique_ptr<Button> btnNextVar_;
        std::unique_ptr<Button> btnRemove_;
        std::unique_ptr<Button> btnView_;
    };

} // namespace dexpert

#endif // SRC_PANELS_PREVIEW_PANEL_H_