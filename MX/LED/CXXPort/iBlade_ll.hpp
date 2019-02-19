#pragma once

#include "color.hpp"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.1415926f
#endif

/**
 * @brief iBladeDriver iBlade底层驱动
 * @details 实现颜色存储，基本两种绘画，定义了绘画接口
 */
class iBladeDriver {
private:
    /** @brief Pixel数量 */
    int numPixe;

protected:
    /**
     * @brief 颜色存储结构 
     */
    RGB *vector;

    inline int imax(int a, int b) { return a > b ? a : b;}
    inline int imin(int a, int b) { return a > b ? b : a;}
public:
    /**
     * @brief 根据指定的Pixel数量构造驱动
     */
    iBladeDriver(size_t pixelNum)
    {
        numPixe = pixelNum;
        vector = new RGB[pixelNum];
    }

    /**
     * @brief 析构
     */
    ~iBladeDriver()
    {
        delete vector;
    }

    /**
     * @brief 返回Pixel数量
     * @note  此函数在绘制函数中大量使用
     */
    int getPixelNum() const
    {
        return numPixe;
    }

    /**
     * @brief 通过数组形式访问颜色
     */
    RGB& operator[](int pos)
    {
        if (pos > numPixe) {
            while(1);
        }

        return vector[pos];
    }

    /**
     * @brief 获取整个数组的只读指针
     */
    const RGB* c_ptr() const
    {
        return vector;
    }

    /**
     * @brief 获取整个数组的指针
     */
    RGB* ptr()
    {
        return vector;
    }

public: // API
    /**
     * @brief drawLine
     * @param driver
     * @param color
     * @param start [0...pixelNum-1]
     * @param end   [0...pixelNum-1]
     */
    void drawLine(RGB& color, int start, int end)
    {
        RGB* _ptr = start >= 0 ? ptr() + start : ptr();
        int num = end - start;
        for (int i = 0; i < num; i++) {
            if (i < 0 || i >= getPixelNum())
                continue;
            *_ptr++ = color;
        }
    }
    /**
     * @brief drawShade
     * @note if colorStart is similar to colorEnd, call drawLine
     * @param driver
     * @param colorStart
     * @param colorEnd
     * @param posStart
     * @param posEnd
     */
    void drawShade(RGB& colorStart,
        RGB& colorEnd,
        int posStart,
        int posEnd)
    {
        if (colorStart.similar(colorEnd))
        {
            drawLine(colorStart, posStart, posEnd);
            return;
        }

        int sub[4];
        RGB* _ptr = posStart >= 0 ? ptr() + posStart : ptr();
        int num = posEnd - posStart;

        sub[0] = colorEnd.R - colorStart.R;
        sub[1] = colorEnd.G - colorStart.G;
        sub[2] = colorEnd.B - colorStart.B;
        sub[3] = colorEnd.W - colorStart.W;

        for (int i = 0; i < num; i++) {
            if (i + posStart < 0 || i + posStart >= getPixelNum())
                continue;
            _ptr->R = colorStart.R + sub[0] * i / num;
            _ptr->G = colorStart.G + sub[1] * i / num;
            _ptr->B = colorStart.B + sub[2] * i / num;
            _ptr++;
        }
    }

    /**
     * @brief 自然渐变
     * @param colorStart 起始颜色
     * @param colorShiftDegree 颜色相位偏移 (单位:°)
     */
    void drawNaturalShade(RGB& colorStart, float colorShiftDegree, int startPos, int endPos)
    {
        HSV hsv(colorStart);
        hsv.h += colorShiftDegree;
        RGB out = hsv;
        drawShade(colorStart, out, startPos, endPos);
    }
    /**
     * @brief 驱动接口
     * @details 通过BLADE_INTERVAL间隔事件调用一次
     */
    virtual void update() = 0;

protected:
    /** Many draw API ************************************/
    /**
     * @brief 彩虹
     * @param shift 红色起始位置的相对位置 0.0f~1.0f
     */
    void drawRainbow(float shift)
    {
        static RGB rgb[3] = { RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255) };
        int startPos = getPixelNum() * shift;
        int interval = getPixelNum() / 3;
        if (startPos >= getPixelNum())
            startPos = getPixelNum();
        if (startPos < 0)
            startPos = 0;

        for (int i = 0; i < 3; i++)
        {
            int pos = (startPos + i * interval) % getPixelNum();
            drawShade(rgb[i], rgb[(i + 1) % 3], pos, pos + interval);
        }
        /** make sure -x~-x+interval filled.*/
        int p = 2 - startPos / interval;
        int pp = startPos % interval - interval;
        drawShade(rgb[p], rgb[(p + 1) % 3], pp, pp + interval);
    }

    /**
     * @brief 限制亮度，应当在某个上下范围内
     * @param max 最大亮度 0~255
     * @param min 最低亮度 0~255
     */
    void filterLimit(int _max, int _min)
    {
        RGB* pC = ptr();
        int light;
        for (int i = 0; i < getPixelNum(); i++)
        {
            light = pC->realLight();

            if (light == 0)
                light = 1;
            if (light > _max)
                pC->W = pC->W * _max / light;
            if (light < _min)
                pC->W = imin(255, pC->W * _min / light);
        }
    }

    /**
     * @brief 将灯设定为统一亮度
     * @param light 0~255
     */
    void filterSet(uint8_t light)
    {
        RGB* pC = ptr();
        for (int i = 0; i < getPixelNum(); i++, pC++)
        {
            pC->W = light;
        }
    }

    /**
     * @brief 设定灯的亮度，设定上下阈值的相对偏移量并转化为正弦变换
     * @param shift 相对偏移量，-1.0->min, 1.0->max
     * @param max 最大亮度 0~255
     * @param min 最小亮度 0~255
     */
    void filterSin(float shift, int _max, int _min)
    {
        float rate = sinf(shift * M_PI) * (_max - _min) / 2 + (_max + _min) / 2;
        int light = static_cast<int>(rate * 2.56f);
        light = imin(255, light);
        light = imax(0, light);
        filterSet(light);
    }

    /**
     * @brief 随机雨点效果
     * @param rate 0~1.0f 值约大，变化越剧烈
     */
    void filterRandomRain(float rate)
    {
        bool drop = rand() % 128 <= rate * 128;
        RGB *pC = ptr();
        int ans;

        if (drop)
        {
            int pos = rand() % getPixelNum();
            pC[pos].W = 0;
        }

        for (int i = 0; i < getPixelNum(); i++)
        {
            ans = pC[i - 1].W + pC[i].W + pC[i + 1].W + 2;
            pC[i] = imin(255, ans / 3);
        }
    }
};