#pragma once

#include "color.hpp"

#include "iBlade_sets.hpp"
#include <assert.h>
#include <iBlade_ll.hpp>
#include <stdint.h>

class FlameBase_t {
protected:
    RGB  vector[ 256 ];
    int* index;
    int  size;
    int  mode;

public:
    FlameBase_t(int pixelNum)
    {
        size  = pixelNum;
        mode  = 0;
        index = new int[ size ];
        for (int i = 0; i < size; i++) {
            index[ i ] = 0;
        }
    }
    ~FlameBase_t()
    {
        delete[] index;
    }

    virtual void initColor(const RGB main, const RGB sub, const iBladeParam* pExtraParam = nullptr) = 0;

    /**
     * @brief update colorMatrix
     * @param RGB* pColor
     */
    virtual void update(iBladeDriver* drv) = 0;

    void _update(iBladeDriver* drv) const
    {
        for (int i = 0; i < size; i++) {
            drv->setColor(i, vector[ index[ i ] ]);
        }
    }
    int getMode() const
    {
        return mode;
    }
};

class FlameMode1_t : public FlameBase_t {
private:
    int rate;
    int minL;
    int maxL;

public:
    FlameMode1_t(int len, int rate = 128, int maxLight = 255, int minLight = 0)
        : FlameBase_t(len)
        , rate(rate)
        , minL(minLight)
        , maxL(maxLight)
    {
        mode = 1;
    }
    ~FlameMode1_t()
    {
        delete[] index;
    }

    /**
     * @brief init/re-init colorbar and other things
     * @note extra param:
     *  - USR.chaosRate
     */
    void initColor(const RGB main, const RGB sub, const iBladeParam* pExtraParam = nullptr) override
    {
        HSV   h[ 2 ] = { HSV(main), HSV(sub) };
        float diffH  = h[ 1 ].h - h[ 0 ].h;
        for (int i = 0; i < 256; i++) {
            float _h = h[ 0 ].h + float(i) / 256.0f * diffH;
            float _l = ((maxL - minL) * i / 256 + minL) / 256.0f;
            float _r = 1.0f - 0.1f * (rand() % 128) / 128.0f;
            HSV   h(_h, _r, _l);
            vector[ i ] = h;
        }

        if (pExtraParam) {
            rate = pExtraParam->chaosRate;
        }
    }

    void update(iBladeDriver* drv) override
    {
        int buf;

        // rand a initial var
        if (rand() % 128 > (128 - rate))
            index[ 0 ] = rand() % 256;

        buf = index[ 0 ];
        buf += rand() % 64 - 32;
        if (buf < 0)
            index[ 0 ] = 255;
        else if (buf > 255)
            index[ 0 ] = 255;
        else {
            index[ 0 ] = buf;
        }

        for (int i = 1; i < size; i++) {
            int a      = rand() % 3;
            a          = (index[ i ] + index[ i - 1 ] + a) / 2;
            index[ i ] = a > 255 ? 255 : a;
        }

        _update(drv);
    }
};

class FlameMode2_t : public FlameBase_t {
private:
    void drawShade(int pS, int pE, const RGB cS, const RGB cE)
    {
        int  len = pE - pS + 1;
        int  sub[ 4 ];
        RGB* ptr = vector + pS;

        sub[ 0 ] = cE.R - cS.R;
        sub[ 1 ] = cE.G - cS.G;
        sub[ 2 ] = cE.B - cS.B;
        sub[ 3 ] = cE.W - cS.W;

        for (int i = 0; i < len; i++, ptr++) {
            float pro = float(i) / len;

            ptr->R = cS.R + sub[ 0 ] * pro;
            ptr->G = cS.G + sub[ 1 ] * pro;
            ptr->B = cS.B + sub[ 2 ] * pro;
            ptr->W = cS.W + sub[ 3 ] * pro;
        }
    }
    class waveGenerate {
    private:
        step_t step;
        int    top;
        int    button;
        int    outOfTime;
        int    coldDownCnt;

        /**
         * @brief generate trangle
         * @note if x < 0.5 : y = 2*x; if x >= 0.5 : y = 1 - 2*x;
         * @param x
         * @return
         */
        static float genTrangle(float x)
        {
            float res;
            while (x > 1.0f) {
                x -= 1.0f;
            }
            while (x < 0.0f) {
                x += 0.0f;
            }
            res = 2 * x;
            if (x < 0.5f) {
                return res;
            } else {
                return 1 - res;
            }
        }

    public:
        waveGenerate()
        {
            outOfTime   = true;
            coldDownCnt = 0;
        }

        void reset(const int stepNum, const int top, const int button)
        {
            step         = step_t(0, stepNum);
            this->top    = top;
            this->button = button;
            outOfTime    = false;
            coldDownCnt  = 0;
        }

