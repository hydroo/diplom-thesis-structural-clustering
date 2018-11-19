#include "common/prereqs.hpp"
#include "call-list/call-list.hpp"
#include "call-matrix/call-matrix.hpp"
#include "call-tree/call-tree.hpp"
#include "common/unifier.hpp"

int main(int argc, char** args) {
	Q_UNUSED(argc); Q_UNUSED(args);

	QList<QString> traces;
	traces.append(QFileInfo(args[0]).path() + "/a.otf");

	foreach (const QString& traceFileName, traces) {
		auto processes = Otf_processes(traceFileName).toList();
		std::sort(processes.begin(), processes.end());

		QMap<process_t, QString> processNames;
		QMap<function_t, QString> functionNames;
		Otf_processNames(traceFileName, &processNames);
		Otf_functionNames(traceFileName, &functionNames);

		Unifier<function_t> unifier;
		unifier.insert((trace_t) 1, traceFileName, functionNames);
		functionNames = unifier.mappedNames();

		CallList l;
		CallMatrix m;
		CallTree t;

		QMap<process_t, CallList> ls;
		QMap<process_t, CallMatrix> ms;
		QMap<process_t, CallTree> ts;

		foreach (process_t p, processes) {
			ProcessTrace pt;
			Otf_processTrace(traceFileName, p, &pt);

			CallList_fromProcessTrace(pt, unifier, (trace_t) 1, true, &ls[p]);
			CallList_finalize(&ls[p]);
			CallMatrix_fromProcessTrace(pt, unifier, (trace_t) 1, true, &ms[p]);
			CallMatrix_finalize(&ms[p]);
			CallTree_fromProcessTrace(pt, p, unifier, (trace_t) 1, true, &ts[p]);
			CallTree_finalize(&ts[p]);

			CallList_fromProcessTrace(pt, unifier, (trace_t) 1, true, &l);
			CallMatrix_fromProcessTrace(pt, unifier, (trace_t) 1, true, &m);
			CallTree_fromProcessTrace(pt, (process_t) 1, unifier, (trace_t) 1, true, &t);
		}

		CallList_finalize(&l);
		CallMatrix_finalize(&m);
		CallTree_finalize(&t);

		if (traceFileName.contains("a.otf")) {

			/* basic test which checks that a profile collected over two processes works */

			assert(ls.size() == 2);
			assert(ms.size() == 2);
			assert(ts.size() == 2);

			assert(ls[1].size() == 2);
			assert(ls[2].size() == 2);

			assert(ls[1].contains(1));
			assert(ls[1].contains(3));
			assert(ls[2].contains(2));
			assert(ls[2].contains(3));

			assert(ls[1][1].statistics->invocationCount == 1);
			assert(ls[1][3].statistics->invocationCount == 5);
			assert(ls[2][2].statistics->invocationCount == 1);
			assert(ls[2][3].statistics->invocationCount == 5);

			assert(ls[1][3].statistics->exclusiveTime.accumulated            == 15);
			assert(ls[1][3].statistics->exclusiveTime.secondPercentile       ==  1);
			assert(ls[1][3].statistics->exclusiveTime.firstQuartile          ==  2);
			assert(ls[1][3].statistics->exclusiveTime.median                 ==  3);
			assert(ls[1][3].statistics->exclusiveTime.thirdQuartile          ==  4);
			assert(ls[1][3].statistics->exclusiveTime.ninetyEighthPercentile ==  5);
			assert(ls[1][3].statistics->exclusiveTime.min                    ==  1);
			assert(ls[1][3].statistics->exclusiveTime.max                    ==  5);
			assert(ls[1][3].statistics->exclusiveTime.mean                   ==  3);

			assert(ls[1][3].statistics->inclusiveTime.accumulated            == 15);
			assert(ls[1][3].statistics->inclusiveTime.secondPercentile       ==  1);
			assert(ls[1][3].statistics->inclusiveTime.firstQuartile          ==  2);
			assert(ls[1][3].statistics->inclusiveTime.median                 ==  3);
			assert(ls[1][3].statistics->inclusiveTime.thirdQuartile          ==  4);
			assert(ls[1][3].statistics->inclusiveTime.ninetyEighthPercentile ==  5);
			assert(ls[1][3].statistics->inclusiveTime.min                    ==  1);
			assert(ls[1][3].statistics->inclusiveTime.max                    ==  5);
			assert(ls[1][3].statistics->inclusiveTime.mean                   ==  3);

			assert(ls[2][3].statistics->exclusiveTime.accumulated            == 31);
			assert(ls[2][3].statistics->exclusiveTime.secondPercentile       ==  1);
			assert(ls[2][3].statistics->exclusiveTime.firstQuartile          ==  6);
			assert(ls[2][3].statistics->exclusiveTime.median                 ==  7);
			assert(ls[2][3].statistics->exclusiveTime.thirdQuartile          ==  8);
			assert(ls[2][3].statistics->exclusiveTime.ninetyEighthPercentile ==  9);
			assert(ls[2][3].statistics->exclusiveTime.min                    ==  1);
			assert(ls[2][3].statistics->exclusiveTime.max                    ==  9);
			assert(ls[2][3].statistics->exclusiveTime.mean                   == 31 / 5.0);

			assert(ls[2][3].statistics->inclusiveTime.accumulated            == 31);
			assert(ls[2][3].statistics->inclusiveTime.secondPercentile       ==  1);
			assert(ls[2][3].statistics->inclusiveTime.firstQuartile          ==  6);
			assert(ls[2][3].statistics->inclusiveTime.median                 ==  7);
			assert(ls[2][3].statistics->inclusiveTime.thirdQuartile          ==  8);
			assert(ls[2][3].statistics->inclusiveTime.ninetyEighthPercentile ==  9);
			assert(ls[2][3].statistics->inclusiveTime.min                    ==  1);
			assert(ls[2][3].statistics->inclusiveTime.max                    ==  9);
			assert(ls[2][3].statistics->inclusiveTime.mean                   == 31 / 5.0);

			assert(l[1].statistics->invocationCount ==  1);
			assert(l[2].statistics->invocationCount ==  1);
			assert(l[3].statistics->invocationCount == 10);

			assert(l[3].statistics->exclusiveTime.accumulated            == 15 + 31);
			assert(l[3].statistics->exclusiveTime.secondPercentile       ==  1);
			assert(l[3].statistics->exclusiveTime.firstQuartile          ==  2);
			assert(l[3].statistics->exclusiveTime.median                 ==  5);
			assert(l[3].statistics->exclusiveTime.thirdQuartile          ==  7);
			assert(l[3].statistics->exclusiveTime.ninetyEighthPercentile ==  9);
			assert(l[3].statistics->exclusiveTime.min                    ==  1);
			assert(l[3].statistics->exclusiveTime.max                    ==  9);
			assert(l[3].statistics->exclusiveTime.mean                   == (15 + 31) / 10.0);

			assert(l[3].statistics->inclusiveTime.accumulated            == 15 + 31);
			assert(l[3].statistics->inclusiveTime.secondPercentile       ==  1);
			assert(l[3].statistics->inclusiveTime.firstQuartile          ==  2);
			assert(l[3].statistics->inclusiveTime.median                 ==  5);
			assert(l[3].statistics->inclusiveTime.thirdQuartile          ==  7);
			assert(l[3].statistics->inclusiveTime.ninetyEighthPercentile ==  9);
			assert(l[3].statistics->inclusiveTime.min                    ==  1);
			assert(l[3].statistics->inclusiveTime.max                    ==  9);
			assert(l[3].statistics->inclusiveTime.mean                   == (15 + 31) / 10.0);

			assert(ms[1].size()    == 1);
			assert(ms[2].size()    == 1);
			assert(ms[1][0].size() == 2);
			assert(ms[2][0].size() == 2);

			assert(ms[1][0].contains(1));
			assert(ms[1][0].contains(3));
			assert(ms[2][0].contains(2));
			assert(ms[2][0].contains(3));

			assert(ms[1][0][1].statistics->invocationCount == 1);
			assert(ms[1][0][3].statistics->invocationCount == 5);
			assert(ms[2][0][2].statistics->invocationCount == 1);
			assert(ms[2][0][3].statistics->invocationCount == 5);

			assert(ms[1][0][3].statistics->exclusiveTime.accumulated            == 15);
			assert(ms[1][0][3].statistics->exclusiveTime.secondPercentile       ==  1);
			assert(ms[1][0][3].statistics->exclusiveTime.firstQuartile          ==  2);
			assert(ms[1][0][3].statistics->exclusiveTime.median                 ==  3);
			assert(ms[1][0][3].statistics->exclusiveTime.thirdQuartile          ==  4);
			assert(ms[1][0][3].statistics->exclusiveTime.ninetyEighthPercentile ==  5);
			assert(ms[1][0][3].statistics->exclusiveTime.min                    ==  1);
			assert(ms[1][0][3].statistics->exclusiveTime.max                    ==  5);
			assert(ms[1][0][3].statistics->exclusiveTime.mean                   ==  3);

			assert(ms[1][0][3].statistics->inclusiveTime.accumulated            == 15);
			assert(ms[1][0][3].statistics->inclusiveTime.secondPercentile       ==  1);
			assert(ms[1][0][3].statistics->inclusiveTime.firstQuartile          ==  2);
			assert(ms[1][0][3].statistics->inclusiveTime.median                 ==  3);
			assert(ms[1][0][3].statistics->inclusiveTime.thirdQuartile          ==  4);
			assert(ms[1][0][3].statistics->inclusiveTime.ninetyEighthPercentile ==  5);
			assert(ms[1][0][3].statistics->inclusiveTime.min                    ==  1);
			assert(ms[1][0][3].statistics->inclusiveTime.max                    ==  5);
			assert(ms[1][0][3].statistics->inclusiveTime.mean                   ==  3);

			assert(ms[2][0][3].statistics->exclusiveTime.accumulated            == 31);
			assert(ms[2][0][3].statistics->exclusiveTime.secondPercentile       ==  1);
			assert(ms[2][0][3].statistics->exclusiveTime.firstQuartile          ==  6);
			assert(ms[2][0][3].statistics->exclusiveTime.median                 ==  7);
			assert(ms[2][0][3].statistics->exclusiveTime.thirdQuartile          ==  8);
			assert(ms[2][0][3].statistics->exclusiveTime.ninetyEighthPercentile ==  9);
			assert(ms[2][0][3].statistics->exclusiveTime.min                    ==  1);
			assert(ms[2][0][3].statistics->exclusiveTime.max                    ==  9);
			assert(ms[2][0][3].statistics->exclusiveTime.mean                   == 31 / 5.0);

			assert(ms[2][0][3].statistics->inclusiveTime.accumulated            == 31);
			assert(ms[2][0][3].statistics->inclusiveTime.secondPercentile       ==  1);
			assert(ms[2][0][3].statistics->inclusiveTime.firstQuartile          ==  6);
			assert(ms[2][0][3].statistics->inclusiveTime.median                 ==  7);
			assert(ms[2][0][3].statistics->inclusiveTime.thirdQuartile          ==  8);
			assert(ms[2][0][3].statistics->inclusiveTime.ninetyEighthPercentile ==  9);
			assert(ms[2][0][3].statistics->inclusiveTime.min                    ==  1);
			assert(ms[2][0][3].statistics->inclusiveTime.max                    ==  9);
			assert(ms[2][0][3].statistics->inclusiveTime.mean                   == 31 / 5.0);

			assert(m[0][1].statistics->invocationCount ==  1);
			assert(m[0][2].statistics->invocationCount ==  1);
			assert(m[0][3].statistics->invocationCount == 10);

			assert(m[0][3].statistics->exclusiveTime.accumulated            == 15 + 31);
			assert(m[0][3].statistics->exclusiveTime.secondPercentile       ==  1);
			assert(m[0][3].statistics->exclusiveTime.firstQuartile          ==  2);
			assert(m[0][3].statistics->exclusiveTime.median                 ==  5);
			assert(m[0][3].statistics->exclusiveTime.thirdQuartile          ==  7);
			assert(m[0][3].statistics->exclusiveTime.ninetyEighthPercentile ==  9);
			assert(m[0][3].statistics->exclusiveTime.min                    ==  1);
			assert(m[0][3].statistics->exclusiveTime.max                    ==  9);
			assert(m[0][3].statistics->exclusiveTime.mean                   == (15 + 31) / 10.0);

			assert(m[0][3].statistics->inclusiveTime.accumulated            == 15 + 31);
			assert(m[0][3].statistics->inclusiveTime.secondPercentile       ==  1);
			assert(m[0][3].statistics->inclusiveTime.firstQuartile          ==  2);
			assert(m[0][3].statistics->inclusiveTime.median                 ==  5);
			assert(m[0][3].statistics->inclusiveTime.thirdQuartile          ==  7);
			assert(m[0][3].statistics->inclusiveTime.ninetyEighthPercentile ==  9);
			assert(m[0][3].statistics->inclusiveTime.min                    ==  1);
			assert(m[0][3].statistics->inclusiveTime.max                    ==  9);
			assert(m[0][3].statistics->inclusiveTime.mean                   == (15 + 31) / 10.0);

			assert(ts[1].processes[1]->children.size() == 2);
			assert(ts[1].processes[1]->children.contains(1));
			assert(ts[1].processes[1]->children.contains(3));
			assert(ts[2].processes[2]->children.size() == 2);
			assert(ts[2].processes[2]->children.contains(2));
			assert(ts[2].processes[2]->children.contains(3));

			assert(ts[1].processes[1]->children[1]->statistics->invocationCount == 1);
			assert(ts[1].processes[1]->children[3]->statistics->invocationCount == 5);
			assert(ts[2].processes[2]->children[2]->statistics->invocationCount == 1);
			assert(ts[2].processes[2]->children[3]->statistics->invocationCount == 5);

			assert(ts[1].processes[1]->children[3]->statistics->exclusiveTime.accumulated            == 15);
			assert(ts[1].processes[1]->children[3]->statistics->exclusiveTime.secondPercentile       ==  1);
			assert(ts[1].processes[1]->children[3]->statistics->exclusiveTime.firstQuartile          ==  2);
			assert(ts[1].processes[1]->children[3]->statistics->exclusiveTime.median                 ==  3);
			assert(ts[1].processes[1]->children[3]->statistics->exclusiveTime.thirdQuartile          ==  4);
			assert(ts[1].processes[1]->children[3]->statistics->exclusiveTime.ninetyEighthPercentile ==  5);
			assert(ts[1].processes[1]->children[3]->statistics->exclusiveTime.min                    ==  1);
			assert(ts[1].processes[1]->children[3]->statistics->exclusiveTime.max                    ==  5);
			assert(ts[1].processes[1]->children[3]->statistics->exclusiveTime.mean                   ==  3);

			assert(ts[1].processes[1]->children[3]->statistics->inclusiveTime.accumulated            == 15);
			assert(ts[1].processes[1]->children[3]->statistics->inclusiveTime.secondPercentile       ==  1);
			assert(ts[1].processes[1]->children[3]->statistics->inclusiveTime.firstQuartile          ==  2);
			assert(ts[1].processes[1]->children[3]->statistics->inclusiveTime.median                 ==  3);
			assert(ts[1].processes[1]->children[3]->statistics->inclusiveTime.thirdQuartile          ==  4);
			assert(ts[1].processes[1]->children[3]->statistics->inclusiveTime.ninetyEighthPercentile ==  5);
			assert(ts[1].processes[1]->children[3]->statistics->inclusiveTime.min                    ==  1);
			assert(ts[1].processes[1]->children[3]->statistics->inclusiveTime.max                    ==  5);
			assert(ts[1].processes[1]->children[3]->statistics->inclusiveTime.mean                   ==  3);

			assert(ts[2].processes[2]->children[3]->statistics->exclusiveTime.accumulated            == 31);
			assert(ts[2].processes[2]->children[3]->statistics->exclusiveTime.secondPercentile       ==  1);
			assert(ts[2].processes[2]->children[3]->statistics->exclusiveTime.firstQuartile          ==  6);
			assert(ts[2].processes[2]->children[3]->statistics->exclusiveTime.median                 ==  7);
			assert(ts[2].processes[2]->children[3]->statistics->exclusiveTime.thirdQuartile          ==  8);
			assert(ts[2].processes[2]->children[3]->statistics->exclusiveTime.ninetyEighthPercentile ==  9);
			assert(ts[2].processes[2]->children[3]->statistics->exclusiveTime.min                    ==  1);
			assert(ts[2].processes[2]->children[3]->statistics->exclusiveTime.max                    ==  9);
			assert(ts[2].processes[2]->children[3]->statistics->exclusiveTime.mean                   == 31 / 5.0);

			assert(ts[2].processes[2]->children[3]->statistics->inclusiveTime.accumulated            == 31);
			assert(ts[2].processes[2]->children[3]->statistics->inclusiveTime.secondPercentile       ==  1);
			assert(ts[2].processes[2]->children[3]->statistics->inclusiveTime.firstQuartile          ==  6);
			assert(ts[2].processes[2]->children[3]->statistics->inclusiveTime.median                 ==  7);
			assert(ts[2].processes[2]->children[3]->statistics->inclusiveTime.thirdQuartile          ==  8);
			assert(ts[2].processes[2]->children[3]->statistics->inclusiveTime.ninetyEighthPercentile ==  9);
			assert(ts[2].processes[2]->children[3]->statistics->inclusiveTime.min                    ==  1);
			assert(ts[2].processes[2]->children[3]->statistics->inclusiveTime.max                    ==  9);
			assert(ts[2].processes[2]->children[3]->statistics->inclusiveTime.mean                   == 31 / 5.0);

			assert(t.processes[1]->children[1]->statistics->invocationCount ==  1);
			assert(t.processes[1]->children[2]->statistics->invocationCount ==  1);
			assert(t.processes[1]->children[3]->statistics->invocationCount == 10);

			assert(t.processes[1]->children[3]->statistics->exclusiveTime.accumulated            == 15 + 31);
			assert(t.processes[1]->children[3]->statistics->exclusiveTime.secondPercentile       ==  1);
			assert(t.processes[1]->children[3]->statistics->exclusiveTime.firstQuartile          ==  2);
			assert(t.processes[1]->children[3]->statistics->exclusiveTime.median                 ==  5);
			assert(t.processes[1]->children[3]->statistics->exclusiveTime.thirdQuartile          ==  7);
			assert(t.processes[1]->children[3]->statistics->exclusiveTime.ninetyEighthPercentile ==  9);
			assert(t.processes[1]->children[3]->statistics->exclusiveTime.min                    ==  1);
			assert(t.processes[1]->children[3]->statistics->exclusiveTime.max                    ==  9);
			assert(t.processes[1]->children[3]->statistics->exclusiveTime.mean                   == (15 + 31) / 10.0);

			assert(t.processes[1]->children[3]->statistics->inclusiveTime.accumulated            == 15 + 31);
			assert(t.processes[1]->children[3]->statistics->inclusiveTime.secondPercentile       ==  1);
			assert(t.processes[1]->children[3]->statistics->inclusiveTime.firstQuartile          ==  2);
			assert(t.processes[1]->children[3]->statistics->inclusiveTime.median                 ==  5);
			assert(t.processes[1]->children[3]->statistics->inclusiveTime.thirdQuartile          ==  7);
			assert(t.processes[1]->children[3]->statistics->inclusiveTime.ninetyEighthPercentile ==  9);
			assert(t.processes[1]->children[3]->statistics->inclusiveTime.min                    ==  1);
			assert(t.processes[1]->children[3]->statistics->inclusiveTime.max                    ==  9);
			assert(t.processes[1]->children[3]->statistics->inclusiveTime.mean                   == (15 + 31) / 10.0);
		}

		CallTree_delete(&t);

		foreach (process_t p, processes) {
			CallTree_delete(&ts[p]);
		}
	}

	return 0;
}

