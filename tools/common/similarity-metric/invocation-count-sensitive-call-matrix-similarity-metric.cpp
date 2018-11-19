#include "common/similarity-metric/invocation-count-sensitive-call-matrix-similarity-metric.hpp"

#include "call-matrix/call-matrix.hpp"

double InvocationCountSensitiveCallMatrixSimilarityMetric::compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u) {

	this->both.clear();
	this->onlyA.clear();
	this->onlyB.clear();

	CallMatrix am, bm;

	CallMatrix_fromProcessTrace(ap, u, a, true, &am);
	CallMatrix_finalize(&am);
	CallMatrix_fromProcessTrace(bp, u, b, true, &bm);
	CallMatrix_finalize(&bm);

	QSet<QPair<function_t, function_t>> alreadyVisited;

	double sum = 0;
	int64_t n  = 0;

	foreach (function_t f, am.keys()) {
		foreach (function_t g, am[f].keys()) {

			assert(am[f][g].statistics->invocationCount > 0); // otherwise there should be no entry

			if (bm.contains(f)) {
				if (bm[f].contains(g)) {
					assert(bm[f][g].statistics->invocationCount > 0);
					double ai = (double) am[f][g].statistics->invocationCount;
					double bi = (double) bm[f][g].statistics->invocationCount;
					double ratio = ai < bi ? ai / bi : bi / ai;
					sum += ratio;
					this->both[f][g] = ratio;
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

			assert(bm[f][g].statistics->invocationCount > 0); // otherwise there should be no entry

			if(alreadyVisited.contains(QPair<function_t, function_t>(f, g)) == false) {
				this->onlyB.insert(QPair<function_t, function_t>(f, g));
				n += 1;
			}
		}
	}

	this->similarity = sum / ((double) (n));

	return this->similarity;
}

QString InvocationCountSensitiveCallMatrixSimilarityMetric::dump(const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	s << QString(" similarity: %1\n").arg(this->similarity);
	s << indent << QString("both: (%1 elements) { ").arg(this->both.size());
	foreach (auto p, this->both.keys()) {
		foreach (auto q, this->both[p].keys()) {
			s << QString("(%1, %2 : %3), ").arg(p).arg(q).arg(this->both[p][q], 0, 'f', 2);
		}
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
