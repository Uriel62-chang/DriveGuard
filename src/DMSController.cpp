#include "DMSController.h"

namespace DriveGuard {
    // 构造函数
    DMSController::DMSController() {
        currentState_ = DriverState::NORMAL;
    }

    // 析构函数
    DMSController::~DMSController() {
        // 暂无资源需要释放
    };

    /**
     * @brief 更新驾驶员状态
     * @param eyesOpen 眼睛是否睁开
     */
    void DMSController::update(bool eyesOpen) {
        auto now = std::chrono::steady_clock::now();
        // 睁眼:重置闭眼计时,恢复正常
        if (eyesOpen) {
            closedSince_.reset();
            currentState_ = DriverState::NORMAL;
        }
        // 闭眼:按累计闭眼时长判定状态(时间制,与帧率无关)
        else {
            if (!closedSince_.has_value())
                closedSince_ = now;
            double seconds = std::chrono::duration<double>(now - *closedSince_).count();
            if (seconds >= SLEEPING_SECONDS_) {
                currentState_ = DriverState::SLEEPING;
            } else if (seconds >= FATIGUE_SECONDS_) {
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