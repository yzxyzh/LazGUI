#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "AlgDialog.h"
#include <LazarusProcedureFactory.h>
#include <QMessageBox>
#include <LazarusUniversalAlgorithm.h>
#include "cvqtconverter.h"
#include <opencv2/highgui/highgui.hpp>
#include <QKeyEvent>
#include <tinyxml.h>
#include <QFileDialog>

using namespace Lazarus;


bool MainWindow::PhaseOneScript(const Lazarus::scriptUnit& inUnit)
{
    cout<<"********************Running Sentence ********************"<<endl;
    cout<<"Current Procedure name  = "<<inUnit.procName<<endl;
    if (WatchDog::ProcedureExist(inUnit.procName))
    {
        algIOTypes curProcIO=WatchDog::GetIoType(inUnit.procName);
        string inputVariables;
        cout<<"InVar Size = "<<inUnit.inVars.size()<<endl;
        for (int j=0;j<inUnit.inVars.size();j++)
        {
            ///如果找到实例，就说明之前已经创建过了，因此不用再次赋值。
            ///如果没有找到，那么就必须创建，并且进行赋值
            if(WatchDog::CreateInstance(curProcIO.inTypes[j],inUnit.inVars[j]))
                WatchDog::GetInstance(inUnit.inVars[j]).objPtr->SetValue(inUnit.inDefVals[j]);
        }
        cout<<"outVar Size = "<<inUnit.outVars.size()<<endl;
        for (int j=0;j<inUnit.outVars.size();j++)
        {
            //输出的实例是无需赋初值的，因为是输出就一定会有值。
            WatchDog::CreateInstance(curProcIO.outTypes[j],inUnit.outVars[j]);
            //WatchDog::GetInstance(WatchDog::currentScript[i].outVars[j]).objPtr->SetValue(WatchDog::currentScript[i].outDefVals[j]);
        }
        Lazarus::Procedure* newProc=Lazarus::ProcedureFactory::CreateProcedure(inUnit.procName);
        if (NULL == newProc)
        {
            cout<<"Can NOT Create Procedure Name = " <<inUnit.procName<<endl;
            return false;
        }
        newProc->SetInput(inUnit.inputVarsFull);
        newProc->SetOutput(inUnit.outputVarsFull);
        newProc->Run();
        delete newProc;
        cout<<"*******************Procedure run complete*****************"<<endl;
        cout<<endl;
    }else{
        WatchDog::WriteLog(Lazarus::Exception::FireException(LAZ_ERROR_USERSPEC,"Procedure not exist in preread Lazarus algorithm list! Maybe you forget to copy it here?"),LOG_NEXTLINE,true,true);
        return false;
    }
    
    return true;
    
}
//保存Script
void MainWindow::on_actionSaveScript_triggered()
{
    if(LazGUI::Pool::Script.size() == 0) return;
    
    TiXmlDocument* doc=new TiXmlDocument;
    TiXmlDeclaration* declare= new TiXmlDeclaration("1.0","UTF-8","no");
    doc->LinkEndChild(declare);
    
    TiXmlElement* scriptSuperNode=new TiXmlElement("Script");
    doc->LinkEndChild(scriptSuperNode);
    
    int scriptSize=LazGUI::Pool::Script.size();
    for (int i=0; i<scriptSize; i++) {
        TiXmlElement* proc_i=new TiXmlElement("Procudure"+StringOperator::ToStr(i));
        proc_i->SetAttribute("name", LazGUI::Pool::Script[i].procName);

        int inSize=LazGUI::Pool::Script[i].inVars.size();
        int outSize=LazGUI::Pool::Script[i].outVars.size();
        string inputVarsFull;
        string outputVarsFull;
        vector<string> saveInDefVal;
        vector<string> saveOutDefVal;
        string inDefValFull;
        string outDefValFull;
        for (int j=0; j<inSize-1; j++) {
            inputVarsFull=inputVarsFull + LazGUI::Pool::Script[i].inVars[j]+",";
        }
        if(inSize>0)
            inputVarsFull+=LazGUI::Pool::Script[i].inVars.back();
        
        for (int j=0; j<inSize; j++) {
            if("" == LazGUI::Pool::Script[i].inDefVals[j])
                saveInDefVal.push_back("NULL");
            else
                saveInDefVal.push_back(LazGUI::Pool::Script[i].inDefVals[j]);
        }
        
        for (int j=0; j<inSize-1; j++) {
            inDefValFull=inDefValFull + saveInDefVal[j]+",";
        }
        if(inSize>0)
            inDefValFull+=saveInDefVal.back();

        
        
        for (int j=0; j<outSize-1; j++) {
            outputVarsFull=outputVarsFull + LazGUI::Pool::Script[i].outVars[j]+",";
        }
        if(outSize>0)
            outputVarsFull+=LazGUI::Pool::Script[i].outVars.back();
        
        for (int j=0; j<outSize; j++) {
            if("" == LazGUI::Pool::Script[i].outDefVals[j])
                saveOutDefVal.push_back("NULL");
            else
                saveOutDefVal.push_back(LazGUI::Pool::Script[i].outDefVals[j]);
        }
        
        for (int j=0; j<outSize-1; j++) {
            outDefValFull=outDefValFull + saveOutDefVal[j]+",";
        }
        if(outSize>0)
            outDefValFull+=saveOutDefVal.back();
        
        proc_i->SetAttribute("inVars", inputVarsFull);
        proc_i->SetAttribute("inDefValue", inDefValFull);
        proc_i->SetAttribute("outVars", outputVarsFull);
        proc_i->SetAttribute("outDefValue", outDefValFull);

        scriptSuperNode->LinkEndChild(proc_i);
    }//end for add
    
    QString saveName=QFileDialog::getSaveFileName(this,"Save Script","","LazScript (*.lazScript)");
    if("" != saveName)
        doc->SaveFile(saveName.toStdString());
    
    delete doc;
    
}

