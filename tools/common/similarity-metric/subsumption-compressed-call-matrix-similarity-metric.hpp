#ifndef SUBSUMPTION_COMPRESSED_CALL_MATRIX_SIMILARITY_METRIC_HPP
#define SUBSUMPTION_COMPRESSED_CALL_MATRIX_SIMILARITY_METRIC_HPP

#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/similarity-metric/similarity-metric.hpp"
#include "common/unifier.hpp"

/*
 * tests whether the call matrix of trace a subsumes the call matrix of trace b
 *
 * - compares two process traces only regarding the call matrix structure
 * - quantities of any sort are ignored
 *
 * - is not commutative (comparing a to b does not yield the same result as comparing b to a)
 *
 * - does not matter:
 *   - timing
 *   - reordering of function calls
 *   - different numbers of iterations
 *   - inlining
 * - matters:
 *   - structure / calling different functions
 *
 *
 * - same as SubsumptionCallTreeSimilarityMetric except without so called multiplicities
 */
class SubsumptionCompressedCallMatrixSimilarityMetric : public SimilarityMetric {
public:

	double compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u);

	QString dump(const QString& indent = "");
};

#endif /* SUBSUMPTION_COMPRESSED_CALL_MATRIX_SIMILARITY_METRIC_HPP */


