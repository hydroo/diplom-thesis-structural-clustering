#include <QApplication>

#include "call-matrix/call-matrix.hpp"
#include "call-matrix/compressed-call-matrix.hpp"
#include "common/prereqs.hpp"
#include "common/timer.hpp"
#include "common/viewer.hpp"

#define HELP_TEXT \
	"call-matrix [options] <tracefilename> [processes-to-be-compared]\n" \
	"\n" \
	"\tprocesses are space separated and can be singular or a range in the form of 1-5\n" \
	"\n" \
	"\toptions:\n" \
	"\t\t-p                          list processes\n" \
	"\t\t-f                          list functions\n" \
	"\t\t-%                          show loading progress\n" \
	"\t\t-%2                         show loading stats. shorter output than -%\n" \
	"\n" \
	"\t\t--console                   raw console output [default]\n" \
	"\t\t--vis                       gui visualization\n" \
	"\t\t--transitive-vis            gui visualization of the transitively closed matrix\n" \
	"\t\t--group <min-similarity>    prints a list of groups of similar processes. similarity ranges between 0.0 and 1.0\n" \
	"\t\t--simple-similarity         prints the compressed simple similarity matrix\n" \
	"\t\t--subsumption-similarity    prints the compressed subsumption similarity matrix\n" \
	"\t\t--compressed-dot            prints the compressed call matrix as graphviz dot format\n" \
	"\t\t--compressed-transitive-dot prints the compressed transitively closed call matrix as graphviz dot format\n" \
	"\t\t--compressed-cxt            prints the compressed call matrix in the cxt format\n" \
	"\t\t--compressed-transitive-cxt prints the compressed transitively closed call matrix in the cxt format\n" \
	"\t\t--compressed-txt            prints the compressed call matrix\n" \
	"\t\t--compressed-txt2           prints the compressed call matrix, prints function pair names instead of identifiers\n" \
	"\t\t--compressed-transitive-txt prints the compressed transitively closed call matrix\n" \

typedef CompressedCallMatrixCell Cell;

