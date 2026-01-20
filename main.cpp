#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include "json.hpp"         // nlohmann/json 库
#include <xgboost/c_api.h>  // XGBoost C API 头文件

using json = nlohmann::json;

// 检查 XGBoost API 调用错误的宏
#define SAFE_XGB(call) { \
    int err = (call); \
    if (err != 0) { \
        fprintf(stderr, "%s:%d: XGBoost Error: %s\n", __FILE__, __LINE__, XGBGetLastError()); \
        exit(1); \
    } \
}

// 模拟 Python 中的 StandardScaler
struct Scaler {
    std::vector<double> mean;
    std::vector<double> scale;
};

// 计算光谱指数 (必须与 Python 中的逻辑和顺序严格一致)
std::vector<float> calculate_indices(const std::vector<float>& b) {
    // 原始波段: 0:B01, 1:B02, 2:B03, 3:B04, 4:B05, 5:B06, 6:B07, 7:B09, 8:B11, 9:B12, 10:B8A
    std::vector<float> f = b; 
    float eps = 1e-10f;

    // 1. NDVI (B8A, B04)
    f.push_back((b[10] - b[3]) / (b[10] + b[3] + eps));
    // 2. NDWI (B03, B8A)
    f.push_back((b[2] - b[10]) / (b[2] + b[10] + eps));
    // 3. SAVI (L=0.5)
    float L = 0.5f;
    f.push_back((1.0f + L) * (b[10] - b[3]) / (b[10] + b[3] + L + eps));
    // 4. CI (B04, B03)
    f.push_back((b[3] - b[2]) / (b[3] + b[2] + eps));
    // 5. B11_B12_ratio
    f.push_back(b[8] / (b[9] + eps));
    // 6. B05_B06_ratio
    f.push_back(b[4] / (b[5] + eps));
    // 7. B11_B12_diff
    f.push_back(b[8] - b[9]);

    return f;
}

int main() {
    try {
        // --- 1. 初始化模型 ---
        BoosterHandle booster;
        SAFE_XGB(XGBoosterCreate(NULL, 0, &booster));
        // 加载 Python 导出的 JSON 模型
        SAFE_XGB(XGBoosterLoadModel(booster, "model/v5_xgb_model.json"));

        // --- 2. 加载标准化参数 ---
        std::ifstream sc_file("model/scaler_params.json");
        if (!sc_file.is_open()) throw std::runtime_error("Cannot find scaler_params.json");
        json j_sc; 
        sc_file >> j_sc;
        Scaler sc = { j_sc["mean"].get<std::vector<double>>(), j_sc["scale"].get<std::vector<double>>() };

        // --- 3. 读取待预测数据 ---
        std::ifstream data_file("data_test.csv");
        if (!data_file.is_open()) throw std::runtime_error("Cannot find data_test.csv");
        
        std::string line;
        std::getline(data_file, line); // 跳过 CSV 标题行

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "  Soil Heavy Metal (Cd) Prediction Result " << std::endl;
        std::cout << "------------------------------------------" << std::endl;

        // --- 4. 逐行预测 ---
        int row_count = 0;
        while (std::getline(data_file, line)) {
            if (line.empty()) continue;
            
            std::stringstream ss(line);
            std::string item;
            std::vector<float> raw_bands;
            int col = 0;

            // 提取第 4 到 14 列 (索引 3-13) 并转换 DN 值为反射率
            while (std::getline(ss, item, ',')) {
                if (col >= 3 && col <= 13) {
                    raw_bands.push_back(std::stof(item) / 10000.0f);
                }
                col++;
            }

            if (raw_bands.size() < 11) continue;

            // 特征工程：增加光谱指数 (总数应为 11 + 7 = 18)
            std::vector<float> features = calculate_indices(raw_bands);

            // 数据标准化
            for (size_t i = 0; i < features.size(); ++i) {
                features[i] = (features[i] - (float)sc.mean[i]) / (float)sc.scale[i];
            }

            // 调用 XGBoost 进行推理
            DMatrixHandle dmatrix;
            SAFE_XGB(XGDMatrixCreateFromMat(features.data(), 1, features.size(), -1.0, &dmatrix));
            
            bst_ulong out_len;
            const float* out_result;
            SAFE_XGB(XGBoosterPredict(booster, dmatrix, 0, 0, 0, &out_len, &out_result));

            // 逆对数变换还原真实浓度
            float prediction = std::exp(out_result[0]);

            std::cout << "Row " << ++row_count << " -> Predicted Cd: " << prediction << " mg/kg" << std::endl;

            // 释放当前行的 DMatrix 内存
            SAFE_XGB(XGDMatrixFree(dmatrix));
        }

        // 释放模型句柄
        SAFE_XGB(XGBoosterFree(booster));
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "Prediction task finished." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}