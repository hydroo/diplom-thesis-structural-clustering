#ifndef DIFFERENCE_HPP
#define DIFFERENCE_HPP

#include "common/measure.hpp"
#include "common/prereqs.hpp"

///* *** basic types *** */
//int32_t difference(int32_t a, int32_t b);
//int64_t difference(int64_t a, int64_t b);
//
///* *** sets of basic types *** */
//template<typename T>
//struct SetDifference2 {
//	QSet<T> both;
//	QSet<T> onlyA;
//	QSet<T> onlyB;
//};
//
//template<typename T>
//QString SetDifference2_print(const SetDifference2<T>& d, const QString& indent = "") {
//	QString ret;
//	QTextStream s(&ret);
//
//	s << indent << "both  " << d.both << "\n";
//	s << indent << "onlyA " << d.onlyA << "\n";
//	s << indent << "onlyB " << d.onlyB << "\n";
//
//	return ret;
//}
//
//template<typename T>
//SetDifference2<T> difference(const QSet<T>& a, const QSet<T>& b) {
//	SetDifference2<T> ret;
//	ret.both  = a; ret.both.intersect(b);
//	ret.onlyA = a; ret.onlyA.subtract(b);
//	ret.onlyB = b; ret.onlyB.subtract(a);
//	return ret;
//}
//
///* *** measures of basic types *** */
//template<typename T>
//Measure<T> difference(const Measure<T>& a, const Measure<T>& b) {
//	Measure<T> ret;
//	ret.accumulated            = std::abs(a.accumulated            - b.accumulated           );
//
//	ret.secondPercentile       = std::abs(a.secondPercentile       - b.secondPercentile      );
//	ret.firstQuartile          = std::abs(a.firstQuartile          - b.firstQuartile         );
//	ret.median                 = std::abs(a.median                 - b.median                );
//	ret.thirdQuartile          = std::abs(a.thirdQuartile          - b.thirdQuartile         );
//	ret.ninetyEighthPercentile = std::abs(a.ninetyEighthPercentile - b.ninetyEighthPercentile);
//
//	ret.min                    = std::abs(a.min                    - b.min                   );
//	ret.max                    = std::abs(a.max                    - b.max                   );
//	ret.mean                   = std::abs(a.mean                   - b.mean                  );
//
//	ret.dataPointCount         = std::abs(a.dataPointCount         - b.dataPointCount        );
//	return ret;
//}

#endif /* DIFFERENCE_HPP */
