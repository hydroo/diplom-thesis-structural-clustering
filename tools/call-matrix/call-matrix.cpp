#include "call-matrix/call-matrix.hpp"

#include <functional>

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QWidgetAction>

/* *** print *** */
QString CallMatrixCell_print(const CallMatrixCell& c, const QString& indent = "") {
	QString ret;
	QTextStream s(&ret);

	if (c.statistics.isNull() == false) {
		s << indent + "inv " << c.statistics->invocationCount << "\n";
		s << indent + "ex: " << Measure_print(c.statistics->exclusiveTime) << "\n";
		s << indent + "in: " << Measure_print(c.statistics->inclusiveTime) << "\n";
	}

	return ret;
}

QString CallMatrix_print(const CallMatrix& m, const QMap<function_t, QString>& functionNames, const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	QMapIterator<function_t, QMap<function_t, CallMatrixCell>> i(m);
	while (i.hasNext()) {
		i.next();
		auto from = i.key();

		if (from == 0) {
			s << indent << "root function {\n";
		} else {
			s << indent << from << " " << functionNames[from] << " {\n";
		}

		QMapIterator<function_t, CallMatrixCell> j(i.value());
		while(j.hasNext()) {
			j.next();
			const auto& c = j.value();
			auto to = j.key();

			if (c.statistics.isNull() == false) {
				s << indent + "\t" << to << " " << functionNames[to] << " {\n";
				s << CallMatrixCell_print(c, indent + "\t\t");
				s << indent + "\t" + "}\n";
			} else {
				s << indent + "\t" << to << " " << functionNames[to] << "\n";
			}
		}

		s << indent + "}\n";
	}

	return ret;
}

/* *** visualize *** */

class GraphicsScene2 : public QGraphicsScene {
	Q_OBJECT
private:
	enum VisualizeWhat {
		ExclusiveTime,
		InclusiveTime,
		InvocationCount
	};

	VisualizeWhat which = VisualizeWhat::InvocationCount;
	bool skipEmptyRowsAndColumns = true;
	bool showIdentifiers = true;

	QMap<process_t, CallMatrix> callMatrixes;
	QMap<process_t, CallList> callLists;
	QList<process_t> processOrder;
	QMap<function_t, QString> functionNames;
	QMap<process_t, QString> processNames;

	QMap<function_t, int64_t> accumulatedInvocationCount; /* used for sorting functions */
	QMap<function_t, double> exclusiveTimeMedianMean;     /* used for sorting functions */
	QMap<function_t, double> inclusiveTimeMedianMean;     /* used for sorting functions */

	QList<QGraphicsSimpleTextItem*> textItems;

