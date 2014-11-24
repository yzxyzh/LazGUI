#include "mainwindow.h"
#include <QApplication>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec *codec=QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForCStrings(codec);
    //QTextCodec::setCodecForTr(codec);
    MainWindow w;
    w.show();

    return a.exec();
}
