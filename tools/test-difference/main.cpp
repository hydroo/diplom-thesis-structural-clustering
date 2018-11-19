#include "common/difference.hpp"
#include "common/prereqs.hpp"

int main(int argc, char** args) {
	Q_UNUSED(argc); Q_UNUSED(args);

	// /* *** basic types *** */
	// assert(difference((int32_t) 1, (int32_t) 2) == 1); assert(1 == difference((int32_t) 2, (int32_t) 1));
	// assert(difference((int64_t) 1, (int64_t) 2) == 1); assert(1 == difference((int64_t) 2, (int64_t) 1));

	// /* *** sets of basic types *** */
	// {
	// 	QSet<int32_t>     a({1, 2, 3});
	// 	QSet<int32_t>     b({1, 2});
	// 	QSet<int32_t> onlyA({      3});

	// 	assert(difference(a, b).both  == b              );
	// 	assert(difference(a, b).onlyA == onlyA          );
	// 	assert(difference(a, b).onlyB == QSet<int32_t>());

	// 	assert(difference(b, a).both  == b              );
	// 	assert(difference(b, a).onlyA == QSet<int32_t>());
	// 	assert(difference(b, a).onlyB == onlyA          );
	// }

	// /* *** measures of basic types *** */
	// {
	// 	Measure<int32_t> m; Measure_init(&m); Measure_record(&m, 1); Measure_record(&m, 2); Measure_record(&m, 3); Measure_finalize(&m);
	// 	Measure<int32_t> n; Measure_init(&n); Measure_record(&n, 2); Measure_record(&n, 3); Measure_record(&n, 4); Measure_finalize(&n);
	// 	auto d = difference(m, n);
	// 	auto e = difference(n, m);

	// 	assert(d.accumulated       == e.accumulated      );
	// 	assert(d.min               == e.min              );
	// 	assert(d.max               == e.max              );
	// 	assert(d.mean              == e.mean             );
	// 	assert(d.standardDeviation == e.standardDeviation);
	// 	assert(d.dataPointCount    == e.dataPointCount   );

	// 	assert(d.accumulated       == llabs(6 - 9));
	// 	assert(d.min               == llabs(1 - 2));
	// 	assert(d.max               == llabs(3 - 4));
	// 	assert(d.mean              ==  fabs(2 - 3));
	// 	assert(d.standardDeviation == 0           );
	// 	assert(d.dataPointCount    == 0           );
	// }
	return 0;
}

