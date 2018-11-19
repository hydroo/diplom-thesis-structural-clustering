TEMPLATE = app
CONFIG += debug warn_on

TARGET = similarity
SOURCES += ./main.cpp

include(../call-list/call-list.pri)
include(../call-matrix/call-matrix.pri)
include(../call-tree/call-tree.pri)
include(../common/common.pri)
include(../common/otf/otf.pri)
include(../common/similarity-metric/similarity-metric.pri)
