# 在 VSCode 环境下完成 C++ 推理的完整操作步骤
## 第一步：环境准备
1. **安装编译器**: 下载并安装 [MinGW-w64](https://github.com/niXman/mingw-builds-binaries/releases)，确保将 `bin` 文件夹路径添加到系统的**环境变量 (Path)** 中。
2. **安装 VS Code 插件**:
- **C/C++** (Microsoft)
- **CMake Tools** (Microsoft)
3. **获取依赖库**:
- **XGBoost**: 在 Python 环境的 `Lib\site-packages\xgboost` 文件夹中找到 `xgboost.dll`。
- **nlohmann/json**: 下载 [json.hpp](https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp) 并放入项目目录。
- **c_api.h**：访问 XGBoost 的 GitHub 发布页：[Releases · dmlc/xgboost](https://github.com/dmlc/xgboost/releases)，找到最新版本（或者与你 pip 安装版本相近的版本），在 Assets 区域，下载 Source code (zip)，解压这个压缩包。
## 第二步：项目目录结构
在 D 盘或 E 盘创建一个名为 `HMs` 的文件夹，按以下结构组织：
```Plaintext
HMs/
├── CMakeLists.txt        # 编译配置文件
├── main.cpp              # C++ 推理代码
├── json.hpp              # JSON 解析库
├── xgboost.dll           # 从外部复制过来的动态库
├── data_test.csv         # 待预测数据
├── include/                <-- (新建这个文件夹)
│   └── xgboost/            <-- (从刚才下载的源码包里复制过来)
│       ├── c_api.h         <-- (确保这个文件在这里!)
│       └── ... (其他头文件)
└── model/
    ├── v5_xgb_model.json # Python 导出的模型
    └── scaler_params.json# Python 导出的标准化参数
```
## 第三步：编写 `CMakeLists.txt`
## 第四步：编写 `main.cpp` (核心代码)
## 第五步：在 VS Code 中编译运行
1. **打开项目**: 在 VS Code 中“打开文件夹”选择 `HMs`。
2. **配置 CMake**:
- 点击底部状态栏的 **CMake: [Release]: Ready** 按钮（或按 `Ctrl+Shift+P` 输入 `CMake: Configure`）。
- 选择你的编译器（例如 `GCC 13.x.x x86_64-w64-mingw32`）。
3. **编译**:
- 点击底部状态栏的 **Build**。
4. **运行**:
- 将 `xgboost.dll` 确保放在 `build` 文件夹中生成的 `.exe` 旁边。
- 点击底部的 **小播放按钮** 运行预测。
## 第六步：执行
### 第一步：找到生成的可执行文件
在 VS Code 的左侧文件资源管理器中，展开 `build` 文件夹。通常情况下，路径是这样的：
- `HMs/build/HMs_Predict.exe`
- 或者 `HMs/build/Debug/HMs_Predict.exe` (取决于你的 CMake 配置)
### 第二步：集结所有必要文件
为了防止“找不到文件”的错误，我们需要把所有依赖文件复制到 `.exe` **所在的同一个文件夹**中。

假设你的 `.exe` 在 `HMs/build/` 下，请确保该文件夹内包含以下所有文件：
1. **HMs_Predict.exe** (这是刚编译出来的)
2. **xgboost.dll** (从你的 python 目录或项目根目录复制到这里)
3. **data_test.csv** (待预测数据)
4. **model 文件夹** (整个文件夹复制过来，里面包含 json 文件)

或者将`HMs_Predict.exe` 从 `build` 中复制出来，总之，以上四种文件要位于同一目录下。
### 第三步：执行预测
推荐使用 **VS Code 的终端 (Terminal)** 来运行，这样能清晰地看到输出结果。
1. **打开终端**：在 VS Code 中按 Ctrl + ` (反引号键) 打开终端。
2. **进入目录**：使用 `cd` 命令进入包含 exe 的目录。
3. **运行程序**： 输入`.\HMs_Predict.exe`命令并回车。