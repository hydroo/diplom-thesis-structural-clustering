TEMPLATE = app
CONFIG += debug warn_on

TARGET = test-similarity-metric
SOURCES += ./main.cpp

include(../call-matrix/call-matrix.pri)
include(../call-tree/call-tree.pri)
include(../common/common.pri)
include(../common/otf/otf.pri)
include(../common/similarity-metric/similarity-metric.pri)
