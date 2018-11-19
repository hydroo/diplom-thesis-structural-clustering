#ifndef CALL_TREE_HPP
#define CALL_TREE_HPP

#include "common/measure.hpp"
#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/unifier.hpp"

struct CallTreeNode {
	function_t id;
	QMap<function_t, CallTreeNode*> children;
	QSet<CallTreeNode*> parents;
	int subTreeEdgeCount;

	struct Statistics {
		int64_t invocationCount;
		Measure<int64_t> exclusiveTime;
		Measure<int64_t> inclusiveTime;
	};

	QScopedPointer<Statistics> statistics;
};

struct CallTree {
	QMap<process_t, CallTreeNode*> processes;
	QMap<CallTreeNode*, process_t> reverseProcesses;
	QSet<CallTreeNode*> nodes;
};

/* note: indent is supposed to consist of tabs only */
QString CallTree_print(const CallTree& tree, const QMap<process_t, QString>& processNames = QMap<process_t, QString>(), const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), const QString& indent = "");

QString CallTree_toDot(const CallTree& tree, const QMap<process_t, QString>& processNames = QMap<process_t, QString>(), const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), bool withIdentifiers = false);

void CallTree_fromProcessTrace(const ProcessTrace& trace, process_t processIdentifier, const Unifier<function_t>& u, trace_t traceIdentifier, bool withStatistics, CallTree* callTree);

void CallTree_finalize(CallTree* t);

/* note: deletes the contents of 'from' */
void CallTree_merge(CallTree* from, CallTree* into);

void CallTree_compress(CallTree* tree);

int CallTree_nodeCount(const CallTree& tree);

// /* note: uses the SimpleCallTreeSimilarityMetric */
// void CallTree_groupSimilarProcesses(const CallTree& tree, double threshold, QList<QList<process_t>>* groups);

/* note: does not delete 'tree', only its contents */
void CallTree_delete(CallTree *tree);

#endif /* CALL_TREE_HPP */
