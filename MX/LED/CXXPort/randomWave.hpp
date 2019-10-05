#pragma once

#include "color.hpp"

#include "MX_def.h"
#include "iBlade_ll.hpp"
#include "iBlade_ulti.hpp"
#include <algorithm>
#include <assert.h>
#include <stdint.h>

struct RandomWaveItem {
    bool   running;
    float  len;
    step_t step;

    RandomWaveItem()
    {
        running = false;
    }

    bool tryEnable(int range[ 2 ])
    {
        int rate = (range[ 0 ] + range[ 1 ]) / 2 * 1024;
        int x    = int(rate / 16.0f);
        if ((rand() % rate) < x) {
            running = true;
            int n   = rand() % abs(range[ 1 ] - range[ 0 ]);
            step    = step_t(0, n + min(range[ 1 ], range[ 0 ]), 0);
            return true;
        }
        return false;
    }

    bool isEnable()
    {
        return running;
    }

    void walk()
    {
        if (step.walk())
            running = false;
    }
};

class RandomWave_t {
private:
    struct RandomWaveItem* items;
    step_t                 totalStep;
    float                  length;
    int                    count;
    int                    maxLight;
    int                    minLight;

public:
    RandomWave_t(int cnt, const step_t& step, float length, int lightMax, int lightMin)
    {
        totalStep = step;
        count     = cnt;

        items = new RandomWaveItem[ cnt ];

        this->length = length;
        for (int i = 0; i < cnt; i++) {
            int n          = rand() % 128;
            items[ i ].len = n * this->length / 128.0f;
        }
        maxLight = lightMax;
        minLight = lightMin;
    }
    ~RandomWave_t()
    {
        if (items != nullptr) {
            delete[] items;
        }
    }

    bool update(iBladeDriver* drv, int maxSpeed)
    {
        for (int i = 0; i < count; i++) {
            if (!items[ i ].isEnable()) {
                int range[ 2 ];
                // maxSpeed
                range[ 0 ] = MX_LED_MS2CNT((drv->getPixelNum() * 1000) / maxSpeed);
                range[ 1 ] = range[ 0 ] * 3;
                items[ i ].tryEnable(range);
                continue;
            } else {
                drv->filterRandomWave(float(items[ i ].step), items[ i ].len, maxLight, minLight);
                items[ i ].walk();
            }
        }
        return !totalStep.walk();
    }
};