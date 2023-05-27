#include "src/stable_diffusion/generator.h"

namespace dexpert
{
    SeedGenerator::SeedGenerator() {
        seed_ = rand();
    }

    int SeedGenerator::newSeed() {
        return ++seed_;
    }

    GeneratorBase::GeneratorBase(std::shared_ptr<SeedGenerator> seed_gen, bool variation)
    {
        variation_ = variation;
        seed_gen_ = seed_gen;
        image_seed_ = seed_gen_->newSeed();
    }

    GeneratorBase::~GeneratorBase()
    {
    }

    bool GeneratorBase::isVariation() {
        return variation_;
    }

    void GeneratorBase::setImage(image_ptr_t image)
    {
        image_ = image;
    }

    int GeneratorBase::getSeed()
    {
        return image_seed_;
    }

    std::shared_ptr<SeedGenerator> GeneratorBase::getSeedGenerator() {
        return seed_gen_;
    }

    void GeneratorBase::setSeed(int value) {
        value;
    }

    RawImage *GeneratorBase::getImage()
    {
        return image_.get();
    }

    void GeneratorBase::clearImage() {
        image_.reset();
    }

} // namespace dexpert
