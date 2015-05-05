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

#include "AlgSolver.h"
#include <QThread>

#include <QTextCursor>
#include "ExperimentDialog.h"

//#include <build-in>

using namespace Lazarus;

//void cb_concrate(int code, string message)
//{
//    qobject_cast<MainWindow*>(QApplication::topLevelWidgets()[0])->UpdateResourceManager();
//}

void MainWindow::MassiveTest()
{
//    if(isScriptSaved)
//    {
//        ShowError("请先保存脚本");
//        return;
//    }
//    
//    LazScript copy = LazGUI::Pool::Script;
    
   //FIXME 这是测试做的，倒时候请修复
    LazScript copy;
    AnalysisScript("/Users/yanzixu/Documents/header_test.lazScript", copy);
    
    ExperimentDialog dlg;
    dlg.SetScript(copy);
    dlg.exec();
}

void MainWindow::ShowError(QString errorMsg)
{
    QMessageBox msg;
    msg.setText(errorMsg);
    msg.exec();
}

bool MainWindow::PhaseOneScriptCondition(const scriptUnit &inUnit,QString& error_msg,bool from_cache,QString baseName)
{
    //phase script,and collect garbage;
    error_msg = "";
    vector<string> newInstances;
    scriptUnit copy = inUnit;
    
    bool return_type=true;
    
    //首先检测哪些实例是新的，这样后续可以无条件释放这些实例
    for (int i=0; i<inUnit.inVars.size(); i++) {
        if(!WatchDog::InstanceExist(inUnit.inVars[i])) newInstances.push_back(inUnit.inVars[i]);
    }
    
    for (int i=0; i<inUnit.outVars.size(); i++) {
        if(!WatchDog::InstanceExist(inUnit.outVars[i])) newInstances.push_back(inUnit.outVars[i]);
    }
    
    try{
        
        if(!from_cache
           || !LazGUI::CanPharseCache(copy, baseName.toStdString())
           || !LazGUI::PhaseOneScriptFromCache(copy, baseName.toStdString()))
        {
            return_type = PhaseOneScript(inUnit);
        }
    }
    catch(std::string& laz_exception_str)
    {
        string currentException;
        currentException="ERROR! Exception Get : "+laz_exception_str;
        error_msg = QString::fromStdString(currentException);
        //garbage collector;
        for (int i=0; i<newInstances.size(); i++) {
            WatchDog::DeleteInstance(newInstances[i]);
        }
        
        return false;
    }
    catch( itk::ExceptionObject & err )
    {
        string itkExcept=(string)err.GetFile()+"Has Error"+(string)err.what();
        error_msg = QString::fromStdString(itkExcept);
        //garbage collector;
        for (int i=0; i<newInstances.size(); i++) {
            WatchDog::DeleteInstance(newInstances[i]);
        }
        return false;
    }
    catch(std::exception& err)
    {
        error_msg = QString::fromStdString(err.what());
        //garbage collector;
        for (int i=0; i<newInstances.size(); i++) {
            WatchDog::DeleteInstance(newInstances[i]);
        }
        return false;
    }
    catch(...)
    {
        error_msg = "Unknown";
        //garbage collector;
        for (int i=0; i<newInstances.size(); i++) {
            WatchDog::DeleteInstance(newInstances[i]);
        }
        return false;
    }

    //如果脚本出错，自动回收垃圾
    if(!return_type)
    {
        for (int i=0; i<newInstances.size(); i++) {
            WatchDog::DeleteInstance(newInstances[i]);
        }
    }
    
    return return_type;
    
}

bool MainWindow::phase_first_script_threaded()
{
    //GUI_BEGIN
    //Lazarus::scriptUnit old_unit = LazGUI::Pool::ScriptTmp.front();
    Lazarus::scriptUnit unit = LazGUI::Pool::ScriptTmp.front();
    QFileInfo finfo(QString::fromStdString(scriptName));
    QString baseName = finfo.baseName();
    bool result = false;
    QString error_msg;
    
    if(LazGUI::Pool::is_reading_from_cache)
    {
        result = PhaseOneScriptCondition(unit, error_msg,true,baseName);
    }else{
        result = PhaseOneScriptCondition(unit, error_msg);
    }
    
    if(!result)
    {
        ShowError(error_msg);
        this->isScriptSaved = false;
        this->scriptName = "";
        return false;
    }
    
    UpdateResourceManager();
    
    return true;
}

