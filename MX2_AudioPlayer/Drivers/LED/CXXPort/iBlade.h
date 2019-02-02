#pragma once

#include "iBlade_ll.hpp"
#include "color.hpp"

class iBlade : public iBladeDriver {
public:
    iBlade(size_t num);
    bool parameterUpdate(void* arg);
    void hanlde(void* arg);
    
    virtual void update();
};