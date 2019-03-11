#pragma once

#include <cmath>
#include <stdint.h>

using namespace std;

/**
 * @brief RGB 颜色空间
 */
class RGB {
public:
    /** @brief R */
    uint8_t R;
    /** @brief G */
    uint8_t G;
    /** @brief B */
    uint8_t B;
    /** 
     * @brief only for iBlade adjust
     * @details d表示透明度 0为全透明 255为不透明
     * @note 用户不可见,只为提高iBlade性能
     */
    uint8_t W; // user cant see it!

    /**
     * @brief 构造
     * @param w 默认值255
     */
    RGB(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t w = 255)
        : R(r)
        , G(g)
        , B(b)
        , W(w)
    {
    }

    /**
     * @brief 拷贝
     */
    RGB(const RGB& r)
    {
        this->R = r.R;
        this->G = r.G;
        this->B = r.B;
        this->W = r.W;
    }

    /**
     * @brief 拷贝(包括加法运算)
     * @warning 因性能考虑已弃用
     */
    RGB(const RGB& r,
        int offset_r,
        int offset_g,
        int offset_b,
        int offset_w)
    {
        R = r.R + offset_r;
        G = r.G + offset_g;
        B = r.B + offset_b;
        W = r.W + offset_w;
    }

    /**
     * @brief 拷贝
     */
    RGB& operator=(const RGB& other)

    {
        if (&other == this)
            return *this;
        R = other.R;
        G = other.G;
        B = other.B;
//        W = other.W;
        return *this;
    }

    /**
     * @brief RGB减法
     * @note 此方法不会产生多余的拷贝和构造
     * @warning 可能会产生溢出
     */
    RGB& operator-=(const RGB& other)
    {
        R -= other.R;
        G -= other.G;
        B -= other.B;
        return *this;
    }

    /**
     * @brief RGB加法
     * @note 此方法不会产生多余的拷贝构造
     * @warning 可能会产生溢出
     */
    RGB& operator+=(const RGB& other)
    {
        R += other.R;
        G += other.G;
        B += other.B;
        return *this;
    }

    /**
     * @brief RGB乘法
     * @note 此方法不会产生多余的拷贝构造
     * @warning 可能会产生溢出
     */
    RGB& operator*=(const float q)
    {
        R *= q;
        G *= q;
        B *= q;
        return *this;
    }

    /**
     * @brief 判断两个class RGB是否全等
     */
    bool operator==(const RGB& other) const
    {
        if (&other == this)
            return true;
        if (R != other.R)
            return false;
        if (G != other.G)
            return false;
        if (B != other.B)
            return false;
        if (W != other.W)
            return false;
        return true;
    }

    /**
     * @brief 判断两个class RGB是否s相似
     * @note  结合了w的透明色
     */
    bool similar(const RGB& other, uint8_t offset = 10) const
    {
        int sub;
        if (&other == this)
            return true;
        sub = wR() - other.wR();
        if (abs(sub) >= offset)
            return false;
        sub = wG() - other.wG();
        if (abs(sub) >= offset)
            return false;
        sub = wB() - other.wB();
//        if (abs(sub) >= offset)
//            return false;
//        return true;
        return abs(sub) < offset;
    }

    /**
     * @brief 获取RGB平均亮度
     * @note  不符合人眼色感
     */
    uint8_t light() const
    {
        int sum = R + G + B;
        return sum / 3;
    }

    /**
     * @brief 获取RGB平均亮度
     * @note  符合人眼色感
     */
    uint8_t realLight() const
    {
        return (uint8_t((R * 299) + (G * 587) + (B * 114)) / 1000);
    }

    /**
     * @brief 获取带w参数的R分量
     */
    uint8_t wR() const
    {
        if (W != 255) {
            return (R * W) >> 8;
        } else {
            return R;
        }
    }

    /**
     * @brief 获取带w参数的G分量
     */
    uint8_t wG() const
    {
        if (W != 255) {
            return (G * W) >> 8;
        } else {
            return G;
        }
    }

