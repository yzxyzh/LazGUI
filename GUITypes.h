#ifndef GUITYPES_H
#define GUITYPES_H

#include <vector>
#include <string>
#include <deque>
#include <LazarusWatchDog.h>
#include <pcl/exceptions.h>
#include <qdebug.h>

using namespace std;
//macros define exception handling.
#define GUI_BEGIN try{

#define GUI_END } \
    catch(std::string& laz_exception_str) \
    { \
    string currentException; \
    currentException="ERROR! Exception Get : "+laz_exception_str;\
    QString error_msg=QString(QString::fromLocal8Bit(currentException.c_str()));\
    QMessageBox msgBox;\
    msgBox.setWindowTitle("ERROR!");\
    msgBox.setText(error_msg);\
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);\
    msgBox.exec(); \
    } \
    catch( itk::ExceptionObject & err ) \
    { \
        string itkExcept=(string)err.GetFile()+"Has Error"+(string)err.what(); \
        cout<<itkExcept<<endl; \
        std::cout << err << std::endl; \
    } \
    catch(pcl::PCLException& e) \
    {\
        QMessageBox msgBox;\
        msgBox.setWindowTitle("ERROR-PCL!");\
        msgBox.setText(e.what());\
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);\
        msgBox.exec(); \
    } \
     catch(std::exception& err) \
    { \
     QMessageBox msgBox;\
     msgBox.setWindowTitle("ERROR-RUNTIME!");\
     msgBox.setText(err.what());\
     msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);\
     msgBox.exec(); \
    } \
    catch(...) \
    { \
      qDebug()<<"Exception!"; \
      QMessageBox msgBox;\
      msgBox.setWindowTitle("ERROR!");\
      msgBox.setText("UNDEFED ERROR");\
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);\
      msgBox.exec(); \
    } \


#define GUI_END_THREADED } \
catch(std::string& laz_exception_str) \
{ \
string currentException; \
currentException="ERROR! Exception Get : "+laz_exception_str;\
emit error(QString::fromStdString(currentException)); \
} \
catch( itk::ExceptionObject & err ) \
{ \
string itkExcept=(string)err.GetFile()+"Has Error"+(string)err.what(); \
cout<<itkExcept<<endl; \
emit error(QString::fromStdString(itkExcept)); \
} \
catch(pcl::PCLException& e) \
{\
emit error(QString::fromStdString(e.what())); \
} \
catch(std::exception& err) \
{ \
std::cout<<(string)err.what()<<std::endl; \
emit error(QString::fromStdString(err.what())); \
} \
catch(...) \
{ \
std::cout<<"unknown error"<<std::endl; \
emit error("unknown error"); \
} \

#define myWarning(w) QMessageBox msgBox;\
                     msgBox.setWindowTitle("ERROR!");\
                     msgBox.setText(#w);\
                     msgBox.setStandardButtons(QMessageBox::Yes);\
                     msgBox.exec(); \



namespace LazGUI {
    
    bool CanPharseCache(Lazarus::scriptUnit &unit, const string &baseName);

    bool PhaseOneScriptFromCache(const Lazarus::scriptUnit& inUnit,const string& baseName);
    
    bool PhaseOneScript(const Lazarus::scriptUnit& inUnit);
    
    bool PhaseOneScriptCondition(const Lazarus::scriptUnit &inUnit,QString& error_msg,bool from_cache,QString baseName);

//ImageStack class. ImageStack contains a stack of images. the baseImg represents
//basic image to be shown on bottom layer, e.g. a raw CT scan.
//masks are masks covering on top of baseImg, drawn as contours. e.g.
//a hepatic segment & a portal segment.
//@note Masks are binaries.
class ImageStack{
public:
  void InsertBaseImg(const std::string& _baseImg){baseImg=_baseImg;}
  void InsertMasks(const std::string& _mask){masks.push_back(_mask);}
  void ClearMasks(){masks.clear();}
  void DeleteMask(const std::string& _mask)
  {
      auto iter=std::find(masks.begin(),masks.end(),_mask);
      if(iter != masks.end())
          masks.erase(iter);
  }

public:
  string baseImg;//basic image
  vector<string> masks;//masks cover on it
};

//global pool of memories,scripts,etc.
class Pool
{
public:
    static int alg_counter;
    static vector<Lazarus::scriptUnit> Script;
    static deque<Lazarus::scriptUnit> ScriptTmp;
    static bool is_alg_running;
    static vector<string> CachedVarName;
    static bool is_reading_from_cache;
    static vector<string> gui_operations;
};



}


#endif // GUITYPES_H
