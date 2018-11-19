TEMPLATE = app
CONFIG += debug warn_on
#CONFIG += release warn_on

TARGET = profile
HEADERS += \
	./profile.hpp \
	./profile.inl
SOURCES += \
	./main.cpp \
	./profile.cpp

include(../common/common.pri)
include(../call-list/call-list.pri)
include(../call-matrix/call-matrix.pri)
include(../call-tree/call-tree.pri)
include(../common/otf/otf.pri)
