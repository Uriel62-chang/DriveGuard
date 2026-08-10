#ifndef DMS_CONTROLLER_H
#define DMS_CONTROLLER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <optional>
#include <chrono>

namespace DriveGuard {

    enum class DriverState {
        NORMAL, // 正常
        FATIGUE, // 疲劳
        SLEEPING // 睡眠
    };

    class DMSController {
    public:
        // 构造函数
        DMSController();

        // 析构函数
        ~DMSController();

        /**
         * @brief 更新驾驶员状态
         * @param eyesOpen 眼睛是否睁开(睁眼则重置,闭眼则累计闭眼时长)
         * 持续闭眼 >=1.5s -> FATIGUE;>=3.0s -> SLEEPING(时间制,与帧率无关)
         */
        void update(bool eyesOpen);

        /**
         * @brief 获取当前警告信息
         */
        std::string getWarning();

        /**
         * @brief 获取状态颜色 (绿/黄/红)
         */
        cv::Scalar getStatusColor();

        /**
         * @brief 是否疲劳或睡眠
         */
        bool isFatigueOrSleeping();

    private:
        static constexpr double FATIGUE_SECONDS_  = 1.5;   // 闭眼超过 1.5s 判为疲劳
        static constexpr double SLEEPING_SECONDS_ = 3.0;   // 闭眼超过 3.0s 判为睡眠

        std::optional<std::chrono::steady_clock::time_point> closedSince_; // 连续闭眼的起始时刻
        DriverState currentState_;   // 驾驶员当前状态
    };
}

#endif