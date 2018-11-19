#include "call-tree/call-tree.hpp"
#include "common/prereqs.hpp"

#define HELP_TEXT \
	"call-tree [options] <tracefilename> [processes-to-be-compared]\n" \
	"\n" \
	"\tprocesses are space separated and can be singular or a range in the form of 1-5\n" \
	"\n" \
	"\toptions:\n" \
	"\t\t-p                list processes\n" \
	"\t\t-f                list functions\n" \
	"\n" \
	"\t\t--console         prints the call trees of all processes [default]\n" \
	"\t\t--compressed-dot  prints the compressed call tree as graphviz dot format\n" \
	"\t\t--dot             prints all call trees as graphviz dot format\n"


int main(int argc, char** args) {

	bool listProcesses = false;
	bool listFunctions = false;
	bool console       = false;
	bool dot           = false;
	bool compressedDot = false;
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
			} else if (arg == "--console") {
				console = true;
			} else if (arg == "--compressed-dot") {
				dot = true;
				compressedDot = true;
			} else if (arg == "--dot") {
				dot = true;
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

	if (console == true && dot == true) {
		qout << "--dot and --console cannot be specified at the same time. aborting.\n";
		exit(0);
	}

	if (console == false && dot == false) {
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

			CallTree ct;
			CallTree_fromProcessTrace(t, p, unifier, (trace_t) 1, true, &ct);
			CallTree_finalize(&ct);

			qout << CallTree_print(ct, processNames, functionNames, "");

			CallTree_delete(&ct);
		}
	} else { /* dot == true */

		// QElapsedTimer timer;
		// timer.start();

		CallTree tree;

		foreach (process_t p, processesToCompare) {

			ProcessTrace t;

			Otf_processTrace(traceFileName, p, &t);
			// qerr << QString("process %1: %2, load trace %3s").arg(p).arg(processNames[p]).arg((timer.restart()) / 1000.0, 0, 'f', 3);

			CallTree tmpTree;

			CallTree_fromProcessTrace(t, p, unifier, (trace_t) 1, false, &tmpTree);
			CallTree_finalize(&tmpTree);

			CallTree_merge(&tmpTree, &tree);
			// qerr << QString(", generate and merge call tree %1s").arg((timer.restart()) / 1000.0, 0, 'f', 3);

			if (compressedDot == true) {
				CallTree_compress(&tree);
				// qerr << QString(", compress call tree %1s").arg((timer.restart()) / 1000.0, 0, 'f', 3);
			}

			// qerr << QString(", node count %1\n").arg(CallTree_nodeCount(tree));
			// qerr.flush();

			CallTree_delete(&tmpTree);
		}

		qout << CallTree_toDot(tree, processNames, functionNames, false) << "\n";
		// qerr << QString("call tree to dot, %1s\n").arg((timer.restart()) / 1000.0, 0, 'f', 3);

		CallTree_delete(&tree);
	}

	return 0;
}

