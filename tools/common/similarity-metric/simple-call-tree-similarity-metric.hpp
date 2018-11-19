#ifndef SIMPLE_CALL_TREE_SIMILARITY_METRIC_HPP
#define SIMPLE_CALL_TREE_SIMILARITY_METRIC_HPP

#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/similarity-metric/similarity-metric.hpp"
#include "common/unifier.hpp"

/* - compares two process traces only regarding the call tree structure
 * - quantities of any sort are ignored
 *
 * - does not matter:
 *   - timing
 *   - reordering of function calls
 *   - different numbers of iterations
 * - matters:
 *   - inlining
 *   - structure / calling different functions
 *
 * possible performance improvements:
 *
 *  - add multiplicity to whole sub trees if forward edges are detected
 *  - avoid traversing the uncompressed tree
 */
class SimpleCallTreeSimilarityMetric : public SimilarityMetric {
public:

	double compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u);

	QString dump(const QString& indent = "");
};

#endif /* SIMPLE_CALL_TREE_SIMILARITY_METRIC_HPP */
