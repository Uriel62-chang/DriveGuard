# DriveGuard - 智能驾驶员监控系统 (DMS)

> **无人载具系统 & 智能感知 课程设计项目**

DriveGuard 是一个基于计算机视觉的**驾驶员监控系统 (Driver Monitoring System)**。它不仅仅是一个人脸识别程序，更是一个集成了**身份认证**、**权限管理**和**疲劳监测**的完整车载安全解决方案原型。

本项目旨在通过轻量级的算法（C++ & OpenCV），在无需昂贵硬件的情况下，实现对驾驶员状态的实时感知与分析。

---

## ✨ 核心功能

### 1. 👥 多用户身份识别与权限管理
系统能够“认识”不同的人，并根据身份赋予不同的权限：
- **驾驶员 (Driver)**：系统重点监控对象。识别成功后显示**绿色**边框，并启动疲劳监测逻辑。
- **乘客 (Passenger)**：系统友好放行对象。识别成功后显示**蓝色**边框，不进行疲劳干扰。
- **陌生人 (Unknown)**：未授权人员。显示**红色**边框并提示警告 (WARNING)，模拟禁止启动车辆。

### 2. 😴 智能疲劳/分心监测
针对驾驶员进行实时眼部状态分析，保障行车安全：
- **实时状态机**：通过算法判断眼睛闭合的持续时间。
- **分级预警**：
    - **疲劳 (Fatigue)**：闭眼超过约 1.5 秒，显示**黄色**警告。
    - **睡眠 (Sleeping)**：闭眼超过约 3 秒，显示**红色**严重报警 (DANGER)。

### 3. 📝 交互式现场录入
无需编写代码或手动处理文件，即可现场注册新用户：
- **一键录入**：运行中按 `R` 键进入录入模式。
- **向导式流程**：在控制台输入姓名、选择角色（驾驶员/乘客），跟随倒计时完成人脸采集。
- **增量学习**：新用户的加入不会影响旧用户的数据，支持多人共存。

---

## 🛠 技术架构

本项目采用**模块化**设计，遵循现代 C++ (C++17) 标准，结构清晰，易于扩展。

- **开发语言**: C++17 (利用 STL, Smart Pointers 管理内存)
- **视觉库**: OpenCV 4.10.0 (Core, Objdetect, Face 模块)
- **构建工具**: CMake (跨平台支持 Windows/Linux)
- **核心算法**:
    - **检测**: Haar Cascade Classifiers (人脸与眼部检测)
    - **识别**: LBPH (局部二值模式直方图) - 具有良好的抗光照干扰能力
    - **决策**: 有限状态机 (FSM) - 处理疲劳判定的时序逻辑

## 📂 项目结构

```text
DriveGuard/
├── CMakeLists.txt          # CMake 构建配置
├── include/                # 头文件 (接口定义)
│   ├── DMSController.h     # 疲劳监测控制器
│   ├── FaceDetector.h      # 视觉检测模块
│   └── FaceRecognizer.h    # 身份识别与数据库模块
├── src/                    # 源代码 (核心逻辑)
│   ├── DMSController.cpp   
│   ├── FaceDetector.cpp    
│   ├── FaceRecognizer.cpp  
│   └── main.cpp            # 主程序与交互逻辑
├── models/                 # 模型与数据存储
│   ├── haarcascade_*.xml   # OpenCV 预训练检测器
│   ├── face_rec.yml        # 训练好的人脸识别模型
│   └── label_to_name.txt   # 用户数据库 (ID:姓名:角色)
└── build/                  # 编译输出目录
```

---

## 🚀 快速开始

### 1. 环境准备
确保你的电脑上安装了：
- C++ 编译器 (MinGW GCC 或 Visual Studio)
- CMake
- OpenCV 4.x (包含 Contrib 模块以支持 Face 功能)

### 2. 编译项目

#### 🪟 Windows (MinGW)
确保环境变量中已配置 MinGW 和 OpenCV。

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

#### 🐧 Linux (Ubuntu/Debian/Raspberry Pi)
首先安装必要的构建工具和 OpenCV 开发包：

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libopencv-dev # 建议直接编译OpenCV源码
```

然后进行编译：

```bash
mkdir build
cd build
cmake ..
make
```

### 3. 运行系统
确保 `models` 目录下包含必要的 `.xml` 文件。

**Windows:**
```bash
./DriveGuard.exe
```

**Linux:**
```bash
./DriveGuard
```

---

## 🎮 操作指南

程序启动后，将自动开启摄像头并进入监控模式。

### 键盘控制
- **`Q` / `ESC`**: 退出程序。
- **`R`**: 进入 **用户录入模式**。

### 录入新用户流程
1. 按下 **`R`** 键，视频画面将暂停。
2. 看向**控制台 (Terminal/CMD)**，按照提示输入：
    - 输入 **姓名** (英文，如 `Teacher_Li`)。
    - 选择 **角色** (输入 `1` 设为驾驶员，输入 `2` 设为乘客)。
3. 看向**摄像头**，等待 5 秒倒计时。
4. 保持头部微动，系统将自动采集 30 张样本并完成训练。
5. 录入完成后，系统自动切换回识别模式，你可以立即测试识别效果。

---

## 🔮 未来展望 (二次开发方向)
本项目作为课设原型，已具备完整的业务闭环。若需进一步商业化或科研深化，可考虑：
1. **算法升级**：将 Haar 检测器升级为深度学习模型 (如 YuNet, SSD)，提升侧脸和暗光下的检测稳定性。
2. **活体检测**：增加红外或动作配合检测，防止照片欺骗。
3. **嵌入式部署**：移植至 Jetson Nano 或树莓派，结合蜂鸣器硬件，打造真实的车载终端。

---
*Course Project for Unmanned Vehicle Systems | 2025*