void MainWindow::mousePressEvent(QMouseEvent *)
{
    UpdateStatusWindow();
}

void MainWindow::UpdateStackWindow()
{
    cout<<"Updating Stack Window......"<<endl;
    int rowNum=ui->StackShow->rowCount();
    cout<<"Row Count = "<<rowNum<<endl;
    while (ui->StackShow->rowCount()>0) {
        ui->StackShow->removeRow(0);
    }
    ui->StackShow->clear();
    if(!isBaseImgSet) return;
    QTableWidgetItem* baseStr=new QTableWidgetItem();
    baseStr->setText("基准图像");
    baseStr->setFlags(Qt::ItemIsEnabled);
    QTableWidgetItem* baseImgStr=new QTableWidgetItem();
    baseImgStr->setText(QString::fromStdString(stack.baseImg));
    //baseImgStr->setFlags(Qt::ItemIsEnabled);
    ui->StackShow->insertRow(0);
    ui->StackShow->setItem(0, 0, baseStr);
    ui->StackShow->setItem(0, 1, baseImgStr);
    
    cout<<"with "<<stack.masks.size()<<" masks.."<<endl;
    
    for (int i=0; i<stack.masks.size(); i++) {
        int g=0;
        int r=255.0*(1+i)/stack.masks.size();
        int b=255.0*(stack.masks.size()-i-1)/stack.masks.size();
        ui->StackShow->insertRow(i+1);
        QTableWidgetItem* maskStr=new QTableWidgetItem();
        maskStr->setText(QString::fromStdString("Mask"+StringOperator::ToStr(i)));
        maskStr->setTextColor(QColor(b,g,r));
        maskStr->setFlags(Qt::ItemIsEnabled);
        QTableWidgetItem* maskImgStr=new QTableWidgetItem();
        maskImgStr->setText(QString::fromStdString(stack.masks[i]));
        ui->StackShow->setItem(i+1, 0, maskStr);
        ui->StackShow->setItem(i+1, 1, maskImgStr);
    }
    cout<<"complete."<<endl;
}

bool MainWindow::isMatch2D(const string &inFile)
{
    GUI_BEGIN
    image_2d_opencv_8uc1* img=dynamic_cast<image_2d_opencv_8uc1*>(WatchDog::GetInstance(inFile).objPtr);
    int nRows=img->GetData()->rows;
    int nCols=img->GetData()->cols;
    
    return (nRows == shownImg[0].rows && nCols == shownImg[0].cols && 1 == maxLayer);
    GUI_END
    
    return false;
}

