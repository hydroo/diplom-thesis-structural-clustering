#include "common/prereqs.hpp"
#include "common/similarity-metric/invocation-count-sensitive-call-matrix-similarity-metric.hpp"
#include "common/similarity-metric/simple-call-matrix-similarity-metric.hpp"
#include "common/similarity-metric/simple-call-tree-similarity-metric.hpp"
#include "common/similarity-metric/simple-compressed-call-matrix-similarity-metric.hpp"
#include "common/similarity-metric/subsumption-compressed-call-matrix-similarity-metric.hpp"
#include "common/similarity-metric/subsumption-call-tree-similarity-metric.hpp"
#include "common/unifier.hpp"

int main(int argc, char** args) {
	Q_UNUSED(argc); Q_UNUSED(args);

	QList<QString> metricStrings;
	QList<SimilarityMetric*> metrics;

	metricStrings.append("SimpleCallMatrix");
	metrics.append(new SimpleCallMatrixSimilarityMetric);
	metricStrings.append("SimpleCompressedCallMatrix");
	metrics.append(new SimpleCompressedCallMatrixSimilarityMetric);
	metricStrings.append("SubsumptionCompressedCallMatrix");
	metrics.append(new SubsumptionCompressedCallMatrixSimilarityMetric);
	metricStrings.append("InvocationCountSensitiveCallMatrix");
	metrics.append(new InvocationCountSensitiveCallMatrixSimilarityMetric);
	metricStrings.append("SimpleCallTree");
	metrics.append(new SimpleCallTreeSimilarityMetric);
	metricStrings.append("SubsumptionCallTree");
	metrics.append(new SubsumptionCallTreeSimilarityMetric);

	QMap<QString, Trace> traces;
	Otf_trace(QFileInfo(args[0]).path() + "/a.otf", &traces[QFileInfo(args[0]).path() + "/a.otf"]);
	Otf_trace(QFileInfo(args[0]).path() + "/b.otf", &traces[QFileInfo(args[0]).path() + "/b.otf"]);
	Otf_trace(QFileInfo(args[0]).path() + "/c.otf", &traces[QFileInfo(args[0]).path() + "/c.otf"]);
	Otf_trace(QFileInfo(args[0]).path() + "/d.otf", &traces[QFileInfo(args[0]).path() + "/d.otf"]);
	Otf_trace(QFileInfo(args[0]).path() + "/e.otf", &traces[QFileInfo(args[0]).path() + "/e.otf"]);
	Otf_trace(QFileInfo(args[0]).path() + "/f.otf", &traces[QFileInfo(args[0]).path() + "/f.otf"]);
	Otf_trace(QFileInfo(args[0]).path() + "/g.otf", &traces[QFileInfo(args[0]).path() + "/g.otf"]);

	foreach (QString name, traces.keys()) {

		QMap<process_t, QMap<process_t, QMap<int, double>>> results;

		Unifier<function_t> unifier;
		QMap<process_t, QString>  pn;
		QMap<function_t, QString> fn;

		Otf_processNames(name, &pn);
		Otf_functionNames(name, &fn);

		unifier.insert((trace_t) 1, "", fn);
		fn = unifier.mappedNames();

		foreach (process_t p, pn.keys()) {
			foreach (process_t q, pn.keys()) {
				for (int i = 0; i < metrics.size(); i += 1) {

					double result = metrics[i]->compare((trace_t) 1, traces[name][p], (trace_t) 1, traces[name][q], unifier);

					if (metricStrings[i] == "SimpleCallMatrix" && p != q) {
						if        (name.contains("a.otf")) {
							assert(result == (double) 5 / (double) 7);
						} else if (name.contains("b.otf")) {
							assert(result == (double) 3 / (double) 4);
						} else if (name.contains("c.otf")) {
							assert(result == (double) 2 / (double) 3);
						} else if (name.contains("d.otf")) {
							assert(result == (double) 0 / (double) 3);
						} else if (name.contains("e.otf")) {
							assert(result == (double) 11 / (double) 13);
						}
					}

					// // results for the alternative (old) metric
					// if (metricStrings[i] == "SubsumptionCompressedCallMatrix" && p != q) {
					// 	if        (name.contains("a.otf")) {
					// 		     if (p == 1 && q == 2) assert(result == (double) 15 / (double) 15);
					// 		else if (p == 2 && q == 1) assert(result == (double) 10 / (double) 15);
					// 	} else if (name.contains("b.otf")) {
					// 		     if (p == 1 && q == 2) assert(result == (double)  6 / (double) 10);
					// 		else if (p == 2 && q == 1) assert(result == (double) 10 / (double) 10);
					// 	} else if (name.contains("c.otf")) {
					// 		     if (p == 1 && q == 2) assert(result == (double)  5 / (double)  5);
					// 		else if (p == 2 && q == 1) assert(result == (double)  3 / (double)  5);
					// 	} else if (name.contains("d.otf")) {
					// 		     if (p == 1 && q == 2) assert(result == (double)  3 / (double)  3);
					// 		else if (p == 2 && q == 1) assert(result == (double)  1 / (double)  3);
					// 	} else if (name.contains("e.otf")) {
					// 		     if (p == 1 && q == 2) assert(result == (double) 29 / (double) 33);
					// 		else if (p == 2 && q == 1) assert(result == (double) 29 / (double) 33);
					// 	}
					// }

					if (metricStrings[i] == "SubsumptionCompressedCallMatrix" && p != q) {
						if        (name.contains("a.otf")) {
							     if (p == 1 && q == 2) assert(result == (double) 10 / (double) 10);
							else if (p == 2 && q == 1) assert(result == (double) 10 / (double) 15);
						} else if (name.contains("b.otf")) {
							     if (p == 1 && q == 2) assert(result == (double)  6 / (double) 10);
							else if (p == 2 && q == 1) assert(result == (double)  6 / (double)  6);
						} else if (name.contains("c.otf")) {
							     if (p == 1 && q == 2) assert(result == (double)  3 / (double)  3);
							else if (p == 2 && q == 1) assert(result == (double)  3 / (double)  5);
						} else if (name.contains("d.otf")) {
							     if (p == 1 && q == 2) assert(result == (double)  1 / (double)  1);
							else if (p == 2 && q == 1) assert(result == (double)  1 / (double)  3);
						} else if (name.contains("e.otf")) {
							     if (p == 1 && q == 2) assert(result == (double) 25 / (double) 29);
							else if (p == 2 && q == 1) assert(result == (double) 25 / (double) 29);
						}
					}

					if (metricStrings[i] == "SimpleCallTree" && p != q) {
						if        (name.contains("a.otf")) {
							assert(result == (double)  5 / (double) 10);
						} else if (name.contains("b.otf")) {
							assert(result == (double)  3 / (double)  4);
						} else if (name.contains("c.otf")) {
							assert(result == (double)  2 / (double)  3);
						} else if (name.contains("d.otf")) {
							assert(result == (double)  0 / (double)  3);
						} else if (name.contains("e.otf")) {
							assert(result == (double) 11 / (double) 13);
						}
					}

					if (metricStrings[i] == "SubsumptionCallTree" && p != q) {
						if        (name.contains("a.otf")) {
							     if (p == 1 && q == 2) assert(result == (double) 31 / (double) 31);
							else if (p == 2 && q == 1) assert(result == (double) 13 / (double) 31);
						} else if (name.contains("b.otf")) {
							     if (p == 1 && q == 2) assert(result == (double)  6 / (double) 10);
							else if (p == 2 && q == 1) assert(result == (double) 10 / (double) 10);
						} else if (name.contains("c.otf")) {
							     if (p == 1 && q == 2) assert(result == (double)  5 / (double)  5);
							else if (p == 2 && q == 1) assert(result == (double)  3 / (double)  5);
						} else if (name.contains("d.otf")) {
							     if (p == 1 && q == 2) assert(result == (double)  3 / (double)  3);
							else if (p == 2 && q == 1) assert(result == (double)  1 / (double)  3);
						} else if (name.contains("e.otf")) {
							     if (p == 1 && q == 2) assert(result == (double) 31 / (double) 35);
							else if (p == 2 && q == 1) assert(result == (double) 31 / (double) 35);
						}
					}

					if (metricStrings[i] == "SimpleCallMatrix" || metricStrings[i] == "SimpleCompressedCallMatrix") {
						/* SimpleCallMatrix and SimpleCompressedCallMatrix are supposed to yield the same results */
						int o = -1;
						for (int j = 0; j < metricStrings.size(); j += 1) {
							if (metricStrings[i] == "SimpleCallMatrix" && metricStrings[j] == "SimpleCompressedCallMatrix") {
								o = j;
								break;
							} else if (metricStrings[i] == "SimpleCompressedCallMatrix" && metricStrings[j] == "SimpleCallMatrix") {
								o = j;
								break;
							}
						}
						assert(o > -1);

						if (results[p][q].contains(o)) {
							if (result != results[p][q][o]) {
								qout << QString("error: %1: %2: %3 compared to %4 is not the same as for %5. %6 != %7\n").arg(metricStrings[i]).arg(name).arg(pn[p]).arg(pn[q]).arg(metricStrings[o]).arg(result).arg(results[p][q][o]);
								exit(-1);
							}
						}
					}

					if (p == q) {
						/* check: sim(p,p) = 1 */
						if (result != 1) {
							qout << QString("error: %1: %2: %3 compared to itself is not 1, but %4.\n").arg(metricStrings[i]).arg(name).arg(pn[p]).arg(result);
							exit(-1);
						}
					} else if (metricStrings[i] != "SubsumptionCallTree" && metricStrings[i] != "SubsumptionCompressedCallMatrix" && results.contains(q) && results[q].contains(p) && results[q][p][i]) {
						/* check: sim(p,q) = sim(q,p) */
						if(result != results[q][p][i]) {
							qout << QString("error: %1: %2: %3 compared to %4 is %5, but %4 to %3 is %6.\n").arg(metricStrings[i]).arg(name).arg(pn[p]).arg(pn[q]).arg(result).arg(results[q][p][i]);
							exit(-1);
						}
					} else {
						results[p][q][i] = result;
					}
				}
			}
		}
	}

	/* clean up */

	for (int i = 0; i < metrics.size(); i += 1) {
		delete metrics[i];
	}

	return 0;
}

