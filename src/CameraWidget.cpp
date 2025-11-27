#include "CameraWidget.h"
#include <ZXing/ReadBarcode.h>
#include <QMessageBox>
#include <QDateTime>

CameraWidget::CameraWidget(QWidget* parent)
    : QWidget(parent)
    , capture(nullptr)
    , timer(nullptr)
    , videoLabel(nullptr)
    , statusLabel(nullptr)
    , mainLayout(nullptr)
    , cameraStarted(false)
{
    // 设置UI
    setWindowTitle("摄像头预览");
    setMinimumSize(800, 600);

    mainLayout = new QVBoxLayout(this);

    // 视频显示区域
    videoLabel = new QLabel(this);
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setStyleSheet("QLabel { background-color: black; }");
    videoLabel->setMinimumSize(640, 480);
    mainLayout->addWidget(videoLabel);

    // 状态标签
    statusLabel = new QLabel("准备启动摄像头...", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("QLabel { color: blue; padding: 5px; }");
    mainLayout->addWidget(statusLabel);

    // 初始化摄像头
    startCamera();
}

CameraWidget::~CameraWidget()
{
    stopCamera();
}

void CameraWidget::startCamera()
{
    if (cameraStarted) return;

    // 初始化摄像头
    capture = new cv::VideoCapture(0);

    if (!capture->isOpened()) {
        statusLabel->setText("无法打开摄像头");
        QMessageBox::warning(this, "错误", "无法打开摄像头");
        delete capture;
        capture = nullptr;
        return;
    }

    // 设置摄像头参数
    capture->set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture->set(cv::CAP_PROP_FPS, 30);

    // 创建定时器用于更新帧
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CameraWidget::updateFrame);
    timer->start(33);  // 约30fps

    cameraStarted = true;
    statusLabel->setText("摄像头已启动");
}

void CameraWidget::stopCamera()
{
    if (timer && timer->isActive()) {
        timer->stop();
    }

    if (capture && capture->isOpened()) {
        capture->release();
    }

    delete timer;
    delete capture;

    timer = nullptr;
    capture = nullptr;
    cameraStarted = false;

    statusLabel->setText("摄像头已停止");
    videoLabel->clear();
}

void CameraWidget::updateFrame()
{
    if (!capture || !capture->isOpened()) return;

    cv::Mat frame;
    *capture >> frame;

    if (frame.empty()) {
        statusLabel->setText("捕获到空帧");
        return;
    }

    // 处理帧（绘制文字等）
    processFrame(frame);

    // 转换为Qt图像格式
    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);

    QImage qtImage(rgbFrame.data,
        rgbFrame.cols,
        rgbFrame.rows,
        rgbFrame.step,
        QImage::Format_RGB888);

    // 显示图像
    QPixmap pixmap = QPixmap::fromImage(qtImage);
    videoLabel->setPixmap(pixmap.scaled(videoLabel->width(),
        videoLabel->height(),
        Qt::KeepAspectRatio));
}

inline ZXing::ImageView ImageViewFromMat(const cv::Mat& image)
{
    using ZXing::ImageFormat;

    auto fmt = ImageFormat::None;
    switch (image.channels()) {
    case 1: fmt = ImageFormat::Lum; break;
    case 2: fmt = ImageFormat::LumA; break;
    case 3: fmt = ImageFormat::BGR; break;
    case 4: fmt = ImageFormat::BGRA; break;
    }

    if (image.depth() != CV_8U || fmt == ImageFormat::None)
        return { nullptr, 0, 0, ImageFormat::None };

    return { image.data, image.cols, image.rows, fmt };
}

inline ZXing::Barcodes ReadBarcodes(const cv::Mat& image, const ZXing::ReaderOptions& options = {})
{
    return ZXing::ReadBarcodes(ImageViewFromMat(image), options);
}

inline void DrawBarcode(cv::Mat& img, ZXing::Barcode barcode)
{
    auto pos = barcode.position();
    auto zx2cv = [](ZXing::PointI p) { return cv::Point(p.x, p.y); };
    auto contour = std::vector<cv::Point>{ zx2cv(pos[0]), zx2cv(pos[1]), zx2cv(pos[2]), zx2cv(pos[3]) };
    const auto* pts = contour.data();
    int npts = contour.size();

    cv::polylines(img, &pts, &npts, 1, true, CV_RGB(0, 255, 0));
    cv::putText(img, barcode.text(), zx2cv(pos[3]) + cv::Point(0, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(0, 255, 0));
}


void CameraWidget::processFrame(cv::Mat& frame)
{
    const auto barcodes = ReadBarcodes(frame);
    for (const auto& barcode : barcodes)
        DrawBarcode(frame, barcode);
}