bool MainWindow::isMatch3D(const string &inFile)
{
    GUI_BEGIN
    image_3d_opencv_8uc1* img=dynamic_cast<image_3d_opencv_8uc1*>(WatchDog::GetInstance(inFile).objPtr);
    int nRows=img->GetLayerData(0).rows;
    int nCols=img->GetLayerData(0).cols;
    
    return (nRows == shownImg[0].rows && nCols == shownImg[0].cols && img->GetLayer() == maxLayer);
    GUI_END
    
    return false;
}
//更新显示图片信息窗口
void MainWindow::UpdatePropertiesWindow(const string &inFileName, int dim)
{
    //首先清空表格
    while (ui->PropertiesShow->rowCount()>0) {
        ui->PropertiesShow->removeRow(0);
    }
    if(dim == 3)
    {
        //cout<<"开始显示图像详细信息"<<endl;
        image_3d_opencv_8uc1* img=dynamic_cast<image_3d_opencv_8uc1*>(WatchDog::GetInstance(inFileName).objPtr);
        int nRows = img->GetLayerData(0).rows;
        int nCols = img->GetLayerData(0).cols;
        int layer = img->GetLayer();
        Mat* Ptr=img->GetData();
        double minVal=0;
		double maxVal=-1;
		for (int z=0;z<layer;z++)
		{
			double currentMaxVal=-1;
			cv::minMaxLoc(Ptr[z],&minVal,&currentMaxVal);
			if (currentMaxVal>maxVal)
				maxVal=currentMaxVal;
		}
        //设置名称
        ui->PropertiesShow->insertRow(0);
        QTableWidgetItem* nameStr=new QTableWidgetItem();
        nameStr->setText("图像名称");
        nameStr->setFlags(Qt::ItemIsEnabled);
        
        QTableWidgetItem* nameStr_detail=new QTableWidgetItem();
        nameStr_detail->setText(QString::fromStdString(inFileName));
        nameStr_detail->setFlags(Qt::ItemIsEnabled);
        ui->PropertiesShow->setItem(0, 0, nameStr);
        ui->PropertiesShow->setItem(0, 1, nameStr_detail);
        //设置三维
        ui->PropertiesShow->insertRow(1);
        QTableWidgetItem* rowStr=new QTableWidgetItem();
        rowStr->setText("ROWS");
        rowStr->setFlags(Qt::ItemIsEnabled);
        
        QTableWidgetItem* rowStr_detail=new QTableWidgetItem();
        rowStr_detail->setText(QString::fromStdString(StringOperator::ToStr(nRows)));
        rowStr_detail->setFlags(Qt::ItemIsEnabled);
        ui->PropertiesShow->setItem(1, 0, rowStr);
        ui->PropertiesShow->setItem(1, 1, rowStr_detail);
        
        ui->PropertiesShow->insertRow(2);
        QTableWidgetItem* colStr=new QTableWidgetItem();
        colStr->setText("COLS");
        colStr->setFlags(Qt::ItemIsEnabled);
        
        QTableWidgetItem* colStr_detail=new QTableWidgetItem();
        colStr_detail->setText(QString::fromStdString(StringOperator::ToStr(nCols)));
        colStr_detail->setFlags(Qt::ItemIsEnabled);
        ui->PropertiesShow->setItem(2, 0, colStr);
        ui->PropertiesShow->setItem(2, 1, colStr_detail);
        
        ui->PropertiesShow->insertRow(3);
        QTableWidgetItem* layerStr=new QTableWidgetItem();
        layerStr->setText("LAYER");
        layerStr->setFlags(Qt::ItemIsEnabled);
        
        QTableWidgetItem* layerStr_detail=new QTableWidgetItem();
        layerStr_detail->setText(QString::fromStdString(StringOperator::ToStr(layer)));
        layerStr_detail->setFlags(Qt::ItemIsEnabled);
        ui->PropertiesShow->setItem(3, 0, layerStr);
        ui->PropertiesShow->setItem(3, 1, layerStr_detail);
        //设置最大最小值
        ui->PropertiesShow->insertRow(4);
        QTableWidgetItem* minStr=new QTableWidgetItem();
        minStr->setText("MIN");
        minStr->setFlags(Qt::ItemIsEnabled);
        
        QTableWidgetItem* minStr_detail=new QTableWidgetItem();
        minStr_detail->setText(QString::fromStdString(StringOperator::ToStr(minVal)));
        minStr_detail->setFlags(Qt::ItemIsEnabled);
        ui->PropertiesShow->setItem(4, 0, minStr);
        ui->PropertiesShow->setItem(4, 1, minStr_detail);
        
        ui->PropertiesShow->insertRow(5);
        QTableWidgetItem* maxStr=new QTableWidgetItem();
        maxStr->setText("MAX");
        maxStr->setFlags(Qt::ItemIsEnabled);
        
        QTableWidgetItem* maxStr_detail=new QTableWidgetItem();
        maxStr_detail->setText(QString::fromStdString(StringOperator::ToStr(maxVal)));
        maxStr_detail->setFlags(Qt::ItemIsEnabled);
        ui->PropertiesShow->setItem(5, 0, maxStr);
        ui->PropertiesShow->setItem(5, 1, maxStr_detail);
    }//end if dim == 3
    
    
}

void MainWindow::UpdateStatusWindow()
{
    //需要有这么一些：现在图的大小，坐标位置，现在图的像素值
    if(isBaseImgSet)
    {//如果设置的基本图像，就要显示图像现在的像素值
        int mx=-1;
        int my=-1;
        
        QPoint mousePos=ui->ImageViewer->mapFromGlobal(QCursor::pos());
        mx=mousePos.x();
        my=mousePos.y();
        //cout<<"mx = "<<mx<<" my = "<<my<<endl;
        
        int width = ui->ImageViewer->width();
        int height = ui->ImageViewer->height();
        
        //cout<<"width = "<<width<<" height = "<<height<<endl;
        
        if(mx<0) mx=0;
        if(mx>=width) mx=width-1;
        if(my<0) my=0;
        if(my>=height) my=height-1;
        
        
        
        mx=mx*imgCols/width;
        my=my*imgRows/height;
        //cout<<"img rows = "<<imgRows<<" img cols = "<<imgCols<<endl;
        
        int pixelVal=-1;
        
        if(3 == imageDimension)
        {
            image_3d_opencv_8uc1* img=dynamic_cast<image_3d_opencv_8uc1*>(WatchDog::GetInstance(stack.baseImg).objPtr);
            pixelVal=img->GetLayerData(currentLayer).at<uchar>(my,mx);
        }else{
            image_2d_opencv_8uc1* img=dynamic_cast<image_2d_opencv_8uc1*>(WatchDog::GetInstance(stack.baseImg).objPtr);
            pixelVal=img->GetData()->at<uchar>(my,mx);
        }
        
        QString status;
        status = status+"currentLayer = "+QString::fromStdString(StringOperator::ToStr(currentLayer))+"\n";
        status = status+"maxLayer = "+QString::fromStdString(StringOperator::ToStr(maxLayer))+"\n";
        status = status+"current mouse position = ("+QString::fromStdString(StringOperator::ToStr(mx))+" , "+QString::fromStdString(StringOperator::ToStr(my))+") \n";
        status = status+"current pixel value = "+QString::fromStdString(StringOperator::ToStr(pixelVal))+"\n";
        status = status+"current Threshold = ["+QString::fromStdString(StringOperator::ToStr(lowerThres))+" , "+QString::fromStdString(StringOperator::ToStr(upperThres))+"] \n";
        
        ui->StatusShow->setText(status);
    }
}

