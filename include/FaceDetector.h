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
         * @param modelPath 人脸级联器模型路径
         */
        explicit FaceDetector(const std::string& modelPath);

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

    private:
        std::unique_ptr<cv::CascadeClassifier> classifier_;
        bool isLoaded_;
        double scaleFactor_;
        int minNeighbors_;
    };

} // namespace DriveGuard

#endif // FACE_DETECTOR_H
