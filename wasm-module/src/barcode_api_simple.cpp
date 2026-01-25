#include <ZXing/BarcodeFormat.h>
#include <ZXing/BitMatrix.h>
#include <ZXing/CharacterSet.h>
#include <ZXing/MultiFormatWriter.h>
#include <ZXing/ReadBarcode.h>
#include <cstdint>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace emscripten;

namespace SimpleBase64 {
static const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "0123456789+/";

inline std::string encode(const std::uint8_t *data, std::size_t len) {
    std::string ret;
    ret.reserve((len + 2) / 3 * 4);
    int val = 0, valb = -6;
    for (std::size_t i = 0; i < len; ++i) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            ret.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        ret.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (ret.size() % 4) {
        ret.push_back('=');
    }
    return ret;
}

inline std::vector<std::uint8_t> decode(const std::string &str) {
    std::vector<std::uint8_t> ret;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) {
        T[static_cast<unsigned char>(base64_chars[i])] = i;
    }

    int val = 0, valb = -8;
    for (unsigned char c : str) {
        if (T[c] == -1) {
            if (c == '=') {
                break;
            } else {
                continue;
            }
        }
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            ret.push_back(static_cast<std::uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return ret;
}
} // namespace SimpleBase64

std::string base64Encode(const std::string &data) {
    return SimpleBase64::encode(reinterpret_cast<const uint8_t *>(data.data()), data.size());
}

std::string base64Decode(const std::string &encoded) {
    auto decoded = SimpleBase64::decode(encoded);
    return std::string(decoded.begin(), decoded.end());
}

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

std::string bitMatrixToSVG(const ZXing::BitMatrix &matrix, int scale = 4) {
    int width = matrix.width();
    int height = matrix.height();
    int scaledWidth = width * scale;
    int scaledHeight = height * scale;

    std::ostringstream svg;
    svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" ";
    svg << "width=\"" << scaledWidth << "\" height=\"" << scaledHeight << "\">\n";
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (matrix.get(x, y)) {
                svg << "<rect x=\"" << (x * scale) << "\" y=\"" << (y * scale) << "\" width=\"" << scale
                    << "\" height=\"" << scale << "\" fill=\"black\"/>\n";
            }
        }
    }

    svg << "</svg>";
    return svg.str();
}

std::string generateBarcode(
    const std::string &content, const std::string &format, int width = 300, int height = 300, bool useBase64 = true) {
    try {
        std::string encodedContent = useBase64 ? base64Encode(content) : content;
        auto barcodeFormat = stringToBarcodeFormat(format);

        ZXing::MultiFormatWriter writer(barcodeFormat);
        writer.setEncoding(ZXing::CharacterSet::UTF8);

        auto matrix = writer.encode(encodedContent, width, height);

        return bitMatrixToSVG(matrix);

    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Barcode generation failed: ") + e.what());
    }
}

struct BarcodeResult {
    bool success;
    std::string content;
    std::string format;
    std::string error;

    BarcodeResult()
        : success(false) {}
};

ZXing::ImageView createImageView(const val &jsImageData) {
    unsigned int width = jsImageData["width"].as<unsigned int>();
    unsigned int height = jsImageData["height"].as<unsigned int>();

    val data = jsImageData["data"];
    unsigned int dataLength = data["length"].as<unsigned int>();

    std::vector<uint8_t> imageBytes(dataLength);

    for (unsigned int i = 0; i < dataLength; ++i) {
        imageBytes[i] = data[i].as<uint8_t>();
    }

    static std::vector<uint8_t> persistentBuffer;
    persistentBuffer = std::move(imageBytes);

    return ZXing::ImageView(
        persistentBuffer.data(), static_cast<int>(width), static_cast<int>(height), ZXing::ImageFormat::RGBA);
}

BarcodeResult decodeBarcode(const val &imageData, bool tryAllFormats = true, bool useBase64 = true) {
    BarcodeResult result;

    try {
        auto imageView = createImageView(imageData);

        ZXing::ReaderOptions hints;
        if (tryAllFormats) {
            hints.setFormats(ZXing::BarcodeFormat::Any);
        }
        hints.setTryHarder(true);
        hints.setTryRotate(true);

        auto barcodeResult = ZXing::ReadBarcode(imageView, hints);

        if (barcodeResult.isValid()) {
            result.success = true;
            result.format = ZXing::ToString(barcodeResult.format());

            std::string rawContent = barcodeResult.text();

            if (useBase64) {
                try {
                    result.content = base64Decode(rawContent);
                } catch (...) { result.content = rawContent; }
            } else {
                result.content = rawContent;
            }
        } else {
            result.error = "No barcode found in image";
        }

    } catch (const std::exception &e) { result.error = std::string("Barcode decoding failed: ") + e.what(); }

    return result;
}

EMSCRIPTEN_BINDINGS(Lab2QRCode) {
    value_object<BarcodeResult>("BarcodeResult")
        .field("success", &BarcodeResult::success)
        .field("content", &BarcodeResult::content)
        .field("format", &BarcodeResult::format)
        .field("error", &BarcodeResult::error);

    function("base64Encode", &base64Encode);
    function("base64Decode", &base64Decode);
    function("generateBarcode", &generateBarcode, allow_raw_pointers());
    function("decodeBarcode", &decodeBarcode, allow_raw_pointers());
}
