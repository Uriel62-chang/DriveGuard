#ifndef FACE_RECOGNIZER_H
#define FACE_RECOGNIZER_H

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
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

    class FaceRecognizer {
    public:
        // 构造函数
        FaceRecognizer();

        // 析构函数
        ~FaceRecognizer();

        // /**
        //  * @brief 训练模型
        //  * @param images 人脸图像列表
        //  * @param labels 人脸标签链表
        //  */
        // void train(const std::vector<cv::Mat>& images, const std::vector<int>& labels);

        /**
         * @brief 更新模型（增量训练）
         * @param images 新的人脸图像列表
         * @param labels 新的人脸标签列表
         */
        void update(const std::vector<cv::Mat>& images, const std::vector<int>& labels);

        /**
         * @brief 预测身份
         * @param image 人脸图像
         * @param confidence 置信度
         * @return 预测结果
         */
        int predict(const cv::Mat& image, double& confidence);

        /**
         * @brief 保存模型到文件
         */
        bool saveModel(const std::string& filepath);

        /**
         * @brief 从文件加载模型
         */
        bool loadModel(const std::string& filepath);

        /**
         * @brief 保存 ID-Name 映射表
         */
        void saveLabelInfo(const std::string& filepath);

        /**
         * @brief 加载 ID-Name 映射表
         */
        void loadLabelInfo(const std::string& filepath);

        /**
         * @brief 获取可用标签
         */
        int getAvailableLabel();

        /**
         * @brief 添加标签与人名的映射
         */
        void setLabelInfo(int label, const std::string& name, UserRole role);

        /**
         * @brief 获取ID对应的名字
         */
        std::string getLabelName(int label);

        /**
         * @brief 获取ID对应的角色
         */
        UserRole getLabelRole(int label);
    private:
        cv::Ptr<cv::face::LBPHFaceRecognizer> model_;
        std::map<int, std::string> labelToName_;
        std::map<int, UserRole> labelToRole_;
    };

}

#endif