//显示信息的消息
void MainWindow::ShowInfomation()
{
    //显示图片信息
    if(0 == ui->ResourceManager->selectedItems().size()) return;
    
    GUI_BEGIN
    
    QTreeWidgetItem* selectedItem=ui->ResourceManager->selectedItems()[0];
    //如果选择的是父节点那么就要返回
    if(nullptr == selectedItem->parent()) return;
    
    string instance_name=selectedItem->text(0).toStdString();
    Instance inst=WatchDog::GetInstance(instance_name);//找不到会抛出异常的
    string type=inst.type;
    //cout<<"type = "<<type<<endl;
    if(type == "IMAGE_3D_OPENCV_8UC1")
    {
        cout<<"开始显示3维图片信息"<<endl;
        UpdatePropertiesWindow(instance_name, 3);
    }
    else if(type == "IMAGE_2D_OPENCV_8UC1")
    {
        UpdatePropertiesWindow(instance_name, 2);
    }
    
    GUI_END

    
    
    
}

void MainWindow::on_LowerThres_sliderMoved(int position)
{
    if(isBaseImgSet)
    {
    lowerThres=position;
    UpdateStatusWindow();
    UpdateScene();
    }
}

void MainWindow::on_UpperThres_sliderMoved(int position)
{
    if(isBaseImgSet)
    {
        upperThres=position;
        UpdateStatusWindow();
        UpdateScene();
    }

    
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    maxLayer(-1),
    currentLayer(0),
    imgRows(-1),
    imgCols(-1),
    lowerThres(0),
    upperThres(255),
    isBaseImgSet(false),
    addBaseImgAction(NULL),
    addMaskAction(NULL),
    RMBMenu(NULL),
    removeItem(NULL),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->grabKeyboard();
    this->setFocusPolicy(Qt::StrongFocus);
    ui->AlgBrowser->setEditTriggers(QListWidget::NoEditTriggers);
    WatchDog::ProjectBegin();
    algList=WatchDog::GetAlgList();
    UpdateAlgBrowser();
    RMBMenu = new QMenu();
    addMaskAction = new QAction(this);
    addBaseImgAction = new QAction(this);
    showInformationAction = new QAction(this);
    addMaskAction ->setText("作为mask插入");
    addBaseImgAction->setText("作为基准图像插入");
    showInformationAction->setText("显示详细信息");
    
    removeItem = new QAction(this);
    removeItem->setText("移除显示对象");
    
    ui->StackShow->setColumnCount(2);
    int stackShowWidth = ui->StackShow->width();
    ui->StackShow->setColumnWidth(0, stackShowWidth/3.1);
    ui->StackShow->setColumnWidth(1, 2*stackShowWidth/3.1);
    ui->StackShow->horizontalHeader()->hide();
    ui->StackShow->verticalHeader()->hide();
    
    ui->PropertiesShow->setColumnCount(2);
    int prop = ui->PropertiesShow->width();
    ui->PropertiesShow->setColumnWidth(0, prop/2.1);
    ui->PropertiesShow->setColumnWidth(1, prop/2.1);
    ui->PropertiesShow->horizontalHeader()->hide();
    ui->PropertiesShow->verticalHeader()->hide();
    
    ui->LowerThres->setMaximum(255);
    ui->UpperThres->setMaximum(255);
    ui->UpperThres->setSliderPosition(255);
    
    
    connect(showInformationAction, SIGNAL(triggered()), this, SLOT(ShowInfomation()));
    QObject::connect(addMaskAction, SIGNAL(triggered()), this, SLOT(on_actionAddMaskAction_triggered()));
    QObject::connect(addBaseImgAction, SIGNAL(triggered()), this, SLOT(on_actionAddBaseImgAction_triggered()));

    
    //RMBMenu->addAction(addBaseImgAction);
    //RMBMenu->addAction(addMaskAction);
    ui->ResourceManager->addAction(addMaskAction);
    ui->ResourceManager->addAction(addBaseImgAction);
    ui->ResourceManager->addAction(showInformationAction);
    ui->ResourceManager->setContextMenuPolicy(Qt::ActionsContextMenu);
    
    ui->StackShow->addAction(removeItem);
    ui->StackShow->setContextMenuPolicy(Qt::ActionsContextMenu);
    
    
    connect(removeItem, SIGNAL(triggered()), this, SLOT(RemoveItem()));
   }

MainWindow::~MainWindow()
{
    delete ui;
    delete RMBMenu;
    delete addBaseImgAction;
    delete addMaskAction;
}

