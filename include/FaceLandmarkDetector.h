#ifndef FACE_LANDMARK_DETECTOR_H
#define FACE_LANDMARK_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

namespace DriveGuard {

    /**
     * @brief 基于 InsightFace 2d106det(106 点关键点,DNN)的眼睛纵横比(EAR)检测
     * 模型: models/2d106det.onnx(加载路径由调用方传入);输入: 人脸裁剪 192x192;输出 (1,212) = 106 点 x,y
     */
    class FaceLandmarkDetector {
    public:
        // onnxPath: 2d106det.onnx 路径
        explicit FaceLandmarkDetector(const std::string& onnxPath);

        /**
         * @brief 对"原图 + 人脸框"返回双眼平均 EAR;关键点拟合失败返回 -1
         * EAR 越大越睁眼(睁眼≈0.3,闭眼<0.2);阈值由调用方定
         */
        double eyeAspectRatio(const cv::Mat& src, const cv::Rect& face);

        // 关键点模型是否加载成功
        bool isReady() const { return !net_.empty(); }

    private:
        cv::dnn::Net net_;
        static const int LM_INPUT_ = 192;
        // 2d106det 编号(已标定):左眼 内39 外35 上睑41,42 下睑36,37;右眼 内89 外93 上睑95,96 下睑90,91
        struct EyeIdx { int inner, outer, up1, up2, low1, low2; };
        static const EyeIdx LEFT_EYE_, RIGHT_EYE_;
        static double earOf(const std::vector<cv::Point2f>& pts, const EyeIdx& e);
    };

}

#endif
