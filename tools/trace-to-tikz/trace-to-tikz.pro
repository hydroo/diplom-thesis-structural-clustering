TEMPLATE = app
CONFIG += debug warn_on
#CONFIG += release warn_on

TARGET = trace-to-tikz
SOURCES += ./main.cpp

include(../common/common.pri)
include(../common/otf/otf.pri)

