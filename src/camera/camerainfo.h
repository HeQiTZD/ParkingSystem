#ifndef CAMERAINFO_H
#define CAMERAINFO_H
#include <QString>
struct CameraInfo {
    int index = 0;
    QString name;
    QString location;
    QString role;
    int width  = 1920;
    int height = 1080;
    int fps    = 30;
};
#endif