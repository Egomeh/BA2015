cmake_minimum_required(VERSION 2.8)

# This project is designed to be built outside the Shark source tree.
# set Shark_DIR to the proper location
project(ExampleProject)

#find gsl
## 
## Try to find gnu scientific library GSL  
## (see http://www.gnu.org/software/gsl/)
## Once run this will define: 
## 
## GSL_FOUND       = system has GSL lib
##
## GSL_LIBRARIES   = full path to the libraries
##    on Unix/Linux with additional linker flags from "gsl-config --libs"
## 
## CMAKE_GSL_CXX_FLAGS  = Unix compiler flags for GSL, essentially "`gsl-config --cxxflags`"
##
## GSL_INCLUDE_DIR      = where to find headers 
##
## GSL_LINK_DIRECTORIES = link directories, useful for rpath on Unix
## GSL_EXE_LINKER_FLAGS = rpath on Unix
##
## Felix Woelk 07/2004
## minor corrections Jan Woetzel
##
## www.mip.informatik.uni-kiel.de
## --------------------------------

IF(WIN32)
  #MESSAGE(SEND_ERROR "FindGSL.cmake: gnu scientific library GSL not (yet) supported on WIN32")
  
ELSE(WIN32)
  IF(UNIX) 
    SET(GSL_CONFIG_PREFER_PATH "$ENV{GSL_HOME}/bin" CACHE STRING "preferred path to OpenSG (osg-config)")
    FIND_PROGRAM(GSL_CONFIG gsl-config
      ${GSL_CONFIG_PREFER_PATH}
      /usr/bin/
      )
    # MESSAGE("DBG GSL_CONFIG ${GSL_CONFIG}")
    
    IF (GSL_CONFIG) 
      # set CXXFLAGS to be fed into CXX_FLAGS by the user:
      SET(GSL_CXX_FLAGS "`${GSL_CONFIG} --cflags`")
      
      # set INCLUDE_DIRS to prefix+include
      EXEC_PROGRAM(${GSL_CONFIG}
        ARGS --prefix
        OUTPUT_VARIABLE GSL_PREFIX)
      SET(GSL_INCLUDE_DIR ${GSL_PREFIX}/include CACHE STRING INTERNAL)

      # set link libraries and link flags
      SET(GSL_LIBRARIES "`${GSL_CONFIG} --libs`")
      
      ## extract link dirs for rpath  
      EXEC_PROGRAM(${GSL_CONFIG}
        ARGS --libs
        OUTPUT_VARIABLE GSL_CONFIG_LIBS )

      ## split off the link dirs (for rpath)
      ## use regular expression to match wildcard equivalent "-L*<endchar>"
      ## with <endchar> is a space or a semicolon
      STRING(REGEX MATCHALL "[-][L]([^ ;])+" 
        GSL_LINK_DIRECTORIES_WITH_PREFIX 
        "${GSL_CONFIG_LIBS}" )
        #      MESSAGE("DBG  GSL_LINK_DIRECTORIES_WITH_PREFIX=${GSL_LINK_DIRECTORIES_WITH_PREFIX}")

      ## remove prefix -L because we need the pure directory for LINK_DIRECTORIES
      
      IF (GSL_LINK_DIRECTORIES_WITH_PREFIX)
        STRING(REGEX REPLACE "[-][L]" "" GSL_LINK_DIRECTORIES ${GSL_LINK_DIRECTORIES_WITH_PREFIX} )
      ENDIF (GSL_LINK_DIRECTORIES_WITH_PREFIX)
      SET(GSL_EXE_LINKER_FLAGS "-Wl,-rpath,${GSL_LINK_DIRECTORIES}" CACHE STRING INTERNAL)
      #      MESSAGE("DBG  GSL_LINK_DIRECTORIES=${GSL_LINK_DIRECTORIES}")
      #      MESSAGE("DBG  GSL_EXE_LINKER_FLAGS=${GSL_EXE_LINKER_FLAGS}")

      #      ADD_DEFINITIONS("-DHAVE_GSL")
      #      SET(GSL_DEFINITIONS "-DHAVE_GSL")
      MARK_AS_ADVANCED(
        GSL_CXX_FLAGS
        GSL_INCLUDE_DIR
        GSL_LIBRARIES
        GSL_LINK_DIRECTORIES
        GSL_DEFINITIONS
      )
      MESSAGE(STATUS "Using GSL from ${GSL_PREFIX}")
      
    ELSE(GSL_CONFIG)
      MESSAGE("FindGSL.cmake: gsl-config not found. Please set it manually. GSL_CONFIG=${GSL_CONFIG}")
    ENDIF(GSL_CONFIG)

  ENDIF(UNIX)
ENDIF(WIN32)


IF(GSL_LIBRARIES)
  IF(GSL_INCLUDE_DIR OR GSL_CXX_FLAGS)

    SET(GSL_FOUND 1)
    
  ENDIF(GSL_INCLUDE_DIR OR GSL_CXX_FLAGS)
ENDIF(GSL_LIBRARIES)
# end find gsl


#########################################################
# FIND GLUT
#########################################################
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)
#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
#########################################################

# Find the Shark libraries and includes
find_package(Shark REQUIRED)
include(${SHARK_USE_FILE})

SET(CE_SRC CrossEntropy/CrossEntropy.cpp)
SET(CE_INCLUDE CrossEntropy/CrossEntropy.h)
add_library(crossentropy ${CE_SRC} ${CE_INCLUDE})
target_link_libraries(crossentropy ${SHARK_LIBRARIES})

include_directories(CrossEntropy)

AUX_SOURCE_DIRECTORY(mdptetris/src MDPTETRIS_SRC) 
AUX_SOURCE_DIRECTORY(mdptetris/include MDPTETRIS_INCLUDE) 

add_library(tetris ${MDPTETRIS_SRC})
target_link_libraries(tetris ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )

add_library(tetris_objective_fun MDPTetris.cpp MDPTetris.h)
target_link_libraries(tetris_objective_fun ${SHARK_LIBRARIES})
target_link_libraries(tetris_objective_fun tetris)

include_directories(mdptetris/include)

SET(MDPTETRIS_DATA_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/data)

configure_file(cconfig.h.in cconfig.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


# Executable
add_executable(ExampleProject HelloWorld.cpp)
target_link_libraries(ExampleProject ${SHARK_LIBRARIES})
target_link_libraries(ExampleProject tetris)
target_link_libraries(ExampleProject gsl -lgslcblas)
target_link_libraries(ExampleProject tetris_objective_fun)
target_link_libraries(ExampleProject crossentropy)

add_executable(ScaleTest scaleTest.cpp)
target_link_libraries(ScaleTest ${SHARK_LIBRARIES})
target_link_libraries(ScaleTest tetris)
target_link_libraries(ScaleTest gsl -lgslcblas)
target_link_libraries(ScaleTest tetris_objective_fun)
target_link_libraries(ScaleTest crossentropy)



