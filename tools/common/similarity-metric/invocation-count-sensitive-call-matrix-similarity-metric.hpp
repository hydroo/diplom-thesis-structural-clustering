#ifndef INVOCATION_COUNT_SENSITIVE_CALL_MATRIX_SIMILARITY_METRIC_HPP
#define INVOCATION_COUNT_SENSITIVE_CALL_MATRIX_SIMILARITY_METRIC_HPP

#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/similarity-metric/similarity-metric.hpp"
#include "common/unifier.hpp"

/* - compares two process trace only regarding how often a function calls another
 * - comparison is done element wise for each pair of existing caller/callee
 *
 * - does not matter:
 *   - timing
 *   - reordering of function calls
 * - matters:
 *   - different numbers of iterations
 *   - inlining
 *   - structure / calling different functions
 */
class InvocationCountSensitiveCallMatrixSimilarityMetric : public SimilarityMetric {
public:

	double compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u);

	QString dump(const QString& indent = "");

private:

	double similarity = -1;
	QMap<function_t, QMap<function_t, double>> both;
	QSet<QPair<function_t, function_t>> onlyA;
	QSet<QPair<function_t, function_t>> onlyB;
};

#endif /* INVOCATION_COUNT_SENSITIVE_CALL_MATRIX_SIMILARITY_METRIC_HPP */
