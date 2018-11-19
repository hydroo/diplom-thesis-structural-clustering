#include "call-list/call-list.hpp"
#include "call-matrix/call-matrix.hpp"
#include "call-tree/call-tree.hpp"
#include "common/prereqs.hpp"
#include "profile/profile.hpp"

#define HELP_TEXT \
	"profile [options] <tracefilename> [processes-to-be-compared]\n" \
	"\n" \
	"\tprocesses are space separated and can be singular or a range in the form of 1-5\n" \
	"\n" \
	"\toptions:\n" \
	"\t\t-p                  list processes\n" \
	"\t\t-f                  list functions\n" \
	"\t\t-%                  show trace file loading progress\n" \
	"\n" \
	"\t\t-l, --call-list     print call list profile [default]\n" \
	"\t\t-m, --call-matrix   print call list profile\n" \
	"\t\t-t, --call-tree     print call list profile\n" \
	"\n" \
	"\t\t-n, --names         display names instead of identifiers\n"

static QString printFunctionGroup(functiongroup_t g, const QMap<functiongroup_t, QString>& functionGroupNames = QMap<functiongroup_t, QString>());
static QString printFunction(function_t f, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>());
static QString printFunctionPair(const FunctionPair& p, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>());
static QString printFunctionTuple(const FunctionTuple& t, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>());

