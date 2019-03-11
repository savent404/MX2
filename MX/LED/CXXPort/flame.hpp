#pragma once

#include "color.hpp"

#include <assert.h>
#include <stdint.h>
#include <iBlade_ll.hpp>

class Flame_t
{
    RGB vector[256];
    unsigned *index;
    int size;
public:
    Flame_t(int PixelNum, int maxLight = 255, int minLight = 0)
    {
        size = PixelNum;
        index = new unsigned[size];
        for (int i = 0; i < size; i++)
            index[i] = 0;
        // RGB[0]->Yellow
        // RGB[end]->Red
        for (int i = 0; i < 256; i++)
        {
            float hh = (i > 512/3) ?
                       (120.0f + float(i) / 256.0f * (-90.0f)) :
                    //    (float(i) / 256.0f * 1.5f * 60.0f);
                       (float(i) / 256.0f * 60.0f);
            float light = ((maxLight - minLight) * i / 256 + minLight) / 256.0f;
            float r = 1.0f - 0.1f * (rand() % 128) / 128.0f;
            HSV h(hh, r, light);
            vector[i] = h.convert2RGB();
        }
    }
    ~Flame_t()
    {
        if (index)
            delete[] index;
    }

    /**
     * @brief update colorMatrix
     * @param RGB* pColor
     * @param rate: (0~128)
     */
    inline void update(iBladeDriver *drv, int rate = 128)
    {
        int buf;

        // rand a initial var
        if (rand() % 128 > (128 - rate))
            index[0] = rand() % 256;
        
        buf = index[0];
        buf += rand() % 64 - 32;
        if (buf < 0)
            index[0] = 255;
        else if (buf > 255)
            index[0] = 255;
        else
        {
            index[0] = buf;
        }
        
        for (int i = 1; i < size; i++)
        {
            int a = rand() % 3;
            a = (index[i] + index[i - 1] + a) / 2;
            index[i] = a > 255 ? 255 : a;
            //*pColor++ = vector[index[i]];
        }
        for (int i = 0; i < size; i++)
        {
            drv->setColor(i, vector[index[i]]);
        }
        
    }
};