    /**
     * @brief 获取带w参数的B分量
     */
    uint8_t wB() const
    {
        if (W != 255) {
            return (B * W) >> 8;
        } else {
            return B;
        }
    }

    /**
     * @param offset 0.0f~1.0f
     */
    static RGB midColor(const RGB& source, const RGB& dest, float offset) 
    {
        uint8_t sub[4];
        sub[0] = dest.R - source.R;
        sub[1] = dest.G - source.G;
        sub[2] = dest.B - source.B;
        sub[3] = dest.W - source.W;

        for (int i = 0; i < 4; i++)
            sub[i] *= offset;
        return RGB(source.R + sub[0],
                   source.G + sub[1],
                   source.B + sub[2],
                   source.W + sub[3]);
    }
};
/**
 * @brief HSV颜色空间
 * @details 使用HSV颜色空间将某一个RGB颜色
 *          调整其色相,所以需要实现RBG-HSV相互转换
 */
typedef struct HSV {
    /** @brief Hue 色相 单位:°*/
    float h;
    /** @brief saturation 饱和度 (0...1.0) */
    float s;
    /** @brief value 亮度 (0..1.0) */
    float v;

    /**
     * @brief 构造:HSV值
     */
    HSV(float _h, float _s, float _v)
    {
        h = _h;
        s = _s;
        v = _v;
    }
    /**
     * @brief 构造:class RGB
     * @param rgb class RGB
     * @note  It's just call HSV::operator=
     */
    HSV(const RGB& rgb)
    {
        *this = rgb;
    }
    /**
     * @brief convert HSV to RGB
     * @return class RGB
     */
    RGB convert2RGB() const
    {
        float _h = h;
        // In-case h is negative
        while (_h < 0)
            _h += 360.0f;
        float C = v * s;
        float P = fmodf(_h / 60.0f, 6.0f);
        float X = C * (1 - fabs(fmodf(P, 2) - 1));
        float m = v - C;
        C += m;
        X += m;

        if (_h < 60) {
            return RGB(C * 255, X * 255, m * 255);
        } else if (_h < 120) {
            return RGB(X * 255, C * 255, m * 255);
        } else if (_h < 180) {
            return RGB(m * 255, C * 255, X * 255);
        } else if (_h < 240) {
            return RGB(m * 255, X * 255, C * 255);
        } else if (_h < 300) {
            return RGB(X * 255, m * 255, C * 255);
        } else if (_h < 360) {
            return RGB(C * 255, m * 255, X * 255);
        } else {
            return RGB(0, 0, 0);
        }
    }

    /**
     * @brief copy from another HSV
     */
    HSV& operator=(const HSV& other)
    {
        if (&other == this) {
            return *this;
        }
        h = other.h;
        s = other.s;
        v = other.v;
    }

    /**
     * @brief convert a RGB to HSV
     */
    HSV& operator=(const RGB& rgb)
    {
        float r = rgb.R / 255.0f;
        float g = rgb.G / 255.0f;
        float b = rgb.B / 255.0f;

        int pmax = 2;
        float max = b, min = b;
        if (max < r) {
            pmax = 0;
            max = r;
        }
        if (min > r) {
            min = r;
        }
        if (max < g) {
            max = g;
            pmax = 1;
        }
        if (min > g) {
            min = g;
        }
        float delta = max - min;

        if (delta <= float(1e-4) && delta >= float(-1e-4)) {
            h = 0;
        } else if (pmax == 0) {
            h = (int((g - b) / delta) % 6) * 60.0f;
        } else if (pmax == 1) {
            h = (((b - r) / delta) + 2) * 60.0f;
        } else if (pmax == 2) {
            h = (((r - g) / delta) + 4) * 60.0f;
        }

        if (max <= float(1e-4) && max >= float(-1e-4)) {
            s = 0;
        } else {
            s = delta / max;
        }

        v = max;
        return *this;
    }

    /**
     * @brief HSV can auto-convert to RGB
     */
    operator class RGB() const {
        return this->convert2RGB();
    }
} HSV;
