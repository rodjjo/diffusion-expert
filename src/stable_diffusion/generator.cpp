#include "src/stable_diffusion/generator.h"

#define MAX_VARIATIONS 4

namespace dexpert
{

    GeneratorBase::GeneratorBase()
    {
        for (int i = 0; i < MAX_VARIATIONS; ++i)
        {
            variations_.push_back(std::make_pair(image_ptr_t(), 0));
        }
    }

    GeneratorBase::~GeneratorBase()
    {
    }

    int GeneratorBase::maxVariations()
    {
        return MAX_VARIATIONS;
    }

    void GeneratorBase::setImage(image_ptr_t image, int seed)
    {
        image_ = image;
        image_seed_ = seed;
    }

    void GeneratorBase::addVariation(image_ptr_t image, int variation, bool left)
    {
        if (left)
        {
            if (variations_.begin()->first)
            {
                for (auto it = variations_.rbegin(); it != variations_.rend(); ++it)
                {
                    auto it2 = it;
                    std::advance(it2, 1);
                    if (it2 != variations_.rend())
                    {
                        it->first = it2->first;
                        it->second = it2->second;
                    }
                }
            }
            variations_.begin()->first = image;
            variations_.begin()->second = variation;
        }
        else
        {
            for (auto it = variations_.begin(); it != variations_.end(); ++it)
            {
                if (!it->first)
                {
                    it->first = image;
                    it->second = variation;
                    return;
                }
            }
            for (auto it = variations_.begin(); it != variations_.end(); ++it)
            {
                auto it2 = it;
                std::advance(it2, 1);
                if (it2 != variations_.end())
                {
                    it->first = it2->first;
                    it->second = it2->second;
                }
            }
            variations_.rbegin()->first = image;
            variations_.rbegin()->second = variation;
        }
    }

    int GeneratorBase::getSeed()
    {
        return image_seed_;
    }

    RawImage *GeneratorBase::getImage()
    {
        return image_.get();
    }

    void GeneratorBase::clearImage() {
        image_.reset();
    }

    void GeneratorBase::clearVariation(int index) {
        if (index < 0 || index >= variations_.size())
            return;
        return variations_[index].first.reset();
    }

    int GeneratorBase::getVariationCount()
    {
        return variations_.size();
    }

    RawImage *GeneratorBase::getVariation(int index)
    {
        if (index < 0 || index >= variations_.size())
            return NULL;
        return variations_[index].first.get();
    }

    int GeneratorBase::getVariationSeed(int index)
    {
        if (index < 0 || index >= variations_.size())
            return 0;
        return variations_[index].second;
    }

    int GeneratorBase::computeVariationSeed(bool left)
    {
        if (left)
        {
            for (auto i = variations_.rbegin(); i != variations_.rend(); ++i)
            {
                if (i->first)
                {
                    return i->second - 1;
                }
            }
        }
        for (auto i = variations_.begin(); i != variations_.end(); ++i)
        {
            if (i->first)
            {
                return i->second + 1;
            }
        }
        return image_seed_ + (left ? -1 : 1);
    }

} // namespace dexpert
