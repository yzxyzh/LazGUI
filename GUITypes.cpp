#include "GUITypes.h"
#include <QFile>
#include <LazarusProcedureFactory.h>

using namespace Lazarus;

namespace LazGUI {
    int Pool::alg_counter=0;
    vector<Lazarus::scriptUnit> Pool::Script={};
    deque<Lazarus::scriptUnit> Pool::ScriptTmp={};
    bool Pool::is_alg_running = false;
    vector<string> Pool::CachedVarName = {};
    bool Pool::is_reading_from_cache = false;
    vector<string> Pool::gui_operations = {"OPT_ADJUST_MASK_WND","OPT_ADJUST_THRES_2D_WND",
        "OPT_CREATE_GE_WINDOW","OPT_CREATE_WND_2DIMAGE","OPT_CREATE_WND_3DIMAGE","OPT_GDCM_IN",
        "OPT_GET_MASK_2D_FROM_2D_WND","OPT_GET_MASK_3D_FROM_3D_WND","OPT_POINT_CLOUD_VISUALIZE",
        "OPT_READ_2D_GRAY_IMAGE","OPT_READ_DICOM_ITK","OPT_READ_MHD_ITK","OPT_READ_SERIES_IMAGES",
        "OPT_SAVE_3D_IMAGE_TO_BMPS","OPT_SAVE_MHD","OPT_SHOW_HIST","ALG_VESSEL_SEPARATOR_3D",
        "OPT_GET_MASK_2D_FROM_3D_WND","OPT_CREATE_HEPATIC_AND_PORTAL_WINDOW","ALG_HEPATIC_PORTAL_TEST",
        "ALG_SEPARATE_VESSEL_BY_H_P_MASKS"};
    
    bool PhaseOneScriptCondition(const scriptUnit &inUnit,QString& error_msg,bool from_cache,QString baseName)
    {
        
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
    
    bool PhaseOneScriptFromCache(const Lazarus::scriptUnit& inUnit,const string& baseName)
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
                
                bool is_inst_cached = (Pool::CachedVarName.end()!=std::find(Pool::CachedVarName.begin(), Pool::CachedVarName.end(), instance_name));
                
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
                        Pool::CachedVarName.push_back(instance_name);
                        //isVarCached.insert(std::make_pair(instance_name, true));
                    }
                    
                }
                
            }
            cout<<"outVar Size = "<<inUnit.outVars.size()<<endl;
            for (int j=0;j<inUnit.outVars.size();j++)
            {
                string instance_name = inUnit.outVars[j];
                const string cache_name = baseName+"/"+instance_name+".cache";
                cout<<"read from cache "<<cache_name<<endl;
                //bool is_inst_cached = (isVarCached.end()!=isVarCached.find(instance_name));
                bool is_inst_cached = (Pool::CachedVarName.end()!=std::find(Pool::CachedVarName.begin(), Pool::CachedVarName.end(), instance_name));
                
                WatchDog::CreateInstance(curProcIO.outTypes[j],inUnit.outVars[j]);
                
                if(is_inst_cached) cout<<"instance has already been cached"<<endl;
                if(!is_inst_cached)
                {
                    //输出的实例是无需赋初值的，因为是输出就一定会有值。
                    
                    bool load_cache_res = WatchDog::GetInstance(inUnit.outVars[j]).objPtr->LoadCache(cache_name);
                    
                    if(!load_cache_res){
                        return false;
                    }else{
                        //isVarCached.insert(std::make_pair(instance_name, true));
                        Pool::CachedVarName.push_back(instance_name);
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

    
    bool CanPharseCache(Lazarus::scriptUnit &unit, const string &baseName)
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
    
    bool PhaseOneScript(const Lazarus::scriptUnit& inUnit)
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


}
