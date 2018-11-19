#include <QApplication>

#include "common/prereqs.hpp"
#include "common/timer.hpp"
#include "common/viewer.hpp"
#include "call-list/call-list.hpp"
#include "call-list/call-list-difference.hpp"
#include "call-list/compressed-call-list.hpp"

#define HELP_TEXT \
	"call-list [options] <tracefilename> [process-to-be-compared]\n" \
	"\n" \
	"\tprocesses are space separated and can be singular or a range in the form of 1-5\n" \
	"\n" \
	"\toptions:\n" \
	"\t\t-p                          list processes\n" \
	"\t\t-f                          list functions\n" \
	"\t\t-%                          show loading progress\n" \
	"\t\t-%2                         show loading stats. shorter output than -%\n" \
	"\n" \
	"\t\t--console raw console output [default]\n" \
	"\t\t--vis     gui visualization\n" \
	"\t\t--group <min-similarity>    prints a list of groups of similar processes. similarity ranges between 0.0 and 1.0\n" \
	"\t\t--simple-similarity         prints the compressed simple similarity matrix\n" \
	"\t\t--subsumption-similarity    prints the compressed subsumption similarity matrix\n" \
	"\t\t--compressed-dot            prints the compressed call list as graphviz dot format\n" \
	"\t\t--compressed-cxt            prints the compressed call list in the cxt format\n" \
	"\t\t--compressed-txt            prints the compressed call list\n" \
	"\t\t--compressed-txt2           prints the compressed call list, prints function names instead of identifiers\n"

