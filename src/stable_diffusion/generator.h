/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_STABLE_DIFFUSION_GENERATOR_H_
#define SRC_STABLE_DIFFUSION_GENERATOR_H_

#include <functional>
#include <vector>
#include <memory>
#include "src/python/raw_image.h"


namespace dexpert
{


typedef enum {
    inpaint_original,
    inpaint_fill,
    inpaint_wholepicture,
    inpaint_wholefill,
    // inpaint_none,
    // keep inpaint_mode_count at the end
    inpaint_mode_count
} inpaint_mode_t;


typedef std::function<void(bool success, const char* msg, image_ptr_t result)> generator_cb_t;

class GeneratorBase {
  public:
    GeneratorBase();
    virtual ~GeneratorBase();
    virtual void generate(
            generator_cb_t cb,
            int seed_index,
            int enable_variation = 0
    ) = 0;

    virtual std::shared_ptr<GeneratorBase> duplicate() = 0;

    RawImage* getImage();
    RawImage* getVariation(int index);
    void clearImage();
    void clearVariation(int index);
    int getVariationSeed(int index);
    int getVariationCount();
    int computeVariationSeed(bool left);

    static int maxVariations();

  protected:
    void setImage(image_ptr_t image, int seed);
    void addVariation(image_ptr_t image,  int variation, bool left);
    int getSeed();

  private:
    int image_seed_ = 0;
    image_ptr_t image_;
    std::vector<std::pair<image_ptr_t, int> > variations_;
};
    
}  // namespace dexpert


#endif  // SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_