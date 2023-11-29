#pragma once

namespace dfe
{

class Terminal {
 public:
    Terminal();
    virtual void append(const char *data) = 0;
    virtual ~Terminal();
};
    
} // namespace dfe
