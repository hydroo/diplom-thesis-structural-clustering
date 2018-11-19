#ifndef SIMPLE_COMPRESSED_CALL_MATRIX_SIMILARITY_METRIC_HPP
#define SIMPLE_COMPRESSED_CALL_MATRIX_SIMILARITY_METRIC_HPP

#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/similarity-metric/similarity-metric.hpp"
#include "common/unifier.hpp"

/* same as SimpleCallMatrixSimilarityMetric */
class SimpleCompressedCallMatrixSimilarityMetric : public SimilarityMetric {
public:

	double compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u);

	QString dump(const QString& indent = "");
};

#endif /* SIMPLE_COMPRESSED_CALL_MATRIX_SIMILARITY_METRIC_HPP */

