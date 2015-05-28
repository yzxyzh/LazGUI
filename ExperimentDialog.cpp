#include "ExperimentDialog.h"
#include "ui_ExperimentDialog.h"

#include <qmessagebox.h>
#include <QToolBar>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>
#include <QFileDialog>

#include "newPathDlg.h"

#include <QDebug>
#include <iostream>
#include <fstream>

#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <QThread>

#include "bindDlg.h"

#include "GenPathDialog.h"


using namespace std;

void ExperimentDialog::GeneratePath()
{
    vector<string> pathNameLists;
    for (int i=0; i<pathes.size(); i++) {
        pathNameLists.push_back(pathes[i].name);
    }
    
    GenPathDialog a(this,pathNameLists);
    
    if(QDialog::Accepted == a.exec())
    {
        a.GenerateData();
        
        auto iter = std::find(pathNameLists.begin(), pathNameLists.end(), a.pathName);
        if(iter == pathNameLists.end()) return;
        
        int dist = std::distance(pathNameLists.begin(), iter);
        
        int pathSize = pathes[dist].list.size();
        
        pathSet newPathSet;
        newPathSet.is_path = a.isAbsolutePath;
        newPathSet.name = a.newName;
        newPathSet.list.clear();
        
        //打开一个路径
        QString basePathName = QFileDialog::getExistingDirectory(NULL, "Open Directory",
                                                         "/home",
                                                         QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
        //如果是绝对路径的话，我们需要创建一系列的路径
        if(newPathSet.is_path)
        {
            for (int i=0; i<pathSize; i++) {
                
                QString newDir = basePathName+"/"+QString::fromStdString(boost::lexical_cast<string>(i));
                QDir dir;
                dir.mkdir(newDir);
                
                newPathSet.list.push_back(newDir.toStdString());
            }
        }else{
            for (int i=0;i<pathSize; i++) {
                
                string newDir = basePathName.toStdString()+"/"+boost::lexical_cast<string>(i)+"."+a.extName;
                newPathSet.list.push_back(newDir);
                
            }
        }
        
        this->pathes.push_back(newPathSet);
        
        UpdatePathList();
        
    }
}

void ExperimentDialog::ShowError(QString msg)
{
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.exec();
}

void ExperimentDialog::OpenPath()
{
    QString filename = QFileDialog::getOpenFileName(this,"open path","","path file (*.txt)");
    
    string filenameStr = filename.toStdString();
    
    if("" == filenameStr) return;
    
    std::ifstream fs(filenameStr,ios::in|ios::binary);
    
    pathSet newSet;
    fs>>newSet.name;
    fs>>newSet.is_path;
    int size;
    fs>>size;
    
    for (int i=0; i<size; i++) {
        string specPath;
        fs>>specPath;
        newSet.list.push_back(specPath);
    }
    
    pathes.push_back(newSet);
    
    UpdatePathList();
}

void ExperimentDialog::AnalyseScriptBinding()
{
    if(0 == script.size())
    {
        ShowError("脚本大小为0！请重新注入脚本");
        return;
    }
    ParallelTest test(script);
    this->BindLines = test.gui_need_replace;
    
//    cout<<"test.gui_need_replace size = "<<test.gui_need_replace.size()<<endl;
//    
//    cout<<" No.0 = "<<test.gui_need_replace[0]<<endl;
//    cout<<" No.1 = "<<test.gui_need_replace[1]<<endl;
    
    this->isScriptAnalysed = true;
}

void ExperimentDialog::UpdateBindWindow()
{
    
    ui->scriptBinder->clear();
    
    if(!isScriptAnalysed) AnalyseScriptBinding();
    
    int lineSize = BindLines.size();
    
    ParallelTest test(script);
    
    //cout<<lineSize<<endl;
    
    for (int i=0; i<lineSize; i++) {
        
         int lineNum = BindLines[i];
        
        QString BindStatus ="line "+QString::fromStdString(boost::lexical_cast<string>(lineNum))+" procedure "+QString::fromStdString(script[lineNum].procName);
        
        //cout<<script[lineNum].procName<<endl;

        string procName = script[lineNum].procName;
        
        auto iter = std::find(test.replace_gui_procedures.begin(), test.replace_gui_procedures.end(), procName);
        
        int dist = std::distance(test.replace_gui_procedures.begin(), iter);
        
        if(test.gui_input_var_num[dist]>0) BindStatus+=" IN ";
        for (int in_index=0; in_index<test.gui_input_var_num[dist]; in_index++) {
            
            BindStatus=BindStatus+QString::fromStdString(boost::lexical_cast<string>(in_index));
            string pathSetName;
            int is_bind = isBinded(lineNum, true, in_index, pathSetName);
            if(is_bind>0)
            {
                BindStatus=BindStatus+" bind with "+QString::fromStdString(pathSetName)+" ";
            }
            else
            {
                BindStatus+=" NOT binded ";
            }
        }
        
        if(test.gui_output_var_num[dist]>0)  BindStatus+=" OUT ";
        for (int out_index=0; out_index<test.gui_output_var_num[dist]; out_index++) {
            BindStatus=BindStatus+QString::fromStdString(boost::lexical_cast<string>(out_index));
            string pathSetName;
            int is_bind = isBinded(lineNum, false, out_index, pathSetName);
            if(is_bind>0)
            {
                BindStatus=BindStatus+" bind with "+QString::fromStdString(pathSetName)+" ";
            }
            else
            {
                BindStatus=BindStatus+" NOT binded ";
            }
        }
        QStringList list;
        list<<BindStatus;
        QTreeWidgetItem* itm = new QTreeWidgetItem(list);
        if(-1==BindStatus.indexOf("NOT binded"))
        {
            itm->setTextColor(0, QColor("blue"));
        }
        else{
            itm->setTextColor(0, QColor("red"));
        }
        ui->scriptBinder->addTopLevelItem(itm);
    }
    
}

int ExperimentDialog::isBinded(const int line_num, const bool is_input, const int index,string& pathSetName)
{
    int retVal = 0;
    
    for (int i=0; i<Bindings.size(); i++) {
        
        string currBind = Bindings[i];
        vector<string> elems;
        boost::split(elems, currBind, boost::is_any_of("@"));
        
        int currLineNum = boost::lexical_cast<int>(elems[2]);
        
        if(currLineNum == line_num)
        {
            if("IN"!=elems[4] && "OUT"!=elems[4]) break;
            if("IN" == elems[4] && !is_input) break;
            if("OUT" == elems[4] && is_input) break;
            
            int currIndex = boost::lexical_cast<int>(elems.back());
            if(currIndex != index) break;
            
            pathSetName = elems[1];
            
            return 1;
        }
        
        
        
    }
    
    
    
    return retVal;
}

void ExperimentDialog::SetScript(Lazarus::LazScript &_in_script)
{
    this->script = _in_script;
    ui->localProgress->setMaximum(_in_script.size());
    UpdateBindWindow();
}

void ExperimentDialog::Encode(const string& input,string& output)
{
    output = input;
    std::replace(output.begin(), output.end(),' ', '@');
}

void ExperimentDialog::Decode(const string& input,string& output)
{
    output = input;
    std::replace(output.begin(), output.end(), '@', ' ');
}

void ExperimentDialog::ReadData()
{
    QFileInfo fileInfo("pathes.txt");
    if(fileInfo.exists())
    {
        fstream fs("pathes.txt",ios::in|ios::binary);
        
        size_t pathes_size;
        fs>>pathes_size;
        
        for (int i=0; i<pathes_size; i++) {
            
            pathSet set;
            string name;
            bool   is_path;
            size_t listSize;
            fs>>name>>is_path>>listSize;
            set.name = name;
            set.is_path = is_path;
            set.list.resize(listSize);
            
            for (int j=0; j<set.list.size(); j++) {
                
                string encoded;
                string decoded;
                fs>>encoded;
                Decode(encoded, decoded);
                set.list[j]=decoded;
            }
            
            pathes.push_back(set);
        
        }

        
        fs.close();
        
        //UpdatePathList();
    }
    
}

void ExperimentDialog::SaveFile()
{
    fstream fs("pathes.txt",ios::out|ios::binary);
    
    fs<<pathes.size()<<" ";
    
    for (int i=0; i<pathes.size(); i++) {
        fs<<pathes[i].name<<" ";
        fs<<pathes[i].is_path<<" ";
        fs<<pathes[i].list.size()<<" ";
        for (int j=0; j<pathes[i].list.size(); j++) {
            
            string encoder;
            Encode(pathes[i].list[j], encoder);
            
            fs<<encoder<<" ";
        }
    }
    
    fs.close();
    
    QMessageBox msgBox;
    msgBox.setText("保存文件完毕");
    msgBox.exec();
}

ExperimentDialog::ExperimentDialog(QWidget *parent):
QDialog(parent),
isScriptAnalysed(false),
ui(new Ui::ExperimentDialog)
{
    //pTest = NULL;
    
    ui->setupUi(this);
    ui->pathList->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->pathList->setContextMenuPolicy(Qt::ActionsContextMenu);
    
    ui->globalProgress->setValue(0);
    ui->localProgress->setValue(0);
    ui->globalProgress->setMinimum(0);
    ui->localProgress->setMinimum(0);
    
    
    pathlist_delete = new QAction(this);
    pathlist_bind = new QAction(this);
    pathlist_add_path = new QAction(this);
    
    pathlist_delete->setText("删除该路径集");
    pathlist_bind->setText("绑定该路径集");
    pathlist_add_path->setText("增加具体路径");
    
    ui->pathList->addAction(pathlist_bind);
    ui->pathList->addAction(pathlist_delete);
    ui->pathList->addAction(pathlist_add_path);
    
    connect(pathlist_add_path, SIGNAL(triggered()), this, SLOT(add_spec_path()));
    connect(pathlist_bind, SIGNAL(triggered()), this, SLOT(bind()));
    connect(pathlist_delete, SIGNAL(triggered()), this, SLOT(delete_path_set()));
    connect(ui->pathList, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(pathListClick(QTreeWidgetItem*,int)));
    connect(ui->addPathBtn, SIGNAL(clicked()), this, SLOT(addNewPath()));
    connect(ui->saveBtn, SIGNAL(clicked()), this, SLOT(SaveFile()));
    connect(ui->runBtn, SIGNAL(clicked()), this, SLOT(Run()));
    
    connect(ui->readPathBtn, SIGNAL(clicked()), this, SLOT(OpenPath()));
    
    connect(ui->GeneratePathBtn, SIGNAL(clicked()), this, SLOT(GeneratePath()));
    
    
    ReadData();
    
    //ParallelTest test(script);
    
    UpdatePathList();

}

void ExperimentDialog::addNewPath()
{
    newPathDlg newDlg;
    if(QDialog::Accepted == newDlg.exec())
    {
        QString new_name;
        bool new_is_path = false;
        newDlg.returnName(new_name, new_is_path);
        //cout<<new_name.toStdString()<<"  "<<new_is_path<<endl;
        
        if(new_name!="")
        {
            pathSet newSet;
            newSet.name = new_name.toStdString();
            newSet.is_path = new_is_path;
            pathes.push_back(newSet);
            UpdatePathList();
        }
    }
    
}

void ExperimentDialog::Run()
{
    ParallelTest* newTest = new ParallelTest(script);
    int size = newTest->gui_need_replace.size();
    
    if(size > Bindings.size())
    {
        ShowError("请绑定所有的内容之后再运行测试！");
        delete newTest;
        return;
    }
    
    int min_size = pathes[0].list.size();
    
    for (int i=0; i<pathes.size(); i++) {
        if(pathes[i].list.size()<min_size) min_size = pathes[i].list.size();
    }

    ui->globalProgress->setMaximum(min_size);

    //绑定
    for (int i=0; i<size; i++) {
        
        int bidLine = newTest->gui_need_replace[i];
        
        ParallelTest::Bind bid;
        
        bid.inPaths.resize(min_size);
        bid.outPaths.resize(min_size);
        
        for (int j=0; j<min_size; j++) {
            bid.inPaths[j].resize(100);
            bid.outPaths[j].resize(100);
        }
        
        for (int j=0; j<Bindings.size(); j++ ) {
        string currLine = Bindings[j];
        
        vector<string> elems;
        boost::split(elems, currLine, boost::is_any_of("@"));
        
        string pathSetName = elems[1];
        int lineNum = boost::lexical_cast<int>(elems[2]);
        
            if(lineNum!=bidLine) continue;
            
        int in_out_index = boost::lexical_cast<int>(elems.back());
        
        
        string procName = script[lineNum].procName;
        
            int path_index = -1;
            
            for (int pp=0; pp<pathes.size(); pp++) {
                if(pathes[pp].name == pathSetName)
                {
                    path_index = pp;
                    break;
                }
            }
            
            if(path_index<0)
            {
                ShowError("path_index < 0");
                return;
            }
            
        if("IN" == elems[4])
        {
            for (int p=0; p<min_size; p++) {
                bid.inPaths[p][in_out_index]=pathes[path_index].list[p];
            }
        }
        
        if("OUT" == elems[4])
        {
            for (int p=0; p<min_size; p++) {
                bid.outPaths[p][in_out_index]=pathes[path_index].list[p];
            }
        }
            
        }//end for j<binding.size()
        
        newTest->myBind(bidLine, bid);
    }
    
    //connect(newTest, SIGNAL(finished(int,QString)), this, SLOT(ParallelTestCallBack(int,QString)));



    QThread* thread = new QThread;
    newTest->moveToThread(thread);
    connect(thread, SIGNAL(started()), newTest, SLOT(process()));
    connect(newTest, SIGNAL(finished(int,QString)), this, SLOT(ParallelTestCallBack(int,QString)));
    connect(newTest, SIGNAL(allDone()), this, SLOT(ParallelTestAllDone()));
    thread->start();

    ui->runBtn->setEnabled(false);
    
}

void ExperimentDialog::ParallelTestCallBack(int code, QString errorMsg)
{
    if(1 == code)
    {
        ShowError(errorMsg);
        return;
    }

    if(0 == code)
    {
        int val = ui->globalProgress->value();
        
        cout<<"current value = "<<val<<endl;
        cout<<"max value = "<<ui->globalProgress->maximum()<<endl;
        
        ui->globalProgress->setValue(val+1);
        ui->localProgress->setValue(0);
    }

    if(code >= 2)
    {
        int val = ui->localProgress->value();
        ui->localProgress->setValue(val+1);
        
        cout<<"current value = "<<val<<endl;
        cout<<"max value = "<<ui->localProgress->maximum()<<endl;

    }

}

void ExperimentDialog::add_spec_path()
{
    int index = findSelectedPathesIndex();
    if(index<0) return;
    
    QString path_or_name;
    bool is_path = pathes[index].is_path;
    
    if(is_path)
    {
        path_or_name = QFileDialog::getExistingDirectory(NULL, "Open Directory",
                                                        "/home",
                                                        QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    }
    else
    {
        path_or_name = QFileDialog::getOpenFileName(NULL,"Open File");
    }
    
    if(path_or_name!="")
    pathes[index].list.push_back(path_or_name.toStdString());
    
    //UpdatePathList();
    UpdatePathDetail();
}

int ExperimentDialog::findSelectedPathesIndex()
{
    int index = -1;
    
    ui->pathDetail->clear();
    if(0 == ui->pathList->selectedItems().size())
    {
        return index;
    }
    
    QTreeWidgetItem* itm = ui->pathList->selectedItems()[0];
    string val = itm->text(0).toStdString();
    
    for (int i=0; i<pathes.size(); i++) {
        if(val == pathes[i].name)
        {
            index = i;
            return index;
        }
    }
    return index;
}

void ExperimentDialog::UpdatePathList()
{
    ui->pathList->clear();
    for (int i=0; i<pathes.size(); i++) {
       
        QStringList str;
        str<<QString::fromStdString(pathes[i].name);
        QTreeWidgetItem* itm = new QTreeWidgetItem(str);
        itm->setFlags(Qt::ItemIsEnabled|Qt::ItemIsEditable|Qt::ItemIsSelectable);
        ui->pathList->addTopLevelItem(itm);
    }
    UpdatePathDetail();
}

void ExperimentDialog::UpdatePathDetail()
{
    ui->pathDetail->clear();
    if(0 == ui->pathList->selectedItems().size())
    {
        return;
    }
    
    QTreeWidgetItem* itm = ui->pathList->selectedItems()[0];
    string val = itm->text(0).toStdString();
    
    for (int i=0; i<pathes.size(); i++) {
        if(val == pathes[i].name)
        {
            for (int j=0; j<pathes[i].list.size(); j++) {
                QStringList str;
                str<<QString::fromStdString(pathes[i].list[j]);
                QTreeWidgetItem* itm_sub = new QTreeWidgetItem(str);
                itm_sub->setFlags(Qt::ItemIsEnabled|Qt::ItemIsEditable|Qt::ItemIsSelectable);
                ui->pathDetail->addTopLevelItem(itm_sub);
            }
            
            break;
        }
    }
    
}

void ExperimentDialog::bind()
{
    
    
    if(ui->pathList->selectedItems().size() == 0) return;
    QTreeWidgetItem* itm = ui->pathList->selectedItems()[0];
    string pathSetName = itm->text(0).toStdString();
    
    int pathSetIndex = -1;
    for (int i=0; i<pathes.size(); i++) {
        if(pathes[i].name == pathSetName)
        {
            pathSetIndex = i;
            break;
        }
    }
    
    bindDlg dlg(NULL,script);
    if(QDialog::Accepted == dlg.exec())
    {
        int line_num;
        bool is_input;
        int  index;
        dlg.ReturnBindResult(line_num, is_input, index);
        
        string procName = script[line_num].procName;
        ParallelTest test(script);
        ParallelTest::PathReturnType type = test.isPathOrFileName(procName,is_input, index);

        if(type == ParallelTest::PATH && !pathes[pathSetIndex].is_path)
        {
            ShowError("目标需要路径，但是路径集是文件名！");
            return;
        }
        
        if(type == ParallelTest::FILENAME && pathes[pathSetIndex].is_path)
        {
            ShowError("目标需要文件名，但是路径集是绝对路径！");
            return;
        }
        
        if(type == ParallelTest::ERROR)
        {
            ShowError("绑定出错！");
            return;

        }
        
        string newBind = "BIND@"+pathSetName+"@"+boost::lexical_cast<string>(line_num)+"@"+procName;
        
        if(is_input)
            newBind+="@IN@";
        else
            newBind+="@OUT@";
        
        newBind+=boost::lexical_cast<string>(index);
        RefineNewBind(newBind);
        
        UpdateBindWindow();
    }
    
    cout<<"current bindings size = "<<Bindings.size()<<endl;
    
}

void ExperimentDialog::RefineNewBind(string &newBind)
{
    vector<string> currElems;
    boost::split(currElems, newBind, boost::is_any_of("@"));
    
    //注意这边是要判断如果以前有其他的内容跟这个proc绑定，那就要把它去掉
    for (auto iter = Bindings.begin(); iter!=Bindings.end(); ++iter) {
        
        vector<string> elems;
        
        boost::split(elems, *iter, boost::is_any_of("@"));
        
        bool isAlreadyBind = true;
        
        for (int i=0; i<elems.size(); i++) {
            
            if(elems[i] != currElems[i] && i!=1)
            {
                isAlreadyBind = false;
                break;
            }
            
        }
        
        if(isAlreadyBind)
        {
            iter = Bindings.erase(iter);
            break;
        }
        
    }
    
    Bindings.push_back(newBind);
    
}

void ExperimentDialog::delete_path_set()
{
    if(ui->pathList->selectedItems().size()==0) return;
    
    QTreeWidgetItem* itm = ui->pathList->selectedItems()[0];
    string text = itm->text(0).toStdString();
    
    for (auto iter = pathes.begin(); iter!=pathes.end(); ++iter) {
        
        if(iter->name == text)
        {
            iter = pathes.erase(iter);
            break;
        }
        
    }
    
    //然后还要删除所有bindings里面的有关内容
    for (auto iter = Bindings.begin(); iter!=Bindings.end();) {
        
        vector<string> elems;
        
        boost::split(elems, *iter, boost::is_any_of("@"));
        
        if(text == elems[1])
        {
            iter = Bindings.erase(iter);
        }else{
            iter++;
        }
        
    }
    
    UpdatePathList();
    UpdateBindWindow();

    
}

void ExperimentDialog::ParallelTestAllDone()
{
    ui->localProgress->setValue(0);
    ui->globalProgress->setValue(0);
    ShowError("所有的测试都已经完成了！");
    ui->runBtn->setEnabled(true);
}

void ExperimentDialog::pathListClick(QTreeWidgetItem *itm, int column)
{
    UpdatePathDetail();
}

ExperimentDialog::~ExperimentDialog()
{
    delete ui;
    delete pathlist_add_path;
    delete pathlist_bind;
    delete pathlist_delete;
    
    //if(pTest) delete pTest;
}