///////////////////////////////////////////
//ParallelTest.h
//这个算法是为了对脚本进行大规模的测试
//可以使脚本作用于大量的数据集上藉此观察结果。
//算法重写了Lazarus中的一部分需要GUI的函数，使其无须GUI，可接受外部参数运行。
//
//
//
////////////////////////////////////////////

#pragma once

#include <QObject>
#include <QString>
#include <QMenu>
#include <string>
#include <LazarusDataWarpper.h>
#include <LazarusWatchDog.h>
#include <LazarusProcedureFactory.h>
#include <map>



using namespace std;
using namespace Lazarus;
//ParallelTest类，对某个脚本进行规模化的测试
//这个类继承与QObject,但是并没有GUI显示，使用时可以作为一个单独的线程代码使用
class ParallelTest : public QObject
{
    Q_OBJECT
public:
    //Bind结构代表了单个procedure所绑定的进入/输出路径或文件名
    typedef struct{
        //inPaths[i][j]代表第i次执行的某个procedure的第j个需要指定的input路径或文件名的值
        vector<vector<string> > inPaths;
        //outPaths[i][j]代表第i次执行的某个procedure的第j个需要指定的output路径或文件名的值
        vector<vector<string> > outPaths;
    } Bind;
    //map[k]代表与第k个procedure bind的路径文件名结构
    typedef map<int,Bind> pathMapType;
    
    enum PathReturnType{
         PATH,
         FILENAME,
         ERROR
    };
    

    ParallelTest(LazScript& _script);
    
    void myBind(int script_line,Bind& bid);
    
//protected:
    //检测path是否是一个合法的路径（或者是文件名）
    bool                  isPath(QString& path);
    bool                  PhaseOneScript(const Lazarus::scriptUnit& inUnit,int scriptLine,QString& error_msg);
    //读取特定的带GUI（换言之有输入输出）的脚本
    bool                  PhaseSpecGUIScript(const Lazarus::scriptUnit& inUnit,int scriptLine,QString& error_msg);
    LazScript             script;//脚本名称
    
    pathMapType           pathMap;
    int                   currentCycle;//做到第几个数据了
    int                   maxCycle;
    
    vector<string>        replace_gui_procedures;//需要重新替换的包含GUI的procedure;
    vector<string>          gui_is_path;//前面那个replace的gui元素需要进入的是path还是文件名
    vector<int>           gui_input_var_num;
    vector<int>           gui_output_var_num;
    //这个类储存了需要用path替代GUI的procedure所在的脚本行数
    vector<int>           gui_need_replace;
    //判断procName的第index个input或output类型应该是路径还是文件名
    //返回值：2 出现错误，0 路径 1 文件名
    PathReturnType        isPathOrFileName(string procName,bool is_input,int index);
    
public slots:
    void process();
    
    
signals:
    void start();
    void finished(int code,QString error_msg);
    void allDone();
};


//class ParallelProcedureFactory : public ProcedureFactory
//{
//    static Procedure* CreateProcedure(const string& name);
//};
//
//class ParallelGDCMIn : public opt_gdcm_in
//{
//    
//    
//};









