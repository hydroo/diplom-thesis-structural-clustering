#include "common/image.hpp"

//#include <QColor>
//#include <QPainter>
//
//void Image_addHorizontally(const QImage& a, const QImage& b, QImage *c) {
//
//	double mm = pixelsPerMillimeter();
//	int margin = (int) mm;
//
//	*c = QImage(a.width() + margin + b.width(), std::max(a.height(), b.height()), a.format());
//
//	if (a.format() != b.format()) {
//		qout << "image formats are not the same (" << a.format() << " != "<< a.format() << "). may create problems. the former format will be used.";
//	}
//
//	c->fill(QColor(255, 255, 255));
//
//	QPainter p;
//	p.begin(c);
//
//	p.drawImage(0, 0, a);
//	p.drawImage(a.width() + margin, 0, b);
//
//	p.end();
//}
//
//void Image_addVertically(const QImage& a, const QImage& b, QImage *c) {
//
//	double mm = pixelsPerMillimeter();
//	int margin = (int) mm;
//
//	*c = QImage(std::max(a.width(), b.width()), a.height() + margin + b.height(), a.format());
//
//	if (a.format() != b.format()) {
//		qout << "image formats are not the same (" << a.format() << " != "<< a.format() << "). may create problems. the former format will be used.";
//	}
//
//	c->fill(QColor(255, 255, 255));
//
//	QPainter p;
//	p.begin(c);
//
//	p.drawImage(0, 0, a);
//	p.drawImage(0, a.height() + margin, b);
//
//	p.end();
//}
//
//void Image_fromText(const QString& s, QImage* i) {
//	*i = QImage(textWidth(s), textHeight(), QImage::Format_ARGB32);
//	i->fill(QColor(255, 255, 255));
//	QPainter p;
//	p.begin(i);
//	p.drawText(0, textHeight(), s);
//	p.end();
//}
