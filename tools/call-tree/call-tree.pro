TEMPLATE = app
CONFIG += debug warn_on
#CONFIG += release warn_on

TARGET = call-tree
SOURCES += ./main.cpp

include(../common/common.pri)
include(../call-tree/call-tree.pri)
include(../common/otf/otf.pri)
