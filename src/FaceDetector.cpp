#include "FaceDetector.h"
#include <iostream>
#include <stdexcept>

namespace DriveGuard {
    /**
     * @brief 构造函数
     * @param faceModelPath 人脸识别模型的路径
     * @param eyeModelPath 眼睛识别模型的路径
     */
    FaceDetector::FaceDetector(const std::string& modelPath, const std::string& eyeModelPath) : isLoaded_(false), scaleFactor_(1.1), minNeighbors_(5) {
        
        // 使用 std::make_unique 创建实例 (C++14特性)
        bool isfaceLoaded = false;
        classifier_ = std::make_unique<cv::CascadeClassifier>();
        if (classifier_->load(modelPath)) {
            isfaceLoaded = true;
            std::cout << "[INFO] 人脸模型加载成功: " << modelPath << std::endl;
        } else {
            std::cerr << "[ERROR] 人脸模型加载失败，请检查路径: " << modelPath << std::endl;
            isfaceLoaded = false;
        }

        // 创建眼睛实例
        bool iseyeLoaded = false;
        eyeClassifier_ = std::make_unique<cv::CascadeClassifier>();
        if (eyeClassifier_->load(eyeModelPath)) {
            iseyeLoaded = true;
            std::cout << "[INFO] 眼睛模型加载成功: " << eyeModelPath << std::endl;
        } else {
            std::cerr << "[ERROR] 眼睛模型加载失败，请检查路径: " << eyeModelPath << std::endl;
            iseyeLoaded = false;
        }

        isLoaded_ = isfaceLoaded && iseyeLoaded ? true : false;

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

    /**
     * @brief 在给定的人脸区域中检测眼睛
     * @param faceROI 人脸区域矩形框
     * @return 检测到的眼睛矩形框列表
     */
    std::vector<cv::Rect> FaceDetector::detectEyes(const cv::Mat& faceROI) {
        std::vector<cv::Rect> eyes;

        // 如果模型未加载或人脸区域为空，返回空列表
        if (!isLoaded_ || faceROI.empty()) {
            return eyes;
        }

        cv::Mat grayROI;
        if (faceROI.channels() == 3) {
            cv::cvtColor(faceROI, grayROI, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(grayROI, grayROI);
        } else {
            grayROI = faceROI; // 假设外部已经处理过灰度
        }

        try {
            // 眼睛检测通常需要稍微不同的参数，这里 minNeighbors 设大一点以减少误检
            eyeClassifier_->detectMultiScale(
                grayROI, eyes, 1.1, 3, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(15, 15)
            );
        } catch (const cv::Exception& e) {
            std::cerr << "[ERROR] Eye Detection Exception: " << e.what() << std::endl;
        }

        return eyes;
    }



} // namespace DriveGuard

