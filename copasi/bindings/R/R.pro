# Copyright (C) 2010 - 2013 by Pedro Mendes, Virginia Tech Intellectual 
# Properties, Inc., University of Heidelberg, and The University 
# of Manchester. 
# All rights reserved. 


TEMPLATE = lib
CONFIG -= qt
# the plugin config option disables the 
# creation of the versioning links for the library
CONFIG += plugin

include(../../common.pri)
include(../../app.pri)

contains(BUILD_OS,WIN32){
   TARGET = COPASI
} else {
   TARGET = COPASI
}

# the code generated by swig has to be compiled with -O1
# since -O2 and higher do things that might break the binary
QMAKE_CFLAGS_RELEASE -= -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O1
QMAKE_CXXFLAGS_RELEASE -= -O3
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O1

COPASI_LIBS += $${COPASI_LIBS_SE}


INCLUDEPATH += ../../..
contains(BUILD_OS,Linux){
  LIBS = -L../../lib \
         $$join(COPASI_LIBS, " -l", -l) \
         $${LIBS}

  POST_TARGETDEPS += $$join(COPASI_LIBS, ".a  ../../lib/lib", ../../lib/lib, .a)

}

contains(BUILD_OS, Darwin) {
  QMAKE_LFLAGS += -Wl,-search_paths_first
  
  LIBS = $$join(COPASI_LIBS, ".a  ../../lib/lib", ../../lib/lib, .a) \
         $${LIBS}
  
  POST_TARGETDEPS += $$join(COPASI_LIBS, ".a  ../../lib/lib", ../../lib/lib, .a)

}

contains(BUILD_OS, WIN32) { 
  CONFIG += debug_and_release

  debug {
    LIBS += $$join(COPASI_LIBS, ".lib  ../../lib/debug/", ../../lib/debug/, .lib)
  }
  release {
    LIBS += $$join(COPASI_LIBS, ".lib  ../../lib/release/", ../../lib/release/, .lib)
  }

  debug {
    PRE_TARGETDEPS += $$join(COPASI_LIBS, ".lib  ../../lib/debug/", ../../lib/debug/, .lib)
}

  release {
    PRE_TARGETDEPS += $$join(COPASI_LIBS, ".lib  ../../lib/release/", ../../lib/release/, .lib)
  }

}

include(../common/swig_files.pri)



#DISTFILE   = $$SWIG_INTERFACE_FILES
#DISTFILES += local.cpp
#DISTFILES += R.i
#DISTFILES += $$UNITTEST_FILES

isEmpty(SWIG_PATH){
    # check if the wrapper file is there
    !exists(copasi_wrapper.cpp){
        error(Wrapper file copasi_wrapper.cpp missing. Please reconfigure with --with-swig=PATH_TO_SWIG.)
    }
}

!isEmpty(SWIG_PATH){
    # check if swig is there and create a target to run it to create
    # copasi_wrapper.cpp
    contains(BUILD_OS, WIN32){
        !exists($${SWIG_PATH}\\swig.exe){
        error(Unable to find swig excecutable in $${SWIG_PATH}. Please use --with-swig=PATH to specify the path where PATH/swig.exe is located.) 
         }
    }
    !contains(BUILD_OS, WIN32){
      !exists($${SWIG_PATH}/bin/swig){
        error(Unable to find swig excecutable in $${SWIG_PATH}/bin/. Please use --with-swig=PATH to specify the path where PATH/bin/swig is located.) 
      }
    }

    DEFINE_COMMANDLINE = $$join(DEFINES," -D",-D)
    contains(BUILD_OS, WIN32){
      # since the wrapper file is in a subdirectory, we need to add 
      # the project directory to the include path
      INCLUDEPATH += .

      WRAPPER_FILE_PATH = "."

      debug{
        WRAPPER_FILE_PATH = debug
        wrapper_source.target = "debug\\COPASI.cpp"
      }	
      release{
        WRAPPER_FILE_PATH = release
        wrapper_source.target = "release\\COPASI.cpp"
      }

      # we force the rebuild of the wrapper sources
      wrapper_source.depends = FORCE

      wrapper_source.commands = $(DEL_FILE) $${wrapper_source.target} & $${SWIG_PATH}\\swig.exe $${DEFINE_COMMANDLINE} -I..\..  -I..\..\.. -c++ -r -o $${wrapper_source.target} R.i

      QMAKE_EXTRA_TARGETS += wrapper_source
      debug {
        QMAKE_CLEAN += debug\\COPASI.cpp 
        QMAKE_CLEAN += debug\\COPASI.R
        QMAKE_CLEAN += debug\\COPASI.so
      }
      release {
        QMAKE_CLEAN += release\\COPASI.cpp 
        QMAKE_CLEAN += release\\COPASI.R
        QMAKE_CLEAN += release\\COPASI.so
      }
    }
    !contains(BUILD_OS, WIN32){
      wrapper_source.target = COPASI.cpp
      wrapper_source.depends = $${SWIG_INTERFACE_FILES} R.i local.cpp
      wrapper_source.commands = $(DEL_FILE) $${wrapper_source.target} ; $${SWIG_PATH}/bin/swig $${DEFINE_COMMANDLINE} -I../.. -I../../.. -c++ -r -o $${wrapper_source.target} R.i
  
      QMAKE_EXTRA_TARGETS += wrapper_source
      QMAKE_CLEAN += COPASI.cpp 
      QMAKE_CLEAN += COPASI.R
      QMAKE_CLEAN += COPASI.so
    }
    PRE_TARGETDEPS += $${wrapper_source.target}
}


SOURCES += $${wrapper_source.target}

# according to the SWIG documentation, we need R to compile the R bindings
#copasi_wrapper   

isEmpty(R_BIN){
  # we just assume it is in the path
  R_BIN = $$system(which R)
}

isEmpty(R_BIN) | !exists($${R_BIN}){
  error("Could not find R binary at \"$${R_BIN}\"."); 
}

# the command the create the library and the R file is PKG_LIBS="example.cxx" R CMD SHLIB COPASI.cpp
# additional variables will be needed to specify include path and libraries PKG_CXXFLAGS PKG_CPPFLAGS PKG_LIBS
# I can probably just specify the corresponding qmake variables for this QMAKE_CXXFLAGS, INCLUDEPATH and LIBS (or QMAKE_LFLAGS) respectively
# I will have to convert the INCLUDEPATH variable into options with -I manually 

PKG_CPPFLAGS=$$join(DEFINES, " -D", -D)
PKG_CPPFLAGS += $$join(INCLUDEPATH, " -I", -I) 
# In order to build a debug version, it would also be good to pass the optimization flags to the build command
# Also if COPASI was configured for a certain architecture, especially on Mac OS X, that information should also be passed to the build command.
# Build flags for R packages can also be set in a file called .R/Makevars in ones home directory, the directory can even contain several Makevars
# files which are specific for a certain platform, e.g. Makevars.win64 

message($$DEFINES)


QMAKE_CXX = echo
QMAKE CXX_FLAGS = ""
# now we have to replace the linker command with the call to R
QMAKE_LINK_SHLIB_CMD = PKG_LIBS=\"$${LIBS} $${QMAKE_LFLAGS}\" PKG_CPPFLAGS=\"$$PKG_CPPFLAGS\" PKG_CXXFLAGS=\"$${QMAKE_CXXFLAGS} $${} \" $${R_BIN} CMD SHLIB $${wrapper_source.target} 

# we have to unset the QMAKE_LFLAGS so that they are not passed to the R command as well
QMAKE_LFLAGS = ""

# under windows qmake seems to ignore the last line of project files


