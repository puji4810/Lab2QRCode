#include "ImageSizeConfig.h"
#include "../logging.h"
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

std::string ImageSizeConfig::unitToString(SizeUnit unit) {
    switch (unit) {
    case SizeUnit::Pixel: return "pixel";
    case SizeUnit::Centimeter: return "centimeter";
    default: return "pixel";
    }
}

SizeUnit ImageSizeConfig::stringToUnit(const std::string &str) {
    if (str == "centimeter" || str == "cm") {
        return SizeUnit::Centimeter;
    }
    return SizeUnit::Pixel;
}

int ImageSizeConfig::cmToPixels(double cm, int ppi) {
    // 公式: 像素值 = (厘米值 / 2.54) * PPI
    return static_cast<int>(std::round((cm / 2.54) * ppi));
}

int ImageSizeConfig::getTargetWidthPixels() const {
    if (unit == SizeUnit::Centimeter) {
        return cmToPixels(width, ppi);
    }
    return static_cast<int>(std::round(width));
}

int ImageSizeConfig::getTargetHeightPixels() const {
    if (unit == SizeUnit::Centimeter) {
        return cmToPixels(height, ppi);
    }
    return static_cast<int>(std::round(height));
}

ImageSizeConfig ImageSizeConfig::loadFromConfig(const std::string &filename) {
    ImageSizeConfig config;
    // 设置默认值
    config.unit = SizeUnit::Pixel;
    config.ppi = 300;
    config.width = 300.0;
    config.height = 300.0;

    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            spdlog::warn("Config file not found, using default image size config: {}", filename);
            return config;
        }

        json configJson;
        file >> configJson;

        if (configJson.contains("image_size")) {
            const auto &imgSize = configJson["image_size"];

            if (imgSize.contains("unit")) {
                config.unit = stringToUnit(imgSize["unit"].get<std::string>());
            }

            if (imgSize.contains("ppi")) {
                config.ppi = imgSize["ppi"].get<int>();
            }

            if (imgSize.contains("width")) {
                config.width = imgSize["width"].get<double>();
            }

            if (imgSize.contains("height")) {
                config.height = imgSize["height"].get<double>();
            }

            spdlog::info("Loaded image size config: unit={}, ppi={}, width={}, height={}",
                         unitToString(config.unit),
                         config.ppi,
                         config.width,
                         config.height);
        } else {
            spdlog::info("No image_size section in config, using defaults");
        }
    } catch (const std::exception &e) { spdlog::error("Failed to load image size config: {}", e.what()); }

    return config;
}

bool ImageSizeConfig::saveToConfig(const std::string &filename, const ImageSizeConfig &config) {
    try {
        // 读取现有配置
        json configJson;
        std::ifstream inFile(filename);
        if (inFile.is_open()) {
            inFile >> configJson;
            inFile.close();
        }

        // 更新image_size部分
        configJson["image_size"] = {
            {"unit",   unitToString(config.unit)},
            {"ppi",    config.ppi               },
            {"width",  config.width             },
            {"height", config.height            }
        };

        // 写回文件
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            spdlog::error("Failed to open config file for writing: {}", filename);
            return false;
        }

        outFile << configJson.dump(4); // 格式化输出，缩进4个空格
        outFile.close();

        spdlog::info("Saved image size config: unit={}, ppi={}, width={}, height={}",
                     unitToString(config.unit),
                     config.ppi,
                     config.width,
                     config.height);
        return true;
    } catch (const std::exception &e) {
        spdlog::error("Failed to save image size config: {}", e.what());
        return false;
    }
}
