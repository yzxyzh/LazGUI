cmake_minimum_required(VERSION 2.8)

project(LazGUI)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#message(${PROJECT_SOURCE_DIR})
set(CMAKE_CONFIGURATION_TYPES "Release")

set(ITK_DIR /Users/yanzixu/Downloads/Programs/InsightToolkit-4.5.2/BUILD)

set(src_header mainwindow.h AlgDialog.h AlgSolver.h ParallelTest.h ExperimentDialog.h
	newPathDlg.h bindDlg.h) 

set(src_source cvqtconverter.cpp main.cpp mainwindow.cpp AlgDialog.cpp AlgSolver.cpp 
	ParallelTest.cpp ExperimentDialog.cpp newPathDlg.cpp bindDlg.cpp)

set(src_ui mainwindow.ui ExperimentDialog.ui newPathDlg.ui bindDlg.ui)

set(normal_header cvqtconverter.h GUITypes.h )

set(normal_source GUITypes.cpp )

#set(src_res ${PROJECT_SOURCE_DIR}/serge/resorce)

set(user_include /usr/local/include)
set(user_lib /usr/local/lib)
#message(${src})
#Add LAZARUS
set(laz_header /Users/yanzixu/Downloads/Programs/Sources)
set(laz_lib /Users/yanzixu/Downloads/Programs/Sources/BUILD_LIB/Release)


find_package(VTK REQUIRED)
find_package(ITK REQUIRED)
find_package(QT4 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PCL REQUIRED)

set(lib_pca_header /Users/yanzixu/Desktop/PCAMKL)
set(lib_pca_lib /Users/yanzixu/Library/Developer/Xcode/DerivedData/PCAMKL-aqditmltnsahnucrkafhzscxqaad/Build/Products/Release)

set(tixml_header /usr/local/Cellar/tinyxml/2.6.2/include)
set(tixml_lib /usr/local/Cellar/tinyxml/2.6.2/lib)

set(ksvd_header /Users/yanzixu/Desktop/KSVD3D)
set(ksvd_lib /Users/yanzixu/Library/Developer/Xcode/DerivedData/KSVD3D-bdtorlzfrjvirqeaoeumrbgkytbt/Build/Products/Release)

set(intel_cpp_lib /opt/intel/composer_xe_2013_sp1.3.166/compiler/lib /opt/intel/composer_xe_2013_sp1.3.166/mkl/lib)

set(glm_header Deps/glm)
#set(Eigen_header Deps/EIGEN)
set(mlpack_header /usr/local/Cellar/mlpack/1.0.8/include)
set(mlpack_lib /usr/local/Cellar/mlpack/1.0.8/lib)
set(libxml2_header /usr/local/Cellar/libxml2/2.9.1/include/libxml2)
set(libxml2_lib /usr/local/Cellar/libxml2/2.9.1/lib)
set(armadillo_include /usr/local/Cellar/armadillo/4.300.3/include)
set(armadillo_lib /usr/local/Cellar/armadillo/4.300.3/lib)
set(boost_lib /usr/local/Cellar/boost/1.55.0_2/lib)

#my own lib dir
set(libsc_root /Users/yanzixu/Downloads/Programs/Sources/BUILD_LIB/Deps/SpectralClustering/Release)
set(libLDA_root /Users/yanzixu/Downloads/Programs/Sources/BUILD_LIB/Deps/LDA/Release)

INCLUDE(${QT_USE_FILE})
INCLUDE(${ITK_USE_FILE})
INCLUDE(${VTK_USE_FILE})


#add pcl defines includes,and linkers
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})

#add pcl definition to main project
add_definitions(${PCL_DEFINITIONS})


include_directories(${user_include} ${CMAKE_CURRENT_BINARY_DIR} ${laz_header})
include_directories(${ksvd_header} ${tixml_header} ${glm_header} ${mlpack_header}
  ${libxml2_header} ${armadillo_include} ${lib_pca_header})
link_directories(${user_lib})
link_directories(${tixml_lib} ${ksvd_lib} ${mlpack_lib} ${libxml2_lib} ${armadillo_lib} ${boost_lib} ${intel_cpp_lib} ${lib_pca_lib}) 
link_directories(${libsc_root} ${libLDA_root})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
ENDIF()

QT4_WRAP_CPP(src_header_MOC ${src_header})
QT4_WRAP_UI(src_ui_Header ${src_ui})
#QT4_ADD_RESOURCES(src_res_RES ${src_res})

add_executable(LazGUI ${src_header} ${src_header_MOC} ${src_source} ${src_ui_Header} ${normal_header} ${normal_source})
#SET_TARGET_PROPERTIES(LiverSerA PROPERTIES LINKER_LANGUAGE C++)
target_link_libraries(LazGUI ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${OpenCV_LIBS} ${QT_LIBRARIES} tiff ${laz_lib}/libLAZARUS.a tinyxml libKSVD3D.a mlpack xml2 armadillo libboost_system.a libboost_thread-mt.a
  libboost_math_c99-mt.a libboost_math_tr1-mt.a libiomp5.a libmkl_intel_thread.a libmkl_intel_lp64.a libmkl_core.a libPCAMKL.a LibSC LDA ${PCL_LIBRARIES})

