#include "src/panels/results_panel.h"
#include "src/dialogs/common_dialogs.h"
#include "src/stable_diffusion/state.h"
#include "src/windows/image_viewer.h"

#define BUTTON_ID_PREVIEW 1
#define BUTTON_ID_NEXT_ROW 2
#define BUTTON_ID_PREV_ROW 3
#define BUTTON_ID_NEXT_COL 4
#define BUTTON_ID_PREV_COL 5
#define BUTTON_ID_AS_INPUT 6
#define BUTTON_ID_REMOVE 7

namespace dexpert
{

    ResultsPanel::ResultsPanel(int x, int y, int w, int h, PaintingPanel *painting) : Fl_Group(x, y, w, h), painting_(painting)
    {   
        this->begin();
        
        auto btn_callback = [this] (FramePanel *p, int id) {
            this->take_action(p, id);
        };

        for (int i = 0; i < get_sd_state()->getMaxResultImages(); i++)
        {
            std::vector<FramePanel *> row;
            for (int j = 0; j < get_sd_state()->getMaxResultVariations(); j++)
            {
                this->begin();
                row.push_back(new FramePanel(0, 0, 1, 1));
                FramePanel *fp = *row.rbegin();
                fp->setGridLocation(i, j);
                fp->enableGrid();
                fp->enableCache();
                fp->setImageSource(image_src_results);
                fp->addButton(BUTTON_ID_PREVIEW, 0, 0, dexpert::xpm::lupe_16x16, btn_callback);
                fp->addButton(BUTTON_ID_AS_INPUT, 1, 1, dexpert::xpm::green_pin_16x16, btn_callback);
                fp->addButton(BUTTON_ID_REMOVE, 1, -1, dexpert::xpm::erase_all_16x16, btn_callback);
            }
            row[0]->addButton(BUTTON_ID_PREV_ROW, 0, 1, dexpert::xpm::arrow_up_16x16, btn_callback);
            row[0]->addButton(BUTTON_ID_NEXT_ROW, 0, -1.0, dexpert::xpm::arrow_down_16x16, btn_callback);
            row[0]->addButton(BUTTON_ID_PREV_COL, -1, 0, dexpert::xpm::arrow_left_16x16, btn_callback);
            row[0]->addButton(BUTTON_ID_NEXT_COL, 1, 0, dexpert::xpm::arrow_right_16x16, btn_callback);
            
            miniatures_.push_back(row);
        }

        this->end();

        alignComponents();
    }

    ResultsPanel::~ResultsPanel()
    {
    }

    void ResultsPanel::updatePanels()
    {
        int max_row = -1;
        int max_col = -1;
        for (int r = 0; r < miniatures_.size(); ++r)
        {
            auto &v = miniatures_[r];
            for (int c = 0; c < v.size(); ++c)
            {
                if (get_sd_state()->getResultsImage(r, c))
                {
                    max_row = r;
                    if (max_col < c) {
                        max_col = c;
                    }
                }
            }
        }

        max_col += 1;
        max_row += 1;
        bool should_redraw = this->visible_r();
        for (int r = 0; r < miniatures_.size(); ++r)
        {
            auto &v = miniatures_[r];
            for (int c = 0; c < v.size(); ++c)
            {
                if (c < max_col && r < max_row)
                {
                    v[c]->show();
                    if (should_redraw && v[c]->visible_r()) {
                        v[c]->redraw();
                    }
                }
                else
                {
                    v[c]->hide();
                }
            }
        }

        alignComponents();
    }

    void ResultsPanel::resize(int x, int y, int w, int h)
    {
        Fl_Group::resize(x, y, w, h);
        updatePanels();
    };

    void ResultsPanel::alignComponents()
    {
        int max_row = 0;
        int max_col = 0;

        for (int r = 0; r < miniatures_.size(); ++r)
        {
            auto &v = miniatures_[r];
            for (int c = 0; c < v.size(); ++c)
            {
                if (v[c]->visible())
                {
                    max_row = r;
                    max_col = c;
                }
            }
        }

        max_row += 1;
        max_col += 1;
        int max_w = w() - 10;
        int max_h = h() - 10;
        int minature_w = (max_w - (5 * max_col) - 5) / max_col;
        int minature_h = (max_h - (5 * max_row) - 5) / max_row;
        int sx = x() + 5;
        int xx = sx;
        int yy = y() + 5;

        for (int r = 0; r < max_row; ++r)
        {
            auto &v = miniatures_[r];
            for (int c = 0; c < max_col; ++c)
            {
                v[c]->resize(xx, yy, minature_w, minature_h);
                xx += minature_w + 5;
            }
            xx = sx;
            yy += minature_h + 5;
        }
    }

    FramePanel *ResultsPanel::getFrame(int col, int row)
    {
        if (row >= miniatures_.size())
            return NULL;
        if (col >= miniatures_.begin()->size())
            return NULL;
        auto it = miniatures_.begin();
        std::advance(it, row);
        auto it2 = it->begin();
        std::advance(it2, col);
        return *(it2);
    }

    void ResultsPanel::take_action(FramePanel *w, int id) {
        switch (id) {
            case BUTTON_ID_AS_INPUT: {
                    if (ask("Do you want to set this as input image ?")) {
                        painting_->setImage(get_sd_state()->getResultsImage(w->gridIndex(), w->gridVariation()));
                        painting_->clearPasteImage();
                    }
                }
                break;
            case BUTTON_ID_REMOVE:
                {
                    if (ask("Do you want remove this image ?")) {
                        get_sd_state()->clearImage(w->gridIndex(), w->gridVariation());
                        updatePanels();
                    }
                }
                break;
            case BUTTON_ID_PREVIEW:
                {
                    auto img = get_sd_state()->getResultsImage(w->gridIndex(), w->gridVariation());
                    if (img) {
                        view_image(img->duplicate());
                    }
                }
                break;
            case BUTTON_ID_NEXT_ROW:
                get_sd_state()->generateNextImage(w->gridIndex());
                break;
            case BUTTON_ID_PREV_ROW:
                get_sd_state()->generatePreviousImage(w->gridIndex());
                break;
            case BUTTON_ID_NEXT_COL:
                get_sd_state()->generateNextVariation(w->gridIndex());
                break;
            case BUTTON_ID_PREV_COL:
                get_sd_state()->generatePreviousVariation(w->gridIndex());
                break;
            default:
                break;
        }
        updatePanels();
    }

} // namespace dexpert
