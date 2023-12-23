#pragma once

#include <string>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Input_Choice.H>

#include "messagebus/messagebus.h"
#include "components/button.h"
#include "components/image_panel.h"

namespace dfe
{

typedef struct {
  std::string name;
  std::string kind;
  std::string filename;
  std::string path;
} embedding_t;

class EmbeddingFrame {
  public:
    EmbeddingFrame(bool lora_embedding, Fl_Group *parent);
    virtual ~EmbeddingFrame();
    void refresh_models();
    void alignComponents();
    embedding_t getSelected();

  private:
    void goNextConcept();
    void goPreviousConcept();
    void selectImage();
    static void searchCmbCallback(Fl_Widget* widget, void *cbdata);
    void searchCmbCallback(Fl_Widget* widget);
    std::string findModel(const std::string& name);
    int findIndex();
  private:
    bool in_search_callback_ = false;
    std::vector<embedding_t> embeddings_;
    bool lora_embedding_ = false;
    Fl_Input_Choice *search_;
    Fl_Group *parent_;
    ImagePanel *img_;
    std::unique_ptr<Button> btnNext_;
    std::unique_ptr<Button> btnUse_;
    std::unique_ptr<Button> btnSetImg_;
    std::unique_ptr<Button> btnPrior_;

};

} // namespace dfe