	QString searchText; // reapply search for every redraw

public:
	GraphicsScene2(const QMap<process_t, CallMatrix>& callMatrixes_, const QMap<process_t, CallList>& callLists_, const QList<process_t>& processOrder_, const QMap<function_t, QString>& functionNames_, const QMap<process_t, QString>& processNames_, QObject* parent = nullptr) : QGraphicsScene(parent), callMatrixes(callMatrixes_), callLists(callLists_), processOrder(processOrder_), functionNames(functionNames_), processNames(processNames_) {

		/* *** taken from call-list/call-list.cpp *** */

		/* determine accumulatedInvocationCount, exclusiveTimeMedianMean, inclusiveTimeMedianMean */
		foreach (const CallList& l, this->callLists) {
			foreach (function_t f, l.keys()) {
				if (this->accumulatedInvocationCount.contains(f) == false) {
					assert(this->exclusiveTimeMedianMean.contains(f) == false);
					assert(this->inclusiveTimeMedianMean.contains(f) == false);
					this->accumulatedInvocationCount[f] = 0;
					this->exclusiveTimeMedianMean[f]    = 0;
					this->inclusiveTimeMedianMean[f]    = 0;
				}
				this->accumulatedInvocationCount[f] += l[f].statistics->invocationCount;
				this->exclusiveTimeMedianMean[f]    += (double) l[f].statistics->exclusiveTime.median * (double) l[f].statistics->invocationCount;
				this->inclusiveTimeMedianMean[f]    += (double) l[f].statistics->inclusiveTime.median * (double) l[f].statistics->invocationCount;
			}
		}

		foreach (function_t f, this->accumulatedInvocationCount.keys()) {
			this->exclusiveTimeMedianMean[f] /= this->accumulatedInvocationCount[f];
			this->inclusiveTimeMedianMean[f] /= this->accumulatedInvocationCount[f];
		}
	}

private:
	void selectedFunctions(QList<function_t>* from, QList<function_t>* to) const {

		from->clear();
		to->clear();

		QList<QGraphicsItem*> items = this->selectedItems();
		foreach (QGraphicsItem* i, items) {
			bool ok1, ok2;

			function_t function = i->data(0).toInt(&ok1);
			int fromOrTo        = i->data(1).toInt(&ok2);

			assert(ok1 == true); assert(ok2 == true);

			if      (fromOrTo == 0) { from->append(function); }
			else if (fromOrTo == 1) { to->append(function); }
			else                    { assert(false); }
		}

		if (from->isEmpty()) {
			QSet<function_t> all;
			foreach (const CallMatrix& m, this->callMatrixes) {
				all |= m.keys().toSet();
			}

			*from = all.toList();
		}
		if (to->isEmpty()) {
			QSet<function_t> all;
			foreach (const CallMatrix& m, this->callMatrixes) {
				foreach (const auto& column, m) {
					all |= column.keys().toSet();
				}
			}
			*to = all.toList();
		}
	}

public:
	void redraw() {

		/* create dummy statistics used later for missing statistics information */
		CallMatrixCell::Statistics dummyStatistics;
		dummyStatistics.invocationCount = 1;
		Measure_init(&dummyStatistics.exclusiveTime);
		Measure_record(&dummyStatistics.exclusiveTime, (int64_t) 1);
		Measure_finalize(&dummyStatistics.exclusiveTime);
		Measure_init(&dummyStatistics.inclusiveTime);
		Measure_record(&dummyStatistics.inclusiveTime, (int64_t) 1);
		Measure_finalize(&dummyStatistics.inclusiveTime);

		QList<function_t> fromFunctions, toFunctions;
		this->selectedFunctions(&fromFunctions, &toFunctions);

		/* filter out empty rows and columns
		 *
		 * note: for some reason the order of equally valued items changes during filtering
		 *       I don't know why, but it shouldn't happen in a proper implementation */
		if (this->skipEmptyRowsAndColumns == true) {
			QList<function_t> fromFunctionsNew, toFunctionsNew;

			foreach (auto from, fromFunctions) {

				bool hasAtLeastOneEntry = false;

				foreach (const CallMatrix& m, this->callMatrixes) {
					if (m.contains(from)) {
						foreach (auto to, toFunctions) {
							if (m[from].contains(to)) {
								hasAtLeastOneEntry = true;
								break;
							}
						}
					}

					if (hasAtLeastOneEntry == true) break;
				}

				if (hasAtLeastOneEntry) { fromFunctionsNew.append(from); }
			}

			foreach (auto to, toFunctions) {

				bool hasAtLeastOneEntry = false;

				foreach (const CallMatrix& m, this->callMatrixes) {
					foreach (auto from, fromFunctions) {
						if (m.contains(from)) {
							if (m[from].contains(to)) {
								hasAtLeastOneEntry = true;
								break;
							}
						}
					}

					if (hasAtLeastOneEntry == true) break;
				}

				if (hasAtLeastOneEntry) { toFunctionsNew.append(to); }
			}

			fromFunctions = fromFunctionsNew;
			toFunctions   = toFunctionsNew;
		}

		this->clear();

		/* root function 0 is inserted into this->callList during the following sorting. shouldn't be a problem though */
		if (this->which == VisualizeWhat::ExclusiveTime) {
			/* sort functions by exclusive time median */
			std::sort(fromFunctions.begin(), fromFunctions.end(), [&](function_t a, function_t b){return this->exclusiveTimeMedianMean[a] > this->exclusiveTimeMedianMean[b];});
			std::sort(toFunctions.begin(), toFunctions.end(), [&](function_t a, function_t b){return this->exclusiveTimeMedianMean[a] > this->exclusiveTimeMedianMean[b];});
		} else if (this->which == VisualizeWhat::InclusiveTime) {
			std::sort(fromFunctions.begin(), fromFunctions.end(), [&](function_t a, function_t b){return this->inclusiveTimeMedianMean[a] > this->inclusiveTimeMedianMean[b];});
			std::sort(toFunctions.begin(), toFunctions.end(), [&](function_t a, function_t b){return this->inclusiveTimeMedianMean[a] > this->inclusiveTimeMedianMean[b];});
		} else { /* if (this->which == VisualizeWhat::InvocationCount) */
			std::sort(fromFunctions.begin(), fromFunctions.end(), [&](function_t a, function_t b){return this->accumulatedInvocationCount[a] > this->accumulatedInvocationCount[b];});
			std::sort(toFunctions.begin(), toFunctions.end(), [&](function_t a, function_t b){return this->accumulatedInvocationCount[a] > this->accumulatedInvocationCount[b];});
		}

		/* determine string lengths for function names on the left and upper side */
		int maxFromIdentifierWidth = -1;
		int maxFromNameWidth       = -1;
		int maxToIdentifierHeight  = -1;
		int maxToNameHeight        = -1;
		foreach (auto f, fromFunctions) {
			maxFromIdentifierWidth = std::max(maxFromIdentifierWidth, textWidth(QString("%1").arg(f), true));
			maxFromNameWidth       = std::max(maxFromNameWidth      , textWidth(QString("%1").arg(this->functionNames[f]), true));
		}
		foreach (auto f, toFunctions) {
			maxToIdentifierHeight = std::max(maxToIdentifierHeight, textWidth(QString("%1").arg(f), true));
			maxToNameHeight       = std::max(maxToNameHeight      , textWidth(QString("%1").arg(this->functionNames[f]), true));
		}
		// assert(maxFromIdentifierWidth >= 0); /* can't check because it is now possible to select 0 functions */
		// assert(maxFromNameWidth       >= 0);
		// assert(maxToIdentifierHeight  >= 0);
		// assert(maxToNameHeight        >= 0);

		int64_t maxExclusiveTime   = -1;
		int64_t maxInclusiveTime   = -1;
		int64_t maxInvocationCount = -1;
		foreach (const CallMatrix& m, this->callMatrixes) {
			foreach (auto from, fromFunctions) {
				if (m.contains(from) == false) continue;
				foreach (auto to, toFunctions) {
					if (m[from].contains(to) == false) continue;
					if (m[from][to].statistics.isNull() == false) {
						maxExclusiveTime   = std::max(maxExclusiveTime  , m[from][to].statistics->exclusiveTime.ninetyEighthPercentile);
						maxInclusiveTime   = std::max(maxInclusiveTime  , m[from][to].statistics->inclusiveTime.ninetyEighthPercentile);
						maxInvocationCount = std::max(maxInvocationCount, m[from][to].statistics->invocationCount);
					} else {
						maxExclusiveTime   = std::max(maxExclusiveTime  , (int64_t) 1);
						maxInclusiveTime   = std::max(maxInclusiveTime  , (int64_t) 1);
						maxInvocationCount = std::max(maxInvocationCount, (int64_t) 1);
					}
				}
			}
		}
		// assert(maxExclusiveTime   >= 0); /* can't check because hits when you select things that don't have records about that */
		// assert(maxInclusiveTime   >= 0);
		// assert(maxInvocationCount >= 0);

		int mm           = (int) pixelsPerMillimeter();
		int entrySize    = graphicsSceneTextHeight() * 1.5;
		int sceneMargin  = (int) (1*mm);

		int fromWidth, toHeight;
		if (this->showIdentifiers == false) {
			fromWidth    = maxFromNameWidth;
			toHeight     = maxToNameHeight + 0.5*graphicsSceneTextHeight();
		} else {
			fromWidth    = maxFromIdentifierWidth + textWidth(", ") + maxFromNameWidth;
			toHeight     = maxToIdentifierHeight  + textWidth(", ") + maxToNameHeight + 0.5*graphicsSceneTextHeight();
		}

		int matrixWidth  = toFunctions.size()*entrySize;
		int matrixHeight = fromFunctions.size()*entrySize;

		int sceneWidth   = fromWidth + sceneMargin + matrixWidth;
		int sceneHeight  = toHeight  + sceneMargin + matrixHeight;

		this->setSceneRect(0, 0, sceneWidth, sceneHeight);

		this->setBackgroundBrush(QBrush(QColor(255, 255, 255)));

		this->textItems.clear();

		/* draw from functions */
		int currentHeight = toHeight + sceneMargin;
		foreach (auto from, fromFunctions) {
			QString fromText;
			if (this->showIdentifiers == false) {
				fromText = QString("%1").arg(this->functionNames[from]);
			} else {
				fromText = QString("%1, %2").arg(from).arg(this->functionNames[from]);
			}
			QGraphicsSimpleTextItem* fromTextItem = this->addSimpleText(fromText);
			this->textItems.append(fromTextItem);
			fromTextItem->setPos(fromWidth - textWidth(fromText, false), currentHeight);

			currentHeight += entrySize;
		}

		/* draw to functions */
		int currentWidth = fromWidth + sceneMargin;
		foreach (auto to, toFunctions) {
			QString toText;
			if (this->showIdentifiers == false) {
				toText = QString("%1").arg(this->functionNames[to]);
			} else {
				toText = QString("%1, %2").arg(to).arg(this->functionNames[to]);
			}
			QGraphicsSimpleTextItem* toTextItem = this->addSimpleText(toText);
			this->textItems.append(toTextItem);
			toTextItem->setRotation(-60);
			toTextItem->setPos(currentWidth, toHeight - 0.5*graphicsSceneTextHeight());

			currentWidth += entrySize;
		}

		searchWidgetTextChanged(this->searchText); // apply bold font to funciton matched by the search text;

		/* construct gradient */
		int gradientResolution = 3000;

		QLinearGradient gradient(QPointF(0,0), QPointF(gradientResolution, 0));
		gradient.setColorAt(0.0     , QColor(0x7fff7f)); // bright green
		gradient.setColorAt(0.333333, QColor(0x00c500)); // green
		gradient.setColorAt(0.666666, QColor(0xffbe00)); // organge
		gradient.setColorAt(1.0     , QColor(0xe00000)); // red

		QImage gradientImage(gradientResolution + 1, 1, QImage::Format_RGB32);
		QPainter p;
		p.begin(&gradientImage);
		p.setPen(QPen(Qt::NoPen));
		p.setBrush(QBrush(gradient));
		p.drawRect(0, 0, gradientResolution + 1, 1);
		p.end();

		auto exclusiveTimeColor   = [&](double value) { Q_ASSERT(maxExclusiveTime >= 0); return QColor(gradientImage.pixel(value/maxExclusiveTime*gradientResolution,0));};
		auto inclusiveTimeColor   = [&](double value) { Q_ASSERT(maxInclusiveTime >= 0); return QColor(gradientImage.pixel(value/maxInclusiveTime*gradientResolution,0));};
		auto invocationCountColor = [&](double value) { Q_ASSERT(maxInvocationCount >= 0); return QColor(gradientImage.pixel(value/maxInvocationCount*gradientResolution,0));};

		/* calculate square sizes: area based parts */
		double secondPercentileParts       = 1;
		double firstQuartileParts          = 3;
		double medianParts                 = 6;
		double thirdQuartileParts          = 3;
		double ninetyEighthPercentileParts = 1;
		double partsSum                    = 14;

		double ninetyEighthPercentilePartsSum = ninetyEighthPercentileParts;
		double thirdQuartilePartsSum          = thirdQuartileParts          + ninetyEighthPercentilePartsSum;
		double medianPartsSum                 = medianParts                 + thirdQuartilePartsSum;
		double firstQuartilePartsSum          = firstQuartileParts          + medianPartsSum;
		double secondPercentilePartsSum       = secondPercentileParts       + firstQuartilePartsSum;

		double area = entrySize*entrySize;
		double secondPercentileArea       = area * secondPercentilePartsSum       / partsSum;
		double firstQuartileArea          = area * firstQuartilePartsSum          / partsSum;
		double medianArea                 = area * medianPartsSum                 / partsSum;
		double thirdQuartileArea          = area * thirdQuartilePartsSum          / partsSum;
		double ninetyEighthPercentileArea = area * ninetyEighthPercentilePartsSum / partsSum;

		double secondPercentileSize       = sqrt(secondPercentileArea      );
		double firstQuartileSize          = sqrt(firstQuartileArea         );
		double medianSize                 = sqrt(medianArea                );
		double thirdQuartileSize          = sqrt(thirdQuartileArea         );
		double ninetyEighthPercentileSize = sqrt(ninetyEighthPercentileArea);

		double secondPercentilePos        = entrySize * 0.5 - secondPercentileSize       * 0.5;
		double firstQuartilePos           = entrySize * 0.5 - firstQuartileSize          * 0.5;
		double medianPos                  = entrySize * 0.5 - medianSize                 * 0.5;
		double thirdQuartilePos           = entrySize * 0.5 - thirdQuartileSize          * 0.5;
		double ninetyEighthPercentilePos  = entrySize * 0.5 - ninetyEighthPercentileSize * 0.5;

		double circumference = 4*1;
		double circumferencePerProcess = circumference / this->processOrder.size();
		double currentCircumference = 0;

		foreach (process_t p, this->processOrder) {

			/* calculate the polygon in which this process will draw its information */
			QVector<QPointF> unitPolygon(3);
			unitPolygon[0] = QPointF(0.5, 0.5); /* middle */

			double c  = currentCircumference;
			double c2 = c + circumferencePerProcess;

			if      (c >= 3)       { unitPolygon[1] = QPointF(0          , 1 - (c-3)); }
			else if (c >= 2)       { unitPolygon[1] = QPointF(1 - (c - 2), 1        ); }
			else if (c >= 1)       { unitPolygon[1] = QPointF(1          ,      c-1 ); }
			else /* if (c >= 0) */ { unitPolygon[1] = QPointF(     c     , 0        ); }

			int lastPolygonIndex = 2;

			if      (c >= 2 && c < 3 && c2 > 3) { unitPolygon[2] = QPointF(0, 1); lastPolygonIndex += 1; }
			else if (c >= 1 && c < 2 && c2 > 2) { unitPolygon[2] = QPointF(1, 1); lastPolygonIndex += 1; }
			else if (c >= 0 && c < 1 && c2 > 1) { unitPolygon[2] = QPointF(1, 0); lastPolygonIndex += 1; }

			unitPolygon.resize(lastPolygonIndex+1);

			if      (c2 >= 3)       { unitPolygon[lastPolygonIndex] = QPointF(0           , 1 - (c2-3)); }
			else if (c2 >= 2)       { unitPolygon[lastPolygonIndex] = QPointF(1 - (c2 - 2), 1         ); }
			else if (c2 >= 1)       { unitPolygon[lastPolygonIndex] = QPointF(1           ,      c2-1 ); }
			else /* if (c2 >= 0) */ { unitPolygon[lastPolygonIndex] = QPointF(     c2     , 0         ); }

			if (circumferencePerProcess == circumference) {
				/* only one process is to be drawn */
				unitPolygon[0] = QPointF(0, 0);
				unitPolygon[1] = QPointF(1, 0);
				unitPolygon[2] = QPointF(1, 1);
				unitPolygon[3] = QPointF(0, 1);
			}

			QVector<QPointF> secondPercentilePolygon, firstQuartilePolygon, medianPolygon, thirdQuartilePolygon, ninetyEighthPercentilePolygon, invocationsPolygon;
			if (lastPolygonIndex == 2)            { secondPercentilePolygon.resize(3); firstQuartilePolygon.resize(3); medianPolygon.resize(3); thirdQuartilePolygon.resize(3); ninetyEighthPercentilePolygon.resize(3); invocationsPolygon.resize(3); }
			else /* if (lastPolygonIndex == 3) */ { secondPercentilePolygon.resize(4); firstQuartilePolygon.resize(4); medianPolygon.resize(4); thirdQuartilePolygon.resize(4); ninetyEighthPercentilePolygon.resize(4); invocationsPolygon.resize(4); }

			/* draw the polygon */
			const CallMatrix& m = this->callMatrixes[p];

			currentHeight = toHeight + sceneMargin;
			foreach (auto from, fromFunctions) {

				if (m.contains(from) == false) { currentHeight += entrySize; continue; }

				currentWidth = fromWidth + sceneMargin;
				foreach (auto to, toFunctions) {

					if (m[from].contains(to) == false) { currentWidth += entrySize; continue; }

					const CallMatrixCell& c = m[from][to];
					const CallMatrixCell::Statistics* s;

					if (c.statistics.isNull() == false) {
						s = c.statistics.data();
					} else {
						s = &dummyStatistics;
					}

					secondPercentilePolygon[0] = QPointF(currentWidth + secondPercentilePos, currentHeight + secondPercentilePos) + unitPolygon[0] * secondPercentileSize;
					secondPercentilePolygon[1] = QPointF(currentWidth + secondPercentilePos, currentHeight + secondPercentilePos) + unitPolygon[1] * secondPercentileSize;
					secondPercentilePolygon[2] = QPointF(currentWidth + secondPercentilePos, currentHeight + secondPercentilePos) + unitPolygon[2] * secondPercentileSize;
					if (lastPolygonIndex == 3) {
						secondPercentilePolygon[3] = QPointF(currentWidth + secondPercentilePos, currentHeight + secondPercentilePos) + unitPolygon[3] * secondPercentileSize;
					}

					firstQuartilePolygon[0] = QPointF(currentWidth + firstQuartilePos, currentHeight + firstQuartilePos) + unitPolygon[0] * firstQuartileSize;
					firstQuartilePolygon[1] = QPointF(currentWidth + firstQuartilePos, currentHeight + firstQuartilePos) + unitPolygon[1] * firstQuartileSize;
					firstQuartilePolygon[2] = QPointF(currentWidth + firstQuartilePos, currentHeight + firstQuartilePos) + unitPolygon[2] * firstQuartileSize;
					if (lastPolygonIndex == 3) {
						firstQuartilePolygon[3] = QPointF(currentWidth + firstQuartilePos, currentHeight + firstQuartilePos) + unitPolygon[3] * firstQuartileSize;
					}

					medianPolygon[0] = QPointF(currentWidth + medianPos, currentHeight + medianPos) + unitPolygon[0] * medianSize;
					medianPolygon[1] = QPointF(currentWidth + medianPos, currentHeight + medianPos) + unitPolygon[1] * medianSize;
					medianPolygon[2] = QPointF(currentWidth + medianPos, currentHeight + medianPos) + unitPolygon[2] * medianSize;
					if (lastPolygonIndex == 3) {
						medianPolygon[3] = QPointF(currentWidth + medianPos, currentHeight + medianPos) + unitPolygon[3] * medianSize;
					}

					thirdQuartilePolygon[0] = QPointF(currentWidth + thirdQuartilePos, currentHeight + thirdQuartilePos) + unitPolygon[0] * thirdQuartileSize;
					thirdQuartilePolygon[1] = QPointF(currentWidth + thirdQuartilePos, currentHeight + thirdQuartilePos) + unitPolygon[1] * thirdQuartileSize;
					thirdQuartilePolygon[2] = QPointF(currentWidth + thirdQuartilePos, currentHeight + thirdQuartilePos) + unitPolygon[2] * thirdQuartileSize;
					if (lastPolygonIndex == 3) {
						thirdQuartilePolygon[3] = QPointF(currentWidth + thirdQuartilePos, currentHeight + thirdQuartilePos) + unitPolygon[3] * thirdQuartileSize;
					}

					ninetyEighthPercentilePolygon[0] = QPointF(currentWidth + ninetyEighthPercentilePos, currentHeight + ninetyEighthPercentilePos) + unitPolygon[0] * ninetyEighthPercentileSize;
					ninetyEighthPercentilePolygon[1] = QPointF(currentWidth + ninetyEighthPercentilePos, currentHeight + ninetyEighthPercentilePos) + unitPolygon[1] * ninetyEighthPercentileSize;
					ninetyEighthPercentilePolygon[2] = QPointF(currentWidth + ninetyEighthPercentilePos, currentHeight + ninetyEighthPercentilePos) + unitPolygon[2] * ninetyEighthPercentileSize;
					if (lastPolygonIndex == 3) {
						ninetyEighthPercentilePolygon[3] = QPointF(currentWidth + ninetyEighthPercentilePos, currentHeight + ninetyEighthPercentilePos) + unitPolygon[3] * ninetyEighthPercentileSize;
					}

					invocationsPolygon[0] = QPointF(currentWidth, currentHeight) + unitPolygon[0] * entrySize;
					invocationsPolygon[1] = QPointF(currentWidth, currentHeight) + unitPolygon[1] * entrySize;
					invocationsPolygon[2] = QPointF(currentWidth, currentHeight) + unitPolygon[2] * entrySize;
					if (lastPolygonIndex == 3) {
						invocationsPolygon[3] = QPointF(currentWidth, currentHeight) + unitPolygon[3] * entrySize;
					}

					QString cellDump = QString("process: %1, %2\n\n%3").arg(p).arg(this->processNames[p]).arg(CallMatrixCell_print(c));

					if (this->which == VisualizeWhat::ExclusiveTime) {
						QColor secondPercentileColor       = exclusiveTimeColor(s->exclusiveTime.secondPercentile);
						QColor firstQuartileColor          = exclusiveTimeColor(s->exclusiveTime.firstQuartile);
						QColor medianColor                 = exclusiveTimeColor(s->exclusiveTime.median);
						QColor thirdQuartileColor          = exclusiveTimeColor(s->exclusiveTime.thirdQuartile);
						QColor ninetyEighthPercentileColor = exclusiveTimeColor(s->exclusiveTime.ninetyEighthPercentile);

						QGraphicsItem* item;
						item = this->addPolygon(secondPercentilePolygon, QPen(Qt::NoPen), QBrush(secondPercentileColor));
						item->setToolTip(QString("exclusiveTime: 2%: %1\n\n%2").arg(s->exclusiveTime.secondPercentile).arg(cellDump));
						item = this->addPolygon(firstQuartilePolygon, QPen(Qt::NoPen), QBrush(firstQuartileColor));
						item->setToolTip(QString("exclusiveTime: 25%: %1\n\n%2").arg(s->exclusiveTime.firstQuartile).arg(cellDump));
						item = this->addPolygon(medianPolygon, QPen(Qt::NoPen), QBrush(medianColor));
						item->setToolTip(QString("exclusiveTime: 50%: %1\n\n%2").arg(s->exclusiveTime.median).arg(cellDump));
						item = this->addPolygon(thirdQuartilePolygon, QPen(Qt::NoPen), QBrush(thirdQuartileColor));
						item->setToolTip(QString("exclusiveTime: 75%: %1\n\n%2").arg(s->exclusiveTime.thirdQuartile).arg(cellDump));
						item = this->addPolygon(ninetyEighthPercentilePolygon, QPen(Qt::NoPen), QBrush(ninetyEighthPercentileColor));
						item->setToolTip(QString("exclusiveTime: 98%: %1\n\n%2").arg(s->exclusiveTime.ninetyEighthPercentile).arg(cellDump));
					} else if (this->which == VisualizeWhat::InclusiveTime) {
						QColor secondPercentileColor       = inclusiveTimeColor(s->inclusiveTime.secondPercentile);
						QColor firstQuartileColor          = inclusiveTimeColor(s->inclusiveTime.firstQuartile);
						QColor medianColor                 = inclusiveTimeColor(s->inclusiveTime.median);
						QColor thirdQuartileColor          = inclusiveTimeColor(s->inclusiveTime.thirdQuartile);
						QColor ninetyEighthPercentileColor = inclusiveTimeColor(s->inclusiveTime.ninetyEighthPercentile);

						QGraphicsItem* item;
						item = this->addPolygon(secondPercentilePolygon, QPen(Qt::NoPen), QBrush(secondPercentileColor));
						item->setToolTip(QString("inclusiveTime: 2%: %1\n\n%2").arg(s->inclusiveTime.secondPercentile).arg(cellDump));
						item = this->addPolygon(firstQuartilePolygon, QPen(Qt::NoPen), QBrush(firstQuartileColor));
						item->setToolTip(QString("inclusiveTime: 25%: %1\n\n%2").arg(s->inclusiveTime.firstQuartile).arg(cellDump));
						item = this->addPolygon(medianPolygon, QPen(Qt::NoPen), QBrush(medianColor));
						item->setToolTip(QString("inclusiveTime: 50%: %1\n\n%2").arg(s->inclusiveTime.median).arg(cellDump));
						item = this->addPolygon(thirdQuartilePolygon, QPen(Qt::NoPen), QBrush(thirdQuartileColor));
						item->setToolTip(QString("inclusiveTime: 75%: %1\n\n%2").arg(s->inclusiveTime.thirdQuartile).arg(cellDump));
						item = this->addPolygon(ninetyEighthPercentilePolygon, QPen(Qt::NoPen), QBrush(ninetyEighthPercentileColor));
						item->setToolTip(QString("inclusiveTime: 98%: %1\n\n%2").arg(s->inclusiveTime.ninetyEighthPercentile).arg(cellDump));
					} else {/* if this->which == VisualizeWhat::InvocationCount */
						QColor invocationsColor = invocationCountColor(s->invocationCount);
						QGraphicsItem* item = this->addPolygon(invocationsPolygon, QPen(Qt::NoPen), QBrush(invocationsColor));
						item->setToolTip(QString("invocationCount: %1\n\n%2").arg(s->invocationCount).arg(cellDump));
					}

					currentWidth += entrySize;
				}

				currentHeight += entrySize;
			}

			currentCircumference += circumferencePerProcess;
		}

		/* draw invisible selectable columns */
		currentHeight = toHeight + sceneMargin;
		foreach (auto from, fromFunctions) {
			QGraphicsItem* columnBackground = this->addRect(0, currentHeight, sceneWidth, entrySize, QPen(Qt::NoPen), QBrush(Qt::NoBrush));
			columnBackground->setFlags(QGraphicsItem::ItemIsSelectable);
			columnBackground->setData(0, from);
			columnBackground->setData(1, 0); /* 0 = "from", not "to"*/
			currentHeight += entrySize;
		}

		/* draw invisible selectable rows */
		currentWidth = fromWidth + sceneMargin;
		foreach (auto to, toFunctions) {
			QGraphicsItem* rowBackground = this->addRect(currentWidth, 0, entrySize, sceneHeight, QPen(Qt::NoPen), QBrush(Qt::NoBrush));
			rowBackground->setFlags(QGraphicsItem::ItemIsSelectable);
			rowBackground->setData(0, to);
			rowBackground->setData(1, 1); /* 0 = "from", not "to"*/
			currentWidth += entrySize;
		}
	}

private slots:
	void invTriggered()                         { this->which = VisualizeWhat::InvocationCount; }
	void excTriggered()                         { this->which = VisualizeWhat::ExclusiveTime;   }
	void incTriggered()                         { this->which = VisualizeWhat::InclusiveTime;   }
	void skipTriggered(bool checked)            { this->skipEmptyRowsAndColumns    = checked;   }
	void showIdentifiersTriggered(bool checked) { this->showIdentifiers = checked; }
	void searchWidgetTextChanged(const QString& searchText) {
		if (this->textItems.isEmpty()) return;

		QFont normalFont = this->textItems[0]->font(); normalFont.setBold(false);
		QFont boldFont   = this->textItems[0]->font(); boldFont.setBold(true);

		foreach (QGraphicsSimpleTextItem* item, this->textItems) {
			if (item->text().contains(searchText, Qt::CaseSensitive) == false || searchText.length() == 0) {
				item->setFont(normalFont);
			} else {
				item->setFont(boldFont);
			}
		}

		this->searchText = searchText;
	}

protected:
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {

		QMenu* menu = new QMenu;
		QActionGroup* group = new QActionGroup(menu);

		QAction *inv   = menu->addAction("invocation count");
		QAction *exc   = menu->addAction("exclusive time");
		QAction *inc   = menu->addAction("inclusive time");
		menu->addSeparator();
		QAction *skip  = menu->addAction("skip empty rows and columns");
		menu->addSeparator();
		QAction *ident = menu->addAction("show function identifiers");
		menu->addSeparator();

		QWidget *searchWidget = new QWidget(menu);
		QLabel *searchLabel   = new QLabel("Search:", searchWidget);
		QLineEdit *searchEdit = new QLineEdit(this->searchText, searchWidget);
		connect(searchEdit, SIGNAL(textEdited(const QString&)), this, SLOT(searchWidgetTextChanged(const QString&)));
		QHBoxLayout *searchWidgetLayout = new QHBoxLayout(searchWidget);
		searchWidgetLayout->addWidget(searchLabel);
		searchWidgetLayout->addWidget(searchEdit);
		searchWidget->setLayout(searchWidgetLayout);
		searchWidgetLayout->setContentsMargins(2, 2, 2, 2);
		QWidgetAction* searchWidgetAction = new QWidgetAction(menu);
		searchWidgetAction->setDefaultWidget(searchWidget);
		menu->addAction(searchWidgetAction);

		inv->setActionGroup(group);
		exc->setActionGroup(group);
		inc->setActionGroup(group);

		connect(inv  , SIGNAL(triggered())    , this, SLOT(invTriggered()));
		connect(exc  , SIGNAL(triggered())    , this, SLOT(excTriggered()));
		connect(inc  , SIGNAL(triggered())    , this, SLOT(incTriggered()));
		connect(skip , SIGNAL(triggered(bool)), this, SLOT(skipTriggered(bool)));
		connect(ident, SIGNAL(triggered(bool)), this, SLOT(showIdentifiersTriggered(bool)));

		inv->setCheckable(true);
		exc->setCheckable(true);
		inc->setCheckable(true);
		skip->setCheckable(true);
		ident->setCheckable(true);

		if (this->which == VisualizeWhat::InvocationCount) { inv->setChecked(true); }
		else if (this->which == VisualizeWhat::ExclusiveTime) { exc->setChecked(true); }
		else if (this->which == VisualizeWhat::InclusiveTime) { inc->setChecked(true); }

		if (this->skipEmptyRowsAndColumns == true) { skip->setChecked(true); }
		if (this->showIdentifiers == true) { ident->setChecked(true); }

		auto which                   = this->which;
		auto skipEmptyRowsAndColumns = this->skipEmptyRowsAndColumns;
		auto showIdentifiers         = this->showIdentifiers;

		menu->exec(e->screenPos());
		delete menu;

		e->accept();
		QGraphicsScene::contextMenuEvent(e);

		if (which != this->which || skipEmptyRowsAndColumns != this->skipEmptyRowsAndColumns || showIdentifiers != this->showIdentifiers) {
			this->redraw();
		}
	}

	virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) {

		bool propagate = true;

		if (e->modifiers() == Qt::NoModifier && e->button() == Qt::LeftButton) {
			this->clearSelection();
		}

		if ((e->modifiers() == Qt::NoModifier || e->modifiers() == Qt::ShiftModifier) && e->button() == Qt::LeftButton) {

			QList<QGraphicsItem*> items = this->items(e->scenePos());

			foreach (auto i, items) {
				if ((i->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable) {
					i->setSelected(true);
				}
			}

			propagate = false;
			e->accept();
		} else if (e->modifiers() == Qt::ControlModifier && e->button() == Qt::LeftButton) {

			QList<QGraphicsItem*> items = this->items(e->scenePos());

			bool selected = true;

			foreach (auto i, items) {
				if ((i->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable) {
					selected &= i->isSelected();
				}
			}

			foreach (auto i, items) {
				if ((i->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable) {
					i->setSelected(selected == false);
				}
			}

			propagate = false;
			e->accept();
		}

		if (e->modifiers() == Qt::ShiftModifier && e->button() == Qt::LeftButton) {

			QRectF to;
			QRectF from;

			foreach (QGraphicsItem* i, this->selectedItems()) {
				if (i->data(1) == 0) {
					from = from.united(i->boundingRect());
				} else if (i->data(1) == 1) {
					to = to.united(i->boundingRect());
				}
			}

			foreach (QGraphicsItem* i, this->items(to, Qt::ContainsItemBoundingRect)) {
				i->setSelected(true);
			}
			foreach (QGraphicsItem* i, this->items(from, Qt::ContainsItemBoundingRect)) {
				i->setSelected(true);
			}
		}

		if (propagate == true) {
			QGraphicsScene::mousePressEvent(e);
		}
	}

	virtual void keyPressEvent(QKeyEvent* e) {
		if (e->key() == Qt::Key_Return) {
			this->redraw();
			e->accept();
		}
		QGraphicsScene::keyPressEvent(e);
	}
};

void CallMatrix_visualize(const QMap<process_t, CallMatrix>& m, const QMap<process_t, CallList>& l, const QList<process_t>& order, const QMap<function_t, QString>& functionNames, const QMap<process_t, QString>& processNames, QGraphicsScene **scene_) {
	auto* scene = new GraphicsScene2(m, l, order, functionNames, processNames);
	scene->redraw();
	*scene_ = scene;
}

void CallMatrix_fromProcessTrace(const ProcessTrace& processTrace, const Unifier<function_t>& u, trace_t t, bool withStatistics, CallMatrix* m) {

	std::function<void(function_t, const QList<FunctionCall>&, const Unifier<function_t>&, trace_t, bool, CallMatrix*)> traverse = [&traverse](function_t from, const QList<FunctionCall>& subCalls, const Unifier<function_t>& u, trace_t t, bool withStatistics, CallMatrix* m) {

		auto newCell = [](function_t from, function_t to, bool withStatistics, CallMatrix* m) {
			QMap<function_t, CallMatrixCell>* f = &((*m)[from]);
			CallMatrixCell* ret;

			if (f->contains(to)) {
				ret = &((*f)[to]);
			} else {
				ret = &((*f)[to]);
				if (withStatistics) {
					ret->statistics.reset(new CallMatrixCell::Statistics);
					Measure_init(&(ret->statistics->exclusiveTime));
					Measure_init(&(ret->statistics->inclusiveTime));
				}
			}

			return ret;
		};

		foreach (const auto f, subCalls) {

			CallMatrixCell *c;

			function_t mappedFrom;
			function_t mappedTo;

			if (from != 0) {
				mappedFrom = u.map(t, from);
			} else {
				mappedFrom = from;
			}
			mappedTo = u.map(t, f.id);

			c = newCell(mappedFrom, mappedTo, withStatistics, m);

			if (withStatistics) {
				int64_t accumulatedSubCallTime = 0;
				foreach (const auto s, f.calls) { accumulatedSubCallTime += s.end - s.begin; }

				int64_t inclusiveTime = f.end - f.begin;
				int64_t exclusiveTime = inclusiveTime - accumulatedSubCallTime;

				Measure_record(&(c->statistics->exclusiveTime), exclusiveTime);
				Measure_record(&(c->statistics->inclusiveTime), inclusiveTime);
			}

			traverse(f.id, f.calls, u, t, withStatistics, m);
		}
	};

	traverse(0, processTrace, u, t, withStatistics, m);
}

void CallMatrix_finalize(CallMatrix* m) {

	bool withStatistics = false;

	QMapIterator<function_t, QMap<function_t, CallMatrixCell>> i(*m);
	while (i.hasNext()) {
		i.next();

		QMapIterator<function_t, CallMatrixCell> j(i.value());
		while (j.hasNext()) {
			j.next();

			if (withStatistics == false && j.value().statistics.isNull() == false) {
				withStatistics = true;
			}

			if (withStatistics == true) {

				Q_ASSERT(j.value().statistics.isNull() == false);

				Measure_finalize(&j.value().statistics->exclusiveTime);
				Measure_finalize(&j.value().statistics->inclusiveTime);

				j.value().statistics->invocationCount = j.value().statistics->exclusiveTime.dataPointCount;

				Q_ASSERT(j.value().statistics->exclusiveTime.dataPointCount == j.value().statistics->inclusiveTime.dataPointCount);
			}
		}
	}
}

#include "call-matrix.moc"