void MainWindow::RemoveItem()
{
  //移除显示对象，首先要判断移除的是Mask还是base image,如果是base image那么isBaseImgShow这个flag要设成false
    if(0 == ui->StackShow->selectedItems().size()) return;
    
    int rowNum=ui->StackShow->row(ui->StackShow->selectedItems()[0]);
    
    if(rowNum == 0)
    {//如果选中的是第一行，那我们要remove图片，并且把flag设成false
        isBaseImgSet=false;
        stack.ClearMasks();
        stack.baseImg="";
        ui->ImageViewer->setPixmap(NULL);
        shownImg.clear();
        currentLayer=0;
        maxLayer=0;
        imageDimension=-1;
        UpdateStackWindow();
        UpdateScene();
    }
    
    if(rowNum > 0)
    {
        //那么选择的就是Mask
        string maskName=stack.masks[rowNum-1];
        cout<<"Before Deleting..."<<stack.masks.size()<<" masks"<<endl;
        stack.DeleteMask(maskName);
        cout<<"After Deleting..."<<stack.masks.size()<<" masks"<<endl;
        UpdateStackWindow();
        BlendImage();
        
        //UpdateScene();
    }
    
}

void MainWindow::keyPressEvent(QKeyEvent *k)
{
    //cout<<"OK!!!"<<endl;
    int key=k->key();
    switch (key) {
        case Key_W:
        {
           if(maxLayer>1)
           {
            currentLayer=(currentLayer+1)%maxLayer;
               UpdateScene();
           }
            UpdateStatusWindow();
        }
        break;
        case Key_S:
        {
            if(maxLayer>1)
            {
                currentLayer=(currentLayer+maxLayer-1)%maxLayer;
                UpdateScene();
            }
            UpdateStatusWindow();
        }
        break;
            
        default:
            break;
    }
    
    
}

void MainWindow::UpdateScene()
{

   //FIXME
    //imshow("Result",shownImg[currentLayer]);
    if(isBaseImgSet)
    {
        Mat tmpImage;
        shownImg[currentLayer].copyTo(tmpImage);
        
        if(imageDimension == 3)
        {
            Mat Data;
            image_3d_opencv_8uc1* img=dynamic_cast<image_3d_opencv_8uc1*>(WatchDog::GetInstance(stack.baseImg).objPtr);
            img->GetLayerData(currentLayer).copyTo(Data);
            for (int x=0; x<imgRows; x++) {
                for(int y=0;y<imgCols;y++)
                {
                    if((int)Data.at<uchar>(x,y)<lowerThres || (int)Data.at<uchar>(x,y)>upperThres)
                        tmpImage.at<Vec3b>(x,y)=Vec3b(0,0,0);
                }
            }
        }else{
            Mat Data;
            image_2d_opencv_8uc1* img=dynamic_cast<image_2d_opencv_8uc1*>(WatchDog::GetInstance(stack.baseImg).objPtr);
            img->GetData()->copyTo(Data);
            for (int x=0; x<imgRows; x++) {
                for(int y=0;y<imgCols;y++)
                {
                    if((int)Data.at<uchar>(x,y)<lowerThres || (int)Data.at<uchar>(x,y)>upperThres)
                        tmpImage.at<Vec3b>(x,y)=Vec3b(255,0,0);
                }
            }

        }
        
        
        QImage newImg=cvQTConverter::CvMat2QImg(tmpImage);
        QPixmap p=QPixmap::fromImage(newImg);
        int width=ui->ImageViewer->width();
        int height=ui->ImageViewer->height();
        ui->ImageViewer->setPixmap(p.scaled(width, height,Qt::IgnoreAspectRatio));
    }
}
void MainWindow::InsertBaseImg(const string &name,int dim)
{
    //insert basic images;
    //load base image z-size;
    //and if the base img size does not correspond to early ones, all the mask must be eliminated.
    //FIXME
    GUI_BEGIN
    if(dim == 3)
    {
    image_3d_opencv_8uc1* img=dynamic_cast<image_3d_opencv_8uc1*>(WatchDog::GetInstance(name).objPtr);
    if(2 == imageDimension || (isBaseImgSet && !isMatch3D(name)))
    {
        stack.ClearMasks();
    }
    this->maxLayer=img->GetLayer();
    this->currentLayer=0;
        this->imgRows=img->GetLayerData(0).rows;
        this->imgCols=img->GetLayerData(0).cols;
    stack.InsertBaseImg(name);
    isBaseImgSet=true;
        imageDimension=3;
        UpdateStackWindow();
        BlendImage();
    }
    if(dim == 2)
    {
        image_2d_opencv_8uc1* img=dynamic_cast<image_2d_opencv_8uc1*>(WatchDog::GetInstance(name).objPtr);
        if(3 == imageDimension || (isBaseImgSet && !isMatch2D(name)))
        {
            stack.ClearMasks();
        }
        this->maxLayer=1;
        this->currentLayer=0;
        this->imgRows=img->GetData()->rows;
        this->imgCols=img->GetData()->cols;
        stack.InsertBaseImg(name);
        isBaseImgSet=true;
        imageDimension = 2;
        UpdateStackWindow();
        BlendImage();
    }
    
    GUI_END
}

