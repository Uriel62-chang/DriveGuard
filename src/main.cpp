#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <thread>
#include <filesystem>
#include "FaceDetector.h"
#include "FaceRecognizer.h"
#include "DMSController.h"

// 配置常量
const std::string WINDOW_NAME = "DriveGuard - DMS";
const std::string MODEL_PATH = "../models/haarcascade_frontalface_default.xml"; // 人脸级联器模型
const std::string EYE_MODEL_PATH = "../models/haarcascade_eye.xml"; // 眼睛级联器模型
const std::string REC_MODEL_PATH = "../models/face_rec.yml"; // 人脸识别模型
const std::string LABEL_TO_NAME_TXT = "../models/label_to_name.txt"; // ID-Name 映射表

// 录入参数配置
const int RECORD_MAX_IMAGES = 30; // 单次录入图片数
const int RECORD_INTERVAL_MS = 100; // 每次采集间隔（毫秒） 
const double CONFIDENCE_THRESHOLD = 80.0; // 置信度阈值（低于该值即通过）（越低越严格）

// 状态机
enum class ModelState {
    DETECTING, // 检测模式（默认） 0
    RECORDING, // 录入模式 1
    RECOGNIZING // 识别模式 2
};

int main(int argc, char* argv[]) {
    std::cout << "===========================================" << std::endl;
    std::cout << "            驾驶员监控系统 - DMS            " << std::endl;
    std::cout << "===========================================" << std::endl;

    // 打开摄像头 (0 通常是默认摄像头)
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "[FATAL] 无法打开摄像头!" << std::endl;
        return -1;
    }
    
    // 设置摄像头分辨率 (可选)
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // 初始化检测器
    DriveGuard::FaceDetector detector(MODEL_PATH, EYE_MODEL_PATH);
    if (!detector.isModelLoaded()) {
        std::cerr << "[FATAL] 初始化检测器失败，程序退出" << std::endl;
        std::cerr << "请确保 '" << MODEL_PATH << "' 和 '" << EYE_MODEL_PATH << "' 文件存在" << std::endl;
        return -1;
    }

    // 初始化识别器
    DriveGuard::FaceRecognizer recognizer;
    ModelState currentState = ModelState::DETECTING;
    if (recognizer.loadModel(REC_MODEL_PATH)) {
        recognizer.loadLabelInfo(LABEL_TO_NAME_TXT);
        currentState = ModelState::RECOGNIZING;
    } else {
        std::cout << "[INFO] 未找到人脸识别模型，如果您为驾驶员，请录入自身的脸部照片……" << std::endl;
    }
    // 录入模式参数
    std::vector<cv::Mat> trainingImages;
    std::vector<int> trainingLabels;
    std::string userName;
    int userLabel;
    DriveGuard::UserRole userRole;
    int recordingCount = 0;

    // 初始化DMS控制器
    DriveGuard::DMSController dms;

    // 捕获镜头帧
    cv::Mat frame;
    std::cout << "[INFO] 系统就绪。按 'Q/q' 退出，按 'R/r' 进入录入模式。" << std::endl;

    // 主循环
    while (true) {
        // 捕获帧
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "[WARN] 捕获到空帧，跳过..." << std::endl;
            continue;
        }

        // 处理帧（人脸检测）
        // 利用 auto 自动推导类型 (C++11)
        auto faces = detector.detect(frame);

        // 绘制结果
        for (const auto& face : faces) {
            cv::Scalar borderColor(0, 255, 0); // 人脸边框默认为绿色

            // ===============================
            // 分支：录入模式
            // ===============================
            if (currentState == ModelState::RECORDING) {
                borderColor = cv::Scalar(255, 0, 0); // 录入模式：人脸边框为蓝色

                if (recordingCount < RECORD_MAX_IMAGES) {
                    // 截取人脸区域（深拷贝）
                    cv::Mat faceROI = frame(face).clone();

                    // 预处理：转灰度并统一大小 (LBPH 需要)
                    cv::Mat gray;
                    if (faceROI.channels() == 3) cv::cvtColor(faceROI, gray, cv::COLOR_BGR2GRAY);
                    else gray = faceROI;
                    cv::resize(gray, gray, cv::Size(100, 100));

                    trainingImages.push_back(gray);
                    trainingLabels.push_back(userLabel);
                    recordingCount++;
                    
                    // 打印录入进度
                    std::string progress = "Sample:" 
                                        + std::to_string(recordingCount)
                                        + "/"
                                        + std::to_string(RECORD_MAX_IMAGES);
                    cv:putText(frame, progress, cv::Point(face.x, face.y - 20),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2);

                    // 稍作延迟，避免录入样本过于重复（注意此处阻塞时间不要太久）
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                } else { // 样本录入完成，开始训练
                    // 录入完成，开始训练
                    cv::putText(frame, "Training...", cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
                    cv::imshow(WINDOW_NAME, frame);
                    cv::waitKey(10); // 处理键盘输入（等待10ms）
                    
                    recognizer.update(trainingImages, trainingLabels);
                    if (recognizer.saveModel(REC_MODEL_PATH)) {
                        
                    }
                    recognizer.setLabelInfo(userLabel, userName, userRole);
                    recognizer.saveLabelInfo(LABEL_TO_NAME_TXT);

                    currentState = ModelState::RECOGNIZING;
                    std::cout << "[INFO] 录入、训练完成，模型已保存" << std::endl;
                }
            }
            // ===============================
            // 分支：识别模式
            // ===============================
            else if (currentState == ModelState::RECOGNIZING) {
                // 预测人脸
                cv::Mat faceROI = frame(face);
                double confidence = 0.0;
                int label = recognizer.predict(faceROI, confidence);

                // 获取人脸名称
                std::string name = "Unknown";
                DriveGuard::UserRole role = DriveGuard::UserRole::UNKNOWN;
                if (label != -1 && confidence < CONFIDENCE_THRESHOLD) {
                    name = recognizer.getLabelName(label);
                    role = recognizer.getLabelRole(label);
                }

                // 如果是驾驶员，检测眼睛并判断疲劳程度
                if (role == DriveGuard::UserRole::DRIVER) {
                    auto driverEyes = detector.detectEyes(faceROI);
                    for (const auto& eye : driverEyes) {
                        // 计算绝对坐标，绘制眼睛边框
                        cv::Rect eyeRect(face.x + eye.x, face.y + eye.y, eye.width, eye.height);
                        cv::rectangle(frame, eyeRect, cv::Scalar(255, 0, 0), 1);
                    }

                    // 显示驾驶员状态
                    dms.update(!driverEyes.empty(), 10.0); // 判断驾驶员当前状态
                    std::string warning = dms.getWarning(); // 获取当前警告信息
                    borderColor = dms.getStatusColor(); // 将人脸边框设置为对应的警告颜色
                    cv::putText(frame, warning, cv::Point(face.x, face.y + face.height + 30),
                        cv::FONT_HERSHEY_SIMPLEX, 0.8, borderColor, 2);
                }
                else if (role == DriveGuard::UserRole::PASSENGER) {
                    borderColor = cv::Scalar(0, 255, 0); // 乘客边框为绿色
                    cv::putText(frame, "Passenger", cv::Point(face.x, face.y + face.height + 20),
                                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
                }

                // 如果是未知人员
                else {
                    borderColor = cv::Scalar(0, 0, 255); // 未知人员边框为红色
                    cv::putText(frame, "Unknown", cv::Point(face.x, face.y + face.height + 20),
                                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
                }

                // 绘制标签
                std::string labelText = name + " (confidence: " + std::to_string((int)confidence) + ")";
                cv::putText(frame, labelText, cv::Point(face.x, face.y - 5),
                            cv::FONT_HERSHEY_SIMPLEX, 0.6, borderColor, 2);
            }

            // 绘制人脸框
            cv::rectangle(frame, face, borderColor, 2);
        }

        // 屏幕状态提示
        if (currentState == ModelState::DETECTING) {
            cv::putText(frame, "System Ready. Press 'R' to Register Driver.", cv::Point(10, 30), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
        } else if (currentState == ModelState::RECOGNIZING) {
            cv::putText(frame, "DMS Monitoring Active", cv::Point(10, 30), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow(WINDOW_NAME, frame);

        // 处理键盘输入 (等待10ms)
        char c = (char)cv::waitKey(10);
        if (c == 27 || c == 'q' || c == 'Q') {
            break;
        }
        else if (c == 'r' || c == 'R') {
            std::cout << "请输入新用户姓名：（英文）" << std::endl;
            std::string newName;
            std::cin >> newName;

            std::cout << "请选择用户角色：" << std::endl;
            std::cout << "1. 驾驶员" << std::endl;
            std::cout << "2. 乘客" << std::endl;
            DriveGuard::UserRole newRole;
            int roleChoice;
            std::cin >> roleChoice;
            if (roleChoice == 1) newRole = DriveGuard::UserRole::DRIVER;
            else if (roleChoice == 2) newRole = DriveGuard::UserRole::PASSENGER;
            else newRole = DriveGuard::UserRole::UNKNOWN;
            
            int newLabel = recognizer.getAvailableLabel();

            std::cout << "请注视摄像头，即将开始录入……" << std::endl;
            for (int i = 5; i > 0; i--) {
                std::cout << i << "秒后开始录入……" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            userName = newName;
            userLabel = newLabel;
            userRole = newRole;

            currentState = ModelState::RECORDING;
            std::cout << "[INFO] 切换至录入模式……" << std::endl;
        }
    }

    // 5. 资源清理
    // VideoCapture 和 Mat 会在析构时自动释放，
    // 但手动 release 是个好习惯，或者 explicitly destroy windows
    cap.release();
    cv::destroyAllWindows();

    std::cout << "[INFO] 程序正常退出。" << std::endl;
    return 0;
}