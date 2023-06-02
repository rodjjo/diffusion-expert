/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_PANELS_PREVIEW_PANEL_H_
#define SRC_PANELS_PREVIEW_PANEL_H_

#include <memory>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

#include "src/controls/image_panel.h"
#include "src/panels/miniature.h"
#include "src/controls/button.h"
#include "src/panels/painting_panel.h"

namespace dexpert
{
    class PreviewPanel: public Fl_Group
    {
    public:
        PreviewPanel(PaintingPanel *painting);
        virtual ~PreviewPanel();
        void setRow(size_t value);
        size_t getRow();
        void resize(int x, int y, int w, int h) override;
        void updateImage();
        void goLastImage();

    private:
        void alignComponents();
        void enableControls(bool should_redraw = false);

    private:
        size_t row_ = 0;
        PaintingPanel *painting_;
        ImagePanel *miniature_;
        Fl_Box *lblCounter_;

        std::unique_ptr<Button> btnUse_;
        std::unique_ptr<Button> btnRemove_;
        std::unique_ptr<Button> btnView_;
        std::unique_ptr<Button> btnScrollLeft_;
        std::unique_ptr<Button> btnScrollRight_;
    };

} // namespace dexpert

#endif // SRC_PANELS_PREVIEW_PANEL_H_