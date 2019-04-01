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
    int mask[2];

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
        mask[0] = 0;
        mask[1] = pixelNum;
    }

    /**
     * @brief 析构
     */
    ~iBladeDriver()
    {
        delete vector;
    }

    inline int& startMask()
    {
        return mask[0];
    }

    inline int& endMask()
    {
        return mask[1];
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
     * @brief 获取整个数组的只读指针
     */
    const RGB* c_ptr() const
    {
        return vector;
    }
private:    
    inline bool isOutofMask(int i)
    {
        return i < imax(0, startMask()) || i >= imin(endMask(), getPixelNum());
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
     * @brief 获取整个数组的指针
     */
    RGB* ptr()
    {
        return vector;
    }

public: // API
    inline void setColor(int pos, const RGB& a)
    {
        if (isOutofMask(pos))
            return;
        vector[pos] = a;
    }
    /**
     * @brief drawLine
     * @param driver
     * @param color
     * @param start [0...pixelNum-1]
     * @param end   [0...pixelNum-1]
     */
    void drawLine(const RGB& color, int start, int end)
    {
		RGB* _ptr = ptr() + start;
        int num = end - start;
		for (int i = start; i < end; i++, _ptr++) {
            if (isOutofMask(i))
                continue;
            *_ptr = color;
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
    void drawShade(const RGB& colorStart,
        const RGB& colorEnd,
        int posStart,
        int posEnd)
    {
        if (colorStart.similar(colorEnd))
        {
            drawLine(colorStart, posStart, posEnd);
            return;
        }

        int sub[4];
		RGB* _ptr = ptr() + posStart;
        int num = posEnd - posStart;

        sub[0] = colorEnd.R - colorStart.R;
        sub[1] = colorEnd.G - colorStart.G;
        sub[2] = colorEnd.B - colorStart.B;
        sub[3] = colorEnd.W - colorStart.W;

        for (int i = 0; i < num; i++, _ptr++) {
            if (isOutofMask(i + posStart))
                continue;
            _ptr->R = colorStart.R + sub[0] * i / num;
            _ptr->G = colorStart.G + sub[1] * i / num;
            _ptr->B = colorStart.B + sub[2] * i / num;
        }
    }

    /**
     * @brief 自然渐变
     * @param colorStart 起始颜色
     * @param colorShiftDegree 颜色相位偏移 (单位:°)
     */
    void drawNaturalShade(const RGB& colorStart, float colorShiftDegree, int startPos, int endPos)
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

private:
    __attribute__((pure)) static inline int getPositivePos(int a, int b)
    {
        if (a < 0)
        {
            int res = a % b;
            return (res + b) % b;
        }
        return a % b;
    }
protected:
    /** Many draw API ************************************/
    /**
     * @brief 彩虹
     * @param shift 红色起始位置的相对位置 0.0f~1.0f
     * @param extraLength 抽象长度 1.0f为默认值
     */
    void drawRainbow(float shift, float extraLength = 1.0f)
    {
        static RGB rgb[3] = { RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255) };
        int interval = int(getPixelNum() * extraLength / 3.0f);
        int startPos = int(getPixelNum() * extraLength * shift) - int(getPixelNum() * extraLength);
        
        int cnt = (getPixelNum() - startPos) / interval + 1;

        for (int i = 0; i < cnt; i++)
        {
            int pos = startPos + i * interval;
            int endPos = pos + interval;
            int color = getPositivePos(i, 3);
            int nextColor = getPositivePos(i + 1, 3);
            drawShade(rgb[color], rgb[nextColor], pos, endPos);
        }
    }

    void drawRandownSpot(const RGB& origin, RGB& sub, float rate)
    {
        int d = int(rate * 128);
        RGB* p = ptr();
        for (int i = 0; i < getPixelNum(); i++, p++)
        {
            if (isOutofMask(i))
                continue;
            int ans = rand() % 128;
            if (ans <= d)
                *p = sub;
            else
                *p = origin;
        }
    }


    void flipColors(float shift)
    {
        RGB* p = ptr();
        RGB t;
        for (int i = 0; i < getPixelNum(); i++, p++)
        {
            if (isOutofMask(i))
                continue;
            HSV hsv(*p);
            hsv.h += shift;
            t = hsv;
            *p = t;
        }
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

    void filterSet(uint8_t light, int startPos, int endPos)
    {
        if (startPos >= endPos)
            return;
        RGB* pC = ptr() + startPos;
        for (int i = startPos; i < endPos; i++, pC++)
        {
            if (isOutofMask(i))
                continue;
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

    // pos should be -1.0f~1.0f
    inline float func_trangle(float pos)
    {
            //  *       *
            //   *     *
            //    *   *
            //     * *
            //      *
            // ***************
            // -1   0   1
        while (pos < -1.0f)
            pos += 2.0f;
        while (pos > 1.0f)
            pos -= 2.0f;
        return pos > 0 ? pos : -pos;
    }
    /**
     * @brief 设定灯的亮度，以三角波流动
     */
    void filterWave(float shift, float extraLen, int _max, int _min)
    {
        RGB *pC = ptr();
        int lightDiff = _max - _min;
        for (int i = 0; i < getPixelNum(); i++, pC++)
        {
            if (isOutofMask(i))
                continue;
            float absPos = (float(i) / getPixelNum() / extraLen - shift) * 2.0f;
            float lightRate = func_trangle(absPos);
            pC->W = lightRate * lightDiff + _min;
        }
    }

    void filterShade(int startPos, int endPos, int startLight, int endLight)
    {
        int dest = endPos - startPos;
        dest = dest == 0 ? 1 : dest;
        float fInc = float(endLight - startLight) / float(dest);
        RGB* pC = ptr() + startPos;
		for (int i = startPos; i < endPos; i++, pC++)
        {
            if (isOutofMask(i))
                continue;
			pC->W = uint8_t(int(startLight + (i - startPos)*fInc) & 0XFF);
        }
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
            if (!isOutofMask(pos))
                pC[pos].W = 0;
        }

        for (int i = 0; i < getPixelNum(); i++)
        {
            if (isOutofMask(i))
                continue;
            ans = pC[i - 1].W + pC[i].W + pC[i + 1].W + 2;
            pC[i] = imin(255, ans / 3);
        }
    }
};
