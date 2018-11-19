QT += widgets

CONFIG -= app_bundle

QMAKE_CXXFLAGS += -Wextra -std=c++11
LIBS += -lrt # for clock_gettime

INCLUDEPATH += ../

HEADERS += \
	../common/conceptlattice.inl \
	../common/conceptlattice.hpp \
	../common/difference.hpp \
	../common/image.hpp \
	../common/measure.hpp \
	../common/prereqs.hpp \
	../common/timer.hpp \
	../common/unifier.inl \
	../common/unifier.hpp \
	../common/viewer.hpp

SOURCES += \
	../common/conceptlattice.cpp \
	../common/difference.cpp \
	../common/image.cpp \
	../common/measure.cpp \
	../common/prereqs.cpp \
	../common/timer.cpp \
	../common/unifier.cpp \
	../common/viewer.cpp

