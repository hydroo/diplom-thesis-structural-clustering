#ifndef PREREQS_HPP
#define PREREQS_HPP

#include <cassert>
#include <cfloat>
#include <climits>
#include <cmath>

#include <QtCore>
#include <QtDebug>
#include <QtGlobal>

/* supposed to be called in main before anything else */
void init();

//int textHeight();
int textWidth(const QString& s, bool bold = false);
int graphicsSceneTextHeight();

double pixelsPerMillimeter();

template<typename T>
QTextStream& operator<<(QTextStream& s, QSet<T> t) {
	foreach(T e, t) {
		s << e << ", ";
	}
	return s;
}

int64_t int64Min();
int64_t int64Max();

extern QTextStream qerr;
extern QTextStream qout;

template<typename T>
QTextStream& operator<<(QTextStream& s, const T* p) {
	s << QString("0x%1").arg((qulonglong) p & 0x00000000ffffffff, 0, 16);
	return s;
}

#endif /* PREREQS_HPP */
