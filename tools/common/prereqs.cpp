#include "common/prereqs.hpp"

#include <climits>

#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <QTextDocument>

QTextStream qerr(stderr, QIODevice::WriteOnly | QIODevice::Text);
QTextStream qout(stdout, QIODevice::WriteOnly | QIODevice::Text);

QFont        g_font;
QFont        g_fontBold;
QFontInfo    g_fontInfo        (g_font);
QFontMetrics g_fontMetrics     (g_font);
QFontMetrics g_fontMetricsBold (g_fontBold);

double       g_graphicsSceneTextHeight;
double       g_pixelsPerMillimeter;

void init() {

	QImage i(1, 1, QImage::Format_ARGB32);
	g_pixelsPerMillimeter = i.logicalDpiX() / 25.4;

	//qout << "init(): pixels per millimeter: "<< g_pixelsPerMillimeter << "\n";

	if (i.logicalDpiX() != i.logicalDpiY()) {
		qout << "logical DPI X (" << i.logicalDpiY() << ") != logical DPI Y(" << i.logicalDpiY() << "). using logical DPI X.\n";
	}

	QPainter p(&i);
	g_font            = p.font();
	g_fontBold        = p.font(); g_fontBold.setBold(true);
	g_fontInfo        = p.fontInfo();
	g_fontMetrics     = p.fontMetrics();
	g_fontMetricsBold = QFontMetrics(g_fontBold, &i);

	// is not the same as textHeight(), because Qt adds a margin above the text.
	// i was unable to remove it.
	QGraphicsScene s;
	QGraphicsTextItem* t = s.addText("abc");
	t->document()->setDocumentMargin(0.0);
	g_graphicsSceneTextHeight = t->boundingRect().height();
}

//int textHeight() {
//	return g_fontInfo.pixelSize();
//}
//
int textWidth(const QString& s, bool bold) {
	if (bold == false) {
		return g_fontMetrics.width(s);
	} else {
		return g_fontMetricsBold.width(s);
	}
}

int graphicsSceneTextHeight() {
	return g_graphicsSceneTextHeight;
}
double pixelsPerMillimeter() {
	return g_pixelsPerMillimeter;
}

int64_t int64Min() { return LLONG_MIN; }
int64_t int64Max() { return LLONG_MAX; }

