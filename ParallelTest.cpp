#include "ParallelTest.h"
#include <LazarusProcedureFactory.h>
#include <QFileInfo>
#include <QDir>
#include "GUITypes.h"
#include <BasicIOs.h>
#include <LazarusScriptProcessor.h>

#include <boost/algorithm/string.hpp>

using namespace LazGUI;


ParallelTest::PathReturnType ParallelTest::isPathOrFileName(string procName, bool is_input, int index)
{
    ParallelTest::PathReturnType type = ParallelTest::ERROR;
    
    auto iter = std::find(replace_gui_procedures.begin(), replace_gui_procedures.end(), procName);
    
    if(iter == replace_gui_procedures.end()) return type;
    
    //cout<<"OK!"<<endl;
    
    int dist = std::distance(replace_gui_procedures.begin(), iter);
    
    if(dist<0) return type;
    
    //cout<<"OK!"<<endl;
    
    vector<string> splitted;
    boost::split(splitted, gui_is_path[dist], boost::is_any_of("_"));
    
    //int in_begin = 0;
    int out_begin = -1;
    
    for (int i=0; i<splitted.size(); i++) {
        if(splitted[i] == "OUT")
        {
            out_begin = i;
            break;
        }
    }

    if(out_begin <=0) return type;
    
    //cout<<"OK!"<<endl;
    
    if(is_input)
    {
        if(index >= out_begin) return type;
        //cout<<"OK_1!"<<endl;
        string tfStr = splitted[index+1];
        
        //cout<<"tfStr = "<<tfStr<<endl;
        if("TRUE" == tfStr) return ParallelTest::PATH;
        if("FALSE" == tfStr) return ParallelTest::FILENAME;
        return type;
    }
    else
    {
        if(index+out_begin+1>=splitted.size()) return type;
        //cout<<"OK_2!"<<endl;
        string tfStr = splitted[index+out_begin+1];
        //cout<<"tfStr = "<<tfStr<<endl;
        if("TRUE" == tfStr) return ParallelTest::PATH;
        if("FALSE" == tfStr) return ParallelTest::FILENAME;
        return type;
        
    }
    
    //cout<<"OK!"<<endl;
    
    return type;
}

void ParallelTest::process()
{
    QString error_msg;
    //先进行一些检查
    maxCycle = 0;
    for (int i=0; i<gui_need_replace.size(); i++) {
        
        int line = gui_need_replace[i];
        
        if(pathMap.end() == pathMap.find(line))
        {
            emit finished(1, "Please bind pathes to script first");
            return;
        }
        
        Bind& bid = pathMap[line];
        
        //cout<<bid.inPaths.size()<<"   "<<bid.outPaths.size()<<endl;
        
        //顺便算一下循环次数
        if(maxCycle<=static_cast<int>(bid.inPaths.size()))//注意这边两个比较要cast一下,否则如果maxCycle小于0就会出问题
        {
            //cout<<"enter this if!"<<endl;
            maxCycle = bid.inPaths.size();
        }
        if(static_cast<int>(bid.outPaths.size())>=maxCycle)
        {
            //cout<<"enter next if!"<<endl;
            maxCycle = bid.outPaths.size();
        }
        cout<<"max cycle = "<<maxCycle<<endl;
        //先检查一下bind string的大小个数有没有问题
        //auto iter = std::find(replace_gui_procedures.begin(), replace_gui_procedures.end(), script[line].procName);
        //int dist = std::distance(replace_gui_procedures.begin(), iter);
//FIXME 这边跳过了路径的大小检查，我觉得是没必要的这个检查。
//        if(bid.inPaths[0].size()!=gui_input_var_num[dist]
//          || bid.outPaths[0].size()!=gui_output_var_num[dist])
//        {
//            string err = "in line "+StringOperator::ToStr(line)+ " proc "+script[line].procName+" bind string size error";
//            error_msg = QString::fromStdString(err);
//            emit finished(1, error_msg);
//            return;
//        }
        
        //下面来检查一下路径之类的是不是有问题
        //不检查这个了，反正会抛出异常
    }
    
    cout<<"currentCycle = "<<currentCycle<<endl;
    cout<<"maxCycle = "<<maxCycle<<endl;
    
        
        while (currentCycle<maxCycle) {
            
            
            try {
            
                QString err;
                for (int i=0; i<script.size(); i++) {
                    
                    scriptUnit unit = script[i];
                    bool result = PhaseOneScript(unit, i, err);
                    if(!result)
                    {
                        string msgg = "the "+StringOperator::ToStr(currentCycle)+" time running failed at line "+StringOperator::ToStr(i)+ " because of "+err.toStdString();
                        emit finished(1, QString::fromStdString(msgg));
                        break;
                    }
                    
                }
            
                string msgg = "the "+StringOperator::ToStr(currentCycle)+" time running complete";
            
                WatchDog::DumpMemory();
                
                emit finished(0, QString::fromStdString(msgg));
                
            } catch(std::string& laz_exception_str)
            {
                string currentException;
                currentException="the "+StringOperator::ToStr(currentCycle)+" time running failed because of "+laz_exception_str;
                WatchDog::DumpMemory();
                emit finished(1, QString::fromStdString(currentException));
            }
            catch( itk::ExceptionObject & err )
            {
                string currentException;
                currentException="the "+StringOperator::ToStr(currentCycle)+" time running failed because of "+err.what();
                WatchDog::DumpMemory();
                emit finished(1, QString::fromStdString(currentException));

            }
            catch(std::exception& err)
            {
                string currentException;
                currentException="the "+StringOperator::ToStr(currentCycle)+" time running failed because of "+err.what();
                WatchDog::DumpMemory();
                emit finished(1, QString::fromStdString(currentException));

            }
            catch(...)
            {
                string currentException;
                currentException="the "+StringOperator::ToStr(currentCycle)+" time running failed because of unknown reason";
                WatchDog::DumpMemory();
                emit finished(1, QString::fromStdString(currentException));

            }


            currentCycle++;
        }
        
        
        
        
    emit allDone();
    
    
    
}

