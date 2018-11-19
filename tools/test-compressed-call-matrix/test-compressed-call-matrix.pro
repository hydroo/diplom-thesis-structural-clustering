TEMPLATE = app
CONFIG += debug warn_on

TARGET = test-compressed-call-matrix
SOURCES += ./main.cpp

include(../call-matrix/call-matrix.pri)
include(../common/common.pri)
include(../common/otf/otf.pri)
