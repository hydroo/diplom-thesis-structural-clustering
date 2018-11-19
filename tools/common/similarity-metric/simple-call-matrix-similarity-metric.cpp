#include "common/similarity-metric/simple-call-matrix-similarity-metric.hpp"

#include "call-matrix/call-matrix.hpp"

double SimpleCallMatrixSimilarityMetric::compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u) {

	this->both.clear();
	this->onlyA.clear();
	this->onlyB.clear();

	CallMatrix am, bm;

	CallMatrix_fromProcessTrace(ap, u, a, false, &am);
	CallMatrix_finalize(&am);
	CallMatrix_fromProcessTrace(bp, u, b, false, &bm);
	CallMatrix_finalize(&bm);

	QSet<QPair<function_t, function_t>> alreadyVisited;

	int64_t sum = 0;
	int64_t n   = 0;

	foreach (function_t f, am.keys()) {
		foreach (function_t g, am[f].keys()) {
			if (bm.contains(f)) {
				if (bm[f].contains(g)) {
					sum += 1;
					this->both.insert(QPair<function_t, function_t>(f, g));
				} else {
					this->onlyA.insert(QPair<function_t, function_t>(f, g));
				}
			} else {
				this->onlyA.insert(QPair<function_t, function_t>(f, g));
			}

			alreadyVisited.insert(QPair<function_t, function_t>(f, g));
			n += 1;
		}
	}

	foreach (function_t f, bm.keys()) {
		foreach (function_t g, bm[f].keys()) {

			if(alreadyVisited.contains(QPair<function_t, function_t>(f, g)) == false) {
				this->onlyB.insert(QPair<function_t, function_t>(f, g));
				n += 1;
			}
		}
	}

	this->similarity = (double) sum / ((double) (n));

	return this->similarity;
}


QString SimpleCallMatrixSimilarityMetric::dump(const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	s << QString(" similarity: %1\n").arg(this->similarity);
	s << indent << QString("both: (%1 elements) { ").arg(this->both.size());
	foreach (auto p, this->both) {
		s << QString("(%1, %2), ").arg(p.first).arg(p.second);
	}
	s << " }\n";

	s << indent << QString("only a: (%1 elements) { ").arg(this->onlyA.size());
	foreach (auto p, this->onlyA) {
		s << QString("(%1, %2), ").arg(p.first).arg(p.second);
	}
	s << " }\n";

	s << indent << QString("only b: (%1 elements) { ").arg(this->onlyB.size());
	foreach (auto p, this->onlyB) {
		s << QString("(%1, %2), ").arg(p.first).arg(p.second);
	}
	s << " }\n";

	return ret;
}
