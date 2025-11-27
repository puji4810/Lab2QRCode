#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <opencv2/opencv.hpp>

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget* parent = nullptr);
    ~CameraWidget();

public slots:
    void startCamera();
    void stopCamera();

private slots:
    void updateFrame();

private:
    void processFrame(cv::Mat& frame);

    cv::VideoCapture* capture;
    QTimer* timer;
    QLabel* videoLabel;
    QLabel* statusLabel;
    QVBoxLayout* mainLayout;
    bool cameraStarted;
};

#endif // CAMERAWIDGET_H