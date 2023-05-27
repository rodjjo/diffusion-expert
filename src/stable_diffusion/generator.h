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


class SeedGenerator {
 public:
    SeedGenerator();
    virtual ~SeedGenerator(){}
    int newSeed();
 private:
    int seed_ = 0;
};

class GeneratorBase {
  public:
    GeneratorBase(std::shared_ptr<SeedGenerator> seed_gen, bool variation);
    virtual ~GeneratorBase();
    virtual void generate(generator_cb_t cb) = 0;

    virtual std::shared_ptr<GeneratorBase> duplicate(bool variation) = 0;

    RawImage* getImage();
    void clearImage();
    int getSeed();

    bool isVariation();

    std::shared_ptr<SeedGenerator> getSeedGenerator();

  protected:
    void setImage(image_ptr_t image);
    void setSeed(int value);

  private:
    int image_seed_ = 0;
    bool variation_ = false;
    image_ptr_t image_;
    std::shared_ptr<SeedGenerator> seed_gen_;
};
    
}  // namespace dexpert


#endif  // SRC_STABLE_DIFFUSION_GENERATOR_TXT2IMG_H_