void MainWindow::phase_next_script_threaded(int code,QString error_msg)
{
    //首先判断是否出错
    if(code == 1)
    {
        ShowError(error_msg);
        this->isScriptSaved = false;
        this->scriptName = "";
        return;
    }
    
    

    QFileInfo finfo(QString::fromStdString(scriptName));
    QString baseName = finfo.baseName();
    
    Lazarus::scriptUnit old_unit = LazGUI::Pool::ScriptTmp.front();
    
    LazGUI::Pool::Script.push_back(old_unit);
    LazGUI::Pool::alg_counter++;
    
    UpdateResourceManager();
    
    //执行下一步：
    LazGUI::Pool::ScriptTmp.pop_front();//退出队列最前端的那个脚本
    
    if(LazGUI::Pool::ScriptTmp.size() == 0)
    {
        //如果没有脚本那就是执行完毕了
        ui->infoLabel->setText("Script running finished");
        ui->progressBar->setValue(0);
        return;
    }
    
    ui->progressBar->setValue(ui->progressBar->value()+1);
    
    Lazarus::scriptUnit unit = LazGUI::Pool::ScriptTmp.front();
    //如果该操作涉及到GUI问题，那么这就必须要在主线程中执行
    while (LazGUI::Pool::gui_operations.end() !=std::find(LazGUI::Pool::gui_operations.begin(), LazGUI::Pool::gui_operations.end(), unit.procName))
    {
        cout<<"this procedure must be processed in main thread"<<endl;
        
        bool result;
        QString current_error;
        if(LazGUI::Pool::is_reading_from_cache)
        {
            result = PhaseOneScriptCondition(unit, current_error,true,baseName);
        }else{
            result = PhaseOneScriptCondition(unit, current_error);
        }
        if(!result)
        {
            ShowError(current_error);
            return;
        }
        LazGUI::Pool::Script.push_back(unit);
        LazGUI::Pool::alg_counter++;
        UpdateResourceManager();
        ui->progressBar->setValue(ui->progressBar->value()+1);
        LazGUI::Pool::ScriptTmp.pop_front();
        if(LazGUI::Pool::ScriptTmp.size() == 0)
        {
            //如果没有脚本那就是执行完毕了
            ui->infoLabel->setText("Script running finished");
            ui->progressBar->setValue(0);
            return;
        }
        unit = LazGUI::Pool::ScriptTmp.front();
        cout<<"main thread procedure running complete."<<endl;
    }

    //执行多线程
    
    QThread* thread = new QThread;
    ScriptReader* solver = new ScriptReader(baseName);
    solver->moveToThread(thread);
    connect(thread, SIGNAL(started()), solver, SLOT(process()));
    connect(solver, SIGNAL(finished(int,QString)), this, SLOT(phase_next_script_threaded(int,QString)));
    connect(solver, SIGNAL(finished(int,QString)), thread, SLOT(quit()));
    connect(solver, SIGNAL(finished(int,QString)), solver, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
    
}


void MainWindow::alg_start(QString msg)
{
    QString color_msg = "<font color = 'blue'>"+msg+"</font>";
    this->ui->infoLabel->setText(color_msg);
    ui->progressBar->setValue(0.5*ui->progressBar->maximum());
}

void MainWindow::alg_end(QString msg)
{
    QString color_msg = "<font color = 'red'>"+msg+"</font>";
    this->ui->infoLabel->setText(color_msg);
    ui->progressBar->setValue(0);
}

void MainWindow::alg_error(QString msg)
{
    //cout<<"GOT!"<<endl;
    QMessageBox msgbox;
    msgbox.setText(msg);
    msgbox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgbox.exec();
}

bool MainWindow::PhaseOneScriptFromCache(const scriptUnit& inUnit,const string& baseName)
{
    cout<<"********************Caching Sentence ********************"<<endl;
    cout<<"Current Procedure name  = "<<inUnit.procName<<endl;
    
    //const vector<Instance> * instList = WatchDog::ReturnInstanceList();
    
    
    
    if (WatchDog::ProcedureExist(inUnit.procName))
    {
        algIOTypes curProcIO=WatchDog::GetIoType(inUnit.procName);
        string inputVariables;
        cout<<"InVar Size = "<<inUnit.inVars.size()<<endl;
        for (int j=0;j<inUnit.inVars.size();j++)
        {
            
            string instance_name = inUnit.inVars[j];
            
            bool is_inst_cached = (isVarCached.end()!=isVarCached.find(instance_name));
            
            const string cache_name = baseName+"/"+instance_name+".cache";
            cout<<"read from cache "<<cache_name<<endl;
            ///如果找到实例，就说明之前已经创建过了，因此不用再次赋值。
            ///如果没有找到，那么就必须创建，并且进行赋值
            WatchDog::CreateInstance(curProcIO.inTypes[j],inUnit.inVars[j]);
            
            if(is_inst_cached) cout<<"instance has already been cached"<<endl;
            
            if(!is_inst_cached)
            {
                
                
                bool load_cache_res = WatchDog::GetInstance(inUnit.inVars[j]).objPtr->LoadCache(cache_name);
                
                if(!load_cache_res)
                {
                    return false;
                }else{
                    isVarCached.insert(std::make_pair(instance_name, true));
                }
                
            }
            
        }
        cout<<"outVar Size = "<<inUnit.outVars.size()<<endl;
        for (int j=0;j<inUnit.outVars.size();j++)
        {
            string instance_name = inUnit.outVars[j];
            const string cache_name = baseName+"/"+instance_name+".cache";
            cout<<"read from cache "<<cache_name<<endl;
            bool is_inst_cached = (isVarCached.end()!=isVarCached.find(instance_name));
            
            WatchDog::CreateInstance(curProcIO.outTypes[j],inUnit.outVars[j]);
            
            if(is_inst_cached) cout<<"instance has already been cached"<<endl;
            if(!is_inst_cached)
            {
                //输出的实例是无需赋初值的，因为是输出就一定会有值。
               
                bool load_cache_res = WatchDog::GetInstance(inUnit.outVars[j]).objPtr->LoadCache(cache_name);
                
                if(!load_cache_res){
                    return false;
                }else{
                    isVarCached.insert(std::make_pair(instance_name, true));
                }
                
            }

            //WatchDog::GetInstance(WatchDog::currentScript[i].outVars[j]).objPtr->SetValue(WatchDog::currentScript[i].outDefVals[j]);
        }
        
        cout<<"********************Caching Complete ********************"<<endl;
    }else{
        WatchDog::WriteLog(Lazarus::Exception::FireException(LAZ_ERROR_USERSPEC,"Procedure not exist in preread Lazarus algorithm list! Maybe you forget to copy it here?"),LOG_NEXTLINE,true,true);
        return false;
    }
    
    return true;

    
    
    
}

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

    this->isScriptSaved = true;
    scriptName = saveName.toStdString();

    delete doc;
    
}


