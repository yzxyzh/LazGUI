#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <vector>
#include "GUITypes.h"
#include <LazarusDataWarpper.h>
#include <LazarusWatchDog.h>
#include <QModelIndex>
#include <QTreeWidget>
#include <opencv2/core/core.hpp>

//thread
#include <thread>
//#include "AlgSolver.h"



using namespace Lazarus;
using namespace std;
using namespace cv;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    //typedef void (MainWindow::*CallBack)(int,std::string);
    
    explicit MainWindow(QWidget *parent = 0);
    void InsertBaseImg(const string& name,int dim = 3);
    void InsertMask(const string& name,int dim = 3);
    ~MainWindow();

//protected:
    //update whole scene.
    void UpdateScene();
    //update search window
    void UpdateAlgBrowser();
    //phase one line of script
    bool PhaseOneScript(const scriptUnit& inUnit);
    
    bool PhaseOneScriptCondition(const scriptUnit& inUnit,QString& error_msg,bool is_reading_from_cache = false,QString baseName = "");
    
    void ShowError(QString errorMsg);
    //phase one line of script by loading cache.
    bool PhaseOneScriptFromCache(const scriptUnit& inUnit,const string& baseName);
    //update resource manager
    void UpdateResourceManager();
    //whether new images match old ones;
    bool isMatch3D(const string& inFile);
    bool isMatch2D(const string& inFile);
    //blend base and masks;
    void BlendImage();
    
    void keyPressEvent ( QKeyEvent * k );
    //更新显示当前Baseimg和masks的Stack的窗口
    void UpdateStackWindow();
    //更新信息显示窗口
    void UpdatePropertiesWindow(const string& inFileName,int dim);
    //更新信息提示窗窗口
    void UpdateStatusWindow();
    
    void mousePressEvent(QMouseEvent *);
    
    bool AnalysisScript(const string& filePath,LazScript& script);
    
    bool CanPharseCache(Lazarus::scriptUnit& unit,const string& baseName);
    
    //如果在slot里面执行算法似乎VTK会崩溃，也不知道是什么原因。。。
    void AlgRun();
    
protected:

    bool            isScriptSaved;
    string          scriptName;

    QString         logs;//logs shown in cmd window;

    //this stack contains names of pics to show on screen.
    LazGUI::ImageStack stack;

    //内部的图像，用于显示
    vector<Mat>     shownImg;
    //bool形map，显示图片是否已经被cache过，防止重复cache。
    std::map<string, bool>  isVarCached;
    //vector<bool>    isVarCached;
    //图像维度，用于识别到底是在显示2D图像还是3D图像
    int             imageDimension;
    
    //图像的二维信息
    int             imgRows;
    int             imgCols;
    
    //Threshold
    int             lowerThres;
    int             upperThres;
    
    //max layer count of 3D image stack
    int             maxLayer;
    //current layer of 3D image stack
    int             currentLayer;
    //whether we have basic image;
    bool            isBaseImgSet;
    //algorithm list
    AlgorithmList   algList;
    //search filter
    QString         filter;
    //right mouse menu action
    QAction*        addBaseImgAction;
    QAction*        addMaskAction;
    QAction*        showInformationAction;
    QAction*        saveCacheAction;
    QAction*        loadCacheAction;
    //right mouse menu
    QMenu*          RMBMenu;
    //移除基准图像与Mask的操作
    QAction*        removeItem;

    //AlgSolver*      solver;
    //whether program is saving caches;
    bool            isSavingCache;
    bool            isReadingScript;
    //opeterations containing gui elements which can only be executed from qt main thread.
    //vector<string>    gui_operations;
    
    bool             phase_first_script_threaded();
    
private slots:
    //显示图片信息
    void ShowInfomation();
    
    //在stackShow窗口中移除mask或者基准图像
    void RemoveItem();
    
    void UpdateRes();
    //开始算法，界面需要显示一些信息
    void alg_start(QString msg);
    //结束算法，界面需要显示相关信息
    void alg_end(QString msg);
    //算法出错，界面需要显示相关信息
    void alg_error(QString msg);
    
    //读取下一条script，注意这个是个回调函数，写在slot中间可以调用
    //code代表信息，0代表运行成功，1代表运行失败。
    //msg代表error msg
    void phase_next_script_threaded(int code,QString error_msg);
    
    void on_saveCache();
    void on_loadCache();

    //更新存储变量快照的进度
    void UpdateCacheStatus(double percentage,QString msg,int code);
    void CacheStart();
    void CacheEnd();


    void on_LowerThres_sliderMoved(int position);
    void on_UpperThres_sliderMoved(int position);
    //添加基准图像与mask的事件
    void on_actionAddBaseImgAction_triggered();
    void on_actionAddMaskAction_triggered();
    
    void on_AlgBrowser_doubleClicked(const QModelIndex &index);
    void on_AlgSelector_textChanged();
    void on_actionPhaseScript_triggered();
    void on_actionSaveScript_triggered();
    void on_actionSaveAllCaches_triggered();
    void on_actionDeleteAllCaches_triggered();
    void on_ResourceManager_itemChanged(QTreeWidgetItem *item, int column);

    void MassiveTest();
    
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
