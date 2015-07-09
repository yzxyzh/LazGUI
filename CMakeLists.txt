cmake_minimum_required(VERSION 2.8)

project(LazGUI)

#set(CMAKE_MACOSX_RPATH 1)

####add caffe root and build root dir.######
set(CAFFE_ROOT "/Users/yanzixu/Documents/caffe-master")
set(CAFFE_BUILD_ROOT "/Users/yanzixu/Documents/caffe-master/BUILD")
message("caffe root set to ${CAFFE_ROOT}, caffe build root set to ${CAFFE_BUILD_ROOT}, change it if necessary")

###### add lazarus dirs##########
set(LAZARUS_ROOT "/Users/yanzixu/Downloads/Programs/LAZARUS/sources")
set(LAZARUS_BUILD_ROOT "/Users/yanzixu/Downloads/Programs/LAZARUS/build_lib")
message("LAZARUS root set to ${LAZARUS_ROOT}, LAZARUS build root set to ${LAZARUS_BUILD_ROOT}, change it if necessary")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    #add caffe CPU_ONLY definition 
    add_definitions(-DCPU_ONLY)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#message(${PROJECT_SOURCE_DIR})
set(CMAKE_CONFIGURATION_TYPES "Release")

#set(ITK_DIR /Users/yanzixu/Downloads/Programs/InsightToolkit-4.5.2/BUILD)

set(src_header mainwindow.h AlgDialog.h AlgSolver.h ParallelTest.h ExperimentDialog.h
	newPathDlg.h bindDlg.h GenPathDialog.h duppathdlg.h) 

set(src_source cvqtconverter.cpp main.cpp mainwindow.cpp AlgDialog.cpp AlgSolver.cpp 
	ParallelTest.cpp ExperimentDialog.cpp newPathDlg.cpp bindDlg.cpp GenPathDialog.cpp
	duppathdlg.cpp)

set(src_ui mainwindow.ui ExperimentDialog.ui newPathDlg.ui bindDlg.ui GenPathDialog.ui duppathdlg.ui)

set(normal_header cvqtconverter.h GUITypes.h )

set(normal_source GUITypes.cpp )

#include LAZARUS dirs
include_directories(${LAZARUS_ROOT})
link_directories(${LAZARUS_BUILD_ROOT})

##################libraries###############

#first find some "BIG" libraries.

#as itk uses vtk in its itkVtkGlue module, it somehow changes VTK configurations during its configuration,
#this is NOT what we want.
find_package(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})
set(FULL_VTK_LIBS ${VTK_LIBRARIES})

find_package(ITK REQUIRED)
find_package(OpenCV REQUIRED)
#not QT nor qt, it's Qt
find_package(Qt4 REQUIRED)

find_package(PCL REQUIRED)
#add pcl defines includes,and linkers
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
#add pcl definition to main project
add_definitions(${PCL_DEFINITIONS})

INCLUDE(${QT_USE_FILE})
INCLUDE(${ITK_USE_FILE})


#tinyxml
#add definition to let tinyxml use STL to prevent error.
add_definitions(-DTIXML_USE_STL)

#armadillo
find_package(Armadillo 3.6.0 REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})
#link against ${ARMADILLO_LIBRARIES}

#libxml2
find_package(LibXml2 2.6.0 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
#link against ${LIBXML2_LIBRARIES}

#boost
set(Boost_ADDITIONAL_VERSIONS
  "1.49.0" "1.50.0" "1.51.0" "1.52.0" "1.53.0" "1.54.0" "1.55.0")
find_package(Boost 1.49 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
#link against ${Boost_LIBRARIES}

#mlpack
find_package(MLPACK REQUIRED)
include_directories(${MLPACK_INCLUDE_DIRS})
#link against ${MLPACK_LIBRARIES}

#glm
set(glm_header ${LAZARUS_ROOT}/Deps/glm)
include_directories(${glm_header})

#mkl
#this is NOT forced to be found
find_package(MKL COMPONENTS MKL_MULTI_THREADED)
if(MKL_FOUND)
   add_definitions(-DEIGEN_USE_MKL_ALL)
   add_definitions(-DUSE_MKL)
   include_directories(${MKL_INCLUDE_DIR})
else()
   message("unable to find mkl. try find openblas instead")
#if mkl is not found, we need openblas.
   find_package(OpenBLAS REQUIRED)
   include_directories(${OpenBLAS_INCLUDE_DIR})
   set(MKL_LIBRARIES ${OpenBLAS_LIB})#set mkl_libraries to openblas ones.
endif()
#link against ${MKL_LIBRARIES}

#eigen
#eigen does NOT need to compile simply add it .
include_directories(${LAZARUS_ROOT}/Deps/EIGEN)

################# next we find google stuffs#############

#ported from caffe.
#glog
include("${PROJECT_SOURCE_DIR}/cmake/External/glog.cmake")
include_directories(${GLOG_INCLUDE_DIRS})
#link against ${GLOG_LIBRARIES}

#gflags
include("${PROJECT_SOURCE_DIR}/cmake/External/gflags.cmake")
include_directories(${GFLAGS_INCLUDE_DIRS})
#link against ${GFLAGS_LIBRARIES}

#protobuf
find_package( Protobuf REQUIRED )
include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIR})
# As of Ubuntu 14.04 protoc is no longer a part of libprotobuf-dev package
# and should be installed  separately as in: sudo apt-get install protobuf-compiler
if(EXISTS ${PROTOBUF_PROTOC_EXECUTABLE})
  message(STATUS "Found PROTOBUF Compiler: ${PROTOBUF_PROTOC_EXECUTABLE}")