        bool try2reset()
        {
            if (coldDownCnt > 0) {
                coldDownCnt--;
                return false;
            }
            return true;
        }

        void gen(int& result)
        {
            if (outOfTime)
                return;
            int res = int(genTrangle(float(step)) * (top - button)) + button;

            res += result;
            if (res > 255) {
                res = 255;
            } else if (res < 0) {
                res = 0;
            }
            result = res;

            if (step.walk()) {
                outOfTime   = true;
                coldDownCnt = 10;
            }
        }

        bool needReset() const
        {
            return outOfTime;
        }
    };

private:
    int          times;
    float        dropVal;
    waveGenerate generater[ 5 ];
    float        speed;
    float        updateCnt;
    int          length[ 2 ];
    int          light[ 2 ];

public:
    FlameMode2_t(int len, float dropPerCall = 9, float avgSpeed = 2.4f, int rangeH = 50, int rangeL = 0, int lightH = 255, int lightL = 0)
        : FlameBase_t(len)
    {
        mode        = 2;
        dropVal     = dropPerCall;
        speed       = avgSpeed;
        length[ 0 ] = rangeH;
        length[ 1 ] = rangeL;
        light[ 0 ]  = lightH;
        light[ 1 ]  = lightL;
        updateCnt   = 0;
        times       = 1;
    }

    ~FlameMode2_t()
    {
        delete[] index;
    }

    void initColor(const RGB main, const RGB sub, const iBladeParam* pExtraParam = nullptr) override
    {
        // Blade--------Main-----------------Sub-------white
        drawShade(0, 31, RGB(0, 0, 0), main);
        drawShade(32, 222, main, sub);
        drawShade(223, 255, sub, RGB(255, 255, 255));
        if (pExtraParam) {
            dropVal = pExtraParam->flameColdDown;
            speed   = pExtraParam->flameSpeed;

            light[ 0 ] = pExtraParam->flameLightH;
            light[ 1 ] = pExtraParam->flameLightL;

            length[ 0 ] = pExtraParam->flameRangeH;
            length[ 1 ] = pExtraParam->flameRangeL;
        }
    }

    void update(iBladeDriver* drv) override
    {

        updateCnt += speed;
        float decVal = updateCnt * dropVal;
        while (updateCnt >= 1.0f) {

            updateCnt -= 1.0f;

            int lenRange   = length[ 0 ] - length[ 1 ];
            int lightRange = light[ 0 ] - light[ 1 ];

            int pInt[ 1 ];

            // generate all new values
            for (int i = times - 1; i >= 0; i--) {
                pInt[ i ] = 0;
                for (waveGenerate& it : generater) {
                    if (it.needReset() && it.try2reset() && (std::rand() % 100 < 10)) {
                        int stepNum = std::rand() % lenRange + length[ 1 ];
                        int hight[ 2 ];
                        for (int& _it : hight) {
                            _it = std::rand() % lightRange + light[ 1 ];
                        }

                        // if hight[1] is bigger, exchange it
                        if (hight[ 1 ] > hight[ 0 ]) {
                            hight[ 1 ] = std::exchange(hight[ 0 ], hight[ 1 ]);
                        }

                        it.reset(stepNum, hight[ 0 ], hight[ 1 ]);
                    }

                    if (!it.needReset())
                        it.gen(pInt[ i ]);
                }
            }

            // shift values into index
            for (int i = size - 1; i >= times; i--) {
                index[ i ] = index[ i - 1 ];
            }
            for (int i = 0; i < times; i++) {
                index[ i ] = pInt[ i ];
            }

        } // while (updateCnt >= 1.0f)

        // dec pre color
        for (int i = 0; i < size; i++) {
            index[ i ] -= dropVal;
            if (index[ i ] < light[ 1 ]) {
                index[ i ] = light[ 1 ];
            } else if (index[ i ] > light[ 0 ]) {
                index[ i ] = light[ 0 ];
            }
        }

        // update color according to the index
        _update(drv);
    }
};

class FlameGen_t {
public:
    /*
    enum FlameMode {
        Null  = 0,
        Mode1 = 1,
        Mode2 = 2,
    };
    */
    static FlameBase_t* generateFlame(int len, int mode, iBladeParam* pParam)
    {
        switch (mode) {
        case 0:
            return nullptr;
        case 1:
            return new FlameMode1_t(len, pParam->chaosRate);
        case 2:
            return new FlameMode2_t(len, pParam->flameColdDown, pParam->flameSpeed, pParam->flameRangeH, pParam->flameRangeL, pParam->flameLightH, pParam->flameLightL);
        default:
            return nullptr;
        }
    }
};
