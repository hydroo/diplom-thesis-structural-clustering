#ifndef CALL_MATRIX_HPP
#define CALL_MATRIX_HPP

#include "call-list/call-list.hpp"
#include "common/measure.hpp"
#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/unifier.hpp"

/*
 * in comparison to call-list, exclusive/inclusive time is accumulated not for each function but
 * for each function that has been called by a certain function.
 *
 * so quantities differ depending on by whom a function has been called.
 */

struct CallMatrixCell {
	struct Statistics {
		int64_t invocationCount;
		Measure<int64_t> exclusiveTime;
		Measure<int64_t> inclusiveTime;
	};

	QScopedPointer<Statistics> statistics;

	CallMatrixCell           () : statistics(nullptr)  {}
	CallMatrixCell           (const CallMatrixCell& a) { *this = a; }
	CallMatrixCell& operator=(const CallMatrixCell& a) {
		if (a.statistics.isNull() == false) {
			this->statistics.reset(new Statistics(*a.statistics)); 
		} else {
			this->statistics.reset(nullptr);
		}
		return *this;
	}
};

/* note:
 *  * indent is supposed to consist of tabs only
 *  * from 0, means there is no parent function
 */
typedef QMap<function_t /*from*/, QMap<function_t /*to*/, CallMatrixCell>> CallMatrix;

QString CallMatrix_print(const CallMatrix& m, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), const QString& indent = "");

/* note:
 *  * CallList is used for sorting functions
 */
void CallMatrix_visualize(const QMap<process_t, CallMatrix>& m, const QMap<process_t, CallList>& l, const QList<process_t>& order, const QMap<function_t, QString>& functionNames, const QMap<process_t, QString>& processNames, QGraphicsScene **s);

void CallMatrix_fromProcessTrace(const ProcessTrace& processTrace, const Unifier<function_t>& u, trace_t traceIdentifier, bool withStatistics, CallMatrix* m);

void CallMatrix_finalize(CallMatrix* m);

#endif /* CALL_MATRIX_HPP */
