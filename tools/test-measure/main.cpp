#include "common/measure.hpp"
#include "common/prereqs.hpp"

int main(int argc, char** args) {
	Q_UNUSED(argc); Q_UNUSED(args);

	QList<int64_t> l({1, 2, 3, 4});
	QList<double>  m({1, 2, 3, 4});

	Measure<int64_t> i1, i2;
	Measure<double>  d1, d2;

	Measure_init(&i1);
	foreach(int64_t i, l) { Measure_record(&i1, i); }
	Measure_finalize(&i1);

	Measure_init(&d1);
	foreach(double i, m) { Measure_record(&d1, i); }
	Measure_finalize(&d1);

	Measure_init(&i2);
	foreach(int64_t i, l) { Measure_record(&i2, i); }
	Measure_finalize(&i2);

	Measure_init(&d2);
	foreach(double i, m) { Measure_record(&d2, i); }
	Measure_finalize(&d2);

	assert(i1.accumulated       == 10     );
	assert(i1.min               ==  1     );
	assert(i1.max               ==  4     );
	assert(i1.mean              ==  2.5   );
	assert(i1.dataPointCount    ==  4     );

	assert(i2.accumulated       == 10     );
	assert(i2.min               ==  1     );
	assert(i2.max               ==  4     );
	assert(i2.mean              ==  2.5   );
	assert(i2.dataPointCount    ==  4     );

	assert(d1.accumulated       == 10     );
	assert(d1.min               ==  1     );
	assert(d1.max               ==  4     );
	assert(d1.mean              ==  2.5   );
	assert(d1.dataPointCount    ==  4     );

	assert(d2.accumulated       == 10     );
	assert(d2.min               ==  1     );
	assert(d2.max               ==  4     );
	assert(d2.mean              ==  2.5   );
	assert(d2.dataPointCount    ==  4     );

	return 0;
}

