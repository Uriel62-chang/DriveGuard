#include "FaceDetector.h"
#include <iostream>
#include <stdexcept>

namespace DriveGuard {
    /**
     * @brief 构造函数
     * @param modelPath 人脸级联器模型路径
     */
    FaceDetector::FaceDetector(const std::string& modelPath) : isLoaded_(false), scaleFactor_(1.1), minNeighbors_(5) {

        // 使用 std::make_unique 创建实例 (C++14特性)
        classifier_ = std::make_unique<cv::CascadeClassifier>();
        if (classifier_->load(modelPath)) {
            isLoaded_ = true;
            std::cout << "[INFO] 人脸模型加载成功: " << modelPath << std::endl;
        } else {
            std::cerr << "[ERROR] 人脸模型加载失败，请检查路径: " << modelPath << std::endl;
            isLoaded_ = false;
        }
    }

    /**
     * @brief 析构函数
     */
    FaceDetector::~FaceDetector() {
        // unique_ptr 会自动释放内存，此处旨在使结构清晰
        std::cout << "[INFO] 释放 FaceDetector 资源" << std::endl;
    }

    /**
     * @brief 检查模型是否加载成功
     * @return true if loaded, false otherwise
     */
    bool FaceDetector::isModelLoaded() const {
        return isLoaded_;
    }

    /**
     * @brief 检测图像中的人脸
     * @param frame 输入的图像帧
     * @return 检测到的人脸矩形框列表
     */
    std::vector<cv::Rect> FaceDetector::detect(const cv::Mat& frame) {
        std::vector<cv::Rect> faces;

        // 如果模型未加载或图像为空，返回空列表
        if (!isLoaded_ || frame.empty()) {
            return faces;
        }

        cv::Mat gray;
        // 转换为灰度图以提高检测速度和准确率
        if (frame.channels() == 3) {
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = frame;
        }

        // 直方图均衡化，改善对比度
        cv::equalizeHist(gray, gray);

        // 多尺度检测
        try {
            classifier_->detectMultiScale(
                gray,
                faces,
                scaleFactor_,
                minNeighbors_,
                0 | cv::CASCADE_SCALE_IMAGE,
                cv::Size(30, 30)
            );
        } catch (const cv::Exception& e) {
            std::cerr << "[ERROR] OpenCV Exception: " << e.what() << std::endl;
        }

        return faces;
    }

} // namespace DriveGuard
