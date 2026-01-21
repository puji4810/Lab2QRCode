#ifndef IMAGESIZECONFIG_H
#define IMAGESIZECONFIG_H

#include <string>

/**
 * @brief 尺寸单位枚举
 */
enum class SizeUnit {
    Pixel,     /**< 像素 */
    Centimeter /**< 厘米 */
};

/**
 * @brief 图像尺寸配置结构体
 */
struct ImageSizeConfig {
    SizeUnit unit = SizeUnit::Pixel; /**< 尺寸单位 */
    int ppi = 300;                   /**< 每英寸像素数（Pixels Per Inch） */
    double width = 300.0;            /**< 宽度值（根据unit可能是像素或厘米） */
    double height = 300.0;           /**< 高度值（根据unit可能是像素或厘米） */

    /**
     * @brief 将尺寸单位转换为字符串
     * @param unit 尺寸单位
     * @return 字符串表示
     */
    static std::string unitToString(SizeUnit unit);

    /**
     * @brief 从字符串解析尺寸单位
     * @param str 字符串表示
     * @return 尺寸单位
     */
    static SizeUnit stringToUnit(const std::string &str);

    /**
     * @brief 计算目标像素宽度
     * @return 像素宽度
     */
    int getTargetWidthPixels() const;

    /**
     * @brief 计算目标像素高度
     * @return 像素高度
     */
    int getTargetHeightPixels() const;

    /**
     * @brief 从配置文件加载图像尺寸配置
     * @param filename 配置文件路径
     * @return 图像尺寸配置
     */
    static ImageSizeConfig loadFromConfig(const std::string &filename);

    /**
     * @brief 保存图像尺寸配置到配置文件
     * @param filename 配置文件路径
     * @param config 图像尺寸配置
     * @return 是否保存成功
     */
    static bool saveToConfig(const std::string &filename, const ImageSizeConfig &config);

private:
    /**
     * @brief 将厘米转换为像素
     * @param cm 厘米值
     * @param ppi 每英寸像素数
     * @return 像素值
     */
    static int cmToPixels(double cm, int ppi);
};

#endif // IMAGESIZECONFIG_H
