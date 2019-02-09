#pragma once

#include "color.hpp"
#include <stdint.h>

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
            drawLine(colorStart, posStart, posEnd);

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
     * @brief 驱动接口
     * @details 通过BLADE_INTERVAL间隔事件调用一次
     */
    virtual void update() = 0;
};