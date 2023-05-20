#ifndef SRC_WINDOWS_RESTORATION_PARAMS_H
#define SRC_WINDOWS_RESTORATION_PARAMS_H

#include <string>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>

namespace dexpert
{

struct restoration_type {
    std::string restoration_type;  // for now: gfpgan
    float weight;
};

class RestorationParams : public Fl_Window {
    public:
        RestorationParams();
        virtual ~RestorationParams();
            
};
    
} // namespace dexpert


#endif