int main(int argc, char** args) {

	bool listProcesses = false;
	bool listFunctions = false;
	bool showProgress = false;
	bool printCallListProfile = false;
	bool printCallMatrixProfile = false;
	bool printCallTreeProfile = false;
	bool printNames = false;
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
				showProgress = true;
			} else if (arg == "-l" || arg == "--call-list") {
				printCallListProfile = true;
			} else if (arg == "-m" || arg == "--call-matrix") {
				printCallMatrixProfile = true;
			} else if (arg == "-t" || arg == "--call-tree") {
				printCallTreeProfile = true;
			} else if (arg == "-n" || arg == "--names") {
				printNames = true;
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
	count += printCallListProfile	== true ? 1 : 0;
	count += printCallMatrixProfile == true ? 1 : 0;
	count += printCallTreeProfile	== true ? 1 : 0;

	if (count > 1) {
		qout << "only one of --call-list, --call-matrix, --call-tree can be specified at the same time. aborting.\n";
		exit(0);
	}

	if (printCallListProfile == false && printCallMatrixProfile == false && printCallTreeProfile == false) {
		printCallListProfile = true;
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
	QMap<functiongroup_t, QString> functionGroupNames;
	QMap<function_t, functiongroup_t> f2g;
	QMap<functiongroup_t, QSet<function_t>> g2f;

	Otf_processNames(traceFileName, &processNames);
	Otf_functionNames(traceFileName, &functionNames);
	Otf_functionGroupNames(traceFileName, &functionGroupNames);
	Otf_functionGroupMembers(traceFileName, &f2g, &g2f);

	if (listProcesses == true) {
		foreach (process_t p, allProcesses) {
			qout << QString("%1: %2\n").arg(p).arg(processNames[p]);
		}
		exit(0);
	}

	Unifier<function_t> functionUnifier;
	functionUnifier.insert((trace_t) 1, traceFileName, functionNames);
	functionNames = functionUnifier.mappedNames();

	Unifier<functiongroup_t> functionGroupUnifier;
	functionGroupUnifier.insert((trace_t) 1, traceFileName, functionGroupNames);
	functionGroupNames = functionGroupUnifier.mappedNames();

	Otf_unifyFunctionGroupMembers((trace_t) 1, functionUnifier, functionGroupUnifier, &f2g, &g2f);

	if (listFunctions == true) {
		auto sortedFunctionNames = functionNames.keys();
		std::sort(sortedFunctionNames.begin(), sortedFunctionNames.end());

		auto sortedFunctionGroupNames = functionGroupNames.keys();
		std::sort(sortedFunctionGroupNames.begin(), sortedFunctionGroupNames.end());

		int maxFunctionIdentifierLength = 0;
		foreach (function_t f, sortedFunctionNames) {
			if (f == 0) {
				continue;
			} else if (f < 0) {
				if (floor(log10(-f)) + 2 > maxFunctionIdentifierLength) {
					maxFunctionIdentifierLength = floor(log10(-f)) + 2;
				}
			} else {
				if (floor(log10(f)) + 1 > maxFunctionIdentifierLength) {
					maxFunctionIdentifierLength = floor(log10(f)) + 1;
				}
			}
		}

		foreach (functiongroup_t g, sortedFunctionGroupNames) {
			Q_ASSERT(functionGroupNames.contains(g));

			qout << QString("%1: %2\n").arg(g).arg(functionGroupNames[g]);

			foreach (function_t f, sortedFunctionNames) {
				if (g2f[g].contains(f) == false) { continue; }
				Q_ASSERT(f2g.contains(f));
				Q_ASSERT(f2g[f] == g);

				qout << QString("\t%1: %2\n").arg(f, maxFunctionIdentifierLength).arg(functionNames[f]);
			}
		}

		exit(0);
	}

	if (processesToCompare.size() == 0) {
		processesToCompare = allProcesses;
	}

	QElapsedTimer timer;
	timer.start();

	int processIdentifierLength = 0;
	if (showProgress) {
		foreach (process_t p, processesToCompare) {
			if (p == 0) { continue; }
			Q_ASSERT(p > 0);
			processIdentifierLength = std::max(processIdentifierLength, int(floor(log10(p))) + 1);
		}
	}

	FunctionGroupProfile gp;
	CallListProfile      clp;
	CallMatrixProfile    cmp;
	CallTreeProfile      ctp;

	foreach (process_t p, processesToCompare) {
		ProcessTrace pt;
		Otf_processTrace(traceFileName, p, &pt);

		if (showProgress) {
			qerr << QString("process %1: load trace %3s").arg(p, processIdentifierLength).arg((timer.restart()) / 1000.0, 0, 'f', 3);
			qerr.flush();
		}

		if (printCallListProfile == true) {

			CallList l;
			CallList_fromProcessTrace(pt, functionUnifier, (trace_t) 1, true, &l);
			CallList_finalize(&l);

			QMap<functiongroup_t, int64_t> groupInvocationCount;
			QMap<functiongroup_t, int64_t> groupAccumulatedExclusiveTime;
			QMap<functiongroup_t, int64_t> groupAccumulatedInclusiveTime;

			QMapIterator<function_t, CallListEntry> i(l);
			while (i.hasNext()) {
				i.next();
				const auto& s = *i.value().statistics;

				CallListProfile_add(i.key(), s.invocationCount, s.exclusiveTime.accumulated, s.inclusiveTime.accumulated, &clp);

				/* accumulate group infos */
				assert(f2g.contains(i.key()));

				functiongroup_t g = f2g[i.key()];

				if (groupInvocationCount.contains(g) == false) {
					groupInvocationCount[g]          = 0;
					groupAccumulatedExclusiveTime[g] = 0;
					groupAccumulatedInclusiveTime[g] = 0;
				}

				groupInvocationCount[g]          += s.invocationCount;
				groupAccumulatedExclusiveTime[g] += s.exclusiveTime.accumulated;
				groupAccumulatedInclusiveTime[g] += s.inclusiveTime.accumulated;
			}

			QMapIterator<functiongroup_t, int64_t> j(groupInvocationCount);
			while (j.hasNext()) {
				j.next();
				FunctionGroupProfile_add(j.key(), groupInvocationCount[j.key()], groupAccumulatedExclusiveTime[j.key()], groupAccumulatedInclusiveTime[j.key()], &gp);
			}

		} else if (printCallMatrixProfile == true) {

			CallMatrix m;
			CallMatrix_fromProcessTrace(pt, functionUnifier, (trace_t) 1, true, &m);
			CallMatrix_finalize(&m);

			QMapIterator<function_t, QMap<function_t, CallMatrixCell>> i(m);
			while (i.hasNext()) {
				i.next();

				QMapIterator<function_t, CallMatrixCell> j(i.value());
				while (j.hasNext()) {
					j.next();
					const auto& s = *j.value().statistics;
					CallMatrixProfile_add(FunctionPair(i.key(), j.key()), s.invocationCount, s.exclusiveTime.accumulated, s.inclusiveTime.accumulated, &cmp);
				}
			}

		} else { /* printCallTreeProfile == true */

			CallTree ct;
			CallTree_fromProcessTrace(pt, p, functionUnifier, (trace_t) 1, true, &ct);
			CallTree_finalize(&ct);

			assert(ct.processes.size() == 1);
			assert(ct.processes.contains(p));
			assert(ct.processes[p]->id == 0);

			std::function<void(const CallTreeNode&, FunctionTuple*, CallTreeProfile*)> traverse = [&traverse](const CallTreeNode& n, FunctionTuple* t, CallTreeProfile* ctp) {
				t->append(n.id);

				const auto& s = *n.statistics;
				CallTreeProfile_add(*t, s.invocationCount, s.exclusiveTime.accumulated, s.inclusiveTime.accumulated, ctp);

				auto keys = n.children.keys();
				std::sort(std::begin(keys), std::end(keys));

				foreach(function_t f, keys) {
					traverse(*n.children[f], t, ctp);
				}

				t->removeLast();
			};

			auto keys = ct.processes[p]->children.keys();
			std::sort(std::begin(keys), std::end(keys));

			FunctionTuple t;

			foreach(function_t f, keys) {
				traverse(*ct.processes[p]->children[f], &t, &ctp);
			}
		}

		if (showProgress) {
			if (printCallListProfile == true) {
				qerr << QString(", cl %1s\n").arg((timer.restart()) / 1000.0, 0, 'f', 3);
			} else if (printCallMatrixProfile == true) {
				qerr << QString(", cm %1s\n").arg((timer.restart()) / 1000.0, 0, 'f', 3);
			} else { /* printCallTreeProfile == true */
				qerr << QString(", ct %1s\n").arg((timer.restart()) / 1000.0, 0, 'f', 3);
			}
			qerr.flush();
		}
	}

	/* add 0 entries for all processes that do have an identifier used by others */
	if (printCallListProfile == true) {
		QMapIterator<functiongroup_t, Measure<int64_t>> i(gp.invocations);
		while (i.hasNext()) {
			i.next();
			int d = i.value().dataPointCount;
			while (d < processesToCompare.size()) {
				FunctionGroupProfile_add(i.key(), 0, 0, 0, &gp);
				d += 1;
			}
		}

		QMapIterator<function_t, Measure<int64_t>> j(clp.invocations);
		while (j.hasNext()) {
			j.next();
			int d = j.value().dataPointCount;
			while (d < processesToCompare.size()) {
				CallListProfile_add(j.key(), 0, 0, 0, &clp);
				d += 1;
			}
		}
	} else if (printCallMatrixProfile == true) {
		QMapIterator<FunctionPair, Measure<int64_t>> i(cmp.invocations);
		while (i.hasNext()) {
			i.next();
			int d = i.value().dataPointCount;
			while (d < processesToCompare.size()) {
				CallMatrixProfile_add(i.key(), 0, 0, 0, &cmp);
				d += 1;
			}
		}
	} else { /* printCallTreeProfile == true */
		QMapIterator<FunctionTuple, Measure<int64_t>> i(ctp.invocations);
		while (i.hasNext()) {
			i.next();
			int d = i.value().dataPointCount;
			while (d < processesToCompare.size()) {
				CallTreeProfile_add(i.key(), 0, 0, 0, &ctp);
				d += 1;
			}
		}
	}

	if (printCallListProfile == true) {
		FunctionGroupProfile_finalize(&gp);
		CallListProfile_finalize(&clp);
	} else if (printCallMatrixProfile == true) {
		CallMatrixProfile_finalize(&cmp);
	} else { /* printCallTreeProfile == true */
		CallTreeProfile_finalize(&ctp);
	}

	if (showProgress) {
		qerr << QString("finalize %1s").arg(timer.restart() / 1000.0, 0, 'f', 3);
		qerr.flush();
	}

	if (printCallListProfile == true) {
		qout << "groups:\n";
		if (printNames == true) {
			qout << FunctionGroupProfile_print(gp, [&functionGroupNames](functiongroup_t g) { return printFunctionGroup(g, functionGroupNames); }, "");
		} else {
			qout << FunctionGroupProfile_print(gp, [](functiongroup_t g) { return printFunctionGroup(g); }, "");
		}

		qout << "functions:\n";
		if (printNames == true) {
			qout << CallListProfile_print(clp, [&functionNames](function_t f) { return printFunction(f, functionNames); }, "");
		} else {
			qout << CallListProfile_print(clp, [](function_t f) { return printFunction(f); }, "");
		}
	} else if (printCallMatrixProfile == true) {
		if (printNames == true) {
			qout << CallMatrixProfile_print(cmp, [&functionNames](const FunctionPair& p) { return printFunctionPair(p, functionNames); }, "");
		} else {
			qout << CallMatrixProfile_print(cmp, [](const FunctionPair& p) { return printFunctionPair(p); }, "");
		}
	} else { /* printCallTreeProfile == true */
		if (printNames == true) {
			qout << CallTreeProfile_print(ctp, [&functionNames](const FunctionTuple& t) { return printFunctionTuple(t, functionNames); }, "");
		} else {
			qout << CallTreeProfile_print(ctp, [](const FunctionTuple& t) { return printFunctionTuple(t); }, "");
		}
	}

	if (showProgress) {
		qerr << QString(", print %1s\n").arg(timer.restart() / 1000.0, 0, 'f', 3);
		qerr.flush();
	}

	return 0;
}

/* *** misc *** */
QString printFunctionGroup(functiongroup_t g, const QMap<functiongroup_t, QString>& functionGroupNames) {
	if (functionGroupNames.contains(g)) { return QString("%1").arg(functionGroupNames[g]); }
	else                                { return QString("%1").arg(g); }
}
QString printFunction(function_t f, const QMap<function_t, QString>& functionNames) {
	if (functionNames.contains(f)) { return QString("%1").arg(functionNames[f]); }
	else                           { return QString("%1").arg(f); }
}
QString printFunctionPair(const FunctionPair& p, const QMap<function_t, QString>& functionNames) {
	QString from, to;

	if (functionNames.contains(p.first))  { from = QString("%1").arg(functionNames[p.first]); }
	else                                  { from = QString("%1").arg(p.first); }
	if (functionNames.contains(p.second)) { to   = QString("%1").arg(functionNames[p.second]); }
	else                                  { to   = QString("%1").arg(p.second); }

	return QString("%1 -> %2").arg(from).arg(to);
}
QString printFunctionTuple(const FunctionTuple& t, const QMap<function_t, QString>& functionNames) {
	QString ret, s;
	for (int i = 0; i < t.size(); i += 1) {
		if (functionNames.contains(t[i])) { ret += QString("%1").arg(functionNames[t[i]]); }
		else                              { ret += QString("%1").arg(t[i]); }
		if (i+1 < t.size()) { ret += " -> "; }
	}
	return ret;
}
