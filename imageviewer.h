#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QLabel>
#include <QDir>
#include <QVBoxLayout>
#include <QQueue>
#include <QMutex>

QT_BEGIN_NAMESPACE
namespace Ui { class ImageViewer; }
QT_END_NAMESPACE

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer(int argc, char *argv[], QWidget *parent = nullptr);
    ~ImageViewer();
    void keyPressEvent(QKeyEvent *event);
    bool setRootDir(QDir dir, int depth, int index0, int index1);
    void loadImage(QString filename);
    void loadGIF(QString filename);
    void setImageName(int index0, int index1, QFileInfo filename);
    void setIndex(int index0, int index1, int index2);
    void setIndex(int depth, int index);
    void handleFunctionKeys(QKeyEvent *event);
    void setTitle();
    void setBackground();
    void randomizeFile(int depth);
    void sortFile(int depth);
    void setUI();
    void searchFile(int depth, QString str);
    void applyCommand();
    void appendImageLoading(QVector<int> indexes);
    void resizeEvent(QResizeEvent* event);
private:
    Ui::ImageViewer *ui;
    QLabel* label;
    QVBoxLayout *layout;
    QPalette pal;
    QVector<QVector<QVector<QFileInfo>>> images;
    QMap<QString, QPixmap> savedImages;
    QVector<QVector<QVector<QFileInfo>>> randImages;
    int index0, index1, index2;
    int moviespeed;
    bool useRandom;
    bool functionKey;
    QString command, rootDir;
    QQueue<QVector<int>> imageLoading;
    QSet<QVector<int>> imageLoaded;
    QMutex imageMutex;
    void startThread();
    QMovie* movie;
};
#endif // IMAGEVIEWER_H
