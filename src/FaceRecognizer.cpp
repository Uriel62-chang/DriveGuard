#include "FaceRecognizer.h"
#include <iostream>
#include <fstream>

namespace DriveGuard {
    // 构造函数
    FaceRecognizer::FaceRecognizer() {
        // 创建 LBPH 识别器
        // radius=1, neighbors=8, grid_x=8, grid_y=8
        // threshold=DBL_MAX (可以后续设置阈值，超过阈值则返回 -1)
        model_ = cv::face::LBPHFaceRecognizer::create();
    }

    // 析构函数
    FaceRecognizer::~FaceRecognizer() {
        // cv::Ptr会自动释放内存
    }

    // /**
    //  * @brief 训练模型
    //  * @param images 人脸图像列表
    //  * @param labels 人脸标签链表
    //  */
    // void FaceRecognizer::train(const std::vector<cv::Mat>& images, const std::vector<int>& labels) {
    //     if (images.empty() || images.size() != labels.size()) {
    //         std::cerr << "[ERROR] 训练数据集为空 或 图片与标签数量不匹配" << std::endl;
    //         return;
    //     }

    //     std::cout << "[INFO] 开始训练模型，图片数量：" << images.size() << std::endl;
    //     model_->train(images, labels);
    //     std::cout << "[INFO] 模型训练完成" << std::endl;
    // }

    /**
     * @brief 更新模型（增量训练）
     * @param images 新的人脸图像列表
     * @param labels 新的人脸标签列表
     */
    void FaceRecognizer::update(const std::vector<cv::Mat>& images, const std::vector<int>& labels) {
        if (images.empty() || images.size() != labels.size()) {
            std::cerr << "[ERROR] 更新数据集为空 或 图片与标签数量不匹配" << std::endl;
            return;
        }

        try {
            std::cout << "[INFO] 开始更新模型，新增样本数：" << images.size() << std::endl;
            model_->update(images, labels);
            std::cout << "[INFO] 模型更新完成" << std::endl;
        } catch (const cv::Exception& e) {
            std::cerr << "[WARN] 更新失败（可能是首次训练），尝试重新训练: " << e.what() << std::endl;
            model_->train(images, labels);
        }
    }

    /**
     * @brief 预测身份
     * @param image 人脸图像
     * @param confidence 置信度
     * @return 预测结果
     */
    int FaceRecognizer::predict(const cv::Mat& image, double& confidence) {
        // 帧为空，略过
        if (image.empty()) return -1;

        // LBPH 需要灰度图
        cv::Mat gray;
        if (image.channels() == 3) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = image;
        }

        // 初始化标签和置信度
        int label = -1;
        confidence = 0.0;

        // 预测
        model_->predict(gray, label, confidence);
        return label;
    }

    /**
     * @brief 保存模型到文件
     */
    bool FaceRecognizer::saveModel(const std::string& filepath) {
        try {
            model_->write(filepath);
            std::cout << "[INFO] 模型已保存至：" << filepath << std::endl; 
            return true;
        } catch (const cv::Exception& e) {
            std::cerr << "[ERROR] 模型保存失败" << e.what() << std::endl;
            return false;
        }
    }

    /**
     * @brief 从文件加载模型
     */
    bool FaceRecognizer::loadModel(const std::string& filepath) {
        try {
            model_->read(filepath);
            std::cout << "[INFO] 模型加载成功" << filepath << std::endl;
            return true;
        } catch (const cv::Exception& e) {
            std::cerr << "[ERROR] 模型加载失败" << e.what() << std::endl;
            return false;
        }
    }

    /**
     * @brief 保存 ID-Name 映射表
     */
    void FaceRecognizer::saveLabelInfo(const std::string& filepath) {
        std::ofstream ofs;
        ofs.open(filepath, std::ios::out);
        if (!ofs.is_open()) {
            std::cerr << "[ERROR] 无法保存映射表到文件：" << filepath << std::endl;
            exit(-1);
        }

        for (const auto& [label, name] : labelToName_) {
            int roleValue = 99;
            if (labelToRole_.count(label)) roleValue = (int)labelToRole_[label];
            // label:name:roleValue
            ofs << label << ":" << name << ":" << roleValue << std::endl; 
        }
        ofs.close();
        std::cout << "[INFO] ID-Name-Role 映射表已保存到：" << filepath << std::endl;
    }

    /**
     * @brief 加载 ID-Name 映射表
     */
    void FaceRecognizer::loadLabelInfo(const std::string& filepath) {
        std::ifstream ifs;
        ifs.open(filepath, std::ios::in);
        if (!ifs.is_open()) {
            std::cerr << "[ERROR] 无法从文件：" << filepath << "加载映射表" << std::endl;
            exit(-1);
        }

        std::string line;
        labelToName_.clear();
        labelToRole_.clear();
        while (getline(ifs, line)) {
            int pos1 = (int)line.find(':');
            if (pos1 == std::string::npos) continue;

            int pos2 = (int)line.find(':', pos1 + 1);
            if (pos2 == std::string::npos) continue;

            try {
                int label = std::stoi(line.substr(0, pos1));
                std::string name = line.substr(pos1 + 1, pos2 - pos1 - 1);
                int roleValue = std::stoi(line.substr(pos2 + 1));

                // if (!name.empty() && name.back() == '\r') name.pop_back();
                labelToName_[label] = name;
                labelToRole_[label] = (UserRole)roleValue;
            } catch (...) {
                continue; 
            };
        }

        ifs.close();
        std::cout << "[INFO] 已从文件：" << filepath << "加载全部用户信息" << std::endl;
    }

    /**
     * @brief 获取可用标签
     */
    int FaceRecognizer::getAvailableLabel() {
        if (labelToName_.empty()) return 0;
        return labelToName_.rbegin()->first + 1;
    }

    /**
     * @brief 添加标签与用户信息的映射
     */
    void FaceRecognizer::setLabelInfo(int label, const std::string& name, UserRole role) {
        labelToName_[label] = name;
        labelToRole_[label] = role;
    }

    /**
     * @brief 获取ID对应的名字
     */
    std::string FaceRecognizer::getLabelName(int label) {
        if (labelToName_.count(label)) {
            return labelToName_[label];
        }

        return "Unknown";
    }

    /**
     * @brief 获取ID对应的角色
     */
    UserRole FaceRecognizer::getLabelRole(int label) {
        if (labelToRole_.count(label)) {
            return labelToRole_[label];
        }

        return UserRole::UNKNOWN;
    }
}