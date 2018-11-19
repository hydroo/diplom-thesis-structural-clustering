#include "call-matrix/call-matrix.hpp"
#include "common/prereqs.hpp"
#include "common/similarity-metric/invocation-count-sensitive-call-matrix-similarity-metric.hpp"
#include "common/similarity-metric/simple-call-matrix-similarity-metric.hpp"
#include "common/similarity-metric/simple-call-tree-similarity-metric.hpp"
#include "common/similarity-metric/simple-compressed-call-matrix-similarity-metric.hpp"
#include "common/similarity-metric/subsumption-compressed-call-matrix-similarity-metric.hpp"
#include "common/similarity-metric/subsumption-call-tree-similarity-metric.hpp"
#include "common/unifier.hpp"

#define HELP_TEXT \
	"call-similarity [options] <tracefilename> [processes-to-be-compared]\n" \
	"\n" \
	"\tprocesses are space separated and can be singular or a range in the form of 1-5\n" \
	"\n" \
	"\toptions:\n" \
	"\t\t-p       list processes\n" \
	"\t\t-f                          list functions\n" \
	"\n" \
	"\t\t-scm     simple call matrix metric\n" \
	"\t\t-sccm    simple compressed call matrix metric (same as -scm)\n" \
	"\t\t-uccm    subsumption compressed call matrix metric\n" \
	"\t\t-icscm   invocation count sensitive call matrix metric\n" \
	"\t\t-sct     simple call tree metric\n" \
	"\t\t-uct     subsumption call tree metric\n"

