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
    
    
    explicit MainWindow(QWidget *parent = 0);
    void InsertBaseImg(const string& name,int dim = 3);
    void InsertMask(const string& name,int dim = 3);
    ~MainWindow();

protected:
    //update whole scene.
    void UpdateScene();
    //update search window
    void UpdateAlgBrowser();
    //phase one line of script
    bool PhaseOneScript(const scriptUnit& inUnit);
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
    
protected:
    //this stack contains names of pics to show on screen.
    LazGUI::ImageStack stack;

    //内部的图像，用于显示
    vector<Mat>     shownImg;
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
    //right mouse menu
    QMenu*          RMBMenu;
    //移除基准图像与Mask的操作
    QAction*        removeItem;
    
private slots:
    //显示图片信息
    void ShowInfomation();
    
    //在stackShow窗口中移除mask或者基准图像
    void RemoveItem();
    
    
    void on_LowerThres_sliderMoved(int position);
    void on_UpperThres_sliderMoved(int position);
    //添加基准图像与mask的事件
    void on_actionAddBaseImgAction_triggered();
    void on_actionAddMaskAction_triggered();
    
    void on_AlgBrowser_doubleClicked(const QModelIndex &index);
    void on_AlgSelector_textChanged();
    void on_actionPhaseScript_triggered();
    void on_actionSaveScript_triggered();
    void on_ResourceManager_itemChanged(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
