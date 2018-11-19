INCLUDEPATH += \
	../ \
	$$system(otfconfig --includes | sed -e 's/-I//g') \
	$$system(otfconfig --cflags | sed -e 's/-I//g') \
	$$system(otf2-config --cflags | sed -e 's/-I//g')

LIBS += \
	$$system(otfconfig --libs | sed -e 's/-lotfaux//' ) \
	$$system(otf2-config --ldflags) \
	$$system(otf2-config --libs) \

SOURCES += ../common/otf/otf.cpp
