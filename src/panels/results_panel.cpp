#include "src/panels/results_panel.h"
#include "src/dialogs/common_dialogs.h"
#include "src/stable_diffusion/state.h"

#define BUTTON_ID_PREVIEW 1
#define BUTTON_ID_NEXT_ROW 2
#define BUTTON_ID_PREV_ROW 3
#define BUTTON_ID_NEXT_COL 4
#define BUTTON_ID_PREV_COL 5

namespace dexpert
{

    ResultsPanel::ResultsPanel(int x, int y, int w, int h) : Fl_Group(x, y, w, h)
    {
        this->begin();
        
        auto btn_callback = [this] (FramePanel *w, int id) {
            this->take_action(w, id);
        };

        for (int i = 0; i < get_sd_state()->getMaxResultImages(); i++)
        {
            std::vector<FramePanel *> row;
            for (int j = 0; j < get_sd_state()->getMaxResultVariations(); j++)
            {
                row.push_back(new FramePanel(0, 0, 1, 1));
                FramePanel *fp = *row.rbegin();
                fp->setGridLocation(i, j);
                fp->enableGrid();
                fp->enableCache();
                fp->setImageSource(image_src_results);
                fp->addButton(BUTTON_ID_PREVIEW, 0, 0, dexpert::xpm::lupe_16x16, btn_callback);
            }
            row[0]->addButton(BUTTON_ID_PREV_ROW, 0, 1, dexpert::xpm::move_16x16, btn_callback);
            row[0]->addButton(BUTTON_ID_NEXT_ROW, 0, -1.0, dexpert::xpm::boss_16x16, btn_callback);
            row[0]->addButton(BUTTON_ID_PREV_COL, -1, 0, dexpert::xpm::eject_16x16, btn_callback);
            row[0]->addButton(BUTTON_ID_NEXT_COL, 1, 0, dexpert::xpm::eye_16x16, btn_callback);
            miniatures_.push_back(row);
        }

        this->end();

        updatePanels();
    }

    ResultsPanel::~ResultsPanel()
    {
    }

    void ResultsPanel::updatePanels()
    {
        int max_row = 0;
        int max_col = 0;
        for (int r = 0; r < miniatures_.size(); ++r)
        {
            auto &v = miniatures_[r];
            for (int c = 0; c < v.size(); ++c)
            {
                if (get_sd_state()->getResultsImage(r, c))
                {
                    max_row = r;
                    max_col = c;
                }
            }
        }

        max_col += 1;
        max_row += 1;

        for (int r = 0; r < miniatures_.size(); ++r)
        {
            auto &v = miniatures_[r];
            for (int c = 0; c < v.size(); ++c)
            {
                if (c < max_col && r < max_row)
                {
                    v[c]->show();
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
                if (v[c]->visible_r())
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
                if (v[c]->visible_r()) 
                    v[c]->redraw();
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
            case BUTTON_ID_PREVIEW:
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