void MainWindow::on_actionSaveAllCaches_triggered()
{
    if(LazGUI::Pool::is_alg_running)
    {
        QMessageBox msgbox;
        msgbox.setText("请等待后台算法运行完毕！");
        msgbox.exec();
        return;
    }
    
    
    if(!isScriptSaved)
    {
        QMessageBox msgbox;
        msgbox.setText("保存所有cache之前请先保存脚本！");
        msgbox.exec();
        return;
    }

    //只要把watchdog里面的内容全部存一遍就可以了
    
    vector<Instance>* instList = WatchDog::ReturnInstanceList();
    int size = instList->size();
    ui->progressBar->setMaximum(size);
    
    if(size>0)
    isSavingCache = true;
    
    for (int i=0; i<size; i++) {
    
        string inst_name = instList->at(i).name;
        
    QThread* thread = new QThread;
    CacheSaver* solver = new CacheSaver(scriptName,inst_name);
    solver->moveToThread(thread);
    connect(thread, SIGNAL(started()), solver, SLOT(process()));
    connect(solver, SIGNAL(started()), this, SLOT(CacheStart()));
    //connect(solver, SIGNAL(finished()), this, SLOT(CacheEnd()));
    connect(solver, SIGNAL(finished()), thread, SLOT(quit()));
    connect(solver, SIGNAL(finished()), solver, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(solver, SIGNAL(progress(double,QString,int)), this, SLOT(UpdateCacheStatus(double,QString,int)));
    thread->start();
        
    //sleep(100000);

    }
    //vector<Lazarus::Instance>* instList = WatchDog::ReturnInstanceList();

}

void MainWindow::on_actionDeleteAllCaches_triggered()
{

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
        double minVal=99999;
		double maxVal=-1;
		for (int z=0;z<layer;z++)
		{
            double currentMinVal=99999;
			double currentMaxVal=-1;
			cv::minMaxLoc(Ptr[z],&currentMinVal,&currentMaxVal);
			if (currentMaxVal>maxVal)
				maxVal=currentMaxVal;
            if (currentMinVal<minVal) minVal = currentMinVal;
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
    isScriptSaved(false),
    scriptName(""),
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
isSavingCache(false),
isReadingScript(false),
    //algThread(nullptr),
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
    saveCacheAction = new QAction(this);
    loadCacheAction = new QAction(this);
    addMaskAction ->setText("作为mask插入");
    addBaseImgAction->setText("作为基准图像插入");
    showInformationAction->setText("显示详细信息");
    saveCacheAction->setText("储存快照");
    loadCacheAction->setText("读取快照");
    
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

    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setMinimum(0);

    
    connect(showInformationAction, SIGNAL(triggered()), this, SLOT(ShowInfomation()));
    QObject::connect(addMaskAction, SIGNAL(triggered()), this, SLOT(on_actionAddMaskAction_triggered()));
    QObject::connect(addBaseImgAction, SIGNAL(triggered()), this, SLOT(on_actionAddBaseImgAction_triggered()));
    connect(saveCacheAction,SIGNAL(triggered()),this,SLOT(on_saveCache()));
    connect(loadCacheAction,SIGNAL(triggered()),this,SLOT(on_loadCache()));
    
    //RMBMenu->addAction(addBaseImgAction);
    //RMBMenu->addAction(addMaskAction);
    ui->ResourceManager->addAction(addMaskAction);
    ui->ResourceManager->addAction(addBaseImgAction);
    ui->ResourceManager->addAction(showInformationAction);
    ui->ResourceManager->addAction(saveCacheAction);
    ui->ResourceManager->addAction(loadCacheAction);
    ui->ResourceManager->setContextMenuPolicy(Qt::ActionsContextMenu);
    
    ui->StackShow->addAction(removeItem);
    ui->StackShow->setContextMenuPolicy(Qt::ActionsContextMenu);
    
    //更新需要GUI的算法
//    this->gui_operations = {"OPT_ADJUST_MASK_WND","OPT_ADJUST_THRES_2D_WND",
//    "OPT_CREATE_GE_WINDOW","OPT_CREATE_WND_2DIMAGE","OPT_CREATE_WND_3DIMAGE","OPT_GDCM_IN",
//    "OPT_GET_MASK_2D_FROM_2D_WND","OPT_GET_MASK_3D_FROM_3D_WND","OPT_POINT_CLOUD_VISUALIZE",
//    "OPT_READ_2D_GRAY_IMAGE","OPT_READ_DICOM_ITK","OPT_READ_MHD_ITK","OPT_READ_SERIES_IMAGES",
//    "OPT_SAVE_3D_IMAGE_TO_BMPS","OPT_SAVE_MHD","OPT_SHOW_HIST"};
    
    //FIXME  这边要删了！
//    LazScript newScript;
//    AnalysisScript("/Users/yanzixu/Documents/完整的血管分割流程.lazScript", newScript);
//    ExperimentDialog* dlg = new ExperimentDialog;
//    dlg->SetScript(newScript);
//    dlg->exec();
    
    connect(removeItem, SIGNAL(triggered()), this, SLOT(RemoveItem()));
    connect(ui->actionMassiveTest, SIGNAL(triggered()), this, SLOT(MassiveTest()));

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

void MainWindow::UpdateCacheStatus(double percentage, QString msg, int code)
{
    int pos = ui->progressBar->value();
    pos++;
    ui->progressBar->setValue(pos);
    if(code == 0)//失败了用红色
    {
        //QString show_msg = "<font color = 'red'>"+msg+"\n </font>";
        QString show_msg = msg +"\n";
        logs=logs+show_msg;
    }
    else
    {
        //QString show_msg = "<font color = 'blue'>"+msg+"\n </font>";
        QString show_msg = msg +"\n";
        logs=logs+show_msg;
    }

    ui->cmdShow->setText(logs);
    QTextCursor c = ui->cmdShow->textCursor();
    c.movePosition(QTextCursor::End);
    ui->cmdShow->setTextCursor(c);
    
    if(ui->progressBar->value()>=ui->progressBar->maximum())
    {
        isSavingCache = false;
        CacheEnd();
    }

}

void MainWindow::CacheStart()
{
    logs.clear();
    ui->infoLabel->setText("<font color = 'blue'>Begin Caching .... </font>");
    ui->cmdShow->setText(logs);
}

void MainWindow::CacheEnd()
{
    ui->infoLabel->setText("<font color = 'red'>Caching finished.... </font>");
    ui->progressBar->setValue(0);
}

void MainWindow::on_saveCache()
{
    if(!isScriptSaved)
    {
        QMessageBox msgBox;
        msgBox.setText("储存cache : 请首先保存一下脚本！");
        msgBox.exec();
        return;
    }

    if(0 == ui->ResourceManager->selectedItems().size()) return;
    GUI_BEGIN

        QTreeWidgetItem* selectedItem=ui->ResourceManager->selectedItems()[0];
        //如果选择的是父节点那么就要返回
        if(nullptr == selectedItem->parent()) return;

        string instance_name=selectedItem->text(0).toStdString();
        Instance inst=WatchDog::GetInstance(instance_name);//找不到会抛出异常的

        QDir dir;
        QFileInfo file(QString::fromStdString(scriptName));
        QString dir_name = file.baseName();
    
        cout<<dir_name.toStdString()<<endl;
    

        if(!dir.exists(dir_name))
            //QDir::mkpath(QString::fromStdString(scriptName));
            dir.mkpath(dir_name);

        const string cache_name = dir_name.toStdString()+"/"+instance_name+".cache";
        bool cache_saved = inst.objPtr->SaveCache(cache_name);

        if(!cache_saved)
        {
            QMessageBox msgBox;
            msgBox.setText("cache failed");
            msgBox.exec();
            return;
        }else
        {
            QString str = "instance " + QString::fromStdString(instance_name)+ " cache completed";
            alg_end(str);
        }


    GUI_END
    
}

void MainWindow::on_loadCache()
{
    if(!isScriptSaved)
    {
        QMessageBox msgBox;
        msgBox.setText("储存cache : 请首先保存一下脚本！");
        msgBox.exec();
        return;
    }
    
    //cout<<"loading..."<<endl;

    if(0 == ui->ResourceManager->selectedItems().size()) return;
    GUI_BEGIN

        QTreeWidgetItem* selectedItem=ui->ResourceManager->selectedItems()[0];
        //如果选择的是父节点那么就要返回
        if(nullptr == selectedItem->parent()) return;

        string instance_name=selectedItem->text(0).toStdString();
        Instance inst=WatchDog::GetInstance(instance_name);//找不到会抛出异常的

        //QDir dir;

        //if(!dir.exists(QString::fromStdString(scriptName)))
            //dir.mkdir(QString::fromStdString(scriptName));

    QFileInfo file(QString::fromStdString(scriptName));
    QString dir_name = file.baseName();


        const string cache_name = dir_name.toStdString()+"/"+instance_name+".cache";
        //cout<<cache_name<<endl;
        bool cache_saved = inst.objPtr->LoadCache(cache_name);

        if(!cache_saved)
        {
            QMessageBox msgBox;
            msgBox.setText("load cache failed");
            msgBox.exec();
            QString str = "instance " + QString::fromStdString(instance_name)+ " load cache failed";
            alg_end(str);
            return;
        }else
        {
            QString str = "instance " + QString::fromStdString(instance_name)+ " load cache completed";
            alg_end(str);
        }


    GUI_END
    
    
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

bool MainWindow::CanPharseCache(Lazarus::scriptUnit &unit, const string &baseName)
{
    if(unit.procName == "" || baseName == "") return false;
    
    const string path = baseName + "/";
    int in_size = unit.inVars.size();
    int out_size = unit.outVars.size();
    
    QFile file;
    
    for (int i=0; i<in_size; i++) {
        string currentName = unit.inVars[i];
        string file_name = path+currentName+".cache";
        if(!file.exists(QString::fromStdString(file_name))) return false;
    }
    
    for (int i=0; i<out_size; i++) {
        string currentName = unit.outVars[i];
        string file_name = path+currentName+".cache";
        if(!file.exists(QString::fromStdString(file_name))) return false;
    }
    
    return true;
}

//为了防止读取脚本出错，uncached的脚本还是按顺序执行,cached的脚本可以后台多线程直接从数据中拉取结果
//换言之，主线程读取uncached的脚本，分线程读取cached的脚本
void MainWindow::on_actionPhaseScript_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Open Script","","LazScript (*.lazScript)");
    GUI_BEGIN
    
    if(!fileName.isEmpty())
    {
        QDir dir;
        QFileInfo finfo(fileName);
        QString baseName = finfo.baseName();
        
        bool is_cache_exist = dir.exists(baseName);
        
        bool is_reading_from_cache = false;
        
        if(is_cache_exist)
        {
            QMessageBox msgBox;
            msgBox.addButton(QMessageBox::Ok);
            msgBox.addButton(QMessageBox::Cancel);
            msgBox.setText("找到该脚本的快照，是否从快照读取？");
            if(QMessageBox::Ok == msgBox.exec())
            {
                is_reading_from_cache = true;
            }
        }
        
        LazGUI::Pool::is_reading_from_cache = is_reading_from_cache;
        
        LazGUI::Pool::Script.clear();

        LazGUI::Pool::alg_counter=0;
        WatchDog::DumpMemory();
        
        LazScript newScript;
        
        if(!AnalysisScript(fileName.toStdString(),newScript))
        {
            QMessageBox msgBox;
            msgBox.addButton(QMessageBox::Ok);
            msgBox.addButton(QMessageBox::Cancel);
            msgBox.setText("脚本有误！");
            return;
        }
        
        this->isScriptSaved = true;
        this->scriptName = fileName.toStdString();
        
//        vector<scriptUnit> cached_script;
//        vector<scriptUnit> uncached_script;
//        if(is_cache_exist)
//        {
//            for (int i=0; i<newScript.size(); i++) {
//                Lazarus::scriptUnit unit = newScript[i];
//                bool can_cache = CanPharseCache(unit, baseName.toStdString());
//                if(can_cache)
//                    cached_script.push_back(unit);
//                else
//                    uncached_script.push_back(unit);
//                
//            }
//            
//            //下面读取cached
//            
//            
//            
//        }
        
        //下面开始读取脚本；
        
        LazGUI::Pool::CachedVarName.clear();
        for (int i=0; i<newScript.size(); i++) {
            LazGUI::Pool::ScriptTmp.push_back(newScript[i]);
        }
        
        ui->progressBar->setMaximum(newScript.size());
        //先把第一个脚本送进去
        if(phase_first_script_threaded()) phase_next_script_threaded(0, "");

        
        
        //isVarCached.resize(newScript.size(),false);
//        isVarCached.clear();
//        
//        
//        for (int i=0; i<newScript.size(); i++) {
//            
//            if(is_cache_exist)
//            {
//                Lazarus::scriptUnit unit = newScript[i];
//                bool can_cache = CanPharseCache(unit, baseName.toStdString());
//                bool cache_success = true;
//                if(can_cache)
//                {
//                    cache_success = PhaseOneScriptFromCache(unit, baseName.toStdString());
//                }
//                if(!cache_success || !can_cache)
//                {
//                    cout<<"cache failed! running in normal..."<<endl;
//                    PhaseOneScript(newScript[i]);
//                }else{
//                    cout<<"Cache Complete!"<<endl;
//                }
//            }
//            else
//            {
//                PhaseOneScript(newScript[i]);
//            }
//            LazGUI::Pool::Script.push_back(newScript[i]);
//            LazGUI::Pool::alg_counter++;
//            UpdateResourceManager();
//        }
//        
    }
//

    
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

void MainWindow::UpdateRes()
{
    //QMessageBox::warning(this, "AAAAA", "aaaaa");
    UpdateResourceManager();
}

void MainWindow::on_AlgSelector_textChanged()
{
    filter=ui->AlgSelector->toPlainText();
    UpdateAlgBrowser();
}

void MainWindow::AlgRun()
{

}

void MainWindow::on_AlgBrowser_doubleClicked(const QModelIndex &index)
{
    GUI_BEGIN
    
    if(LazGUI::Pool::is_alg_running)
    {
        QMessageBox msgBox;
        msgBox.setText("有算法正在执行，请等待算法执行完毕再运行别的算法！");
        msgBox.exec();
        return;
    }
    
    if(isSavingCache)
    {
        QMessageBox msgBox;
        msgBox.setText("正在保存快照，请等待保存完以后在进行更改！");
        msgBox.exec();
        return;
    }
    
    int selectedRow=index.row();
    QString selected_name=ui->AlgBrowser->item(selectedRow)->text();
    int old_scrip_num=LazGUI::Pool::Script.size();
    AlgDialog dlg(this,selected_name.toStdString());
    dlg.exec();
    
    //cout<<LazGUI::Pool::Script.size()<<endl;
    if(LazGUI::Pool::Script.size()>old_scrip_num)
    {
        scriptUnit unit = LazGUI::Pool::Script.back();
        if(LazGUI::Pool::gui_operations.end()!=std::find(LazGUI::Pool::gui_operations.begin(), LazGUI::Pool::gui_operations.end(), unit.procName))//functions required gui features must be done in main thread:
        {
            bool result;
            QString error_msg;
            result = PhaseOneScriptCondition(unit, error_msg,false);
            
            //cout<<"running finished."<<endl;
            
            if(!result)
            {
                ShowError(error_msg);
                LazGUI::Pool::Script.pop_back();
                LazGUI::Pool::alg_counter--;
            }
            UpdateResourceManager();
            //getchar();
            //cout<<"update finished."<<endl;
        }
        else
        {
            QThread* thread = new QThread;
            AlgSolver* solver = new AlgSolver;
            solver->moveToThread(thread);
            connect(thread, SIGNAL(started()), solver, SLOT(process()));
            connect(solver, SIGNAL(finished()), this, SLOT(UpdateRes()));
            connect(solver, SIGNAL(end(QString)), this, SLOT(alg_end(QString)));
            connect(solver, SIGNAL(error(QString)), this, SLOT(alg_error(QString)));
            connect(solver, SIGNAL(finished()), thread, SLOT(quit()));
            connect(solver, SIGNAL(finished()), solver, SLOT(deleteLater()));
            connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
            connect(solver, SIGNAL(start(QString)), this, SLOT(alg_start(QString)));
            thread->start();
        }
    }
    
    GUI_END

}

void MainWindow::UpdateResourceManager()
{
//    if(nullptr != algThread)
//    {
//        algThread->join();
//        delete algThread;
//        algThread = nullptr;
//    }
    
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
            
            if(LazGUI::Pool::is_alg_running)
            {
                QMessageBox msgBox;
                msgBox.setText("有算法正在执行，修改文字描述无效！");
                msgBox.exec();
                item->setText(0, QString::fromStdString(old_name));
                //UpdateResourceManager();
                //QMessageBox::information(this, , "ERROR!");
                return;
            }
            
            if(isSavingCache)
            {
                QMessageBox msgBox;
                msgBox.setText("正在保存cache!不能修改变量名！");
                msgBox.exec();
                item->setText(0, QString::fromStdString(old_name));
                //UpdateResourceManager();
                //QMessageBox::information(this, , "ERROR!");
                return;
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