void MainWindow::InsertMask(const string &name,int dim)
{
    //作为mask插入有BUG
    //FIXME
    //insert mask
    //here a judge is needed to see whether the mask and the baseimg are the same size.
    GUI_BEGIN
    if(dim != imageDimension) return;
    if(dim == 3)
    {
    image_3d_opencv_8uc1* mask=dynamic_cast<image_3d_opencv_8uc1*>(WatchDog::GetInstance(name).objPtr);
    if(!UniversalAlgorithm::isBinary(mask)) return;
    if(!isBaseImgSet || !isMatch3D(name)) return;
    stack.InsertMasks(name);
        UpdateStackWindow();
        BlendImage();
    }
    if(dim == 2)
    {
        image_2d_opencv_8uc1* mask=dynamic_cast<image_2d_opencv_8uc1*>(WatchDog::GetInstance(name).objPtr);
        if(!UniversalAlgorithm::isBinary(mask)) return;
        if(!isBaseImgSet || !isMatch2D(name)) return;
        stack.InsertMasks(name);
        UpdateStackWindow();
        BlendImage();
    }
    
    GUI_END
}
//开始读取脚本
void MainWindow::on_actionPhaseScript_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Open Script","","LazScript (*.lazScript)");
    GUI_BEGIN
    
    if(!fileName.isEmpty())
    {
        LazGUI::Pool::Script.clear();
        LazGUI::Pool::alg_counter=0;
        WatchDog::DumpMemory();
        
        LazScript newScript;
        
        if(!AnalysisScript(fileName.toStdString(),newScript)) return;
        
        //LazGUI::Pool::alg_counter=LazGUI::Pool::Script.size();
        
        for (int i=0; i<newScript.size(); i++) {
            PhaseOneScript(newScript[i]);
            LazGUI::Pool::Script.push_back(newScript[i]);
            LazGUI::Pool::alg_counter++;
            UpdateResourceManager();
        }
        
    }
    
    
    GUI_END
}

bool MainWindow::AnalysisScript(const string &filepath, LazScript &script)
{
    script.clear();
    TiXmlDocument newScript;
    if(!newScript.LoadFile(filepath.c_str()))
    {
        //无法读取脚本
        cerr<<"can not read script"<<endl;
        return false;
    }
    TiXmlElement* nodeScriptBegin=newScript.FirstChildElement();
    if ((string)(nodeScriptBegin->Value())!="Script")
    {
        cerr<<"Script Header ERROR"<<endl;
        return false;
    }
    TiXmlElement* nodeScrpUnit=nodeScriptBegin->FirstChildElement();
    while(nodeScrpUnit)
    {
        scriptUnit newUnit;
        TiXmlAttribute* name=nodeScrpUnit->FirstAttribute();
        TiXmlAttribute* inVars=name->Next();
        TiXmlAttribute* inDefVals=inVars->Next();
        TiXmlAttribute* outVars=inDefVals->Next();
        TiXmlAttribute* outDefVals=outVars->Next();
        
        newUnit.procName=name->Value();
        newUnit.inputVarsFull=inVars->Value();
        newUnit.outputVarsFull=outVars->Value();
        //cout<<"inPut Full = "<<newUnit.inputVarsFull<<endl;
        //cout<<"output Full = "<<newUnit.outputVarsFull<<endl;
        newUnit.inVars=Lazarus::StringOperator::GetVariables(inVars->Value());
        newUnit.inDefVals=Lazarus::StringOperator::GetVariables(inDefVals->Value());
        newUnit.outVars=Lazarus::StringOperator::GetVariables(outVars->Value());
        newUnit.outDefVals=Lazarus::StringOperator::GetVariables(outDefVals->Value());
        
        script.push_back(newUnit);
        
        nodeScrpUnit=nodeScrpUnit->NextSiblingElement();
    }
    return true;
}

void MainWindow::BlendImage()
{
    GUI_BEGIN
    if(imageDimension == 3)
    {
        cout<<"Blending..."<<endl;
        //Retrieve Data;
        int maskNum=stack.masks.size();
//        cout<<"Mask Size = "<<maskNum<<endl;
//        for (auto p : stack.masks) {
//            cout<<"Mask = "<<p<<endl;
//        }
        image_3d_opencv_8uc1* base_image=dynamic_cast<image_3d_opencv_8uc1*>(WatchDog::GetInstance(stack.baseImg).objPtr);
        Mat* base=base_image->GetData();
        vector<Mat*> masksPtr(maskNum);
        for (int i=0; i<maskNum; i++) {
            masksPtr[i]=dynamic_cast<image_3d_opencv_8uc1*>(WatchDog::GetInstance(stack.masks[i]).objPtr)->GetData();
        }
        
        int nRows=base[0].rows;
        int nCols=base[0].cols;
        
    
        bool isBinary=UniversalAlgorithm::isBinary(base_image);
        
        shownImg.clear();
        shownImg.resize(maxLayer);
        for (int i=0; i<maxLayer; i++) {
            Mat currLayer;
            
            if(!isBinary){
                cvtColor(base[i], currLayer, CV_GRAY2BGR);
            }
            else
            {
                cvtColor(base[i]*255, currLayer, CV_GRAY2BGR);
            }
            
            if(0 == maskNum) currLayer.copyTo(shownImg[i]);
            else
            {//否则我们要画出轮廓线
                for (int mask_index=0; mask_index<maskNum; mask_index++) {
                    int g=0;
                    int r=255.0*(1+mask_index)/maskNum;
                    int b=255.0*(maskNum-mask_index-1)/maskNum;
                    for (int x=1; x<nRows-1; x++) {
                        for (int y=1; y<nCols-1; y++) {
                            
                            if(masksPtr[mask_index][i].at<uchar>(x,y)>0)
                            {
                                int nei_val=masksPtr[mask_index][i].at<uchar>(x-1,y)+masksPtr[mask_index][i].at<uchar>(x+1,y)
                                +masksPtr[mask_index][i].at<uchar>(x,y-1)+masksPtr[mask_index][i].at<uchar>(x,y+1);
                                
                                if (nei_val<4)
                                    currLayer.at<Vec3b>(x,y)=Vec3b(r,g,b);
                            }
                        }
                    }
                    
                    
                }
                currLayer.copyTo(shownImg[i]);
                
            }
            
        }
        
        
    }
    
    UpdateScene();
    
    GUI_END
    
}

