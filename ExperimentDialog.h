#pragma once
#include <QDialog>
#include <vector>
#include <string>
#include <QMenu>
#include <QTreeWidget>
#include <LazarusDataWarpper.h>
#include <LazarusWatchDog.h>

#include "ParallelTest.h"

using namespace std;

namespace Ui {
    class ExperimentDialog;
}

class ExperimentDialog : public QDialog
{
    Q_OBJECT
    
    typedef vector<string> pathList;
    
    typedef struct{
        pathList list;
        string   name;
        bool     is_path;//是否是一个路径而非文件名
    }pathSet;
    
public:
    
    
    explicit ExperimentDialog(QWidget *parent = 0);
    ~ExperimentDialog();
    
    void     SetScript(Lazarus::LazScript& _in_script);
    
    void     RefineNewBind(string& newBind);
    
protected:
    
    void             UpdatePathList();
    void             UpdatePathDetail();
    void             UpdateBindWindow();
    
    
    QAction*         pathlist_bind;
    QAction*         pathlist_delete;
    QAction*         pathlist_add_path;
    
    vector<pathSet> pathes;
    //返回被选择的pathes的index，如果没有选择返回-1
    int              findSelectedPathesIndex();
    
    void             ReadData();
    
    void             Decode(const string& input,string& output);
    void             Encode(const string& input,string& output);
    //是否bind,-1代表出错，0代表未绑定，1代表绑定
    int             isBinded(const int line_num,const bool is_input,const int index,string& pathSetName);
    
    Lazarus::LazScript     script;
    bool             isScriptAnalysed;
    //需要绑定数据的行数
    vector<int>      BindLines;
    //pathlist和算法行进行的Binding，格式为：
    //BIND@（路径集名）@（行号）@（procedure名）@（IN/OUT）@(INDEX)
    vector<string>   Bindings;
    void             AnalyseScriptBinding();
    
    void             ShowError(QString msg);
    //ParallelTest*    pTest;
private:
    Ui::ExperimentDialog* ui;
    
signals:
    
public slots:
    void add_spec_path();
    void delete_path_set();
    void bind();
    void pathListClick(QTreeWidgetItem* itm,int column);
    void addNewPath();//增加一个新的路径
    void SaveFile();
    void Run();
    void ParallelTestCallBack(int code,QString errorMsg);
    void ParallelTestAllDone();
    void OpenPath();
    
    
    void GeneratePath();//用现有的路径生成一个新的人造路径。
    
};






