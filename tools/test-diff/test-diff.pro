TEMPLATE = app
CONFIG += debug warn_on

TARGET = test-diff
SOURCES += ./main.cpp

include(../diff/diff.pri)
include(../common/common.pri)
include(../common/otf/otf.pri)
