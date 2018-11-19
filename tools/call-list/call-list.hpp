#ifndef CALL_LIST_HPP
#define CALL_LIST_HPP

#include "common/difference.hpp"
#include "common/measure.hpp"
#include "common/prereqs.hpp"
#include "common/otf/otf.hpp"
#include "common/unifier.hpp"

struct QGraphicsScene;

struct CallListEntry {
	struct Statistics {
		int64_t invocationCount;
		Measure<int64_t> exclusiveTime;
		Measure<int64_t> inclusiveTime;
	};

	QScopedPointer<Statistics> statistics;

	QSet<function_t> calls;
	QSet<function_t> calledBy; /* 0 means it's the first function on the stack */

	CallListEntry           () : statistics(nullptr)  {}
	CallListEntry           (const CallListEntry& e) { *this = e; }
	CallListEntry& operator=(const CallListEntry& e) {
		if (e.statistics.isNull() == false) {
			this->statistics.reset(new Statistics(*e.statistics)); 
		} else {
			this->statistics.reset(nullptr);
		}
		return *this;
	}
};

typedef QMap<function_t, CallListEntry> CallList;

/* note: indent is supposed to consist of tabs only */
QString CallList_print(const CallList& l, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), const QString& indent = "");

void CallList_visualize(const QMap<process_t, CallList>& m, const QList<process_t>& order, const QMap<function_t, QString>& functionNames, const QMap<process_t, QString>& processNames, QGraphicsScene **s);

void CallList_fromProcessTrace(const ProcessTrace& processTrace, const Unifier<function_t>& u, trace_t traceIdentifier, bool withStatistics, CallList* l);

void CallList_finalize(CallList* l);

#endif /* CALL_LIST_HPP */
