TEMPLATE = app
#CONFIG += debug warn_on
CONFIG += release warn_on

#QMAKE_CXXFLAGS += -pg
#QMAKE_LFLAGS += -pg

TARGET = call-matrix
SOURCES += ./main.cpp

include(../common/common.pri)
include(../call-list/call-list.pri)
include(../call-matrix/call-matrix.pri)
include(../common/otf/otf.pri)