int main(int argc, char** args) {

	QApplication app(argc, args);

	init();

	bool listProcesses = false;
	bool listFunctions = false;
	bool computeTiming = false;
	bool showProgress = false;
	bool console = false;
	bool vis = false;
	bool group = false;
	double minimumSimilarity = -1.0;
	bool simpleSimilarity = false;
	bool subsumptionSimilarity = false;
	bool compressedDot = false;
	bool compressedCxt = false;
	bool compressedTxt = false;
	bool compressedTxt2 = false;
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
			} else if (arg == "-%") {
				computeTiming = true;
				showProgress = true;
			} else if (arg == "-%2") {
				computeTiming = true;
				showProgress = false;
			} else if (arg == "--console") {
				console = true;
			} else if (arg == "--vis") {
				vis = true;
			} else if (arg == "--group") {
				group = true;
				if ((i+1 < argc) == false) {
					qout << "you need to specify a minimum similarity for --group. aborting.\n";
					exit(0);
				}
				bool ok;
				minimumSimilarity = QString(args[i+1]).toDouble(&ok);
				if (ok == false || minimumSimilarity < 0.0 || minimumSimilarity > 1.0) {
					qout << "\"" << args[i+1] << "\" is not a valid minimum similarity. needs to be a number between 0.0 and 1.0. aborting.\n";
					exit(0);
				}
				i+= 1;
			} else if (arg == "--simple-similarity") {
				simpleSimilarity = true;
			} else if (arg == "--subsumption-similarity") {
				subsumptionSimilarity = true;
			} else if (arg == "--compressed-dot") {
				compressedDot = true;
			} else if (arg == "--compressed-cxt") {
				compressedCxt = true;
			} else if (arg == "--compressed-txt") {
				compressedTxt = true;
			} else if (arg == "--compressed-txt2") {
				compressedTxt2 = true;
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

	int count = 0;
	count += console               == true ? 1 : 0;
	count += vis                   == true ? 1 : 0;
	count += group                 == true ? 1 : 0;
	count += simpleSimilarity      == true ? 1 : 0;
	count += subsumptionSimilarity == true ? 1 : 0;
	count += compressedDot         == true ? 1 : 0;
	count += compressedCxt         == true ? 1 : 0;
	count += compressedTxt         == true ? 1 : 0;
	count += compressedTxt2        == true ? 1 : 0;

	if (count > 1) {
		qout << "only one of --console, --vis, --group, --simple-similarity, --compressed-dot can be specified at the same time. aborting.\n";
		exit(0);
	}

	if (console == false && vis == false && group == false && simpleSimilarity == false && subsumptionSimilarity == false && compressedDot == false && compressedCxt == false && compressedTxt == false && compressedTxt2 == false) {
		console = true;
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

	if (processesToCompare.size() == 0) {
		processesToCompare = allProcesses;
	}

	if (console == true) {

		QMap<process_t, CallList> m;

		/* record and print call lists */
		foreach (process_t p, processesToCompare) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);
			CallList_fromProcessTrace(t, unifier, (trace_t) 1, true, &m[p]);
			CallList_finalize(&m[p]);

			qout << p << " " << processNames[p] << " {" << "\n";
			qout << CallList_print(m[p], functionNames, "\t");
			qout << "}\n";
		}

	} else if (vis == true) {

		QMap<process_t, CallList> m;

		foreach (process_t p, processesToCompare) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);
			CallList_fromProcessTrace(t, unifier, (trace_t) 1, true, &m[p]);
			CallList_finalize(&m[p]);
		}

		QGraphicsScene *s;
		CallList_visualize(m, processesToCompare, functionNames, processNames, &s);
		QString windowTitle = "CallList: ";
		foreach (function_t p, processesToCompare) { windowTitle += QString("%1, ").arg(p); }
		Viewer_show(s, windowTitle);
	} else { /* group == true || simpleSimilarity == true || subsumptionSimilarity == true || compressedDot == true || compressedCxt == true || compressedTxt == true || compressedTxt2 == true */

		Timer timer;
		double totalTime = 0.0;
		double loadingTime = 0.0;
		double latticeTime = 0.0;
		if (computeTiming == true && showProgress == true) {
			qerr << QString("timer resolution %1ns\n").arg(timer.resolution());
		}

		CompressedCallList cl;

		foreach (process_t p, processesToCompare) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				loadingTime += t;
				if (showProgress) {
					qerr << QString("process %1: %2, load trace %3s").arg(p).arg(processNames[p]).arg(t, 0, 'f', 6);
					qerr.flush();
				}
			}

			CallList n;
			CallList_fromProcessTrace(t, unifier, (trace_t) 1, false, &n);
			CallList_finalize(&n);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				loadingTime += t;
				if (showProgress) {
					qerr << QString(", generate call list %1s").arg(t, 0, 'f', 6);
					qerr.flush();
				}
			}

			CompressedCallList_merge(n, p, &cl);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				latticeTime += t;
				if (showProgress) {
					qerr << QString(", merge into compressed call list %1s, node count %2\n").arg(t, 0, 'f', 6).arg(cl.attributeSets.size());
					qerr.flush();
				}
			}
		}

		CompressedCallList_finalize(&cl);
		int nodeCountBeforeRemovingEmptyNodes = cl.attributeSets.size();
		CompressedCallList_removeEmptyNodes(&cl);

		if (computeTiming) {
			double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
			totalTime += t;
			latticeTime += t;

			QSet<function_t> allFunctions;
			CompressedCallList_functions(cl, &allFunctions);

			// foreach (function_t f, allFunctions) {
			// 	qerr << functionNames[f] << "\n";
			// }

			qerr << QString("finalize compressed call list %1s, function count %2, node count w/ %3, node count w/o %4, group count %5").arg(t, 0, 'f', 6).arg(allFunctions.size()).arg(nodeCountBeforeRemovingEmptyNodes).arg(cl.attributeSets.size()).arg(cl.attributesToObjects.size());
			qerr.flush();
		}

		if (group == true) {
			QList<QSet<QSet<function_t>*>> groups;
			CompressedCallList_group(cl, minimumSimilarity, &groups);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				// missing some global time output to qerr
				qerr << QString(", calculate grouping %1s\n").arg(t, 0, 'f', 6);
				qerr.flush();
			}

			for (int i = 0; i < groups.size(); i += 1) {
				QList<process_t> processes;
				foreach (QSet<function_t>* set, groups[i]) {
					processes.append(cl.attributesToObjects[set].toList());
				}
				std::sort(processes.begin(), processes.end());
				for (int j = 0; j < processes.size(); j += 1) {
					qout << processes[j];
					if (j < processes.size()-1) { qout << ", "; }
				}
				qout << "\n";
			}

		} else if (simpleSimilarity == true || subsumptionSimilarity == true) {

			/* sort sets by minimum process identifier contained. this makes the grouping stable and repeatable. */
			QMap<QSet<function_t>*, process_t> minimumProcess;
			foreach (QSet<function_t>* set, cl.attributesToObjects.keys()) {
				minimumProcess[set] = *std::min_element(cl.attributesToObjects[set].begin(), cl.attributesToObjects[set].end());
			}

			QList<QSet<function_t>*> sets = cl.attributesToObjects.keys();
			auto compareFunction = [&](QSet<function_t>* a, QSet<function_t>* b) {
				Q_ASSERT(minimumProcess.contains(a));
				Q_ASSERT(minimumProcess.contains(b));
				return minimumProcess[a] < minimumProcess[b];
			};
			std::sort(sets.begin(), sets.end(), compareFunction);

			qout << "#group: processes\n";

			for(int i = 0; i < sets.size(); i += 1) {
				QList<process_t> processes = cl.attributesToObjects[sets[i]].toList();
				std::sort(processes.begin(), processes.end());
				qout << QString("%1: ").arg(i + 1, 3);
				for (int j = 0; j < processes.size(); j += 1) {
					qout << processes[j];
					if (j < processes.size()-1) { qout << ", "; }
				}
				qout << "\n";
			}
			qout << "\n";

			qout << "#group x group\n";

			qout << QString(" ").repeated(4);
			for (int j = 0; j < sets.size(); j += 1) {
				qout << QString("%1").arg(j+1, 4);
				if (j < sets.size()-1) { qout << " "; }
			}
			qout << "\n";

			if (simpleSimilarity == true) {
				for (int j = 0; j < sets.size(); j += 1) {
					qout << QString("%1 ").arg(j+1, 3);
					for (int k = 0; k < j; k += 1 ) {
						qout << QString("%1").arg(CompressedCallList_simpleSimilarity(sets[j], sets[k], cl), 4, 'f', 2);
						if (k < sets.size()-1) { qout << " "; }
					}
					qout << "\n";
				}
			} else { /* if subsumptionSimilarity == true*/
				for (int j = 0; j < sets.size(); j += 1) {
					qout << QString("%1 ").arg(j+1, 3);
					for (int k = 0; k < sets.size(); k += 1 ) {
						if (j != k) {
							qout << QString("%1").arg(CompressedCallList_subsumptionSimilarity(sets[j], sets[k], cl), 4, 'f', 2);
							if (k < sets.size()-1) { qout << " "; }
						} else {
							qout << QString(" ").repeated(5);
						}
					}
					qout << "\n";
				}
			}

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				latticeTime += t;
				qerr << QString(", calculate similarity %1s, total %2s, loading %3s, lattice %4s, lattice/loading %5\n").arg(t, 0, 'f', 6).arg(totalTime, 0, 'f', 6).arg(loadingTime, 0, 'f', 6).arg(latticeTime, 0, 'f', 6).arg(latticeTime / loadingTime, 0, 'f');
				qerr.flush();
			}

		} else if (compressedDot == true) {

			qout << CompressedCallList_toDot(cl, processNames, functionNames);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				// missing some global time output to qerr
				qerr << QString(", print dot %1s\n").arg(t, 0, 'f', 6);
				qerr.flush();
			}

		} else if (compressedCxt == true) {

			qout << CompressedCallList_toCxt(cl, processNames, functionNames);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				// missing some global time output to qerr
				qerr << QString(", print cxt %1s\n").arg(t, 0, 'f', 6);
				qerr.flush();
			}

		} else { /* compressedTxt == true || compressedTxt2 == true */

			QString s = CompressedCallList_print(cl, processNames, functionNames);

			if (compressedTxt2 == true) {
				QStringList l = s.split("\n");

				for(int i = 0; i < l.size(); i += 1) {
					if (l[i].contains("attributes: (") == false) { continue; }

					QMapIterator<function_t, QString> j(functionNames);
					while (j.hasNext()) {
						j.next();
						l[i].replace(QString(" %1,").arg(j.key()), QString(" %1,").arg(j.value()));
					}
				}

				s = l.join("\n");
			}

			qout << s;

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				// missing some global time output to qerr
				qerr << QString(", print txt %1s\n").arg(t, 0, 'f', 6);
				qerr.flush();
			}

		}

		CompressedCallList_delete(&cl);
	}

	Viewer_wait();

	return 0;
}

