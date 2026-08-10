# DriveGuard

基于 C++ / OpenCV 的驾驶员监控系统 (Driver Monitoring System, DMS),实现实时人脸身份识别与疲劳状态监测。

## 特性

- **人脸识别**:基于 SFace (ArcFace) 128 维特征与余弦相似度比对;特征库为向量集合,新增用户无需重训练
- **疲劳监测**:基于 2d106det 关键点计算眼睛纵横比 (EAR),时间制状态机判定疲劳与睡眠,与帧率无关
- **多角色权限**:驾驶员(启动疲劳监控)、乘客(仅识别)、陌生人(告警)
- **现场录入**:运行时按 `R` 交互式注册新用户,特征实时入库,无需训练等待

## 技术栈

- C++17,CMake 3.10+
- OpenCV 4.5.4+(Core / Objdetect / DNN,无需 Contrib)

## 模型

| 模型 | 用途 | 来源 |
|---|---|---|
| `face_recognition_sface_2021dec.onnx` | 人脸特征提取 (128 维) | [OpenCV Zoo](https://github.com/opencv/opencv_zoo) |
| `2d106det.onnx` | 106 点关键点,EAR 闭眼检测 | InsightFace |
| `haarcascade_frontalface_default.xml` | 人脸检测 | OpenCV 预训练模型 |

模型文件位于 `models/` 目录,运行前请确保存在。`face_rec.yml` 与 `label_to_name.txt` 为运行时生成的数据文件,无需手动维护。

## 构建

### 依赖

- 支持 C++17 的编译器 (GCC / Clang / MSVC)
- CMake 3.10+
- OpenCV 4.5.4+(含 `objdetect` 与 `dnn` 模块)

### Linux / macOS

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

### Windows (MinGW)

```bash
cmake -G "MinGW Makefiles" -B build
mingw32-make -C build
```

构建产物输出至 `bin/DriveGuard`。

## 运行

```bash
./bin/DriveGuard                              # 默认摄像头 (设备 0)
./bin/DriveGuard 1                            # 指定设备号
./bin/DriveGuard video.mp4                    # 视频文件
./bin/DriveGuard http://localhost:9000/video  # MJPEG 流 (如 Windows 推流 → WSL)
```

首次运行(无特征库)时,系统进入检测模式并提示注册用户。

## 使用

### 键盘控制

| 按键 | 功能 |
|---|---|
| `R` | 进入用户录入模式 |
| `Q` / `ESC` | 退出程序 |

### 注册新用户

1. 按 `R`,在控制台输入姓名(英文)并选择角色:`1` 驾驶员、`2` 乘客
2. 注视摄像头,等待 5 秒倒计时
3. 系统逐帧提取 30 个特征实时写入特征库(无需训练)
4. 录入完成自动切换回识别模式,可立即验证识别效果

## 配置

| 参数 | 默认值 | 位置 |
|---|---|---|
| 人脸识别相似度阈值 | 0.363 | `FaceRecognizer::setThreshold`,低于视为陌生人 |
| EAR 闭眼阈值 | 0.25 | `main.cpp`,低于视为闭眼 |
| 疲劳判定时长 | 1.5 s | `DMSController.h` 中 `FATIGUE_SECONDS_` |
| 睡眠判定时长 | 3.0 s | `DMSController.h` 中 `SLEEPING_SECONDS_` |
| 录入样本数 | 30 | `main.cpp` 中 `RECORD_MAX_IMAGES` |

## 项目结构

```text
DriveGuard/
├── CMakeLists.txt                    # 构建配置
├── include/                          # 公共头文件
│   ├── DMSController.h               # 疲劳状态控制器
│   ├── FaceDetector.h                # 人脸检测
│   ├── FaceLandmarkDetector.h        # 关键点 EAR 闭眼检测
│   └── FaceRecognizer.h              # SFace 人脸识别与特征库
├── src/                              # 实现源码
│   ├── DMSController.cpp
│   ├── FaceDetector.cpp
│   ├── FaceLandmarkDetector.cpp
│   ├── FaceRecognizer.cpp
│   └── main.cpp                      # 主程序与交互逻辑
├── models/                           # 模型与运行数据
│   ├── haarcascade_frontalface_default.xml
│   ├── face_recognition_sface_2021dec.onnx
│   ├── 2d106det.onnx
│   ├── face_rec.yml                  # 特征库(运行时生成)
│   └── label_to_name.txt             # 用户映射(运行时生成)
├── build/                            # 构建产物(已忽略)
└── bin/                              # 可执行文件(已忽略)
```
