TEMPLATE = app
CONFIG += debug warn_on
#CONFIG += release warn_on

TARGET = diff
SOURCES += ./main.cpp

include(../common/common.pri)
include(../common/otf/otf.pri)
include(../diff/diff.pri)
