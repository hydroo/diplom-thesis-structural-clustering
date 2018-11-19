#include "diff/diff.hpp"
#include "common/prereqs.hpp"

#define HELP_TEXT \
	"diff [options] <tracefilename> [processes-to-be-compared]\n" \
	"\n" \
	"\tprocesses are space separated and can be singular or a range in the form of 1-5\n" \
	"\n" \
	"\toptions:\n" \
	"\t\t-p                       list processes\n" \
	"\t\t-f                          list functions\n" \
	"\n" \
	"\t\t--flat-sequence          print the flat sequences [default]\n" \
	"\t\t--tree-sequence          print the hierarchic sequences\n" \
	"\t\t--flat-diff              print the flat diff\n" \
	"\t\t--flat-score             print the score of the flat diff\n" \
	"\t\t--tree-diff              print the tree diff\n" \
	"\t\t--tree-score             print the score of the tree diff\n" \
	"\t\t    --simple             use the most simple tree comparison algorithm [default]\n" \
	"\t\t    --steve              compare trees according to steve's method\n" \
	"\t\t    --ronny1             compare trees according to ronny's method\n" \
	"\n" \
	"\t\t--extract                copy long subsequences into separate trace files\n"

int main(int argc, char** args) {

	bool listProcesses  = false;
	bool listFunctions  = false;
	bool flatSequence   = false;
	bool treeSequence   = false;
	bool flatDiff       = false;
	bool flatScore      = false;
	bool treeDiff       = false;
	bool treeScore      = false;
	bool treeDiffSimple = false;
	bool treeDiffSteve  = false;
	bool treeDiffRonny1 = false;
	bool extract        = false;
	const int  extractMinimumAddedSequenceLengths = 10000;
	const int  longAddedSequenceLengths = extractMinimumAddedSequenceLengths;
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
			} else if (arg == "--flat-sequence") {
				flatSequence = true;
			} else if (arg == "--tree-sequence") {
				treeSequence = true;
			} else if (arg == "--flat-diff") {
				flatDiff = true;
			} else if (arg == "--flat-score") {
				flatScore = true;
			} else if (arg == "--tree-diff") {
				treeDiff = true;
			} else if (arg == "--tree-score") {
				treeScore = true;
			} else if (arg == "--simple") {
				treeDiffSimple = true;
			} else if (arg == "--steve") {
				treeDiffSteve = true;
			} else if (arg == "--ronny1") {
				treeDiffRonny1 = true;
			} else if (arg == "--extract") {
				extract = true;
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
	count += flatSequence == true ? 1 : 0;
	count += treeSequence == true ? 1 : 0;
	count += flatDiff     == true ? 1 : 0;
	count += flatScore    == true ? 1 : 0;
	count += treeDiff     == true ? 1 : 0;
	count += treeScore    == true ? 1 : 0;
	count += extract      == true ? 1 : 0;

	if (count > 1) {
		qout << "only one of --flat-sequence, --tree-sequence, --flat-diff and --flat-score, --tree-diff, --tree-score can be specified at the same time. aborting.\n";
		exit(0);
	}

	if (count == 0) {
		flatSequence = true;
	}

	if (treeDiff == false && treeScore == false) {
		if (treeDiffSimple == true || treeDiffSteve == true || treeDiffRonny1 == true) {
			qout << "anchor points can only be used with --tree-diff and --tree-score. aborting.\n";
			exit(0);
		}
	}

	int count2 = 0;
	count2 += treeDiffSimple == true ? 1 : 0;
	count2 += treeDiffSteve  == true ? 1 : 0;
	count2 += treeDiffRonny1 == true ? 1 : 0;
	if (count2 > 1) {
		qout << "only one of --simple, --steve and --ronny1 can be specified at the same time. aborting.\n";
		exit(0);
	}

	if (treeDiff == true || treeScore == true) {
		if (count2 == 0) {
			treeDiffSimple = true;
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

	if (processesToCompare.size() == 0) {
		processesToCompare = allProcesses;
	}

	if ((flatDiff == true || flatScore || treeDiff == true || treeScore == true || extract == true) && processesToCompare.size() != 2) {
		qout << "you need to specify exactly two processes to be used. less than makes no sense. more than is not yet supported. aborting.\n";
		exit(0);
	}

	if (flatSequence == true || treeSequence == true) {

		foreach (process_t p, processesToCompare) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);
			if (flatSequence == true) {
				FlatSequence s;
				FlatSequence_fromProcessTrace(t, unifier, (trace_t) 1, &s);
				qout << "process " << p << ": " << FlatSequence_print(s, functionNames) << "\n";
			} else if (treeSequence == true) {
				TreeSequence s;
				TreeSequence_fromProcessTrace(t, unifier, (trace_t) 1, &s);
				qout << "process " << p << ":\n" << TreeSequence_print(s, functionNames, "\t");
				TreeSequence_delete(&s);
			}
		}

	} else if (flatDiff == true || flatScore == true) {
		QMap<process_t, FlatSequence> sequences;

		foreach (process_t p, processesToCompare) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);
			FlatSequence_fromProcessTrace(t, unifier, (trace_t) 1, &sequences[p]);
		}

		assert(sequences.size() == 2);

		Diff diff;
		FlatSequence_compare(sequences[sequences.keys()[0]], sequences[sequences.keys()[1]], &diff);

		if (flatDiff == true) {
			qout << Diff_print(diff, functionNames) << "\n";
		} else { /* flatScore == true */
			qout << Diff_score(diff) << " of min: " << Diff_minScore(diff) << ", max achievable: "<< Diff_maxAchievableScore(diff) << ", max: "<< Diff_maxScore(diff) << "\n";
		}
	} else if (treeDiff == true || treeScore == true) {
		QMap<process_t, TreeSequence> sequences;

		foreach (process_t p, processesToCompare) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);
			TreeSequence_fromProcessTrace(t, unifier, (trace_t) 1, &sequences[p]);
		}

		assert(sequences.size() == 2);

		TreeDiff diff;

		if (treeDiffSimple == true) {
			TreeSequence_compareSimple(sequences[sequences.keys()[0]], sequences[sequences.keys()[1]], &diff);
		} else if (treeDiffSteve == true) {
			TreeSequence_compareSteve(sequences[sequences.keys()[0]], sequences[sequences.keys()[1]], longAddedSequenceLengths, &diff);
		} else { // treeDiffRonny1 == true
			TreeSequence_compareRonny1(sequences[sequences.keys()[0]], sequences[sequences.keys()[1]], longAddedSequenceLengths, &diff);
		}

		if (treeDiff == true) {
			qout << TreeDiff_print(diff, functionNames) << "\n";
		} else { /* treeScore == true */
			qout << TreeDiff_score(diff) << " of min: " << TreeDiff_minScore(diff) << ", max achievable: "<< TreeDiff_maxAchievableScore(diff) << ", max: "<< TreeDiff_maxScore(diff) << "\n";
		}

		QMutableMapIterator<process_t, TreeSequence> i(sequences);
		while (i.hasNext()) {
			i.next();
			TreeSequence_delete(&i.value());
		}

	} else if (extract == true) {
		QMap<process_t, TreeSequence> sequences;

		foreach (process_t p, processesToCompare) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);
			TreeSequence_fromProcessTrace(t, unifier, (trace_t) 1, &sequences[p]);
		}

		process_t ap = sequences.keys()[0];
		process_t bp = sequences.keys()[1];
		process_t ap2 = ap; // rename 0 identifier processes to 1, because otf1 doesn't allow 0 as an identifier
		process_t bp2 = bp;
		if (ap2 < bp2) {
			if (ap2 == 0) {
				ap2 = 1;
				if (bp2 == 1) {
					bp2 = 2;
				}
			}
		} else {
			if (bp2 == 0) {
				bp2 = 1;
				if (ap2 == 1) {
					ap2 = 2;
				}
			}
		}

		TreeSequence_extractLongSubsequences(traceFileName, sequences[ap], sequences[bp], ap2, bp2, processNames[ap], processNames[bp], functionNames, extractMinimumAddedSequenceLengths);

		QMutableMapIterator<process_t, TreeSequence> i(sequences);
		while (i.hasNext()) {
			i.next();
			TreeSequence_delete(&i.value());
		}

	}

	return 0;
}