int main(int argc, char** args) {

	QList<QString> metricStrings;
	QList<SimilarityMetric*> metrics;

	bool listProcesses = false;
	bool listFunctions = false;
	QString traceFileName;
	QList<process_t> processesToCompare;

	/* evaluate arguments */
	for (int i = 1; i < argc; i += 1) {
		QString arg(args[i]);
		if (arg.startsWith("-")) {
			if (arg == "--help" || arg == "-h") {
				qout << HELP_TEXT;
				exit(0);
			} else if (arg == "-p") {
				listProcesses = true;
			} else if (arg == "-f") {
				listFunctions = true;
			} else if (arg == "-scm") {
				if (metricStrings.toSet().contains("scm") == false) {
					metricStrings.append("scm");
					metrics.append(new SimpleCallMatrixSimilarityMetric);
				}
			} else if (arg == "-sccm") {
				if (metricStrings.toSet().contains("sccm") == false) {
					metricStrings.append("sccm");
					metrics.append(new SimpleCompressedCallMatrixSimilarityMetric);
				}
			} else if (arg == "-uccm") {
				if (metricStrings.toSet().contains("uccm") == false) {
					metricStrings.append("uccm");
					metrics.append(new SubsumptionCompressedCallMatrixSimilarityMetric);
				}
			} else if (arg == "-icscm") {
				if (metricStrings.toSet().contains("icscm") == false) {
					metricStrings.append("icscm");
					metrics.append(new InvocationCountSensitiveCallMatrixSimilarityMetric);
				}
			} else if (arg == "-sct") {
				if (metricStrings.toSet().contains("sct") == false) {
					metricStrings.append("sct");
					metrics.append(new SimpleCallTreeSimilarityMetric);
				}
			} else if (arg == "-uct") {
				if (metricStrings.toSet().contains("uct") == false) {
					metricStrings.append("uct");
					metrics.append(new SubsumptionCallTreeSimilarityMetric);
				}
			} else {
				qout << "unknown parameter \"" << arg << "\". aborting.\n";
				exit(0);
			}
		} else {
			if (traceFileName.isEmpty()) {
				traceFileName = arg;
			} else {

				bool ok;

				process_t p = (process_t) arg.toULongLong(&ok);

				if (ok == true) {

					foreach (process_t q, processesToCompare) {
						if (q == p) {
							qout << "process " << q << " has already been included. aborting.\n";
							exit(0);
						}
					}

					processesToCompare.append(p);

				} else {

					if (arg.count('-') != 1) {
						qout << arg << " is not an integer, but is supposed to be a process identifier. aborting.\n";
						exit(0);
					} else {
						bool ok2, ok3;
						process_t from = arg.split('-')[0].toULongLong(&ok2);
						process_t to   = arg.split('-')[1].toULongLong(&ok3);

						if (ok2 == false || ok3 == false) {
							qout << arg << " is not a valid range of process identifers. aborting.\n";
							exit(0);
						}

						foreach (process_t q, processesToCompare) {
							if (q >= from && q <= to) {
								qout << "process " << q << " has already been included. aborting.\n";
								exit(0);
							}
						}

						for (process_t q = from; q <= to; q += 1) {
							processesToCompare.append(q);
						}
					}
				}
			}
		}
	}

	if (traceFileName.isEmpty()) {
		qout << HELP_TEXT;
		exit(0);
	}

	auto allProcesses = Otf_processes(traceFileName).toList();
	std::sort(allProcesses.begin(), allProcesses.end());

	foreach (process_t p, processesToCompare) {
		if (allProcesses.contains(p) == false) {
			qout << "process " << p << " is not contained in the trace. aborting.\n";
			exit(0);
		}
	}

	QMap<process_t, QString> processNames;
	QMap<function_t, QString> functionNames;

	Otf_processNames(traceFileName, &processNames);
	Otf_functionNames(traceFileName, &functionNames);

	if (listProcesses == true) {
		foreach (process_t p, allProcesses) {
			qout << QString("%1: %2\n").arg(p).arg(processNames[p]);
		}
		exit(0);
	}

	Unifier<function_t> unifier;
	unifier.insert((trace_t) 1, traceFileName, functionNames);
	functionNames = unifier.mappedNames();

	if (listFunctions == true) {
		auto sortedFunctionNames = functionNames.keys();
		std::sort(sortedFunctionNames.begin(), sortedFunctionNames.end());

		int maxIdentifierLength = 0;
		foreach (function_t f, sortedFunctionNames) {
			if (f == 0) {
				continue;
			} else if (f < 0) {
				if (floor(log10(-f)) + 2 > maxIdentifierLength) {
					maxIdentifierLength = floor(log10(-f)) + 2;
				}
			} else {
				if (floor(log10(f)) + 1 > maxIdentifierLength) {
					maxIdentifierLength = floor(log10(f)) + 1;
				}
			}
		}

		foreach (function_t f, sortedFunctionNames) {
			qout << QString("%1: %2\n").arg(f, maxIdentifierLength).arg(functionNames[f]);
		}
		exit(0);
	}

	if (metrics.isEmpty()) {
		qout << "at least one metrics needs to be specified. aborting.\n";
		exit(0);
	}

	if (processesToCompare.size() == 1) {
		qout << "you should at least compare two different processes. aborting.\n";
		exit(0);
	}

	if (processesToCompare.size() == 0) {
		processesToCompare = allProcesses;
	}

	int maxIdentifierLength = 0;
	int maxNameLength       = 0;
	foreach (process_t p, processesToCompare) {
		if (QString("%1").arg(p).length() > maxIdentifierLength) {
			maxIdentifierLength = QString("%1").arg(p).length();
		}
		if (processNames[p].length() > maxNameLength) {
			maxNameLength = processNames[p].length();
		}
	}

	QMap<process_t, ProcessTrace> m;

	QMap<process_t, QMap<process_t, QMap<int, double>>> results;

	foreach (process_t p, processesToCompare) {
		QString pn = processNames[p];

		if (m.contains(p) == false) {
			Otf_processTrace(traceFileName, p, &m[p]);
		}

		foreach (process_t q, processesToCompare) {
			QString qn = processNames[q];

			if (m.contains(q) == false) {
				Otf_processTrace(traceFileName, q, &m[q]);
			}

			QString metricsString;
			for(int i = 0; i < metrics.size(); i += 1) {
				double result = metrics[i]->compare((trace_t) 1, m[p], (trace_t) 1, m[q], unifier);
				metricsString += QString("%1: %2, ").arg(metricStrings[i]).arg(result, 0, 'f', 5);

				if (p == q) {
					if (result != 1) {
						qout << QString("error: %1, \"%2\" compared to itself is not 1, but %3.\n").arg(p).arg(pn).arg(result);
					}
				} else if (metricStrings[i] != "uct" && metricStrings[i] != "uccm" &&  results.contains(q) && results[q].contains(p) && results[q][p][i]) {
					if(result != results[q][p][i]) {
						qout << QString("error: %1, \"%2\" compared to %3, \"%4\" is %5, but %3 to %1 is %6.\n").arg(p).arg(pn).arg(q).arg(qn).arg(result).arg(results[q][p][i]);
					}
				} else {
					results[p][q][i] = result;
					// qDebug() << metrics[i]->dump();
				}
			}

			qout << QString("%1: \"%2\" =/= %3: \"%4\" -> %5\n").arg(p, maxIdentifierLength).arg(pn, maxNameLength).arg(q, maxIdentifierLength).arg(qn, maxNameLength).arg(metricsString);
			qout.flush();

		}
	}

	for (int i = 0; i < metrics.size(); i += 1) {
		delete metrics[i];
	}

	return 0;
}
