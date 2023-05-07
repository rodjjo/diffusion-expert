#ifndef SRC_PANELS_EMBEDDING_PANEL_H
#define SRC_PANELS_EMBEDDING_PANEL_H

#include <memory>
#include <vector>
#include <string>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

#include "src/panels/frame_panel.h"
#include "src/controls/button.h"
#include "src/python/raw_image.h"

namespace dexpert
{

typedef enum {
  embedding_event_selected,
  embedding_event_updated,
  embedding_define_image
} embedding_event_t;

typedef enum {
  embedding_textual_inv,
  embedding_lora
} embedding_type_t;

struct embedded_t {
  std::string name;
  std::string filename;
  std::string path;
};

class EmbeddingPanel: public Fl_Group {
  public:
    EmbeddingPanel(embedding_type_t embedding_type, int x, int y, int w, int h);
    virtual ~EmbeddingPanel();
    void update(bool force);
    embedded_t *getSelectedEmbedding();
    void resize(int x, int y, int w, int h) override;
    void setSelectedImage(image_ptr_t image);
    
  private:
    void alignComponents();
    void updateData();
    void clickedSelectEmbedding(FramePanel *pn);
    void clickedSelectImage(FramePanel *pn);

  private:
    embedding_type_t embedding_type_ = embedding_textual_inv;
    size_t index_ = 0;
    size_t selected_ = 0;
    std::vector<embedded_t> embedded_;

  private:
    Fl_Box *labelKind_;
    std::vector<Fl_Box *> labels_;
    std::vector<FramePanel *> images_;
    std::unique_ptr<Button> btnLeft_;
    std::unique_ptr<Button> btnRight_;
};

} // namespace dexpert


#endif  // SRC_PANELS_EMBEDDING_PANEL_H