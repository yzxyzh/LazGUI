#pragma once

#include <string>
#include "mainwindow.h"
#include <QObject>
//进行一个procedure计算的独立线程代码
class AlgSolver : public QObject
{
    Q_OBJECT
    
public:
    vector<string> newInstances;
public slots:
    void process();
    
signals:
    void finished();
    void error(QString msg);
    void start(QString msg);
    void end(QString msg);
};


//多线程读取脚本的线程类
class ScriptReader : public QObject
{
    Q_OBJECT
    
public:
    ScriptReader(QString _baseName);

protected:
    QString baseName;
    
    
public slots:
    void process();
    
    
    
signals:
    /**
     *  发送给主线程的消息，代表分线程现在的进度
     *
     *  @param code      0代表无误完成，1代表出现错误
     *  @param error_msg 如果code == 1，这里代表错误信息，可能来源于算法，可能来源于系统。
     */
    void finished(int code,QString error_msg);
};



//变量储存cache的线程类
class CacheSaver : public QObject
{
    Q_OBJECT

public:
    CacheSaver(std::string& path,std::string& instance_name);
public slots:
    void process();

signals:
    void finished();
    //释放进程的progress，percentage代表当前进度百分比，msg代表信息，code代表成攻（1）失败（0）
    void progress(double percentage,QString msg,int code);
    void started();
    void error(QString msg);

private:
    std::string scriptName;
    std::string instanceName;

};

