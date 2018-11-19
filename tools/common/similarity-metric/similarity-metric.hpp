#ifndef SIMILARITY_METRIC_HPP
#define SIMILARITY_METRIC_HPP

#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/unifier.hpp"

class SimilarityMetric {
public:

	virtual ~SimilarityMetric() {}

	/* returns number between 0 and 1 */
	virtual double compare(trace_t a, const ProcessTrace& ap, trace_t b, const ProcessTrace& bp, const Unifier<function_t>& u) = 0;

	virtual QString dump(const QString& indent = "");
};

#endif /* SIMILARITY_METRIC_HPP */
