TEMPLATE = app
#CONFIG += debug warn_on
CONFIG += release warn_on

TARGET = call-list
SOURCES += ./main.cpp

include(../common/common.pri)
include(../call-list/call-list.pri)
include(../common/otf/otf.pri)
