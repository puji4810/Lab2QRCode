/**
 * @file barcode_api.cpp
 * @brief Lab2QRCode WASM API - 条码编解码核心接口
 * 
 * 提供给 JavaScript 调用的 WASM 接口，包括：
 * - 条码生成（编码）
 * - 条码识别（解码）
 * - Base64 编解码
 * - 图像处理辅助功能
 */

#include <SimpleBase64.h>
#include <ZXing/BarcodeFormat.h>
#include <ZXing/BitMatrix.h>
#include <ZXing/CharacterSet.h>
#include <ZXing/MultiFormatWriter.h>
#include <ZXing/ReadBarcode.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <memory>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>
#include <vector>

using namespace emscripten;

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 将 ZXing::BitMatrix 转换为 PNG 格式的图像数据（Base64）
 * @param matrix ZXing 生成的位矩阵
 * @param scale 放大倍数
 * @return Base64 编码的 PNG 图像数据
 */
std::string bitMatrixToPngBase64(const ZXing::BitMatrix &matrix, int scale = 4) {
    int width = matrix.width();
    int height = matrix.height();

    // 创建 OpenCV 图像（白色背景）
    cv::Mat img(height * scale, width * scale, CV_8UC1, cv::Scalar(255));

    // 填充黑色像素
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (matrix.get(x, y)) {
                cv::rectangle(img,
                              cv::Point(x * scale, y * scale),
                              cv::Point((x + 1) * scale, (y + 1) * scale),
                              cv::Scalar(0),
                              -1);
            }
        }
    }

    // 编码为 PNG
    std::vector<uchar> buffer;
    cv::imencode(".png", img, buffer);

    // 转换为 Base64
    return SimpleBase64::encode(buffer.data(), buffer.size());
}

/**
 * @brief 将 JavaScript Uint8Array 转换为 cv::Mat
 * @param data 图像数据（Uint8Array）
 * @return OpenCV Mat 对象
 */
cv::Mat uint8ArrayToMat(const val &jsData) {
    // 获取数据长度
    unsigned int length = jsData["length"].as<unsigned int>();

    // 创建 vector 并复制数据
    std::vector<uchar> data(length);
    val memory = val::module_property("HEAPU8");
    val memoryView = jsData["constructor"].new_(memory["buffer"], reinterpret_cast<uintptr_t>(data.data()), length);
    memoryView.call<void>("set", jsData);

    // 解码图像
    cv::Mat img = cv::imdecode(data, cv::IMREAD_COLOR);
    if (img.empty()) {
        throw std::runtime_error("Failed to decode image data");
    }

    return img;
}

/**
 * @brief 将 cv::Mat 转换为 ZXing::ImageView
 */
ZXing::ImageView matToImageView(const cv::Mat &image) {
    using ZXing::ImageFormat;
    auto fmt = ImageFormat::None;

    switch (image.channels()) {
    case 1: fmt = ImageFormat::Lum; break;
    case 3: fmt = ImageFormat::BGR; break;
    case 4: fmt = ImageFormat::BGRA; break;
    default: throw std::runtime_error("Unsupported image format");
    }

    if (image.depth() != CV_8U) {
        throw std::runtime_error("Image must be 8-bit");
    }

    return {image.data, image.cols, image.rows, fmt};
}

// ============================================================================
// Base64 编解码接口
// ============================================================================

/**
 * @brief Base64 编码
 * @param data 原始数据（字符串）
 * @return Base64 编码后的字符串
 */
