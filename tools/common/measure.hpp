#ifndef MEASURE_HPP
#define MEASURE_HPP

#include "common/prereqs.hpp"

#include <limits>

#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QPen>

template<typename T>
struct Recording;

// only use this with types that support values smaller than 0
template<typename T>
struct Measure {
	T accumulated;

	T secondPercentile;
	T firstQuartile;
	T median;
	T thirdQuartile;
	T ninetyEighthPercentile;

	T min;
	T max;
	double mean;

	int dataPointCount;

	Recording<T> *r;
};

template<typename T>
struct Recording {
	QList<T> l;
};

template<typename T>
void Measure_init(Measure<T>* m) {
	m->accumulated            = 0;

	m->secondPercentile       = 0;
	m->firstQuartile          = 0;
	m->median                 = 0;
	m->thirdQuartile          = 0;
	m->ninetyEighthPercentile = 0;

	m->min                    = std::numeric_limits<T>::max();
	m->max                    = std::numeric_limits<T>::min();
	m->mean                   = 0;

	m->dataPointCount         = 0;

	m->r = new Recording<T>;
}

template<typename T>
void Measure_record(Measure<T>* m, T dataPoint) {
	assert(m->r != nullptr);

	m->accumulated    += dataPoint;
	m->min             = std::min(m->min, dataPoint);
	m->max             = std::max(m->max, dataPoint);
	m->dataPointCount += 1;

	m->r->l.append(dataPoint);
}

template<typename T>
void Measure_finalize(Measure<T> *m) {
	assert(m->r != nullptr);

	if (m->dataPointCount > 0) {
		std::sort(m->r->l.begin(), m->r->l.end());

		assert(m->dataPointCount == m->r->l.length());

		m->secondPercentile       = m->r->l[(int) (m->dataPointCount*0.02)];
		m->firstQuartile          = m->r->l[(int) (m->dataPointCount*0.25)];
		m->median                 = m->r->l[(int) (m->dataPointCount*0.50)]; // this might be a bad idea, because it rounds up
		m->thirdQuartile          = m->r->l[(int) (m->dataPointCount*0.75)];
		m->ninetyEighthPercentile = m->r->l[(int) (m->dataPointCount*0.98)];

		m->mean = (double) m->accumulated / (double) m->dataPointCount;

	} else {

		m->secondPercentile       = 0;
		m->firstQuartile          = 0;
		m->median                 = 0;
		m->thirdQuartile          = 0;
		m->ninetyEighthPercentile = 0;
		m->min                    = 0;
		m->max                    = 0;
		m->mean                   = 0;
	}


	delete m->r; m->r = nullptr;
}

template<typename T>
QString Measure_print(const Measure<T>& m) {
	QString ret;
	QTextStream s(&ret);

	s << "min " << m.min << ", secondPercentile " << m.secondPercentile << ", firstQuartile " << m.firstQuartile << ", median " << m.median << ", mean " << m.mean << ", thirdQuartile " << m.thirdQuartile << ", ninetyEighthPercentile " << m.ninetyEighthPercentile << ", max " << m.max << ", # " << m.dataPointCount << ", acc " << m.accumulated;

	return ret;
}

/*
 * box plot
 *
 *       25%       median   75%
 *          +---------+----+
 *  2%      |         |    |          98%
 *    |-----+         |    +---------|
 *          |         |    |
 *          +---------+----+
 *
 * returns a QGraphicsItemGroup that still needs to be positioned on the scene (all painting starts at 0,0).
 * 'group' does not have to be deleted
 */
template<typename T>
void Measure_visualize(const Measure<T>& m, T measureWidth, double width, double height, const QColor& color, QGraphicsScene *scene, QGraphicsItem **item) {
	double w = width;
	double h = height;

	double mm = pixelsPerMillimeter();

	double penWidth = mm * 0.5; // chosen
	double halfPenWidth = penWidth / 2;

	double ww = w - 2*halfPenWidth; // we don't want to paint below 0 and above width/height and therefore always subtract half the pen width
	QPen pen;
	pen.setWidth(penWidth);
	pen.setStyle(Qt::SolidLine);
	pen.setColor(color);

	double pixelsPerUnit = ww / (double) measureWidth;
	double leftWhisker   = m.secondPercentile       * pixelsPerUnit + halfPenWidth;
	double left          = m.firstQuartile          * pixelsPerUnit + halfPenWidth;
	double middle        = m.median                 * pixelsPerUnit + halfPenWidth;
	double right         = m.thirdQuartile          * pixelsPerUnit + halfPenWidth;
	double rightWhisker  = m.ninetyEighthPercentile * pixelsPerUnit + halfPenWidth;


	QList<QGraphicsItem*> items; items.reserve(6);

	items.append(scene->addLine(QLineF(QPointF(leftWhisker, 1*(h/4) + halfPenWidth), QPointF(leftWhisker , 3*(h/4) - halfPenWidth)), pen)); // left whisker
	items.append(scene->addLine(QLineF(QPointF(leftWhisker, h/2), QPointF(left, h/2)), pen)); // left line
	items.append(scene->addLine(QLineF(QPointF(middle, halfPenWidth), QPointF(middle, h - halfPenWidth)), pen)); // middle line
	items.append(scene->addRect(QRectF(QPointF(left, halfPenWidth), QPointF(right, h - halfPenWidth)), pen)); // box
	items.append(scene->addLine(QLineF(QPointF(right, h/2), QPointF(rightWhisker, h/2)), pen)); // right line
	items.append(scene->addLine(QLineF(QPointF(rightWhisker, 1*(h/4) + halfPenWidth), QPointF(rightWhisker, 3*(h/4) - halfPenWidth)), pen)); // right whisker

	*item = (QGraphicsItem*) scene->createItemGroup(items);
}
#endif /* MEASURE_HPP */