void ParallelTest::myBind(int script_line,Bind& bid)
{
    pathMap.insert(std::make_pair(script_line, bid));
}

ParallelTest::ParallelTest(LazScript& _script)
{
    this->script = _script;
    replace_gui_procedures = {"OPT_GDCM_IN","OPT_READ_2D_GRAY_IMAGE","OPT_READ_DICOM_ITK","OPT_READ_MHD_ITK","OPT_READ_SERIES_IMAGES","OPT_SAVE_3D_IMAGE_TO_BMPS","OPT_SAVE_MHD"};
    gui_is_path = {"IN_TRUE_OUT_NULL","IN_FALSE_OUT_NULL","IN_TRUE_OUT_NULL","IN_FALSE_OUT_NULL","IN_TRUE_OUT_NULL","IN_NULL_OUT_TRUE","IN_NULL_OUT_FALSE"};
    gui_input_var_num = {1,1,1,1,1,0,0};
    gui_output_var_num = {0,0,0,0,0,1,1};
    if(0 == script.size())
    {
        emit finished(1, "脚本为空！");
        return;
    }
    //下面要判断哪些脚本中的语句需要bind特殊的string
    int size = script.size();
    for (int i=0;i<size; i++) {
        string currName = script[i].procName;
        if(replace_gui_procedures.end()!=std::find(replace_gui_procedures.begin(), replace_gui_procedures.end(), currName))
        {
            gui_need_replace.push_back(i);
        }
    }
    currentCycle = 0;
}

