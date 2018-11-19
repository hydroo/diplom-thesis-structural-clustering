#include "common/similarity-metric/subsumption-call-tree-similarity-metric.hpp"

#include "call-tree/call-tree.hpp"

static void traverseNodes(QStack<const CallTreeNode*>* s, bool ignoreMultiplicity, QMap<function_t, QMap<function_t, int>>* edges, QMap<function_t, int>* rootEdges) {
	foreach (const CallTreeNode* m, s->top()->children) {
		if (rootEdges->contains(m->id) == false) {
			(*rootEdges)[m->id]  = 1;
		} else {
			if (ignoreMultiplicity == false) (*rootEdges)[m->id] += 1;
		}
		for (int i = 1; i < s->size(); i += 1) {
			if ((*edges)[(*s)[i]->id].contains(m->id) == false) {
				(*edges)[(*s)[i]->id][m->id]  = 1;
			} else {
				if (ignoreMultiplicity == false) (*edges)[(*s)[i]->id][m->id] += 1;
			}
		}
		s->push(m);
		traverseNodes(s, ignoreMultiplicity, edges, rootEdges);
		s->pop();
	}
}

static void traverseRoot(QStack<const CallTreeNode*>* s, bool ignoreMultiplicity, QMap<function_t, QMap<function_t, int>>* edges, QMap<function_t, int>* rootEdges) {
	foreach (const CallTreeNode* m, s->top()->children) {
		if (rootEdges->contains(m->id) == false) {
			(*rootEdges)[m->id]  = 1;
		} else {
			if (ignoreMultiplicity == false) (*rootEdges)[m->id] += 1;
		}
		s->push(m);
		traverseNodes(s, ignoreMultiplicity, edges, rootEdges);
		s->pop();
	}
}

static double edgeSimilarity(const CallTree& tree, process_t p, process_t q, bool ignoreMultiplicity) {

	QMap<function_t, QMap<function_t, int /*multiplicity*/>> pe, qe;
	QMap<function_t, int> pr, qr;

	QStack<const CallTreeNode*> stack;

	stack.push(tree.processes[p]);
	traverseRoot(&stack, ignoreMultiplicity, &pe, &pr);

	stack.clear();
	stack.push(tree.processes[q]);
	traverseRoot(&stack, ignoreMultiplicity, &qe, &qr);

	// qDebug() << "pr" << pr << "pe" << pe;
	// qDebug() << "qr" << qr << "qe" << qe;

	int equal   = 0;
	int unequal = 0;

	function_t from, to;

	QMapIterator<function_t, int> i(pr);
	while (i.hasNext()) {
		i.next();
		to = i.key();
		Q_ASSERT(pr[to] > 0);
		if (qr.contains(to)) {
			if (pr[to] >= qr[to]) {
				equal   += pr[to];
				unequal += 0;
			} else {
				equal   += pr[to];
				unequal += qr[to] - pr[to];
				// qDebug() << QString("uneq1: root -> %1: p1 = %2, p2 = %3").arg(to).arg(pr[to]).arg(qr[to]);
			}
		} else {
			equal   += pr[to];
			unequal += 0;
		}
	}

	i = qr;
	while (i.hasNext()) {
		i.next();
		to = i.key();
		Q_ASSERT(qr[to] > 0);
		if (pr.contains(to) == false) {
			unequal += qr[to];
			// qDebug() << QString("uneq2: root -> %1: p1 = %2, p2 = %3").arg(to).arg(0).arg(qr[to]);
		}
	}

	QMapIterator<function_t, QMap<function_t, int>> j(pe);
	while (j.hasNext()) {
		j.next();
		from = j.key();
		QMapIterator<function_t, int> k(j.value());
		while (k.hasNext()) {
			k.next();
			to = k.key();

			Q_ASSERT(pe[from][to] > 0);

			if (qe[from].contains(to)) {
				if (pe[from][to] >= qe[from][to]) {
					equal   += pe[from][to];
					unequal += 0;
				} else {
					equal   += pe[from][to];
					unequal += qe[from][to] - pe[from][to];
					// qDebug() << QString("uneq3: %1 -> %2: p1 = %3, p2 = %4").arg(from).arg(to).arg(pe[from][to]).arg(qe[from][to]);
				}
			} else {
				equal += pe[from][to];
			}
		}
	}

	j = qe;
	while (j.hasNext()) {
		j.next();
		from = j.key();
		QMapIterator<function_t, int> k(j.value());
		while (k.hasNext()) {
			k.next();
			to = k.key();

			Q_ASSERT(qe[from][to] > 0);

			if (pe[from].contains(to) == false) {
				unequal += qe[from][to];
				// qDebug() << QString("uneq4: %1 -> %2: p1 = %3, p2 = %4").arg(from).arg(to).arg(0).arg(qe[from][to]);
			}
		}
	}

	Q_ASSERT(unequal + equal < 500000000); // that would be unexpectedly large

	// qDebug() << "eq" << equal << "uneq" << unequal << "quot" << (double) equal / (double) (unequal + equal);

	return (double) equal / (double) (unequal + equal);
}

double SubsumptionCallTreeSimilarityMetric::compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u) {

	CallTree at;
	CallTree bt;

	CallTree_fromProcessTrace(ap, (process_t) 1, u, a, false, &at);
	CallTree_finalize(&at);
	CallTree_fromProcessTrace(bp, (process_t) 2, u, b, false, &bt);
	CallTree_finalize(&bt);

	CallTree_merge(&bt, &at);
	CallTree_compress(&at);

	double sim = edgeSimilarity(at, (process_t) 1, (process_t) 2, false);

	CallTree_delete(&at);
	CallTree_delete(&bt);

	return sim;
}


QString SubsumptionCallTreeSimilarityMetric::dump(const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	Q_UNUSED(indent);

	return ret;
}
