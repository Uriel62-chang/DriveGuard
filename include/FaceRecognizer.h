#ifndef FACE_RECOGNIZER_H
#define FACE_RECOGNIZER_H

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/face.hpp>
#include <vector>
#include <string>
#include <map>

namespace DriveGuard {

    // [新增] 定义用户角色枚举
    enum class UserRole {
        DRIVER = 0,    // 驾驶员 (监控疲劳)
        PASSENGER = 1, // 乘客 (仅识别)
        UNKNOWN = 99   // 未知
    };

    /**
     * @brief 基于 SFace(ArcFace 128 维特征 + 余弦相似度)的人脸识别器
     * 特征库是"向量集合",新增用户无需重训练,人数扩展不受限。
     */
    class FaceRecognizer {
    public:
        // recModelPath: SFace onnx 模型路径(face_recognition_sface_2021dec.onnx)
        explicit FaceRecognizer(const std::string& recModelPath);

        // 注册:对"原图 + 人脸框"提取特征并入对应 label 的特征库
        void update(const cv::Mat& srcImage, const cv::Rect& faceBox, int label);

        /**
         * @brief 识别:返回 label(余弦相似度 >= 阈值);未识别返回 -1
         * @param srcImage 原图(彩色)
         * @param faceBox 人脸框
         * @param confidence 输出余弦相似度([0,1],越大越像)
         */
        int predict(const cv::Mat& srcImage, const cv::Rect& faceBox, double& confidence);

        // 特征库持久化(yml)
        bool saveModel(const std::string& filepath);
        bool loadModel(const std::string& filepath);

        // label ↔ 姓名/角色 映射
        void saveLabelInfo(const std::string& filepath);
        void loadLabelInfo(const std::string& filepath);
        std::string getLabelName(int label) const;
        UserRole getLabelRole(int label) const;
        int getAvailableLabel() const;
        void setLabelInfo(int label, const std::string& name, UserRole role);

        // 相似度阈值(低于视为陌生人),默认 0.363(SFace 官方推荐)
        void setThreshold(double t) { threshold_ = t; }

        // 识别器是否就绪(SFace 模型是否加载成功)
        bool isReady() const { return !recognizer_.empty(); }

    private:
        cv::Ptr<cv::FaceRecognizerSF> recognizer_;          // SFace 特征提取器
        std::map<int, std::vector<cv::Mat>> gallery_;       // label -> 特征向量集合
        std::map<int, std::string> labelToName_;            // label -> 姓名
        std::map<int, UserRole> labelToRole_;               // label -> 角色
        double threshold_ = 0.363;                          // 余弦相似度阈值
    };

}

#endif