void MainWindow::UpdateAlgBrowser()
{
//    int currentRow=ui->AlgBrowser->count();
//    for (int i=0; i<currentRow; i++) {
//        delete ui->AlgBrowser->takeItem(i);
//        //ui->AlgBrowser->removeItemWidget(ui->AlgBrowser->item(i));
//    }
//    ui->AlgBrowser->update();
    ui->AlgBrowser->clear();
    int totalSize=static_cast<int>(algList.size());
    QString Fil=filter.toUpper();
    //cout<<Fil.toStdString()<<endl;
    int algCount=0;
    for (int i=0; i<totalSize; i++) {
        QString currInstance(QString::fromLocal8Bit(algList[i].c_str()));
        if(Fil=="" || -1 != currInstance.indexOf(Fil))
        {
            ui->AlgBrowser->addItem(currInstance);
            algCount++;
        }
    }

    //cout<<"AlgCount = "<<algCount<<endl;
}

void MainWindow::on_AlgSelector_textChanged()
{
    filter=ui->AlgSelector->toPlainText();
    UpdateAlgBrowser();
}

void MainWindow::on_AlgBrowser_doubleClicked(const QModelIndex &index)
{
    GUI_BEGIN
    int selectedRow=index.row();
    QString selected_name=ui->AlgBrowser->item(selectedRow)->text();
    int old_scrip_num=LazGUI::Pool::Script.size();
    AlgDialog dlg(this,selected_name.toStdString());
    dlg.exec();
    //cout<<LazGUI::Pool::Script.size()<<endl;
    if(LazGUI::Pool::Script.size()>old_scrip_num)
    {
    PhaseOneScript(LazGUI::Pool::Script.back());
    cout<<"OK"<<endl;
    UpdateResourceManager();
    }
    return;
    //FIXME
    //next a algorithm selection box will pop up.
    //cout<<selected_name.toStdString()<<endl;
    GUI_END
    
    //如果说在try里跳出了，那就会执行到这儿
    LazGUI::Pool::Script.pop_back();
    LazGUI::Pool::alg_counter--;
}

void MainWindow::UpdateResourceManager()
{
    //use tree structures to show current scripts;
    //注意，中间修改了参数值的话，后面所有的脚本全部要重新做一遍。
    ui->ResourceManager->clear();
    ui->ResourceManager->setColumnCount(3);
    int width=ui->ResourceManager->width();
    ui->ResourceManager->setColumnWidth(0, width/3);
    ui->ResourceManager->setColumnWidth(1, width/2);
    ui->ResourceManager->setColumnWidth(2, width/6);
    ui->ResourceManager->headerItem()->setText(0, "变量名称");
    ui->ResourceManager->headerItem()->setText(1, "变量描述");
    ui->ResourceManager->headerItem()->setText(2, "变量取值");
    vector<Lazarus::scriptUnit>* scriptPtr=&LazGUI::Pool::Script;
    int scriptSize=scriptPtr->size();
    for (int i=0; i<scriptSize; i++) {
        
        
        
        //cout<<"Script "<<i<<" inSize = "<<scriptPtr->at(i).inVars.size()<<" outSize = "<<scriptPtr->at(i).outVars.size()<<endl;
        algIOTypes currIOType=WatchDog::GetIoType(scriptPtr->at(i).procName);
        QStringList procList;
        procList<<QString::fromStdString("操作 "+StringOperator::ToStr(i))<<QString::fromStdString(scriptPtr->at(i).procName);
        QTreeWidgetItem* procName=new QTreeWidgetItem(procList);
        //procName->setFlags(Qt::ItemIsEditable|Qt::ItemIsEnabled);
        //procName->setTextColor(0, QColor("red"));
        for (int j=0; j<scriptPtr->at(i).inVars.size(); j++)
        {
            //cout<<"in "<<currIOType.InDescription[j]<<endl;
            QStringList inVars;
            inVars<<QString::fromStdString(scriptPtr->at(i).inVars[j])<<QString::fromStdString(currIOType.InDescription[j])<<QString::fromStdString(scriptPtr->at(i).inDefVals[j]);
            QTreeWidgetItem* inVarNode=new QTreeWidgetItem(inVars);
            inVarNode->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            inVarNode->setTextColor(0, QColor("red"));
            procName->addChild(inVarNode);
        }
        
        for (int j=0; j<scriptPtr->at(i).outVars.size(); j++)
        {
            //cout<<"out : "<<currIOType.OutDescription[j]<<endl;
            QStringList outVars;
            outVars<<QString::fromStdString(scriptPtr->at(i).outVars[j])<<QString::fromStdString(currIOType.OutDescription[j])<<QString::fromStdString(scriptPtr->at(i).outDefVals[j]);
            QTreeWidgetItem* outVarNode=new QTreeWidgetItem(outVars);
            outVarNode->setTextColor(0, QColor("blue"));
            outVarNode->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            procName->addChild(outVarNode);
        }
        ui->ResourceManager->addTopLevelItem(procName);
        ui->ResourceManager->expandAll();
        ui->ResourceManager->setEditTriggers(QAbstractItemView::DoubleClicked);

        //cout<<"Script "<<i<<" OK!"<<endl;
    }
    
    //cout<<"ALL OK!"<<endl;
}
void MainWindow::on_actionAddMaskAction_triggered()
{
    if(0 == ui->ResourceManager->selectedItems().size()) return;
    
    GUI_BEGIN
    
    QTreeWidgetItem* selectedItem=ui->ResourceManager->selectedItems()[0];
    //如果选择的是父节点那么就要返回
    if(nullptr == selectedItem->parent()) return;
    
    string instance_name=selectedItem->text(0).toStdString();
    Instance inst=WatchDog::GetInstance(instance_name);//找不到会抛出异常的
    string type=inst.type;
    //cout<<"type = "<<type<<endl;
    if(type == "IMAGE_3D_OPENCV_8UC1")
    {
        //cout<<"Add 3D mask..."<<endl;
        InsertMask(instance_name,3);
    }
    else if(type == "IMAGE_2D_OPENCV_8UC1")
    {
        //cout<<"Add 2D mask..."<<endl;
    }
    
    GUI_END
}

