#ifndef SIMPLE_CALL_MATRIX_SIMILARITY_METRIC_HPP
#define SIMPLE_CALL_MATRIX_SIMILARITY_METRIC_HPP

#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/similarity-metric/similarity-metric.hpp"
#include "common/unifier.hpp"

/* - compares two process trace only regarding if a function calls another or if not
 * - quantities of any sort are ignored
 * - comparison is done element wise for each pair of existing caller/callee
 *
 * - does not matter:
 *   - timing
 *   - reordering of function calls
 *   - different numbers of iterations
 * - matters:
 *   - inlining
 *   - structure / calling different functions
 */
class SimpleCallMatrixSimilarityMetric : public SimilarityMetric {
public:

	double compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u);

	QString dump(const QString& indent = "");

private:

	double similarity = -1;
	QSet<QPair<function_t, function_t>> both;
	QSet<QPair<function_t, function_t>> onlyA;
	QSet<QPair<function_t, function_t>> onlyB;
};

#endif /* SIMPLE_CALL_MATRIX_SIMILARITY_METRIC_HPP */