bool ParallelTest::isPath(QString &path)
{
    //FIXME 我不知道这样对不对！
    QFileInfo fileInfo(path);
    return fileInfo.isDir();
}
bool ParallelTest::PhaseSpecGUIScript(const Lazarus::scriptUnit& inUnit,int scriptLine,QString& error_msg)
{
    if(pathMap.end() == pathMap.find(scriptLine))
    {
        string msg = "Cannot Find correpond path name in procedure " + inUnit.procName+" in script line "+StringOperator::ToStr(scriptLine);
        error_msg = QString::fromStdString(msg);
        return false;
    }
    
    const Bind* bid = &pathMap[scriptLine];
    
    //这个暂时先这样写，因为我也想不到具体该怎么写比较好
    algIOTypes curProcIO=WatchDog::GetIoType(inUnit.procName);
    string inputVariables;
    cout<<"InVar Size = "<<inUnit.inVars.size()<<endl;
    for (int j=0;j<inUnit.inVars.size();j++)
    {
        if(WatchDog::CreateInstance(curProcIO.inTypes[j],inUnit.inVars[j]))
            WatchDog::GetInstance(inUnit.inVars[j]).objPtr->SetValue(inUnit.inDefVals[j]);
    }
    cout<<"outVar Size = "<<inUnit.outVars.size()<<endl;
    for (int j=0;j<inUnit.outVars.size();j++)
    {
        WatchDog::CreateInstance(curProcIO.outTypes[j],inUnit.outVars[j]);
    }
    
    string name = inUnit.procName;
    
    
    //下面来死做。。。因为没啥办法
    if(name == "OPT_GDCM_IN")
    {
        GDCMIN::LoadDCMFiles(
                             bid->inPaths[currentCycle][0],
                             40,
                             350,
                             dynamic_cast<image_3d_opencv_8uc1*>(WatchDog::GetInstance(inUnit.outVars[0]).objPtr)
                             );
        return true;
    }
    
    if(name == "OPT_READ_2D_GRAY_IMAGE")
    {
        cv::Mat image = cv::imread(bid->inPaths[currentCycle][0],CV_LOAD_IMAGE_GRAYSCALE);
        dynamic_cast<image_2d_opencv_8uc1*>(WatchDog::GetInstance(inUnit.outVars[0]).objPtr)->SetData(&image);
        return true;
    }
    
    if(name == "OPT_READ_DICOM_ITK")
    {
        GDCMIN::LoadDCMFiles(
                             bid->inPaths[currentCycle][0],
                             dynamic_cast<image_3d_itk*>(WatchDog::GetInstance(inUnit.outVars[0]).objPtr));
        return true;
    }
    
    if(name == "OPT_READ_MHD_ITK")
    {
        MHDIO::ReadMHDFiles(bid->inPaths[currentCycle][0], (image_3d_itk*)WatchDog::GetInstance(inUnit.outVars[0]).objPtr);
        return true;
    }
    
    if(name == "OPT_READ_SERIES_IMAGES")
    {
        ImgSeriesIn::ReadSeries(bid->inPaths[currentCycle][0], (image_3d_opencv_8uc1*)WatchDog::GetInstance(inUnit.outVars[0]).objPtr);
        return true;
    }
    
    if(name == "OPT_SAVE_3D_IMAGE_TO_BMPS")
    {
        BMPOUT::WriteToBMPs((image_3d_opencv_8uc1*)WatchDog::GetInstance(inUnit.inVars[0]).objPtr, bid->outPaths[currentCycle][0]);
        return true;
    }
    
    if(name == "OPT_SAVE_MHD")
    {
        MHDIO::SaveMHDFiles((image_3d_opencv_8uc1*)WatchDog::GetInstance(inUnit.inVars[0]).objPtr, bid->outPaths[currentCycle][0]);
    }
    
    return false;
    
}

bool ParallelTest::PhaseOneScript(const Lazarus::scriptUnit& inUnit,int scriptLine,QString& error_msg)
{
    cout<<"********************Running Sentence ********************"<<endl;
    cout<<"Current Procedure name  = "<<inUnit.procName<<endl;
    
    //bool result = true;
    
    if (WatchDog::ProcedureExist(inUnit.procName))
    {
        bool is_gui = (Pool::gui_operations.end()!=std::find(Pool::gui_operations.begin(), Pool::gui_operations.end(), inUnit.procName));
        bool is_spec_gui = (replace_gui_procedures.end()!=std::find(replace_gui_procedures.begin(), replace_gui_procedures.end(), inUnit.procName));
        
        if(is_spec_gui)
        {
            return PhaseSpecGUIScript(inUnit, scriptLine, error_msg);
        }
        
        if(!is_spec_gui && is_gui)
        {
            error_msg="脚本中出现了不可以有的GUI代码！请参考源代码中规定的限定类型";
            return false;
        }
        
        
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

        emit finished(2,"");

        cout<<"*******************Procedure run complete*****************"<<endl;
        cout<<endl;
    }else{
        WatchDog::WriteLog(Lazarus::Exception::FireException(LAZ_ERROR_USERSPEC,"Procedure not exist in preread Lazarus algorithm list! Maybe you forget to copy it here?"),LOG_NEXTLINE,true,true);
        return false;
    }
    
    return true;

}


