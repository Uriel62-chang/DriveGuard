#include "FaceLandmarkDetector.h"
#include <iostream>

namespace DriveGuard {

    const FaceLandmarkDetector::EyeIdx FaceLandmarkDetector::LEFT_EYE_  = {39, 35, 41, 42, 36, 37};
    const FaceLandmarkDetector::EyeIdx FaceLandmarkDetector::RIGHT_EYE_ = {89, 93, 95, 96, 90, 91};

    // 构造函数:加载 2d106det ONNX 关键点模型
    FaceLandmarkDetector::FaceLandmarkDetector(const std::string& onnxPath) {
        try {
            net_ = cv::dnn::readNetFromONNX(onnxPath);
        } catch (const cv::Exception& e) {
            std::cerr << "[ERROR] 关键点模型加载异常: " << e.what() << std::endl;
        }
        if (net_.empty())
            std::cerr << "[ERROR] 关键点模型加载失败: " << onnxPath << std::endl;
    }

    static double dist2(const cv::Point2f& a, const cv::Point2f& b) { return cv::norm(a - b); }

    double FaceLandmarkDetector::earOf(const std::vector<cv::Point2f>& pts, const EyeIdx& e)
    {
        double A = dist2(pts[e.up1], pts[e.low1]);
        double B = dist2(pts[e.up2], pts[e.low2]);
        double C = dist2(pts[e.inner], pts[e.outer]);
        return (A + B) / (2.0 * C + 1e-6);
    }

    /**
     * @brief 对"原图 + 人脸框"返回双眼平均 EAR;失败返回 -1
     */
    double FaceLandmarkDetector::eyeAspectRatio(const cv::Mat& src, const cv::Rect& face)
    {
        if (net_.empty()) return -1.0;

        cv::Rect fr = face & cv::Rect(0, 0, src.cols, src.rows);
        if (fr.width <= 0 || fr.height <= 0) return -1.0;

        try {
            cv::Mat crop = src(fr);
            cv::Mat blob = cv::dnn::blobFromImage(crop, 1.0, cv::Size(LM_INPUT_, LM_INPUT_));
            net_.setInput(blob);
            cv::Mat out = net_.forward();                     // 1x212
            if (out.total() < 212 || out.type() != CV_32F) return -1.0;

            std::vector<cv::Point2f> pts(106);
            const float* p = out.ptr<float>();
            for (int i = 0; i < 106; ++i) {
                // 中心归一化输出 [-1,1] -> 裁剪图内像素坐标
                pts[i] = cv::Point2f(p[2*i] * (LM_INPUT_/2) + LM_INPUT_/2,
                                     p[2*i+1] * (LM_INPUT_/2) + LM_INPUT_/2);
                pts[i].x += fr.x;                             // 转回全图坐标(EAR 为比值,平移不影响)
                pts[i].y += fr.y;
            }
            return (earOf(pts, LEFT_EYE_) + earOf(pts, RIGHT_EYE_)) / 2.0;
        } catch (const cv::Exception& e) {
            // 推理失败按"无法判定"处理,不影响主循环
            return -1.0;
        }
    }

}
