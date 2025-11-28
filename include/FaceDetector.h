#ifndef FACE_DETECTOR_H
#define FACE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>

namespace DriveGuard {

    /**
     * @brief 人脸检测器类
     * 封装了OpenCV的级联分类器，用于实现人脸检测功能
     */
    class FaceDetector {
    public:
        /**
         * @brief 构造函数
         * @param faceModelPath 人脸识别模型的路径
         * @param eyeModelPath 眼睛识别模型的路径
         */
        explicit FaceDetector(const std::string& modelPath, const std::string& eyeModelPath);

        /**
         * @brief 析构函数
         */
        ~FaceDetector();

        /**
         * @brief 检查模型是否加载成功
         * @return true if loaded, false otherwise
         */
        bool isModelLoaded() const;

        /**
         * @brief 检测图像中的人脸
         * @param frame 输入的图像帧
         * @return 检测到的人脸矩形框列表
         */
        std::vector<cv::Rect> detect(const cv::Mat& frame);

        /**
         * @brief 在给定的人脸区域中检测眼睛
         * @param faceROI 人脸区域矩形框
         * @return 检测到的眼睛矩形框列表
         */
        std::vector<cv::Rect> detectEyes(const cv::Mat& faceROI);

    private:
        // 使用智能指针虽然对于cv::CascadeClassifier不是必须的（它自己管理内存），
        // 但这里为了演示现代C++内存管理风格而使用
        std::unique_ptr<cv::CascadeClassifier> classifier_;
        std::unique_ptr<cv::CascadeClassifier> eyeClassifier_;
        bool isLoaded_;
        double scaleFactor_;
        int minNeighbors_;
    };

} // namespace DriveGuard

#endif // FACE_DETECTOR_H

