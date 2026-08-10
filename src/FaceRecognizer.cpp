#include "FaceRecognizer.h"
#include <iostream>
#include <fstream>

namespace DriveGuard {

    // 余弦相似度:衡量两个 128 维特征向量的相似程度([-1,1],越大越像)
    static double cosineSimilarity(const cv::Mat& a, const cv::Mat& b)
    {
        double dot = a.dot(b);
        double na = cv::norm(a), nb = cv::norm(b);
        if (na < 1e-6 || nb < 1e-6) return 0.0;
        return dot / (na * nb);
    }

    // 构造函数:加载 SFace ONNX 模型
    FaceRecognizer::FaceRecognizer(const std::string& recModelPath) {
        try {
            recognizer_ = cv::FaceRecognizerSF::create(recModelPath, "");
        } catch (const cv::Exception& e) {
            std::cerr << "[ERROR] SFace 模型加载异常: " << e.what() << std::endl;
        }
        if (recognizer_.empty())
            std::cerr << "[ERROR] SFace 模型加载失败: " << recModelPath << std::endl;
    }

    // 人脸框 -> alignCrop 所需的 1x4 输入 (x,y,w,h)
    static cv::Mat faceBoxToMat(const cv::Rect& faceBox) {
        return (cv::Mat_<double>(1, 4) << faceBox.x, faceBox.y, faceBox.width, faceBox.height);
    }

    /**
     * @brief 注册:对"原图 + 人脸框"提取特征并入对应 label 的特征库
     */
    void FaceRecognizer::update(const cv::Mat& srcImage, const cv::Rect& faceBox, int label) {
        if (recognizer_.empty() || srcImage.empty() || faceBox.width <= 0 || faceBox.height <= 0) return;
        try {
            cv::Mat aligned, feat;
            recognizer_->alignCrop(srcImage, faceBoxToMat(faceBox), aligned);   // 按人脸关键点对齐
            recognizer_->feature(aligned, feat);                  // 128 维特征
            gallery_[label].push_back(feat);
        } catch (const cv::Exception& e) {
            std::cerr << "[WARN] 特征提取异常: " << e.what() << std::endl;
        }
    }

    /**
     * @brief 识别:返回 label(余弦相似度 >= 阈值);未识别返回 -1
     */
    int FaceRecognizer::predict(const cv::Mat& srcImage, const cv::Rect& faceBox, double& confidence) {
        confidence = -1.0;
        if (recognizer_.empty() || gallery_.empty() || srcImage.empty() || faceBox.width <= 0 || faceBox.height <= 0)
            return -1;

        cv::Mat aligned, feat;
        try {
            recognizer_->alignCrop(srcImage, faceBoxToMat(faceBox), aligned);
            recognizer_->feature(aligned, feat);
        } catch (const cv::Exception& e) {
            std::cerr << "[WARN] 特征提取异常: " << e.what() << std::endl;
            return -1;
        }
        if (feat.empty()) return -1;

        int bestLabel = -1;
        double bestSim = -1.0;
        for (const auto& [label, feats] : gallery_) {
            for (const auto& f : feats) {
                double s = cosineSimilarity(feat, f);
                if (s > bestSim) { bestSim = s; bestLabel = label; }
            }
        }
        if (bestSim >= threshold_) { confidence = bestSim; return bestLabel; }
        return -1;   // 陌生人
    }

    /**
     * @brief 保存特征库到 yml(label_N -> N x 128 矩阵)
     */
    bool FaceRecognizer::saveModel(const std::string& filepath) {
        try {
            cv::FileStorage fs(filepath, cv::FileStorage::WRITE);
            if (!fs.isOpened()) {
                std::cerr << "[ERROR] 特征库保存失败,无法打开文件: " << filepath << std::endl;
                return false;
            }
            for (const auto& [label, feats] : gallery_) {
                cv::Mat all;   // N x 128
                for (const auto& f : feats) {
                    cv::Mat row = f.reshape(1, 1);
                    all.push_back(row);
                }
                fs << ("label_" + std::to_string(label)) << all;
            }
            std::cout << "[INFO] 特征库已保存至: " << filepath << std::endl;
            return true;
        } catch (const cv::Exception& e) {
            std::cerr << "[ERROR] 特征库保存异常: " << e.what() << std::endl;
            return false;
        }
    }

    /**
     * @brief 从 yml 加载特征库
     */
    bool FaceRecognizer::loadModel(const std::string& filepath) {
        try {
            cv::FileStorage fs(filepath, cv::FileStorage::READ);
            if (!fs.isOpened()) {
                std::cerr << "[ERROR] 特征库加载失败: " << filepath << std::endl;
                return false;
            }
            gallery_.clear();
            cv::FileNode root = fs.root();
            for (auto it = root.begin(); it != root.end(); ++it) {
                std::string key = (*it).name();
                if (key.rfind("label_", 0) != 0) continue;   // 跳过非特征库节点(如旧版 LBPH 模型)
                int label = -1;
                try { label = std::stoi(key.substr(6)); } catch (...) { continue; }
                cv::Mat all;                                              // N x 128
                fs[key] >> all;
                if (all.empty()) continue;
                for (int r = 0; r < all.rows; ++r)
                    gallery_[label].push_back(all.row(r).clone());
            }
            if (gallery_.empty()) {
                std::cerr << "[ERROR] 特征库为空或格式不兼容(旧版 LBPH 模型需删除后重新录入): " << filepath << std::endl;
                return false;
            }
            std::cout << "[INFO] 特征库加载成功: " << filepath << std::endl;
            return true;
        } catch (const cv::Exception& e) {
            std::cerr << "[ERROR] 特征库加载异常: " << e.what() << std::endl;
            return false;
        }
    }

    /**
     * @brief 保存 label:name:role 映射表(文本)
     */
    void FaceRecognizer::saveLabelInfo(const std::string& filepath) {
        std::ofstream ofs;
        ofs.open(filepath, std::ios::out);
        if (!ofs.is_open()) {
            std::cerr << "[ERROR] 无法保存映射表到文件: " << filepath << std::endl;
            exit(-1);
        }
        for (const auto& [label, name] : labelToName_) {
            int roleValue = 99;
            if (labelToRole_.count(label)) roleValue = (int)labelToRole_[label];
            ofs << label << ":" << name << ":" << roleValue << std::endl;
        }
        ofs.close();
        std::cout << "[INFO] ID-Name-Role 映射表已保存到: " << filepath << std::endl;
    }

    /**
     * @brief 加载 label:name:role 映射表(文本)
     */
    void FaceRecognizer::loadLabelInfo(const std::string& filepath) {
        std::ifstream ifs;
        ifs.open(filepath, std::ios::in);
        if (!ifs.is_open()) {
            std::cerr << "[ERROR] 无法从文件: " << filepath << " 加载映射表" << std::endl;
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
                labelToName_[label] = name;
                labelToRole_[label] = (UserRole)roleValue;
            } catch (...) {
                continue;
            }
        }
        ifs.close();
        std::cout << "[INFO] 已从文件: " << filepath << " 加载全部用户信息" << std::endl;
    }

    /**
     * @brief 获取可用标签
     */
    int FaceRecognizer::getAvailableLabel() const {
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
    std::string FaceRecognizer::getLabelName(int label) const {
        auto it = labelToName_.find(label);
        return it != labelToName_.end() ? it->second : "Unknown";
    }

    /**
     * @brief 获取ID对应的角色
     */
    UserRole FaceRecognizer::getLabelRole(int label) const {
        auto it = labelToRole_.find(label);
        return it != labelToRole_.end() ? it->second : UserRole::UNKNOWN;
    }

}
