#include "mainwindow.h"
#include <QApplication>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QTextCodec>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <time.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    srand(time(NULL));
    
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = 1;
    
    QTextCodec *codec=QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForCStrings(codec);
    //QTextCodec::setCodecForTr(codec);
    MainWindow w;
    //w.showFullScreen();
    w.show();

    return a.exec();
}
