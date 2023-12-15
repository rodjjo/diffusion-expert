
#include "misc/gen_parameters.h"
#include "misc/config.h"


namespace dfe
{

    py11::dict ControlnetParameters::toDict() {
        py11::dict d;
        py11::dict img;
        image->toPyDict(img);
        d["mode"] = mode;
        d["image"] = img;
        return d;
    }

    py11::dict LoraParameters::toDict() {
        py11::dict d;
        d["model"] = model;
        d["weight"] = weight;
        return d;
    }

    py11::dict GeneratorParameters::toDict() {
        py11::dict d;
        d["positive_prompt"] = positive_prompt;
        d["negative_prompt"] = negative_prompt;
        d["mode"] = mode;
        d["model"] = model;
        d["scheduler_name"] = scheduler_name;
        d["inpaint_model"] = inpaint_model;
        d["cfg"] = cfg;
        d["seed"] = seed;
        d["width"] = width;
        d["height"] = height;
        d["steps"] = steps;
        d["strength"] = strength;
        d["batch_size"] = batch_size;
        d["use_lcm_lora"] = use_lcm_lora;
        d["use_tiny_vae"] = use_tiny_vae;
        d["keep_in_memory"] = get_config()->keep_in_memory();
        d["filter_nsftw"] = get_config()->filter_nsfw();
        d["use_float16"] = get_config()->use_float16();
        if (image.get()) {
            py11::dict img;
            image->toPyDict(img);
            d["image"] = img;
        } else {
            d["image"] = py11::dict();
        }
        if (mask.get()) {
            py11::dict img;
            mask->toPyDict(img);
            d["mask"] = img;
        } else {
            d["mask"] = py11::dict();
        }
        if (!controlnets.empty()) {
            py11::list ctrls(0);
            for (auto & c : controlnets ) {
                ctrls.append(c.toDict());
            }
            d["controlnets"] = ctrls;
        } else {
            d["controlnets"] = py11::list(0);
        }
        if (!lora_list.empty()) {
            py11::list loras(0);
            for (auto &l : lora_list) {
                loras.append(l.toDict());
            }
            d["lora_list"] = loras;
        } else {
            d["lora_list"] = py11::list(0);
        }
        return d;
    }

    
    
    
} // namespace dfe
