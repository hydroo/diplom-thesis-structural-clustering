#include "call-list/call-list.hpp"

#include <functional>

#include <QAction>
#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QTextDocument>
#include <QWidgetAction>

/* *** print *** */
static QString CallListEntry_print(const CallListEntry& e, const QString& indent = "") {
	QString ret;
	QTextStream s(&ret);

	s << indent + "calls "    << e.calls << "\n";
	s << indent + "calledBy " << e.calledBy << "\n";
	if (e.statistics.isNull() == false) {
		s << indent + "inv "      << e.statistics->invocationCount << "\n";
		s << indent + "ex: "      << Measure_print(e.statistics->exclusiveTime) << "\n";
		s << indent + "in: "      << Measure_print(e.statistics->inclusiveTime) << "\n";
	}

	return ret;
}

QString CallList_print(const CallList& l, const QMap<function_t, QString>& functionNames, const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	QMapIterator<function_t, CallListEntry> i(l);
	while(i.hasNext()) {
		i.next();
		const auto& e = i.value();
		auto to = i.key();

		s << indent << to << " " << functionNames[to] << " {\n";
		s << CallListEntry_print(e, indent + "\t");
		s << indent << "}\n";
	}

	return ret;
}

/* *** visualize *** */

class GraphicsScene : public QGraphicsScene {
	Q_OBJECT
private:
	enum VisualizeWhat {
		ExclusiveTime,
		InclusiveTime,
		InvocationCount
	};

	VisualizeWhat which = VisualizeWhat::InvocationCount;

	QMap<process_t, CallList> callLists;
	QList<process_t> processOrder;
	QMap<function_t, QString> functionNames;
	QMap<process_t, QString> processNames;

	QMap<function_t, int64_t> accumulatedInvocationCount; /* used for sorting functions */
	QMap<function_t, double> exclusiveTimeMedianMean;     /* used for sorting functions */
	QMap<function_t, double> inclusiveTimeMedianMean;     /* used for sorting functions */

	double boxWidth = 600;

