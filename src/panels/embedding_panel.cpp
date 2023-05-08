#include <Fl/Fl.H>

#include "src/dialogs/common_dialogs.h"
#include "src/data/event_manager.h"
#include "src/data/xpm.h"
#include "src/python/helpers.h"
#include "src/python/wrapper.h"


#include "src/panels/embedding_panel.h"


namespace dexpert
{

EmbeddingPanel::EmbeddingPanel(embedding_type_t embedding_type, int x, int y, int w, int h): Fl_Group(x, y, w, h), embedding_type_(embedding_type) {
    box(FL_DOWN_BOX);
    color(fl_rgb_color(180, 180, 180));
    labelKind_ = new Fl_Box(1, 1, 1, 1, embedding_type_ == embedding_lora ? "Loras:": "Textual inversions:");
    btnLeft_.reset(new Button(xpm::image(xpm::arrow_left_16x16), [this]{
        if (index_ > 0) {
            --index_;
        }
        updateData();
    }));

    btnRight_.reset(new Button(xpm::image(xpm::arrow_right_16x16), [this]{
        ++index_;
        updateData();
    }));

    btnLeft_->tooltip("Previous");
    btnRight_->tooltip("Next");

    update(false);
    alignComponents();
    this->end();
}

EmbeddingPanel::~EmbeddingPanel() {

}

void EmbeddingPanel::resize(int x, int y, int w, int h) {
    Fl_Group::resize(x, y, w, h);
    alignComponents();
}

embedded_t *EmbeddingPanel::getSelectedEmbedding() {
    if (selected_ < embedded_.size()) {
        return &embedded_[selected_];
    }
    return NULL;
}

void EmbeddingPanel::alignComponents() {
    int minature_size = 100;
    int button_w = 30;
    int display_w = w() - button_w * 2 - 10;
    int max_miniatures =  display_w / (minature_size + 5);
    labelKind_->resize(x() + 2, y() + 2, w(), 20);
    btnLeft_->size(30, 30);
    btnRight_->size(30, 30);
    btnLeft_->position(x() + 3, y() + h() / 2 - 15);
    if (max_miniatures > embedded_.size()) {
        max_miniatures = embedded_.size();
        if (max_miniatures < 1) {
            max_miniatures = 1;
        }
    }
    int remains = max_miniatures - (int)images_.size();
    if (remains > 0) {
        this->begin();
        for (int i = 0; i < remains; i++) {
            images_.push_back(new FramePanel(0, 0, 1, 1));
            labels_.push_back(new Fl_Box(0, 0, 1, 1));
            (*images_.rbegin())->ensureButtonEnabled();
            (*images_.rbegin())->addButton(0, 0, 1, xpm::arrow_up_16x16, [this] (FramePanel * pn, int id) {
                clickedSelectEmbedding(pn);
            });
            (*images_.rbegin())->addButton(1, 0, -0.75, xpm::yellow_pin_16x16, [this] (FramePanel * pn, int id) {
                clickedSelectImage(pn);
            });
        }
        this->end();
    } else if (remains < 0) {
        remains = -remains;
        if (remains > images_.size()) {
            remains = images_.size();
        }
        for (int i = 0; i < remains; i++) {
            Fl::delete_widget(*images_.rbegin());
            Fl::delete_widget(*labels_.rbegin());
            images_.erase(images_.begin() + (images_.size() - 1));
            labels_.erase(labels_.begin() + (labels_.size() - 1));
        }
        if (remains > 0) {
            Fl::do_widget_deletion();
        }
    }
    int left = 35;
    for (int i = 0; i < images_.size(); i++) {
        images_[i]->resize(x() + left, labelKind_->y() + labelKind_->h() + 3 , minature_size, minature_size);
        labels_[i]->resize(x() + left, images_[i]->y() + 3 + images_[i]->h() + 2, minature_size, 20);
        left += minature_size + 5;
    }
    btnRight_->position(x() + left, y() + h() / 2 - 15);
    updateData();
}

void EmbeddingPanel::clickedSelectEmbedding(FramePanel *pn) {
    selected_ = pn->getTag();
    trigger_event(this, embedding_event_selected);
}

void EmbeddingPanel::clickedSelectImage(FramePanel *pn) {
    selected_ = pn->getTag();
    trigger_event(this, embedding_define_image);
}

void EmbeddingPanel::updateData() {
    if (index_ + images_.size() > embedded_.size()) {
        if ((int)embedded_.size() - (int)images_.size() >= 0) {
            index_ = embedded_.size() - images_.size();
        } else {
            index_ = 0;
        }
    }

    for (int i = 0; i < images_.size(); ++i) {
        int pos = index_ + i;
        images_[i]->setTag(pos);
        
        if (pos < embedded_.size()) {
            labels_[i]->copy_label(embedded_[pos].name.c_str());
            images_[i]->setImage(embedded_[pos].img);
        } else {
            labels_[i]->copy_label("None");
            images_[i]->setImage(dexpert::image_ptr_t());
        }
    }
}

void EmbeddingPanel::update(bool force) {
    static dexpert::py::embedding_list_t result;

    bool error = false;
    const char *errorMessage = NULL;

    if (result.empty() || force) {
        dexpert::py::embedding_list_t temp;
        dexpert::py::get_py()->execute_callback(
            dexpert::py::list_embeddings([&error, &errorMessage, &temp] (bool success, const char *msg, dexpert::py::embedding_list_t embs) {
                if (!success) {
                    error = true;
                    errorMessage = msg;
                } else {
                    temp = embs;
                }
            })
        );
        result = temp;
    }

    if (errorMessage) {
        show_error(errorMessage);
        return;
    }
    
    embedded_.clear();

    for (auto it = result.begin(); it != result.end(); it++) {
        if ((it->kind == "lora" && embedding_type_ == embedding_lora) || 
            (it->kind == "textual_inv" && embedding_type_ == embedding_textual_inv)) {
                embedded_t e;
                e.filename = it->filename;
                e.name = it->name;
                e.path = it->path;
                e.img = it->img;
                embedded_.push_back(e);
        }
    }

    alignComponents();

    trigger_event(this, embedding_event_updated);
}

void EmbeddingPanel::setSelectedImage(image_ptr_t image) {
    if (selected_ >= embedded_.size() || !image.get()) {
        return;
    }
    for (int i = 0; i < images_.size(); ++i) {
        if (images_[i]->getTag() == selected_) {
            images_[i]->setImage(image);
            if (images_[i]->visible_r()) {
                images_[i]->redraw();
            }
            std::string path = embedded_[i].path;
            path += ".jpg";
            bool success = false;
            const char *msg = NULL;
            dexpert::py::get_py()->execute_callback(
                dexpert::py::save_image(path.c_str(), image.get(), [&success, &msg] (bool b, const char *m) {
                    success = b;      
                    msg = m;
                })
            );
            if (msg) {
                show_error(msg);
            }
            return;
        }
    }
}

} // namespace dexpert
