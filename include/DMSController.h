#ifndef DMS_CONTROLLER_H
#define DMS_CONTROLLER_H

#include <opencv2/opencv.hpp>
#include <string>

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
         * @param getsEyes 是否检测到眼睛
         * @param fps 当前帧率（用于计算时间）
         */
        void update(bool getsEyes, double fps);

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
        const int FATIGUE_THRESHOLD_ = 10; // 疲劳阈值
        const int SLEEPING_THRESHOLD_ = 30; // 睡眠阈值

        int NoEyesCount_; // 连续未检测到眼睛的帧数
        DriverState currentState_; //  驾驶员当前状态
    };
}

#endif