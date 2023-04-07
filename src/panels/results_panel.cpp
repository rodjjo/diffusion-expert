#include "src/panels/results_panel.h"
#include "src/stable_diffusion/state.h"

namespace dexpert
{

    ResultsPanel::ResultsPanel(int x, int y, int w, int h) : Fl_Group(x, y, w, h)
    {
        this->begin();
        for (int i = 0; i < get_sd_state()->getMaxResultImages(); i++)
        {
            std::vector<FramePanel *> row;
            for (int j = 0; j < get_sd_state()->getMaxResultVariations(); j++)
            {
                row.push_back(new FramePanel(0, 0, 1, 1));
                (*row.rbegin())->setGridLocation(i, j);
                (*row.rbegin())->enableGrid();
                (*row.rbegin())->enableCache();
                (*row.rbegin())->setImageSource(image_src_results);
            }
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
                if (v[c]->shown())
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

} // namespace dexpert
