#include "call-list/call-list-difference.hpp"

//QString CallListDifference_print(const CallListDifference& l, const QMap<function_t, QString>& functionNames, const QString& indent) {
//	QString ret;
//	QTextStream s(&ret);
//
//	s << indent << "both {\n";
//
//	QMapIterator<function_t, CallListDifferenceEntry> i(l.both);
//	while (i.hasNext()) {
//		i.next();
//		function_t f = i.key();
//		const CallListDifferenceEntry& e = i.value();
//
//		s << indent + "\t" << f << " " << functionNames[f] << " {\n";
//
//		s << indent + "\t" + "\t" << "calls:\n";
//		s << SetDifference2_print(e.calls   , indent + "\t\t\t") << "\n";
//		s << indent + "\t" + "\t" << "calledBy:\n";
//		s << SetDifference2_print(e.calledBy, indent + "\t\t\t") << "\n";
//
//		s << indent + "\t" + "\t" << "ex: " << Measure_print(e.exclusiveTime) << "\n";
//		s << indent + "\t" + "\t" << "in: " << Measure_print(e.inclusiveTime) << "\n";
//
//		s << indent + "\t" << "}\n";
//	}
//	s << indent << "}\n";
//
//	s << indent << "onlyA {\n";
//	s << CallList_print(l.onlyA, functionNames, indent + "\t") << "\n";
//	s << indent << "}\n";
//
//	s << indent << "onlyB {\n";
//	s << CallList_print(l.onlyB, functionNames, indent + "\t") << "\n";
//	s << indent << "}\n";
//
//	return ret;
//}
//
//void CallListDifference_fromCallLists(const CallList& a, const CallList& b, CallListDifference *d) {
//
//	QSet<function_t> af = QSet<function_t>::fromList(a.keys());
//	QSet<function_t> bf = QSet<function_t>::fromList(b.keys());
//
//	QSet<function_t> both  = af; both.intersect(bf);
//	QSet<function_t> onlyA = af; onlyA.subtract(bf);
//	QSet<function_t> onlyB = bf; onlyB.subtract(af);
//
//	foreach(function_t f, both) {
//		CallListDifferenceEntry e;
//		e.calls         = difference(a[f].calls        , b[f].calls        );
//		e.calledBy      = difference(a[f].calledBy     , b[f].calledBy     );
//		e.exclusiveTime = difference(a[f].exclusiveTime, b[f].exclusiveTime);
//		e.inclusiveTime = difference(a[f].inclusiveTime, b[f].inclusiveTime);
//		d->both.insert(f, e);
//	}
//
//	foreach(function_t f, onlyA) { d->onlyA.insert(f, a[f]); }
//	foreach(function_t f, onlyB) { d->onlyB.insert(f, b[f]); }
//}
