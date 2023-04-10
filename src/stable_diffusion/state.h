/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_STABLE_DIFFUSION_STATE_H_
#define SRC_STABLE_DIFFUSION_STATE_H_

#include <string>
#include <vector>
#include <list>
#include <memory>

#include "src/python/raw_image.h"
#include "src/stable_diffusion/generator.h"

namespace dexpert
{

class StableDiffusionState;
std::shared_ptr<StableDiffusionState> get_sd_state();

typedef struct {
    std::string name;
    std::string hash;
    std::wstring path;
    size_t model_size;
} model_info_t;

typedef struct {
    int seed_increment = 0;
    std::shared_ptr<GeneratorBase> g;
} generator_t;


typedef std::vector<image_ptr_t> image_list_t;
typedef std::vector<image_list_t> image_grid_t;

class StableDiffusionState {
 private:
    friend std::shared_ptr<StableDiffusionState> get_sd_state();
    StableDiffusionState();

 public:
    virtual ~StableDiffusionState();
    StableDiffusionState (const StableDiffusionState &) = delete;
    StableDiffusionState & operator = (const StableDiffusionState &) = delete;

    // models
    bool reloadSdModelList();
    const std::list<model_info_t> &getSdModels() const;
    void setSdModel(const std::string& name);
    std::wstring getSdModelPath(const std::string& name);

    // generation
    bool generatorAdd(std::shared_ptr<GeneratorBase> generator);
    bool generatePreviousImage(int index);
    bool generateNextImage(int index);
    bool generateNextVariation(int index);
    bool generatePreviousVariation(int index);

    generator_cb_t generatorMakeCallback();
    
    int randomSeed();

    // input images
    // bool openInputImage(const char *path);
    // bool saveInputImage(const char *path);

    const char* lastError();

    // get images
    RawImage *getResultsImage(int index, int variation);
    RawImage *getControlNetImage(int index);

    int getMaxResultImages();
    int getMaxResultVariations();
    void clearAllResultImages();
    void clearResult(int index, int variation);

private:
    void scroll_down_generators();
    void scroll_up_generators();
private:
    std::list<model_info_t> sdModels_;
    std::wstring currentSdModel_;
    std::string last_error_;
    std::vector<generator_t> generators_;
};
    
} // namespace dexpert


#endif // SRC_STABLE_DIFFUSION_STATE_H_