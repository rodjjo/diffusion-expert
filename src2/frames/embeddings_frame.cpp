#include <algorithm>

#include "components/xpm/xpm.h"
#include "python/routines.h"
#include "frames/embeddings_frame.h"

namespace dfe
{

namespace {
        std::vector<embedding_t> text_inv_cache;
        std::vector<embedding_t> loras_cache;
}

EmbeddingFrame::EmbeddingFrame(bool lora_embedding, Fl_Group *parent) {
    parent_ = parent;
    lora_embedding_ = lora_embedding;
    img_ = new NonEditableImagePanel(0, 0, 1, 1, lora_embedding ? "LoraEmbeddingImage" : "TextualInversionImage");
    search_ = new Fl_Input_Choice(0, 0, 1, 1, lora_embedding ? "Lora:" : "Textual Inversion:");

    btnNext_.reset(new Button(xpm::image(xpm::img_24x24_forward), [this] {
        goNextConcept();
    }));

    btnUse_.reset(new Button(xpm::image(xpm::img_24x24_green_pin), [this] {
        publish_event(this, lora_embedding_ ? event_prompt_lora_selected_ : event_prompt_textual_selected_, NULL);
    }));

    btnSetImg_.reset(new Button(xpm::image(xpm::img_24x24_folder), [this] {
        selectImage();
    }));

    btnPrior_.reset(new Button(xpm::image(xpm::img_24x24_back), [this] {
        goPreviousConcept();
    }));

    btnNext_->tooltip("Next");
    btnPrior_->tooltip("Previous");
    btnUse_->tooltip("Use this concept");
    btnSetImg_->tooltip("Define image");

    alignComponents();
    search_->align(FL_ALIGN_TOP_LEFT);
    search_->callback(&EmbeddingFrame::searchCmbCallback, this);
}

EmbeddingFrame::~EmbeddingFrame() {

}

void EmbeddingFrame::alignComponents() {
    search_->resize(parent_->x() + 5, parent_->y() + 15, parent_->w() - 10, 30);
    img_->resize(parent_->x() + 5, search_->y() + search_->h() + 5, search_->w(), 100);
    
    btnPrior_->position(parent_->x() + 5, img_->y() + img_->h() + 5); 
    btnPrior_->size(img_->w() / 2 - 37, 30);
    
    btnUse_->position(btnPrior_->x() + btnPrior_->w() + 5, btnPrior_->y());
    btnUse_->size(30, 30);

    btnSetImg_->position(btnUse_->x() + btnUse_->w() + 5, btnPrior_->y());
    btnSetImg_->size(30, 30);

    btnNext_->position(btnSetImg_->x() + btnSetImg_->w() + 5, btnPrior_->y());
    btnNext_->size(btnPrior_->w(), btnPrior_->h());
}

void EmbeddingFrame::searchCmbCallback(Fl_Widget* widget, void *cbdata) {
    static_cast<EmbeddingFrame *>(cbdata)->searchCmbCallback(widget);
}

void EmbeddingFrame::searchCmbCallback(Fl_Widget* widget) {
    if (in_search_callback_) {
        return;
    }
    in_search_callback_ = true;
    auto input = search_->input();
    int sel_pos = input->insert_position();
    int sel_mark = input->mark();
    std::string text = input->value();
    if (sel_pos > sel_mark) {
        sel_pos = sel_mark ^ sel_pos;
        sel_mark = sel_mark ^ sel_pos;
        sel_pos = sel_mark ^ sel_pos;
    }
    if (sel_pos > 0) {
        text = text.substr(0, sel_pos);
        if (!text.empty()) {
            text = findModel(text);
            if (!text.empty())  {
                input->value(text.c_str());
                input->insert_position(sel_pos, text.size());
            }
        }
    }
    in_search_callback_ = false;
}

void EmbeddingFrame::selectImage() {

}

int EmbeddingFrame::findIndex() {
    if (embeddings_.empty()) {
        return -1;
    }
    std::string current = search_->value();
    for (size_t i = 0; i < embeddings_.size(); i++) {
        if (embeddings_[i].name == current) {
            return (int)i;
        }
    }
    return -1;
}

void EmbeddingFrame::goNextConcept() {
    if (embeddings_.empty()) {
        return;
    }
    int next_index = findIndex() + 1;
    if (next_index < embeddings_.size()) {
        in_search_callback_ = true;
        search_->value(next_index);
        in_search_callback_ = false;
    }
}

void EmbeddingFrame::goPreviousConcept() {
    if (embeddings_.empty()) {
        return;
    }
    int next_index = findIndex() - 1;
    if (next_index >= 0) {
        in_search_callback_ = true;
        search_->value(next_index);
        in_search_callback_ = false;
    } 
}

std::string EmbeddingFrame::findModel(const std::string& name) {
    std::string result;
    std::string name_lower = name;
    std::string comp;
    auto lower_it = [](unsigned char c){ return std::tolower(c); };
    std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), lower_it);
    for (auto & e : embeddings_ ) {
        comp = e.name;
        std::transform(comp.begin(), comp.end(), comp.begin(), lower_it);
        if (comp.substr(0, name_lower.length()) == name_lower) {
            if (result.empty() || comp.length() < result.length()) {
                result = e.name;
            }
        }
    }
    return result;
}

embedding_t EmbeddingFrame::getSelected() {
    int index = findIndex();
    if (index >= 0) {
        return embeddings_[index];
    }
    embedding_t result;
    return result;
}

void EmbeddingFrame::refresh_models() {
    embeddings_.clear();
    search_->clear();
    if (lora_embedding_) {
        embeddings_ = loras_cache;
    } else {
        embeddings_ = text_inv_cache;
    }
    if (!embeddings_.empty()) {
        for (auto & e: embeddings_) {
            search_->add(e.name.c_str());
        }
        return;
    }
    auto embeddings = py::list_embeddings(lora_embedding_);
    try{
        for (auto & e: embeddings) {
            auto d = py11::cast<py11::dict>(e);
            embedding_t value;
            if (d.contains("name")) {
                value.name = d["name"].cast<std::string>();
            }
            if (d.contains("filename")) {
                value.filename = d["filename"].cast<std::string>();
            }
            if (d.contains("path")) {
                value.path = d["path"].cast<std::string>();
            }
            if (d.contains("kind")) {
                value.kind = d["kind"].cast<std::string>();
            }
            if (value.filename.empty() || value.name.empty() || value.path.empty()) {
                continue;
            }
            embeddings_.push_back(value);
            search_->add(value.name.c_str());
        }
        if (lora_embedding_) {
            loras_cache = embeddings_;
        } else {
            text_inv_cache = embeddings_;
        }
    } catch(std::exception e) {
        printf("Error refreshing embedding model list %s", e.what());
    }
}

} // namespace dfe