else()
  message(FATAL_ERROR "Could not find PROTOBUF Compiler")
endif()
#link against ${PROTOBUF_LIBRARIES}

##################################found all google stuffs##################

################## find database stuffs####################
#ported from caffe.

#HDF5
find_package(HDF5 COMPONENTS HL REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS} ${HDF5_HL_INCLUDE_DIR})
#link against ${HDF5_LIBRARIES}

#LMDB
find_package(LMDB REQUIRED)
include_directories(${LMDB_INCLUDE_DIR})
#link against ${LMDB_LIBRARIES}

#LevelDB
find_package(LevelDB REQUIRED)
include_directories(${LevelDB_INCLUDE})
#link against ${LevelDB_LIBRARIES}

#snappy
find_package(Snappy REQUIRED)
include_directories(${Snappy_INCLUDE_DIR})
#link against ${Snappy_LIBRARIES}

#####################end finding databases#######################

#we are NOT going to find CUDA on MAC.

####################Add Caffe##########################
#add caffe includes and caffe protobuf generate includes.
include_directories(${CAFFE_ROOT}/include ${CAFFE_BUILD_ROOT}/include)
set(CAFFE_LIBRARIES caffe)
link_directories(${CAFFE_BUILD_ROOT}/lib)
#if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
#    set(CAFFE_LIBRARIES ${CAFFE_BUILD_ROOT}/lib/${CMAKE_BUILD_TYPE}/libcaffe.dylib)
#else()
	#in linux ,we use .so as dynamic library.
#	set(CAFFE_LIBRARIES ${CAFFE_BUILD_ROOT}/lib/${CMAKE_BUILD_TYPE}/libcaffe.so)
#endif()

#next merge all my files into one single variable
set(ADDITIONAL_LINKERS 
	${ARMADILLO_LIBRARIES} 
	${LIBXML2_LIBRARIES} 
#	${MKL_LIBRARIES} 
	${MLPACK_LIBRARIES} 
	${GLOG_LIBRARIES}
	${GFLAGS_LIBRARIES}
	${PROTOBUF_LIBRARIES}
	${HDF5_LIBRARIES}
	${LevelDB_LIBRARIES}
	${LMDB_LIBRARIES}
	${Snappy_LIBRARIES}
	${CAFFE_LIBRARIES}
	)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(ksvd_header /Users/yanzixu/Desktop/KSVD3D)
    set(ksvd_lib /Users/yanzixu/Library/Developer/Xcode/DerivedData/KSVD3D-bdtorlzfrjvirqeaoeumrbgkytbt/Build/Products)
    include_directories(${ksvd_header})
    link_directories(${ksvd_lib})
	set(KSVD_LIBRARY "KSVD3D")
else()
	set(KSVD_LIBRARY "")#dummy ksvd linker
endif()

include_directories(${PROJECT_BINARY_DIR})

QT4_WRAP_CPP(src_header_MOC ${src_header})
QT4_WRAP_UI(src_ui_Header ${src_ui})

add_executable(LazGUI ${src_header} ${src_header_MOC} ${src_source} ${src_ui_Header} ${normal_header} ${normal_source})
#SET_TARGET_PROPERTIES(LiverSerA PROPERTIES LINKER_LANGUAGE C++)
#first static then dynamic. seems Boost_LIBRARIES has some problems, we'll link to boost manually.
target_link_libraries(LazGUI 
	LAZARUS
	${KSVD_LIBRARY}
	${OpenCV_LIBS} 
	${VTK_LIBRARIES} 
	${ITK_LIBRARIES} 
	${QT_LIBRARIES} 
	${PCL_LIBRARIES}
	${ADDITIONAL_LINKERS}
	libmkl_intel_thread.a 
	libmkl_intel_lp64.a 
	libmkl_core.a 
	libiomp5.a
	#${Boost_LIBRARIES}
	boost_system
	boost_thread-mt
	boost_math_c99
	boost_math_tr1
	tiff 
	)

#set_target_properties(LazGUI PROPERTIES INSTALL_RPATH ${CAFFE_BUILD_ROOT}/lib/Release)

