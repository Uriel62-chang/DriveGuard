#include "DMSController.h"

namespace DriveGuard {
    // 构造函数
    DMSController::DMSController() {
        NoEyesCount_ = 0;
        currentState_ = DriverState::NORMAL;
    }

    // 析构函数
    DMSController::~DMSController() {
        // 暂无资源需要释放
    };

    /**
     * @brief 更新驾驶员状态
     * @param getsEyes 是否检测到眼睛
     * @param fps 当前帧率（用于计算时间）
     */
    void DMSController::update(bool getsEyes, double fps) {
        // 如果检测到眼睛，重置状态
        if (getsEyes) {
            NoEyesCount_ = 0;
            currentState_ = DriverState::NORMAL;
        }
        // 如果未检测到眼睛，判断当前状态
        else {
            NoEyesCount_++;
            if (NoEyesCount_ >= SLEEPING_THRESHOLD_) {
                currentState_ = DriverState::SLEEPING;
            } else if (NoEyesCount_ >= FATIGUE_THRESHOLD_) {
                currentState_ = DriverState::FATIGUE;
            }
        }
    }

    /**
     * @brief 获取当前警告信息
     */
    std::string DMSController::getWarning() {
        switch (currentState_) {
            case DriverState::NORMAL: return "Driver(State: NORMAL)";
            case DriverState::FATIGUE: return "Driver(State: FATIGUE!)";
            case DriverState::SLEEPING: return "Driver(State: SLEEPING!!!)";
            default: return "Driver(State: UNKNOWN)";
        }
    }

    /**
     * @brief 获取状态颜色 (绿/黄/红)
     */
    cv::Scalar DMSController::getStatusColor() {
        switch (currentState_) {
            case DriverState::NORMAL: return cv::Scalar(0, 255, 0); // 正常：绿色
            case DriverState::FATIGUE: return cv::Scalar(0, 255, 255); // 疲劳：黄色
            case DriverState::SLEEPING: return cv::Scalar(0, 0, 255); // 睡眠：红色
            default: return cv::Scalar(0, 0, 0); // 未知：黑色
        }
    }

    /**
     * @brief 是否疲劳或睡眠
     */
    bool DMSController::isFatigueOrSleeping() {
        return currentState_ == DriverState::FATIGUE || currentState_ == DriverState::SLEEPING;
    }
}