	QMap<process_t, QColor> processColors;

public:
	GraphicsScene(const QMap<process_t, CallList>& callLists_, const QList<process_t>& processOrder_, const QMap<function_t, QString>& functionNames_, const QMap<process_t, QString>& processNames_, QObject* parent = nullptr) : QGraphicsScene(parent), callLists(callLists_), processOrder(processOrder_), functionNames(functionNames_), processNames(processNames_) {

		/* test one CallListEntry for the existance of the statistics */
		QMapIterator<process_t, CallList> i(callLists);
		while (i.hasNext()) {
			i.next();
			const CallList& someCallList = i.value();
			QMapIterator<function_t, CallListEntry> j(someCallList);
			if (j.hasNext()) {
				j.next();
				const CallListEntry& e = j.value();
				if (e.statistics.isNull() == true) {
					qerr << "you need statistics for the visualization to work. aborting.\n";
					qerr.flush();
					assert(e.statistics.isNull() == false);
				} else {
					break;
				}
			} else {
				continue;
			}
		}

		/* determine accumulatedInvocationCount, exclusiveTimeMedianMean, inclusiveTimeMedianMean */
		foreach (const CallList& l, callLists) {
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

		/* determine process colors */
		if (this->processOrder.size() > 0) { this->processColors[processOrder[0]] = QColor((QRgb) 0x00000000); }
		if (this->processOrder.size() > 1) { this->processColors[processOrder[1]] = QColor((QRgb) 0x001919b8); }
		if (this->processOrder.size() > 2) { this->processColors[processOrder[2]] = QColor((QRgb) 0x0019bd19); }
		if (this->processOrder.size() > 3) { this->processColors[processOrder[3]] = QColor((QRgb) 0x00bb1919); }
		if (this->processOrder.size() > 4) { this->processColors[processOrder[4]] = QColor((QRgb) 0x00bc6e19); }
		if (this->processOrder.size() > 5) { this->processColors[processOrder[5]] = QColor((QRgb) 0x00ba19ba); }
		if (this->processOrder.size() > 6) { this->processColors[processOrder[6]] = QColor((QRgb) 0x0019bcbc); }
		for (int i = 7; i < this->processOrder.size(); i += 1) {
			process_t p = this->processOrder[i];
			uint32_t hash = (uint32_t) qHash( QString("%1").arg(p) + this->processNames[p]);
			this->processColors[p] = QColor::fromRgb((hash >> 24)&0xff, (hash >> 16)&0xff, (hash >> 8)&0xff);
		}
	}

private:
	QList<function_t> selectedFunctions() const {
		QList<QGraphicsItem*> items = this->selectedItems();
		if (items.size() == 0) {
			QSet<function_t> allFunctions;
			foreach (const CallList& l, callLists) {
				allFunctions |= l.keys().toSet();
			}
			return allFunctions.toList();
		} else {
			QList<function_t> functions;
			foreach (QGraphicsItem* i, items) {
				bool ok;
				functions.append(i->data(0).toInt(&ok));
				assert(ok == true);
			}
			return functions;
		}
	}

public:
	void redraw() {

		QList<function_t> sortedFunctions = selectedFunctions();

		this->clear();

		if (this->which == VisualizeWhat::ExclusiveTime) {
			/* sort functions by exclusive time median */
			std::sort(sortedFunctions.begin(), sortedFunctions.end(), [&](function_t a, function_t b){return this->exclusiveTimeMedianMean[a] > this->exclusiveTimeMedianMean[b];});
		} else if (this->which == VisualizeWhat::InclusiveTime) {
			std::sort(sortedFunctions.begin(), sortedFunctions.end(), [&](function_t a, function_t b){return this->inclusiveTimeMedianMean[a] > this->inclusiveTimeMedianMean[b];});
		} else { /* if (this->which == VisualizeWhat::InvocationCount) */
			std::sort(sortedFunctions.begin(), sortedFunctions.end(), [&](function_t a, function_t b){return this->accumulatedInvocationCount[a] > this->accumulatedInvocationCount[b];});
		}

		int maxFunctionNameWidth   = -1;
		int64_t maxExclusiveTime   = -1;
		int64_t maxInclusiveTime   = -1;
		int64_t maxInvocationCount = -1;
		foreach (auto f, sortedFunctions) {
			foreach (const CallList& l, this->callLists) {
				maxFunctionNameWidth = std::max(maxFunctionNameWidth, textWidth(functionNames[f]));
				maxExclusiveTime     = std::max(maxExclusiveTime    , l[f].statistics->exclusiveTime.ninetyEighthPercentile);
				maxInclusiveTime     = std::max(maxInclusiveTime    , l[f].statistics->inclusiveTime.ninetyEighthPercentile);
				maxInvocationCount   = std::max(maxInvocationCount  , l[f].statistics->invocationCount);
			}
		}
		assert(maxFunctionNameWidth >= 0);
		assert(maxExclusiveTime     >= 0);
		assert(maxInclusiveTime     >= 0);
		assert(maxInvocationCount   >= 0);

		int mm                  = (int) pixelsPerMillimeter();
		int boxHeight           = (int) (1.8*graphicsSceneTextHeight());        // chosen
		int spaceBetweenEntries = (int) (1*mm);                    // chosen
		int entryHeight         = boxHeight + spaceBetweenEntries;
		int sceneMargin         = (int) (1*mm);
		int sceneHeight         = (sortedFunctions.size() + 2) * entryHeight + 2*sceneMargin;
		int sceneWidth          = boxWidth + maxFunctionNameWidth + 3*sceneMargin;

		this->setSceneRect(0, 0, sceneWidth, sceneHeight);

		this->setBackgroundBrush(QBrush(QColor(255, 255, 255)));

		int currentHeight = 0;

		/* misc info */
		QGraphicsSimpleTextItem* infoText1 = this->addSimpleText("miscellaneous info");
		infoText1->setPos(maxFunctionNameWidth - textWidth("miscellaneous info") + sceneMargin, sceneMargin);
		QString min, max;
		if (this->which == VisualizeWhat::ExclusiveTime)              { min = "0 ticks"; max = QString("%1 ticks").arg(maxExclusiveTime); }
		else if (this->which == VisualizeWhat::InclusiveTime)         { min = "0 ticks"; max = QString("%1 ticks").arg(maxInclusiveTime); }
		else /* if (this->which == VisualizeWhat::InvocationCount) */ { min = "0";       max = QString("%1").arg(maxInvocationCount); }
		QGraphicsSimpleTextItem* infoText2 = this->addSimpleText(QString("min: %1, max: %2").arg(min).arg(max));
		infoText2->setPos(maxFunctionNameWidth + 2*sceneMargin, sceneMargin);

		currentHeight += entryHeight;

		/* process colors */
		QGraphicsSimpleTextItem* processColorsText = this->addSimpleText("process colors");
		processColorsText->setPos(maxFunctionNameWidth - textWidth("process colors") + sceneMargin, currentHeight + sceneMargin);

		double widthPerEntry = this->boxWidth / this->processOrder.size();

		for (int i = 0; i < this->processOrder.size(); i += 1) {
			process_t p = this->processOrder[i];
			QGraphicsItem* box = this->addRect(sceneMargin + maxFunctionNameWidth + sceneMargin + widthPerEntry*i, currentHeight + sceneMargin, widthPerEntry, boxHeight, QPen(Qt::NoPen), QBrush(this->processColors[p]));
			box->setToolTip(QString("id: %1, name: %2").arg(p).arg(this->processNames[p]));
		}

		currentHeight += entryHeight;

		/* boxplot */
		foreach (auto function, sortedFunctions) {

			QGraphicsRectItem* textBackground = this->addRect(0, 0, maxFunctionNameWidth, boxHeight, QPen(Qt::NoPen));
			QGraphicsSimpleTextItem* textText = this->addSimpleText(functionNames[function]);
			textText->setPos(maxFunctionNameWidth - textWidth(functionNames[function]), 0);
			QList<QGraphicsItem*> textItems; textItems.append(textBackground); textItems.append(textText);
			QGraphicsItemGroup* text = this->createItemGroup(textItems);
			text->setPos(sceneMargin, currentHeight + sceneMargin);

			QString toolTip;
			foreach (process_t p, this->processOrder) {
				if (this->callLists[p].contains(function)) {
					toolTip += QString("%1 %2 {\n").arg(p).arg(processNames[p]);
					toolTip += CallListEntry_print(this->callLists[p][function], "\t");
					toolTip += "}\n";
				}
			}
			text->setToolTip(toolTip);

			QList<QGraphicsItem*> items;

			for (int i = 0; i < this->processOrder.size(); i += 1) {
				process_t p = this->processOrder[i];

				if (this->callLists[p].contains(function) == false) { continue; }

				QGraphicsItem* item;

				if (this->which == VisualizeWhat::ExclusiveTime) {
					Measure_visualize(this->callLists[p][function].statistics->exclusiveTime, maxExclusiveTime, boxWidth, boxHeight, this->processColors[p], this, &item);
				} else if (this->which == VisualizeWhat::InclusiveTime) {
					Measure_visualize(this->callLists[p][function].statistics->inclusiveTime, maxInclusiveTime, boxWidth, boxHeight, this->processColors[p], this, &item);
				} else { /* if (this->which == VisualizeWhat::InvocationCount) */
					/* following code is copied/adapted from Measure_visualize() */
					double w = boxWidth;
					double h = boxHeight;
					double mm = pixelsPerMillimeter();
					double penWidth = mm * 0.5; // chosen
					double halfPenWidth = penWidth / 2;
					double ww = w - 2*halfPenWidth; // we don't want to paint below 0 and above width/height and therefore always subtract half the pen width
					double pixelsPerUnit = ww / (double) maxInvocationCount;

					double invocationCount =  this->callLists[p][function].statistics->invocationCount * pixelsPerUnit + halfPenWidth;

					QPen pen;
					pen.setWidth(penWidth);
					pen.setStyle(Qt::SolidLine);
					pen.setJoinStyle(Qt::MiterJoin);
					pen.setColor(this->processColors[p]);

					auto* line = this->addLine(QLineF(QPointF(invocationCount, halfPenWidth), QPointF(invocationCount , h - halfPenWidth)), pen);
					item = line;
				}

				items.prepend(item);
			}

			/* order of items: background first, then last process, ...., first process */

			QGraphicsItem* boxBackground = this->addRect(0, 0, boxWidth, boxHeight, QPen(Qt::NoPen), QBrush(QColor(230, 230, 230))); // box background
			items.prepend(boxBackground);
			QGraphicsItemGroup* box = this->createItemGroup(items);
			box->setPos(maxFunctionNameWidth + 2*sceneMargin, currentHeight + sceneMargin);

			QList<QGraphicsItem*> columnItems; columnItems.append(text); columnItems.append(box);
			QGraphicsItemGroup* column = this->createItemGroup(columnItems);
			column->setFlags(QGraphicsItem::ItemIsSelectable);
			column->setData(0, function); // store function identifier in the selectable item

			currentHeight += entryHeight;
		}
	}

private slots:
	void invTriggered() { this->which = VisualizeWhat::InvocationCount;}
	void excTriggered() { this->which = VisualizeWhat::ExclusiveTime;  }
	void incTriggered() { this->which = VisualizeWhat::InclusiveTime;  }
	void boxWidthChanged(const QString& text) {
		bool ok;
		double newBoxWidth = text.toInt(&ok);
		if (ok == true && newBoxWidth > 0) {
			this->boxWidth = newBoxWidth;
		}
	}

protected:
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {

		QMenu* menu = new QMenu;
		QActionGroup* group = new QActionGroup(menu);

		QAction *inv = menu->addAction("invocation count");
		QAction *exc = menu->addAction("exclusive time");
		QAction *inc = menu->addAction("inclusive time");

		QLineEdit* boxWidthEdit = new QLineEdit(menu);
		boxWidthEdit->setText(QString("%1").arg(this->boxWidth));
		boxWidthEdit->setToolTip("box width in pixels");
		QWidgetAction* boxWidthAction = new QWidgetAction(menu);
		boxWidthAction->setDefaultWidget(boxWidthEdit);
		connect(boxWidthEdit, SIGNAL(textChanged(const QString&)), this, SLOT(boxWidthChanged(const QString&)));
		menu->addAction(boxWidthAction);

		inv->setActionGroup(group);
		exc->setActionGroup(group);
		inc->setActionGroup(group);

		connect(inv, SIGNAL(triggered()), this, SLOT(invTriggered()));
		connect(exc, SIGNAL(triggered()), this, SLOT(excTriggered()));
		connect(inc, SIGNAL(triggered()), this, SLOT(incTriggered()));

		inv->setCheckable(true);
		exc->setCheckable(true);
		inc->setCheckable(true);

		if (this->which == VisualizeWhat::InvocationCount) { inv->setChecked(true); }
		else if (this->which == VisualizeWhat::ExclusiveTime) { exc->setChecked(true); }
		else if (this->which == VisualizeWhat::InclusiveTime) { inc->setChecked(true); }

		double oldBoxWidth = this->boxWidth;
		VisualizeWhat oldWhich = this->which;

		menu->exec(e->screenPos());

		delete menu;

		if (oldBoxWidth != this->boxWidth || oldWhich != this->which) {
			redraw();
		}

		e->accept();
		QGraphicsScene::contextMenuEvent(e);
	}

	virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) {

		if (e->modifiers() == Qt::ControlModifier && e->button() == Qt::LeftButton) {

			foreach (auto i, this->items(e->scenePos())) {
				if ((i->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable) {
					i->setSelected(i->isSelected() == false);
				}
			}

			e->accept();
		} else if (e->modifiers() == Qt::ShiftModifier && e->button() == Qt::LeftButton) {

			QRectF r;
			foreach (QGraphicsItem* i, this->selectedItems()) {
				if ((i->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable) {
					r = r.united(i->boundingRect());
				}
			}

			foreach (QGraphicsItem* i, this->items(e->scenePos())) {
				if ((i->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable) {
					r = r.united(i->boundingRect());
				}
			}

			foreach (QGraphicsItem* i, this->items(r)) {
				if ((i->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable) {
					i->setSelected(true);
				}
			}

			e->accept();
		} else {
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

void CallList_visualize(const QMap<process_t, CallList>& m, const QList<process_t>& processOrder, const QMap<function_t, QString>& functionNames, const QMap<process_t, QString>& processNames, QGraphicsScene **scene_) {
	auto* scene = new GraphicsScene(m, processOrder, functionNames, processNames);
	scene->redraw();
	*scene_ = scene;
}

void CallList_fromProcessTrace(const ProcessTrace& trace, const Unifier<function_t>& u, trace_t traceIdentifier, bool withStatistics, CallList* l) {

	std::function<void(function_t, const QList<FunctionCall>&, const Unifier<function_t>&, trace_t, bool, CallList*)> traverse = [&traverse](function_t from, const QList<FunctionCall>& subCalls, const Unifier<function_t>& u, trace_t traceIdentifier, bool withStatistics, CallList *l) {

		auto newEntry = [](function_t f, bool withStatistics, CallList *l) -> CallListEntry* {
			CallListEntry* ret;

			if (l->contains(f)) {
				ret = &((*l)[f]);
			} else {
				ret = &((*l)[f]);
				if (withStatistics) {
					ret->statistics.reset(new CallListEntry::Statistics);
					Measure_init(&(ret->statistics->exclusiveTime));
					Measure_init(&(ret->statistics->inclusiveTime));
				}
			}

			return ret;
		};

		function_t mappedFrom;
		CallListEntry* pre;

		if (from != 0) {
			mappedFrom = u.map(traceIdentifier, from);
			pre = newEntry(mappedFrom, withStatistics, l);
		} else {
			mappedFrom = from;
			pre = nullptr;
		}

		foreach (const auto f, subCalls) {

			function_t to = f.id;
			function_t mappedTo = u.map(traceIdentifier, to);

			if (from != 0) {
				pre->calls.insert(mappedTo);
			}

			CallListEntry *e = newEntry(mappedTo, withStatistics, l);

			e->calledBy.insert(mappedFrom);

			if (withStatistics) {
				int64_t accumulatedSubCallTime = 0;
				foreach (const auto s, f.calls) { accumulatedSubCallTime += s.end - s.begin; }

				int64_t inclusiveTime = f.end - f.begin;
				int64_t exclusiveTime = inclusiveTime - accumulatedSubCallTime;

				Measure_record(&(e->statistics->exclusiveTime), exclusiveTime);
				Measure_record(&(e->statistics->inclusiveTime), inclusiveTime);
			}

			traverse(to, f.calls, u, traceIdentifier, withStatistics, l);
		}
	};

	traverse(0, trace, u, traceIdentifier, withStatistics, l);
}

void CallList_finalize(CallList* l) {

	bool withStatistics = false;

	QMapIterator<function_t, CallListEntry> i(*l);
	while (i.hasNext()) {
		i.next();

		if (withStatistics == false && i.value().statistics.isNull() == false) {
			withStatistics = true;
		}

		if (withStatistics == true) {

			Q_ASSERT(i.value().statistics.isNull() == false);

			Measure_finalize(&i.value().statistics->exclusiveTime);
			Measure_finalize(&i.value().statistics->inclusiveTime);

			i.value().statistics->invocationCount = i.value().statistics->exclusiveTime.dataPointCount;

			Q_ASSERT(i.value().statistics->exclusiveTime.dataPointCount == i.value().statistics->inclusiveTime.dataPointCount);
		}
	}
}

#include "call-list.moc"
