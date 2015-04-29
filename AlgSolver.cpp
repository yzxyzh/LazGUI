#include "AlgSolver.h"
#include "GUITypes.h"
#include <LazarusDataWarpper.h>
#include <LazarusWatchDog.h>
#include <LazarusProcedureFactory.h>

#include <QMessageBox>
#include <mutex>
#include <thread>

#include <QApplication>
#include <QFileInfo>
#include <QDir>
//#include "mainwindow.h"

using namespace std;



ScriptReader::ScriptReader(QString _baseName)
{
    this->baseName = _baseName;
}

void ScriptReader::process()
{
    try{
    
    std::mutex locker;
    
    vector<string> newInstances;
    
    locker.lock();
    LazGUI::Pool::is_alg_running = true;
    scriptUnit inUnit = LazGUI::Pool::ScriptTmp.front();
    locker.unlock();
    
    bool result = false;
    QString error_msg;
    
    if(LazGUI::Pool::is_reading_from_cache)
    {
        result = LazGUI::PhaseOneScriptCondition(inUnit, error_msg,true,baseName);
    }else{
        result = LazGUI::PhaseOneScriptCondition(inUnit, error_msg,false,"");
    }
    
    locker.lock();
    LazGUI::Pool::is_alg_running = false;
    locker.unlock();
    
    if(!result)
    {
        emit finished(1, error_msg);

    }else{
        string end_msg = "procedure : "+inUnit.procName+" finished with no problem";
        emit finished(0, QString::fromStdString(end_msg));
    }

    }//end try
    catch(std::exception& e)
    {
        emit finished(1, e.what());
    }
        
}

void AlgSolver::process()
{
    std::mutex locker;
    
    GUI_BEGIN
    
    newInstances.clear();
    
    locker.lock();
    LazGUI::Pool::is_alg_running = true;
    scriptUnit inUnit = LazGUI::Pool::Script.back();
    locker.unlock();
    
    string start_msg = "procedure : "+inUnit.procName+" running...";
    emit start(QString::fromStdString(start_msg));
    
    cout<<"********************Running Sentence ********************"<<endl;
    cout<<"Current Procedure name  = "<<inUnit.procName<<endl;
    if (WatchDog::ProcedureExist(inUnit.procName))
    {
        algIOTypes curProcIO=WatchDog::GetIoType(inUnit.procName);
        string inputVariables;
        cout<<"InVar Size = "<<inUnit.inVars.size()<<endl;
        for (int j=0;j<inUnit.inVars.size();j++)
        {
            if(!WatchDog::InstanceExist(inUnit.inVars[j]))
            {
                newInstances.push_back(inUnit.inVars[j]);
            }
            ///如果找到实例，就说明之前已经创建过了，因此不用再次赋值。
            ///如果没有找到，那么就必须创建，并且进行赋值
            if(WatchDog::CreateInstance(curProcIO.inTypes[j],inUnit.inVars[j]))
                WatchDog::GetInstance(inUnit.inVars[j]).objPtr->SetValue(inUnit.inDefVals[j]);
        }
        cout<<"outVar Size = "<<inUnit.outVars.size()<<endl;
        for (int j=0;j<inUnit.outVars.size();j++)
        {
            if(!WatchDog::InstanceExist(inUnit.outVars[j]))
            {
                newInstances.push_back(inUnit.outVars[j]);
            }
            
            //输出的实例是无需赋初值的，因为是输出就一定会有值。
            WatchDog::CreateInstance(curProcIO.outTypes[j],inUnit.outVars[j]);
            //WatchDog::GetInstance(WatchDog::currentScript[i].outVars[j]).objPtr->SetValue(WatchDog::currentScript[i].outDefVals[j]);
        }
        Lazarus::Procedure* newProc=Lazarus::ProcedureFactory::CreateProcedure(inUnit.procName);
        if (NULL == newProc)
        {
            WatchDog::WriteLog(Lazarus::Exception::FireException(LAZ_ERROR_USERSPEC,"Can NOT create procedure!"),LOG_NEXTLINE,true,true);
        }
        newProc->SetInput(inUnit.inputVarsFull);
        newProc->SetOutput(inUnit.outputVarsFull);
        newProc->Run();
        delete newProc;
        cout<<"*******************Procedure run complete*****************"<<endl;
        cout<<endl;
    }else{
        WatchDog::WriteLog(Lazarus::Exception::FireException(LAZ_ERROR_USERSPEC,"Procedure not exist in preread Lazarus algorithm list! Maybe you forget to copy it here?"),LOG_NEXTLINE,true,true);
    }
    
    locker.lock();
    LazGUI::Pool::is_alg_running = false;
    locker.unlock();
    
    //cout<<"everything's fine..."<<endl;
    emit finished();
    
    string end_msg = "procedure : "+inUnit.procName+" finished with no problem";
    
    emit end(QString::fromStdString(end_msg));

    return;
    
    GUI_END_THREADED
    
    string end_msg_error = "procedure : "+LazGUI::Pool::Script.back().procName+" finished with some problems";
    
    emit end(QString::fromStdString(end_msg_error));
    
    //如果出错跳出，必须要清空内容
    locker.lock();
    for (int i=0; i<newInstances.size(); i++) {
        WatchDog::DeleteInstance(newInstances[i]);
    }
    
    //cout<<"something's wrong..."<<endl;
    LazGUI::Pool::is_alg_running = false;
    LazGUI::Pool::Script.pop_back();
    LazGUI::Pool::alg_counter--;
    

    
    locker.unlock();

    emit finished();
    
    
}

CacheSaver::CacheSaver(std::string &path,std::string& instance_name) :
scriptName(path),
instanceName(instance_name)
{
}

void CacheSaver::process()
{
    GUI_BEGIN

    int saving_success = 0;
    int saving_failure = 0;
        //int total_count = 0;
    //vector<Lazarus::Instance>* instList = Lazarus::WatchDog::ReturnInstanceList();

        QDir  dir;
        QFileInfo file(QString::fromStdString(scriptName));
        QString dir_name = file.baseName();
        if(!dir.exists(dir_name))
            //QDir::mkpath(QString::fromStdString(scriptName));
            dir.mkpath(dir_name);

        //int size = instList->size();

        emit started();

       // for (int i = 0; i < size; ++i) {

            //Instance inst = instList->at(i);

            string name = this->instanceName;
            const string cache_name = dir_name.toStdString()+"/"+name+".cache";
            bool cache_saved = WatchDog::GetInstance(instanceName).objPtr->SaveCache(cache_name);

            //double per = (double)(i+1)/(double)size;

            if(cache_saved)
            {
                saving_success++;
                QString info = "caching "+QString::fromStdString(name)+" succeedded";
                emit progress(1.0,info,1);
            }
            else
            {
                saving_failure++;
                QString info = "caching "+QString::fromStdString(name)+" failed";
                emit progress(1.0,info,0);
            }



       // }

        emit finished();



    GUI_END_THREADED
}