void MainWindow::on_actionAddBaseImgAction_triggered()
{
    if(0 == ui->ResourceManager->selectedItems().size()) return;
    GUI_BEGIN
    
    QTreeWidgetItem* selectedItem=ui->ResourceManager->selectedItems()[0];
    //如果选择的是父节点那么就要返回
    if(nullptr == selectedItem->parent()) return;
    
    string instance_name=selectedItem->text(0).toStdString();
    Instance inst=WatchDog::GetInstance(instance_name);//找不到会抛出异常的
    string type=inst.type;
    cout<<"type = "<<type<<endl;
    if(type == "IMAGE_3D_OPENCV_8UC1")
    {
        //cout<<"Add 3D image..."<<endl;
        InsertBaseImg(instance_name,3);
    }
    else if(type == "IMAGE_2D_OPENCV_8UC1")
    {
        //cout<<"Add 2D image..."<<endl;
    }
    
    GUI_END

}

void MainWindow::on_ResourceManager_itemChanged(QTreeWidgetItem *item, int column)
{
    switch (column) {
        case 0:
        {
           
            
            //如果修改的是col1，也就是变量名，要把所有相关的变量全部改名。
            QTreeWidgetItem* parent=item->parent();
            //下面得到选择的index
            int index=ui->ResourceManager->indexOfTopLevelItem(parent);
            int child_index=parent->indexOfChild(item);
            //获得原先的内容并且修改
            string new_name=item->text(0).toStdString();
            string old_name;
            

            
            int inVarCount=LazGUI::Pool::Script[index].inVars.size();
            int outVarCount=LazGUI::Pool::Script[index].outVars.size();
            
            if(child_index>=inVarCount)
            {
                int outIndex=child_index-inVarCount;
                old_name=LazGUI::Pool::Script[index].outVars[outIndex];
            }else{
                old_name=LazGUI::Pool::Script[index].inVars[child_index];
            }
            
            if(WatchDog::InstanceExist(new_name))
            {
                myWarning(实例名称已经存在);
                item->setText(0, QString::fromStdString(old_name));
                return;
            }
            
            
            
            //下面要修改所有与之同名的实例和watchdog里面的名称
            WatchDog::RenameInstance(old_name, new_name);
            
            int script_size=LazGUI::Pool::Script.size();
            for (int i=0; i<script_size; i++) {
                for (int j=0; j<LazGUI::Pool::Script[i].inVars.size(); j++) {
                    if(old_name == LazGUI::Pool::Script[i].inVars[j])
                        LazGUI::Pool::Script[i].inVars[j]=new_name;
                }
                for (int j=0; j<LazGUI::Pool::Script[i].outVars.size(); j++) {
                    if(old_name == LazGUI::Pool::Script[i].outVars[j])
                        LazGUI::Pool::Script[i].outVars[j]=new_name;
                }

            }
             //FIXME 需要把stack里面同名的给改掉。
            if(stack.baseImg == old_name) stack.baseImg = new_name;
            for (auto& p : stack.masks) {
                if(p == old_name) p = new_name;
            }
            //改名字就要清空Propeties show的窗体
            while (ui->PropertiesShow->rowCount()>0) {
                ui->PropertiesShow->removeRow(0);
            }
            
            UpdateResourceManager();
            UpdateStackWindow();
            
        }
            break;
        case 1:
        case 2:
        {
            //FIXME
            //暂时不允许修改数值，数值的修改在后期应当是被允许的。
            QMessageBox box;
            box.setWindowTitle("ERROR");
            box.setIcon(QMessageBox::Warning);
            box.setText("不能修改算法描述！");
            box.setStandardButtons(QMessageBox::Yes);
            box.exec();
            UpdateResourceManager();
        }
            break;
//        case 2:
//            break;
        default:
            break;
    }
    
}