std::string base64Encode(const std::string &data) {
    return SimpleBase64::encode(reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

/**
 * @brief Base64 解码
 * @param encoded Base64 编码的字符串
 * @return 解码后的原始数据（字符串）
 */
std::string base64Decode(const std::string &encoded) {
    auto decoded = SimpleBase64::decode(encoded);
    return std::string(decoded.begin(), decoded.end());
}

// ============================================================================
// 条码格式转换
// ============================================================================

/**
 * @brief 将字符串转换为 ZXing::BarcodeFormat
 */
ZXing::BarcodeFormat stringToBarcodeFormat(const std::string &format) {
    static const std::map<std::string, ZXing::BarcodeFormat> formatMap = {
        {"QRCode",          ZXing::BarcodeFormat::QRCode         },
        {"MicroQRCode",     ZXing::BarcodeFormat::MicroQRCode    },
        {"RMQRCode",        ZXing::BarcodeFormat::RMQRCode       },
        {"Aztec",           ZXing::BarcodeFormat::Aztec          },
        {"DataMatrix",      ZXing::BarcodeFormat::DataMatrix     },
        {"PDF417",          ZXing::BarcodeFormat::PDF417         },
        {"MaxiCode",        ZXing::BarcodeFormat::MaxiCode       },
        {"Code128",         ZXing::BarcodeFormat::Code128        },
        {"Code39",          ZXing::BarcodeFormat::Code39         },
        {"Code93",          ZXing::BarcodeFormat::Code93         },
        {"Codabar",         ZXing::BarcodeFormat::Codabar        },
        {"EAN8",            ZXing::BarcodeFormat::EAN8           },
        {"EAN13",           ZXing::BarcodeFormat::EAN13          },
        {"UPCA",            ZXing::BarcodeFormat::UPCA           },
        {"UPCE",            ZXing::BarcodeFormat::UPCE           },
        {"ITF",             ZXing::BarcodeFormat::ITF            },
        {"DataBar",         ZXing::BarcodeFormat::DataBar        },
        {"DataBarExpanded", ZXing::BarcodeFormat::DataBarExpanded},
        {"DataBarLimited",  ZXing::BarcodeFormat::DataBarLimited },
        {"DXFilmEdge",      ZXing::BarcodeFormat::DXFilmEdge     },
    };

    auto it = formatMap.find(format);
    if (it == formatMap.end()) {
        throw std::runtime_error("Unknown barcode format: " + format);
    }
    return it->second;
}

// ============================================================================
// 条码生成（编码）接口
// ============================================================================

/**
 * @brief 生成条码图片
 * @param content 要编码的内容
 * @param format 条码格式（如 "QRCode", "Code128" 等）
 * @param width 图像宽度
 * @param height 图像高度
 * @param useBase64 是否对内容进行 Base64 编码
 * @return Base64 编码的 PNG 图像数据
 */
std::string generateBarcode(
    const std::string &content, const std::string &format, int width = 300, int height = 300, bool useBase64 = true) {
    try {
        // 内容预处理
        std::string encodedContent = useBase64 ? base64Encode(content) : content;

        // 转换格式
        auto barcodeFormat = stringToBarcodeFormat(format);

        // 创建 Writer
        ZXing::MultiFormatWriter writer(barcodeFormat);
        writer.setEncoding(ZXing::CharacterSet::UTF8);

        // 生成条码
        auto matrix = writer.encode(encodedContent, width, height);

        // 转换为 PNG Base64
        return bitMatrixToPngBase64(matrix);

    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Barcode generation failed: ") + e.what());
    }
}

// ============================================================================
// 条码识别（解码）接口
// ============================================================================

/**
 * @brief 条码识别结果
 */
struct BarcodeResult {
    bool success;        // 是否成功识别
    std::string content; // 识别内容
    std::string format;  // 条码格式
    std::string error;   // 错误信息

    BarcodeResult()
        : success(false) {}
};

/**
 * @brief 识别条码（从图像数据）
 * @param imageData 图像数据（Uint8Array，JPEG/PNG 格式）
 * @param tryAllFormats 是否尝试所有格式
 * @param useBase64 识别后是否进行 Base64 解码
 * @return 识别结果
 */
BarcodeResult decodeBarcode(const val &imageData, bool tryAllFormats = true, bool useBase64 = true) {
    BarcodeResult result;

    try {
        // 转换图像数据
        cv::Mat img = uint8ArrayToMat(imageData);
        if (img.empty()) {
            result.error = "Invalid image data";
            return result;
        }

        // 转换为 ZXing ImageView
        auto imageView = matToImageView(img);

        // 配置读取选项
        ZXing::DecodeHints hints;
        if (tryAllFormats) {
            hints.setFormats(ZXing::BarcodeFormat::Any);
        }
        hints.setTryHarder(true);
        hints.setTryRotate(true);

        // 识别条码
        auto barcodeResult = ZXing::ReadBarcode(imageView, hints);

        if (barcodeResult.isValid()) {
            result.success = true;
            result.format = ZXing::ToString(barcodeResult.format());

            std::string rawContent = barcodeResult.text();

            // Base64 解码（如果启用）
            if (useBase64) {
                try {
                    result.content = base64Decode(rawContent);
                } catch (...) {
                    // 如果 Base64 解码失败，返回原始内容
                    result.content = rawContent;
                }
            } else {
                result.content = rawContent;
            }
        } else {
            result.error = "No barcode found in image";
        }

    } catch (const std::exception &e) { result.error = std::string("Barcode decoding failed: ") + e.what(); }

    return result;
}

/**
 * @brief 批量识别条码（从图像中识别多个条码）
 * @param imageData 图像数据
 * @param tryAllFormats 是否尝试所有格式
 * @param useBase64 识别后是否进行 Base64 解码
 * @return 识别结果数组
 */
std::vector<BarcodeResult> decodeBarcodes(const val &imageData, bool tryAllFormats = true, bool useBase64 = true) {
    std::vector<BarcodeResult> results;

    try {
        // 转换图像数据
        cv::Mat img = uint8ArrayToMat(imageData);
        if (img.empty()) {
            BarcodeResult error;
            error.error = "Invalid image data";
            results.push_back(error);
            return results;
        }

        // 转换为 ZXing ImageView
        auto imageView = matToImageView(img);

        // 配置读取选项
        ZXing::DecodeHints hints;
        if (tryAllFormats) {
            hints.setFormats(ZXing::BarcodeFormat::Any);
        }
        hints.setTryHarder(true);
        hints.setTryRotate(true);

        // 识别多个条码
        auto barcodeResults = ZXing::ReadBarcodes(imageView, hints);

        for (const auto &br : barcodeResults) {
            if (br.isValid()) {
                BarcodeResult result;
                result.success = true;
                result.format = ZXing::ToString(br.format());

                std::string rawContent = br.text();

                // Base64 解码（如果启用）
                if (useBase64) {
                    try {
                        result.content = base64Decode(rawContent);
                    } catch (...) { result.content = rawContent; }
                } else {
                    result.content = rawContent;
                }

                results.push_back(result);
            }
        }

        if (results.empty()) {
            BarcodeResult error;
            error.error = "No barcodes found in image";
            results.push_back(error);
        }

    } catch (const std::exception &e) {
        BarcodeResult error;
        error.error = std::string("Barcode decoding failed: ") + e.what();
        results.push_back(error);
    }

    return results;
}

// ============================================================================
// Emscripten 绑定
// ============================================================================

EMSCRIPTEN_BINDINGS(Lab2QRCode) {
    // BarcodeResult 结构体
    value_object<BarcodeResult>("BarcodeResult")
        .field("success", &BarcodeResult::success)
        .field("content", &BarcodeResult::content)
        .field("format", &BarcodeResult::format)
        .field("error", &BarcodeResult::error);

    // 注册 vector<BarcodeResult>
    register_vector<BarcodeResult>("BarcodeResultVector");

    // Base64 编解码
    function("base64Encode", &base64Encode);
    function("base64Decode", &base64Decode);

    // 条码生成
    function("generateBarcode", &generateBarcode, allow_raw_pointers());

    // 条码识别
    function("decodeBarcode", &decodeBarcode, allow_raw_pointers());

    function("decodeBarcodes", &decodeBarcodes, allow_raw_pointers());
}
