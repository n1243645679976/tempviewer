QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = opencvtest
TEMPLATE = app

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    imageviewer.cpp

HEADERS += \
    imageviewer.h

FORMS += \
    imageviewer.ui


INCLUDEPATH += C:\Users\n1243\Downloads\opencv-4.6.0\build\install\include

LIBS += C:\Users\n1243\Downloads\opencv-4.6.0\build\bin\libopencv_core460.dll
LIBS += C:\Users\n1243\Downloads\opencv-4.6.0\build\bin\libopencv_highgui460.dll
LIBS += C:\Users\n1243\Downloads\opencv-4.6.0\build\bin\libopencv_imgcodecs460.dll
LIBS += C:\Users\n1243\Downloads\opencv-4.6.0\build\bin\libopencv_imgproc460.dll
LIBS += C:\Users\n1243\Downloads\opencv-4.6.0\build\bin\libopencv_features2d460.dll
LIBS += C:\Users\n1243\Downloads\opencv-4.6.0\build\bin\libopencv_calib3d460.dll
