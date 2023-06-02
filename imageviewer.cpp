#include "imageviewer.h"
#include "ui_imageviewer.h"
#include <QKeyEvent>
#include <QDir>
#include <QLabel>
#include <QDirIterator>
#include <QThread>
#include <QString>
#include <QVBoxLayout>
#include <QPalette>
#include <QMovie>
#include <algorithm>
#include <QMutexLocker>
#include <opencv2/opencv.hpp>

ImageViewer::ImageViewer(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageViewer)
    , label(new QLabel(this))
    , pal(palette())
    , layout(new QVBoxLayout(this->centralWidget()))
    , index0(0), index1(0), index2(0)
    , useRandom(false)
    , command("")
    , rootDir("E:\\Pictures\\PIC\\")
    , movie(NULL)
{
    if(argc>1){
        QFileInfo fileInfo(QString::fromUtf8(argv[1]));
        rootDir = fileInfo.absolutePath();
    }
    for(auto& c: imageLoading)
        imageLoaded.insert(c);

    QImageReader::setAllocationLimit(0);
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
    setRootDir(QDir(rootDir), 1, -1, -1);
    setIndex(0, 0, 0);
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    layout->addWidget(label);
    this->setCentralWidget(label);
    this->show();
    showMaximized();
}

ImageViewer::~ImageViewer()
{
    delete ui;
}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);
   setUI();
}
void ImageViewer::appendImageLoading(QVector<int> indexes)
{
    if(indexes[0] >= images.size() || indexes[0] < 0) return;
    if(indexes[1] >= images[indexes[0]].size()) return;
    if(indexes[2] >= images[indexes[0]][indexes[1]].size()) return;
    if(imageLoaded.contains(indexes)) return;
    imageLoading.append(indexes);
    imageLoaded.insert(indexes);
}
void ImageViewer::startThread()
{

    QThread *thread = new QThread(this);

    connect(thread, &QThread::started, [=]() {
        QVector<int> indexes;
        while(!imageLoading.empty()){
            imageMutex.lock();
            indexes = imageLoading.front();
            imageLoading.pop_front();
            appendImageLoading({indexes[0]+1, 0, 0});
            appendImageLoading({indexes[0]-1, 0, 0});
            appendImageLoading({indexes[0], indexes[1]+1, 0});
            appendImageLoading({indexes[0], indexes[1], indexes[2]+1});
            imageMutex.unlock();
            QString filename = images[indexes[0]][indexes[1]][indexes[2]].filePath();
            QString ext = images[indexes[0]][indexes[1]][indexes[2]].suffix().toLower();
            if(ext != "gif")
                savedImages[filename] = QPixmap(filename);
            qDebug() << filename ;
            qDebug() <<indexes;
        }
    });

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}
void ImageViewer::setTitle(){

    if(command.isEmpty()){
        QString title = QString("Image Viewer [%1/%2] [%3/%4] [%5/%6]")
                .arg(QString::number(index0+1), QString::number(images.size()),
                     QString::number(index1+1), QString::number(images[index0].size()),
                     QString::number(index2+1), QString::number(images[index0][index1].size()));
        qDebug() <<title;
        qDebug() <<images[index0][index1][index2].filePath();
        this->setWindowTitle(title);
    }
    else{
        this->setWindowTitle(command);
    }
}
void ImageViewer::loadImage(QString filename){
    QPixmap pixmap;
    //if(!savedImages.contains(filename)) savedImages[filename] = QPixmap(filename);
    //pixmap = savedImages[filename];
    if(filename.endsWith(".webp") || filename.endsWith(".tiff")){
        QImageReader reader;
        cv::Mat img = cv::imread((filename.toLocal8Bit().toStdString()));
        QImage image(img.data, img.cols, img.rows, img.step, QImage::Format_BGR888);
        pixmap = QPixmap::fromImage(image);
    }
    else{
        pixmap = QPixmap(filename);
    }
    double scaled_multi = fmin(1.0 * this->size().width() / pixmap.size().width(), 1.0 * this->size().height()/pixmap.size().height());
    scaled_multi = fmin(scaled_multi, 1);
    QPixmap scaledPixmap = pixmap.scaled(pixmap.width() * scaled_multi, pixmap.height() * scaled_multi, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    label->setPixmap(scaledPixmap);
}
void ImageViewer::loadGIF(QString filename){
    moviespeed = 100;
    movie = new QMovie(filename);
    movie->start();
    movie->stop();
    QImage pixmap = movie->currentImage();
    double scaled_multi = fmin(1.0 * this->size().width() / pixmap.size().width(), 1.0 * this->size().height()/pixmap.size().height());
    scaled_multi = fmin(scaled_multi, 1);
    movie->setScaledSize({int(pixmap.width() * scaled_multi), int(pixmap.height() * scaled_multi)});
    movie->jumpToFrame(0);
    label->setMovie(movie);
    movie->start();

}
void ImageViewer::setBackground(){
    if(images[index0][index1].size() != 1){
        pal.setColor(QPalette::Window, QColor(0xcccccc));
        setPalette(pal);
    }
    else if(images[index0].size() != 1){
        pal.setColor(QPalette::Window, Qt::white);
        setPalette(pal);
    }
    else{
        pal.setColor(QPalette::Window, Qt::black);
        setPalette(pal);
    }
}

void ImageViewer::setIndex(int index0, int index1, int index2){
    this->index0 = fmin(fmax(index0, 0), images.size()-1);
    this->index1 = fmin(fmax(index1, 0), images[this->index0].size()-1);
    this->index2 = fmin(fmax(index2, 0), images[this->index0][this->index1].size()-1);
    setUI();
}
void ImageViewer::setIndex(int depth, int index){
    if(depth == 1){
        index0 = fmin(fmax(index, 0), images.size()-1);
        index1 = 0;
        index2 = 0;
    }
    if(depth == 2){
        index1 = fmin(fmax(index, 0), images[this->index0].size()-1);
        index2 = 0;
    }
    if(depth == 3){
        index2 = fmin(fmax(index2, 0), images[this->index0][this->index1].size()-1);;
    }
    setUI();
}
void ImageViewer::setUI(){
    if(movie != NULL) delete movie, movie = NULL;
    setTitle();
    setBackground();
    QString ext = images[this->index0][this->index1][this->index2].suffix().toLower();
    if(ext == "gif")
        loadGIF(images[this->index0][this->index1][this->index2].filePath());
    else
        loadImage(images[this->index0][this->index1][this->index2].filePath());
}
void ImageViewer::randomizeFile(int depth){
    qDebug() << "shuffle";
    std::random_shuffle(images[0][0].begin(), images[0][0].end());
    if(depth == 1) {
        std::random_shuffle(images.begin(), images.end());
        setIndex(0, 0, 0);
    }
    else if(depth == 2){
        std::random_shuffle(images[index0].begin(), images[index0].end());
        setIndex(index0, 0, 0);
    }
    else {
        std::random_shuffle(images[index0][index1].begin(), images[index0][index1].end());
        setIndex(index0, index1, 0);
    }
}
void ImageViewer::searchFile(int depth, QString str){
    if(depth == 1){
        for(int i=0;i<images.size();i++){
            auto filename = images[i][0][0].filePath().sliced(rootDir.size()).split('/');
            if(filename.size() > 0 && filename[0].toUpper().startsWith(str)){
                setIndex(1, i);
                return;
            }
        }
    }
    if(depth == 2){
        for(int i=0;i<images[index0].size();i++){
            auto filename = images[index0][i][0].filePath().split('/');
            if(filename.size() > 1 && filename[1].toUpper().startsWith(str)){
                setIndex(2, i);
                return;
            }
        }
    }
    if(depth == 3){
        for(int i=0;i<images[index0][index1].size();i++){
            auto filename = images[index0][index1][i].filePath().split('/');
            if(filename.size() > 2 && filename[2].toUpper().startsWith(str)){
                setIndex(3, i);
                return;
            }
        }
    }
}
void ImageViewer::applyCommand(){
    int index = 0;
    bool ok = false;
    while(index < command.size() && command[index] == '/') index++;
    if(index != 0){
        searchFile(index, command.sliced(index));
        command = "";
        return;
    }
    while(index < command.size() && command[index] == ';') index++;
    if(index != 0){
        int tar = command.sliced(index).toInt(&ok);
        if(ok) setIndex(index, tar);
        command = "";
        return;
    }

}
void ImageViewer::handleFunctionKeys(QKeyEvent *event){
    qDebug()<<event->key();
    qDebug()<<(event->key()< 256);
    switch(event->key()){
    case Qt::Key_Return:
    case Qt::Key_Enter:
        applyCommand();
        setTitle();
        break;
    case Qt::Key_Backspace:
        if(!command.isEmpty())
            command.chop(1);
        setTitle();
        break;
    default:
        if(event->key() < 256){
            command.push_back(QChar(event->key()));
            setTitle();
        }
    }
}
void ImageViewer::keyPressEvent(QKeyEvent *event)
{
    if(command.isEmpty()){
        switch(event->key()){
        //navigate
        case Qt::Key_Y: setIndex(0, 0, 0); break;
        case Qt::Key_U: setIndex(index0-1, 0, 0); break;
        case Qt::Key_I: setIndex(index0+1, 0, 0); break;
        case Qt::Key_O: setIndex(images.size()-1, 0, 0); break;
        case Qt::Key_H: setIndex(index0, 0, 0); break;
        case Qt::Key_J: setIndex(index0, index1-1, 0); break;
        case Qt::Key_K: setIndex(index0, index1+1, 0); break;
        case Qt::Key_L: setIndex(index0, images[index0].size() - 1, 0); break;
        case Qt::Key_N: setIndex(index0, index1, 0); break;
        case Qt::Key_M: setIndex(index0, index1, index2-1); break;
        case Qt::Key_Comma: setIndex(index0, index1, index2+1); break;
        case Qt::Key_Period: setIndex(index0, index1, images[index0][index1].size() - 1); break;
        case Qt::Key_Minus:
            moviespeed -= 10;
            if(movie != NULL)
                movie->setSpeed(moviespeed);
            break;
        case Qt::Key_Equal:
            moviespeed += 10;
            if(movie != NULL)
                movie->setSpeed(moviespeed);
            break;


        //random
        case Qt::Key_7:
            if(!useRandom) {
                useRandom = !useRandom;
                images.swap(randImages);
            }
            randomizeFile(1);
            break;
        case Qt::Key_8:
            if(!useRandom) {
                useRandom = !useRandom;
                images.swap(randImages);
            }
            randomizeFile(2);
            break;
        case Qt::Key_9:
            if(!useRandom) {
                useRandom = !useRandom;
                images.swap(randImages);
            }
            randomizeFile(3);
            break;
        case Qt::Key_0:
            if(useRandom) {
                useRandom = !useRandom;
                images.swap(randImages);
            }
            setUI();
            break;

        //function
        case Qt::Key_Slash:
        case Qt::Key_Semicolon:
            handleFunctionKeys(event);
            break;
        }
    }
    else{
        handleFunctionKeys(event);
    }

}

void ImageViewer::setImageName(int index0, int index1, QFileInfo filename){
    while(images.size() <= index0) {images.append(QVector<QVector<QFileInfo>>());randImages.append(QVector<QVector<QFileInfo>>());}
    while(images[index0].size() <= index1) {images[index0].append(QVector<QFileInfo>());randImages[index0].append(QVector<QFileInfo>());}
    images[index0][index1].append(filename);
    randImages[index0][index1].append(filename);
}

bool ImageViewer::setRootDir(QDir dir, int depth, int index0, int index1){
    int index = 0;
    QFileInfoList fileInfoList;
    if(depth != 3)
        fileInfoList = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    else
        fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);


    std::sort(fileInfoList.begin(), fileInfoList.end(), [](const QFileInfo& a, const QFileInfo& b) {
        return a.lastModified() < b.lastModified();
    });
    foreach (QFileInfo fileInfo, fileInfoList) {
        bool added = false;
        if (fileInfo.isDir()) {
            if(index0 >= 0){
                if(setRootDir(dir.filePath(fileInfo.filePath()), depth+1, index0, index)) added = true;
            }
            else{
                if(setRootDir(dir.filePath(fileInfo.filePath()), depth+1, index, -1)) added = true;
            }
        } else {
            // 如果是檔案，檢查是否是圖片檔
            QString ext = fileInfo.suffix().toLower();
            if (ext == "jpg" || ext == "jpeg" || ext == "gif" || ext == "png" || ext == "jfif" || ext == "webp" || ext == "bmp" || ext == "tiff") {
                if(index1 >= 0)
                    setImageName(index0, index1, fileInfo);
                else if(index0 >= 0)
                    setImageName(index0, index, fileInfo);
                else
                    setImageName(index, 0, fileInfo);
                added = true;
            }
        }
        if(added) index++;
    }
    return index != 0;
}
