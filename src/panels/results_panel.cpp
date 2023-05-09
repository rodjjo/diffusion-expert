#include "src/panels/results_panel.h"
#include "src/dialogs/common_dialogs.h"
#include "src/stable_diffusion/state.h"
#include "src/windows/image_viewer.h"


namespace dexpert
{

    ResultsPanel::ResultsPanel(int x, int y, int w, int h, PaintingPanel *painting) : EventListener(), Fl_Group(x, y, w, h), painting_(painting)
    {   
        this->begin();
       
        for (int i = 0; i < get_sd_state()->getMaxResultImages(); i++)
        {
            std::vector<PreviewPanel *> row;
            for (int j = 0; j < get_sd_state()->getMaxResultVariations(); j++)
            {
                this->begin();
                row.push_back(new PreviewPanel(0, 0, 1, 1, j == 0));
                PreviewPanel *fp = *row.rbegin();
                fp->setRow(i);
                fp->setCol(j);
            }
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
                v[c]->updateImage();
                xx += minature_w + 5;
            }
            xx = sx;
            yy += minature_h + 5;
        }
    }

    PreviewPanel *ResultsPanel::getFrame(int col, int row)
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

    void ResultsPanel::event_trigged(const void *sender, int event, void *data) {
        PreviewPanel *pn = NULL;
        for (size_t i = 0; i < miniatures_.size(); ++i) {
            for (size_t j = 0; j < miniatures_[i].size(); ++j) {
                if (miniatures_[i][j] == sender) {
                    pn = miniatures_[i][j];
                    break;
                }
            }
        }

        if (!pn) {
            return;
        }

        switch (event) {
            case preview_event_use: {
                    if (ask("Do you want to set this as input image ?")) {
                        painting_->setImage(get_sd_state()->getResultsImage(pn->getRow(), pn->getCol()));
                        painting_->clearPasteImage();
                    }
                }
                break;
            case preview_event_remove:
                {
                    if (ask("Do you want remove this image ?")) {
                        get_sd_state()->clearImage(pn->getRow(), pn->getCol());
                        updatePanels();
                    }
                }
                break;
            case preview_event_view:
                {
                    auto img = get_sd_state()->getResultsImage(pn->getRow(), pn->getCol());
                    if (img) {
                        view_image(img->duplicate());
                    }
                }
                break;
            case preview_event_next:
                get_sd_state()->generateNextImage(pn->getRow());
                break;
            case preview_event_previous:
                get_sd_state()->generatePreviousImage(pn->getRow());
                break;
            case preview_event_next_var:
                get_sd_state()->generateNextVariation(pn->getRow());
                break;
            case preview_event_previous_var:
                get_sd_state()->generatePreviousVariation(pn->getRow());
                break;
            default:
                break;
        }
        updatePanels();
    } 

} // namespace dexpert
