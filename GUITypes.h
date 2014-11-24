#ifndef GUITYPES_H
#define GUITYPES_H

#include <vector>
#include <string>
#include <LazarusWatchDog.h>
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
    catch(...) \
    { \
      QMessageBox msgBox;\
      msgBox.setWindowTitle("ERROR!");\
      msgBox.setText("UNDEFED ERROR");\
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);\
      msgBox.exec(); \
    } \

#define myWarning(w) QMessageBox msgBox;\
                     msgBox.setWindowTitle("ERROR!");\
                     msgBox.setText(#w);\
                     msgBox.setStandardButtons(QMessageBox::Yes);\
                     msgBox.exec(); \



namespace LazGUI {

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
};



}


#endif // GUITYPES_H