int main(int argc, char** args) {

	QApplication app(argc, args);

	init();

	bool listProcesses = false;
	bool listFunctions = false;
	bool computeTiming = false;
	bool showProgress = false;
	bool console = false;
	bool vis = false;
	bool transitiveVis = false;
	bool group = false;
	bool simpleSimilarity = false;
	bool subsumptionSimilarity = false;
	double minimumSimilarity = -1.0;
	bool compressedDot = false;
	bool transitiveDot = false;
	bool compressedCxt = false;
	bool transitiveCxt = false;
	bool compressedTxt = false;
	bool compressedTxt2 = false;
	bool transitiveTxt = false;
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
			} else if (arg == "--transitive-vis") {
				vis = true;
				transitiveVis = true;
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
			} else if (arg == "--compressed-transitive-dot") {
				compressedDot = true;
				transitiveDot = true;
			} else if (arg == "--compressed-cxt") {
				compressedCxt = true;
			} else if (arg == "--compressed-transitive-cxt") {
				compressedCxt = true;
				transitiveCxt = true;
			} else if (arg == "--compressed-txt") {
				compressedTxt = true;
			} else if (arg == "--compressed-txt2") {
				compressedTxt2 = true;
			} else if (arg == "--compressed-transitive-txt") {
				compressedTxt = true;
				transitiveTxt = true;
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
	count += compressedTxt2         == true ? 1 : 0;

	if (count > 1) {
		qout << "only one of --console, --vis, --group, --simple-similarity, --subsumption-similarity, --compressed-dot, --compressed-cxt can be specified at the same time. aborting.\n";
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

		foreach (process_t p, processesToCompare) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);
			CallMatrix m;
			CallMatrix_fromProcessTrace(t, unifier, (trace_t) 1, true, &m);
			CallMatrix_finalize(&m);

			qout << p << " " << processNames[p] << " {" << "\n";
			qout << CallMatrix_print(m, functionNames, "\t");
			qout << "}\n";
		}

	} else if (vis == true) {

		QMap<process_t, CallMatrix> m;
		QMap<process_t, CallList> l;

		foreach (process_t p, processesToCompare) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);
			CallMatrix_fromProcessTrace(t, unifier, (trace_t) 1, true, &m[p]);
			CallMatrix_finalize(&m[p]);
			if (transitiveVis == true) {
				CompressedCallMatrix_addTransitiveCells(&m[p]);
			}
			CallList_fromProcessTrace(t, unifier, (trace_t) 1, true, &l[p]);
			CallList_finalize(&l[p]);
		}

		QGraphicsScene *s;
		CallMatrix_visualize(m, l, processesToCompare, functionNames, processNames, &s);
		QString windowTitle = "CallMatrix: ";
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

		CompressedCallMatrix m;

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

			CallMatrix n;
			CallMatrix_fromProcessTrace(t, unifier, (trace_t) 1, false, &n);
			CallMatrix_finalize(&n);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				loadingTime += t;
				if (showProgress) {
					qerr << QString(", generate call matrix %1s").arg(t, 0, 'f', 6);
					qerr.flush();
				}
			}

			if (transitiveDot == true || transitiveCxt == true || transitiveTxt == true || subsumptionSimilarity == true) {
				CompressedCallMatrix_addTransitiveCells(&n);

				if (computeTiming) {
					double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
					totalTime += t;
					loadingTime += t; // should/could be an own timing part
					if (showProgress) {
						qerr << QString(", calculate transitive closure %1s").arg(t, 0, 'f', 6);
						qerr.flush();
					}
				}
			}

			CompressedCallMatrix_merge(n, p, &m);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				latticeTime += t;
				if (showProgress) {
					qerr << QString(", merge into compressed call matrix %1s, node count %2\n").arg(t, 0, 'f', 6).arg(m.attributeSets.size());
					qerr.flush();
				}
			}
		}

		CompressedCallMatrix_finalize(&m);
		int nodeCountBeforeRemovingEmptyNodes = m.attributeSets.size();
		CompressedCallMatrix_removeEmptyNodes(&m);

		if (computeTiming) {
			double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
			totalTime += t;
			latticeTime += t;

			QSet<Cell> allCells;
			CompressedCallMatrix_cells(m, &allCells);

			// foreach (Cell c, allCells) {
			// 	qerr << functionNames[c.from] << " -> " << functionNames[c.to] << "\n";
			// }

			qerr << QString("finalize compressed call matrix %1s, cell count %2, node count w/ %3, node count w/o %4, group count %5").arg(t, 0, 'f', 6).arg(allCells.size()).arg(nodeCountBeforeRemovingEmptyNodes).arg(m.attributeSets.size()).arg(m.attributesToObjects.size());
			qerr.flush();
		}

		if (group == true) {
			QList<QSet<QSet<Cell>*>> groups;
			CompressedCallMatrix_group(m, minimumSimilarity, &groups);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				// missing some global time output to qerr
				qerr << QString(", calculate grouping %1s\n").arg(t, 0, 'f', 6);
				qerr.flush();
			}

			for (int i = 0; i < groups.size(); i += 1) {
				QList<process_t> processes;
				foreach (QSet<Cell>* set, groups[i]) {
					processes.append(m.attributesToObjects[set].toList());
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
			QMap<QSet<Cell>*, process_t> minimumProcess;
			foreach (QSet<Cell>* set, m.attributesToObjects.keys()) {
				minimumProcess[set] = *std::min_element(m.attributesToObjects[set].begin(), m.attributesToObjects[set].end());
			}

			QList<QSet<Cell>*> sets = m.attributesToObjects.keys();
			auto compareFunction = [&](QSet<Cell>* a, QSet<Cell>* b) {
				Q_ASSERT(minimumProcess.contains(a));
				Q_ASSERT(minimumProcess.contains(b));
				return minimumProcess[a] < minimumProcess[b];
			};
			std::sort(sets.begin(), sets.end(), compareFunction);

			qout << "#group: processes\n";

			for(int i = 0; i < sets.size(); i += 1) {
				QList<process_t> processes = m.attributesToObjects[sets[i]].toList();
				std::sort(processes.begin(), processes.end());
				qout << QString("%1: ").arg(i + 1, 3);
				for (int j = 0; j < processes.size(); j += 1) {
					qout << processes[j];
					if (j < processes.size()-1) { qout << ", "; }
				}
				qout << "\n";

				// // prints process names instead of identifiers
				//qout << QString("%1: ").arg(i + 1, 3);
				//for (int j = 0; j < processes.size(); j += 1) {
				//	qout << processNames[processes[j]];
				//	if (j < processes.size()-1) { qout << ", "; }
				//}
				//qout << "\n";
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
						qout << QString("%1").arg(CompressedCallMatrix_simpleSimilarity(sets[j], sets[k], m), 4, 'f', 2);
						if (k < sets.size()-1) { qout << " "; }
					}
					qout << "\n";
				}
			} else { /* if subsumptionSimilarity == true*/
				for (int j = 0; j < sets.size(); j += 1) {
					qout << QString("%1 ").arg(j+1, 3);
					for (int k = 0; k < sets.size(); k += 1 ) {
						if (j != k) {
							qout << QString("%1").arg(CompressedCallMatrix_subsumptionSimilarity(sets[j], sets[k], m), 4, 'f', 2);
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
				qerr << QString(", calculate similarity %1s, total %2s, loading %3s, lattice %4s, lattice/loading %5\n").arg(t, 0, 'f', 6).arg(totalTime, 0, 'f', 6).arg(loadingTime, 0, 'f', 6).arg(latticeTime, 0, 'f', 6).arg(latticeTime / loadingTime, 0, 'f', 6);
				qerr.flush();
			}

		} else if ( compressedDot == true ) {

			qout << CompressedCallMatrix_toDot(m, processNames, functionNames);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				// missing some global time output to qerr
				qerr << QString(", print dot %1s\n").arg(t, 0, 'f', 6);
				qerr.flush();
			}

		} else if (compressedCxt == true) {

			qout << CompressedCallMatrix_toCxt(m, processNames, functionNames);

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				// missing some global time output to qerr
				qerr << QString(", print cxt %1s\n").arg(t, 0, 'f', 6);
				qerr.flush();
			}

		} else { /* compressedTxt == true || compressedTxt2 == true */

			QString s = CompressedCallMatrix_print(m, processNames, functionNames);

			if (compressedTxt2 == true) {
				QStringList l = s.split("\n");

				for(int i = 0; i < l.size(); i += 1) {
					if (l[i].contains("attributes: (") == false) { continue; }

					QMapIterator<function_t, QString> j(functionNames);
					while (j.hasNext()) {
						j.next();
						l[i].replace(QString(" %1,").arg(j.key()), QString(" %1,").arg(j.value()));
						l[i].replace(QString(", %1 ").arg(j.key()), QString(", %1 ").arg(j.value()));
					}
				}

				s = l.join("\n");
			}

			s += "\n";
			s += CompressedCallMatrix_printAdditional(m, processNames, functionNames);

			qout << s;

			if (computeTiming) {
				double t = timer.restart() / (1000.0 * 1000.0 * 1000.0);
				totalTime += t;
				// missing some global time output to qerr
				qerr << QString(", print txt %1s\n").arg(t, 0, 'f', 6);
				qerr.flush();
			}

		}

		CompressedCallMatrix_delete(&m);
	}

	Viewer_wait();

	return 0;
}
