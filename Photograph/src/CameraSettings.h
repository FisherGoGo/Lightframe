#pragma once

#include <cmath>

// App 保存唯一可编辑设置；传感器固定为全画幅 36 x 24 mm。
struct CameraSettings
{
    static constexpr float sensorWidthMm = 36.0f;
    static constexpr float sensorHeightMm = 24.0f;
    static constexpr float minFocalLengthMm = 24.0f;
    static constexpr float maxFocalLengthMm = 85.0f;
    static constexpr float defaultFocalLengthMm = 50.0f;

    float focalLengthMm = defaultFocalLengthMm;

    // 数值编辑、预设和滚轮共用校验；无效输入不改变当前值。
    bool setFocalLengthMm(float value)
    {
        if (!std::isfinite(value) || value < minFocalLengthMm || value > maxFocalLengthMm)
        {
            return false;
        }
        focalLengthMm = value;
        return true;
    }

    float verticalFovRadians() const
    {
        return 2.0f * std::atan(sensorHeightMm / (2.0f * focalLengthMm));
    }

    float aspectRatio() const
    {
        return sensorWidthMm / sensorHeightMm;
    }
};
