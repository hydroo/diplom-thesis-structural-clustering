#include "common/similarity-metric/simple-compressed-call-matrix-similarity-metric.hpp"

#include "call-matrix/compressed-call-matrix.hpp"

typedef CompressedCallMatrixCell Cell;

double SimpleCompressedCallMatrixSimilarityMetric::compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u) {

	CallMatrix am, bm;
	CallMatrix_fromProcessTrace(ap, u, a, false, &am);
	CallMatrix_finalize(&am);
	CallMatrix_fromProcessTrace(bp, u, b, false, &bm);
	CallMatrix_finalize(&bm);

	CompressedCallMatrix m;
	CompressedCallMatrix_merge(am, (process_t) 1, &m);
	CompressedCallMatrix_merge(bm, (process_t) 2, &m);
	CompressedCallMatrix_finalize(&m);

	double sim = CompressedCallMatrix_simpleSimilarity(m.objectToAttributes[(process_t) 1], m.objectToAttributes[(process_t) 2], m);

	CompressedCallMatrix_delete(&m);

	return sim;
}

QString SimpleCompressedCallMatrixSimilarityMetric::dump(const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	Q_UNUSED(indent);

	return ret;
}
