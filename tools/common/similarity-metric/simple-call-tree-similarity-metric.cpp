#include "common/similarity-metric/simple-call-tree-similarity-metric.hpp"

#include "call-tree/call-tree.hpp"

static void traverseNodes(const CallTreeNode& n, QMap<function_t, QMap<function_t, int>>* edges) {
	foreach (const CallTreeNode* m, n.children) {
		if ((*edges)[n.id].contains(m->id) == false) {
			(*edges)[n.id][m->id]  = 1;
		} else {
			(*edges)[n.id][m->id] += 1;
		}
		traverseNodes(*m, edges);
	}
}

static void traverseRoot(const CallTreeNode& n, QMap<function_t, QMap<function_t, int>>* edges, QSet<function_t>* roots) {
	foreach (const CallTreeNode* m, n.children) {
		roots->insert(m->id);
		traverseNodes(*m, edges);
	}
}

static double edgeSimilarity(const CallTree& tree, process_t p, process_t q) {

	QMap<function_t, QMap<function_t, int /*multiplicity*/>> pe, qe;
	QSet<function_t> pr, qr;

	traverseRoot(*tree.processes[p], &pe, &pr);
	traverseRoot(*tree.processes[q], &qe, &qr);

	// qDebug() << "a edges" << pe;
	// qDebug() << "b edges" << qe;

	int equal   = 0;
	int unequal = 0;

	equal   += (pr & qr).size();
	unequal += (pr - qr).size() + (qr - pr).size();

	QMapIterator<function_t, QMap<function_t, int>> i(pe);
	function_t from, to;
	while (i.hasNext()) {
		i.next();
		from = i.key();
		QMapIterator<function_t, int> j(i.value());
		while (j.hasNext()) {
			j.next();
			to = j.key();

			Q_ASSERT(pe[from][to] > 0);

			if (qe[from].contains(to)) {
				unequal += abs(pe[from][to] - qe[from][to]);
				equal   += pe[from][to] < qe[from][to] ? pe[from][to] : qe[from][to];
			} else {
				unequal += pe[from][to];
			}
		}
	}

	i = qe;
	while (i.hasNext()) {
		i.next();
		from = i.key();
		QMapIterator<function_t, int> j(i.value());
		while (j.hasNext()) {
			j.next();
			to = j.key();

			Q_ASSERT(qe[from][to] > 0);

			if (pe[from].contains(to) == false) {
				unequal += qe[from][to];
			}
		}
	}

	Q_ASSERT(unequal + equal < 500000000); // that would be unexpectedly large

	// qDebug() << QString("eq %1, uneq %2").arg(equal).arg(unequal);

	return (double) equal / (double) (unequal + equal);
}

double SimpleCallTreeSimilarityMetric::compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u) {

	CallTree at;
	CallTree bt;

	CallTree_fromProcessTrace(ap, (process_t) 1, u, a, false, &at);
	CallTree_finalize(&at);
	CallTree_fromProcessTrace(bp, (process_t) 2, u, b, false, &bt);
	CallTree_finalize(&bt);

	CallTree_merge(&bt, &at);
	CallTree_compress(&at);

	double sim = edgeSimilarity(at, (process_t) 1, (process_t) 2);

	CallTree_delete(&at);
	CallTree_delete(&bt);

	return sim;
}


QString SimpleCallTreeSimilarityMetric::dump(const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	Q_UNUSED(indent);

	return ret;
}
