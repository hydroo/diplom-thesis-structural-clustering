#include "common/prereqs.hpp"
#include "diff/diff.hpp"
#include "common/unifier.hpp"

static void anchorPointsTest();

int main(int argc, char** args) {
	Q_UNUSED(argc); Q_UNUSED(args);

	/* *** simple diff testing *** */
	FlatSequence pre = {1};
	FlatSequence suf = {3, 4};
	Diff diff;
	// a starts with b
	diff.clear(); FlatSequence_compare(pre + suf, pre, &diff);
	assert(diff.size() == 2);
	assert(diff[0].op == DiffEntry::Equal  && diff[0].seq == QList<function_t>({1}));
	assert(diff[1].op == DiffEntry::Delete && diff[1].seq == QList<function_t>({3, 4}));
	// a ends with b
	diff.clear(); FlatSequence_compare(pre + suf, suf, &diff);
	assert(diff.size() == 2);
	assert(diff[0].op == DiffEntry::Delete && diff[0].seq == QList<function_t>({1}));
	assert(diff[1].op == DiffEntry::Equal  && diff[1].seq == QList<function_t>({3, 4}));
	// b starts with a
	diff.clear(); FlatSequence_compare(pre , pre + suf, &diff);
	assert(diff.size() == 2);
	assert(diff[0].op == DiffEntry::Equal  && diff[0].seq == QList<function_t>({1}));
	assert(diff[1].op == DiffEntry::Insert && diff[1].seq == QList<function_t>({3, 4}));
	// b ends with a
	diff.clear(); FlatSequence_compare(suf, pre + suf, &diff);
	assert(diff.size() == 2);
	assert(diff[0].op == DiffEntry::Insert && diff[0].seq == QList<function_t>({1}));
	assert(diff[1].op == DiffEntry::Equal  && diff[1].seq == QList<function_t>({3, 4}));

	// arbitrary 1, according to diff unix command line tool
	diff.clear(); FlatSequence_compare({1, 2, 11, 2, 12, 2, 1}, {1, 2, 12, 2, 11, 2, 1}, &diff);
	assert(diff.size() == 5);
	assert(diff[0].op == DiffEntry::Equal  && diff[0].seq == QList<function_t>({1, 2}));
	assert(diff[1].op == DiffEntry::Delete && diff[1].seq == QList<function_t>({11, 2}));
	assert(diff[2].op == DiffEntry::Equal  && diff[2].seq == QList<function_t>({12}));
	assert(diff[3].op == DiffEntry::Insert && diff[3].seq == QList<function_t>({2, 11}));
	assert(diff[4].op == DiffEntry::Equal  && diff[4].seq == QList<function_t>({2, 1}));

	// arbitrary 2, according to diff unix command line tool
	diff.clear(); FlatSequence_compare({1, 4, 6, 7, 8, 1, 10, 3, 10, 1}, {6, 7, 8}, &diff);
	assert(diff.size() == 3);
	assert(diff[0].op == DiffEntry::Delete && diff[0].seq == QList<function_t>({1, 4}));
	assert(diff[1].op == DiffEntry::Equal  && diff[1].seq == QList<function_t>({6, 7, 8}));
	assert(diff[2].op == DiffEntry::Delete && diff[2].seq == QList<function_t>({1, 10, 3, 10, 1}));

	/* *** testing with traces *** */
	QList<QString> traces;
	traces.append(QFileInfo(args[0]).path() + "/a.otf");
	traces.append(QFileInfo(args[0]).path() + "/b.otf");
	traces.append(QFileInfo(args[0]).path() + "/c.otf");
	traces.append(QFileInfo(args[0]).path() + "/d.otf");
	traces.append(QFileInfo(args[0]).path() + "/e.otf");

	foreach (const QString& traceFileName, traces) {
		// qDebug() << QString("--- %1 ---").arg(traceFileName);

		auto processes = Otf_processes(traceFileName).toList();
		std::sort(processes.begin(), processes.end());

		QMap<process_t, QString> processNames;
		QMap<function_t, QString> functionNames;
		Otf_processNames(traceFileName, &processNames);
		Otf_functionNames(traceFileName, &functionNames);

		Unifier<function_t> unifier;
		unifier.insert((trace_t) 1, traceFileName, functionNames);
		functionNames = unifier.mappedNames();

		QMap<process_t, FlatSequence> flatSequences;
		QMap<process_t, TreeSequence> treeSequences;

		foreach (process_t p, processes) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);

			FlatSequence_fromProcessTrace(t, unifier, (trace_t) 1, &flatSequences[p]);
			TreeSequence_fromProcessTrace(t, unifier, (trace_t) 1, &treeSequences[p]);

			FlatSequence fu;
			FlatSequence_fromTreeSequence(treeSequences[p], &fu);

			// qerr << FlatSequence_print(flatSequences[p]) << "\n";
			// qerr << TreeSequence_print(treeSequences[p]);
			// qerr.flush();

			assert(flatSequences[p] == fu);

			if (traceFileName.contains("a.otf")) {
				if (p == 1) {
					FlatSequence ft = {1, 2, 3, 2, 3, 2, 1, 2, 3, 2, 3, 4, 3, 2, 3, 2, 1};
					assert(flatSequences[p] == ft);

					/* 1: 2: 3, 3
					 *    2: 3
					 *       3: 4
					 *       3
					 */
					TreeNode c;   c.id = 3;
					TreeNode b2; b2.id = 2; b2.calls += &c; b2.calls += &c;
					TreeNode d;   d.id = 4;
					TreeNode cd; cd.id = 3; cd.calls += &d;
					TreeNode b3; b3.id = 2; b3.calls += &c; b3.calls += &cd; b3.calls += &c;
					TreeNode a;   a.id = 1;  a.calls += &b2; a.calls += &b3;
					TreeSequence tt; tt += &a;
					assert(TreeSequence_equals(treeSequences[p], tt));
				} else { /* p == 2*/
					FlatSequence ft = {1, 2, 3, 2, 3, 2, 1, 2, 3, 2, 3, 2, 3, 4, 3, 2, 1};
					assert(flatSequences[p] == ft);

					/* 1: 2: 3, 3
					 *    2: 3
					 *       3
					 *       3: 4
					 */
					TreeNode c;  c.id = 3;
					TreeNode b2; b2.id = 2; b2.calls += &c; b2.calls += &c;
					TreeNode d;  d.id = 4;
					TreeNode cd; cd.id = 3; cd.calls += &d;
					TreeNode b3; b3.id = 2; b3.calls += &c; b3.calls += &c; b3.calls += &cd;
					TreeNode a;   a.id = 1;  a.calls += &b2; a.calls += &b3;
					TreeSequence tt; tt += &a;
					assert(TreeSequence_equals(treeSequences[p], tt));
				}
			} else if (traceFileName.contains("b.otf")) {
				if (p == 1) {
					FlatSequence ft = {1, 2, 3, 2, 1, 4, 5, 4, 1};
					assert(flatSequences[p] == ft);

					/* 1: 2: 3
					 *    4: 5
					 */
					TreeNode c; c.id = 3;
					TreeNode b; b.id = 2; b.calls += &c;
					TreeNode e; e.id = 5;
					TreeNode d; d.id = 4; d.calls += &e;
					TreeNode a; a.id = 1; a.calls += &b; a.calls += &d;
					TreeSequence tt; tt += &a;
					assert(TreeSequence_equals(treeSequences[p], tt));
				} else { /* p == 2*/
					FlatSequence ft = {1, 3, 1, 4, 5, 4, 1};
					assert(flatSequences[p] == ft);

					/* 1: 3
					 *    4: 5
					 */
					TreeNode c; c.id = 3;
					TreeNode e; e.id = 5;
					TreeNode d; d.id = 4; d.calls += &e;
					TreeNode a; a.id = 1; a.calls += &c; a.calls += &d;
					TreeSequence tt; tt += &a;
					assert(TreeSequence_equals(treeSequences[p], tt));
				}
			} else {
				// add more flat- and tree sequence assertions here
			}
		}
		assert(flatSequences.keys() == QList<process_t>({1, 2}));

		Diff diff;
		FlatSequence_compare(flatSequences[(process_t) 1], flatSequences[(process_t) 2], &diff);

		assert(diff.size() > 0);
		foreach (const DiffEntry& e, diff) {
			assert(e.seq.size() > 0);
		}

		assert(Diff_minScore(diff)           == 0);
		assert(Diff_minScore(diff)           <= Diff_score(diff));
		assert(Diff_score(diff)              <= Diff_maxAchievableScore(diff));
		assert(Diff_maxAchievableScore(diff) <= Diff_maxScore(diff));
		assert(Diff_maxAchievableScore(diff) == std::min(flatSequences[(process_t) 1].size(), flatSequences[(process_t) 2].size()));
		assert(Diff_maxScore(diff)           == std::max(flatSequences[(process_t) 1].size(), flatSequences[(process_t) 2].size()));

		TreeDiff treeDiff;
		TreeSequence_compareSimple(treeSequences[(process_t) 1], treeSequences[(process_t) 2], &treeDiff);

		// // if (traceFileName.contains("b.otf")) {
		// 	qDebug() << "";
		// 	qDebug() << FlatSequence_print(flatSequences[(process_t) 1]) << FlatSequence_print(flatSequences[(process_t) 2]) << Diff_print(diff);
		// 	qDebug() << "process 1:\n" << TreeSequence_print(treeSequences[(process_t) 1]);
		// 	qDebug() << "process 2:\n" << TreeSequence_print(treeSequences[(process_t) 2]);
		// 	qDebug() << "score: flat: " << Diff_score(diff) << ", tree:" << TreeDiff_score(treeDiff);
		// 	qDebug() << "max-score: flat: " << Diff_maxScore(diff) << ", tree:" << TreeDiff_maxScore(treeDiff);
		// 	qDebug() << "tree-diff:\n" << TreeDiff_print(treeDiff);
		// // }

		assert(TreeDiff_minScore(treeDiff)           == 0);
		assert(TreeDiff_minScore(treeDiff)           <= TreeDiff_score(treeDiff));
		assert(TreeDiff_score(treeDiff)              <= TreeDiff_maxAchievableScore(treeDiff));
		assert(TreeDiff_maxAchievableScore(treeDiff) <= TreeDiff_maxScore(treeDiff));
		assert(TreeDiff_minScore(treeDiff)           == Diff_minScore(diff));
		assert(TreeDiff_maxAchievableScore(treeDiff) == Diff_maxAchievableScore(diff));
		assert(TreeDiff_maxScore(treeDiff)           == Diff_maxScore(diff));

		if (traceFileName.contains("a.otf")) {
			assert(diff.size() == 5);
			assert(diff[0].op == DiffEntry::Equal  && diff[0].seq == QList<function_t>({1, 2, 3, 2, 3, 2, 1, 2, 3, 2, 3}));
			assert(diff[1].op == DiffEntry::Delete && diff[1].seq == QList<function_t>({4, 3}));
			assert(diff[2].op == DiffEntry::Equal  && diff[2].seq == QList<function_t>({2}));
			assert(diff[3].op == DiffEntry::Insert && diff[3].seq == QList<function_t>({3, 4}));
			assert(diff[4].op == DiffEntry::Equal  && diff[4].seq == QList<function_t>({3, 2, 1}));

			assert(Diff_minScore(diff)           ==  0);
			assert(Diff_score(diff)              == 15);
			assert(Diff_maxAchievableScore(diff) == 17);
			assert(Diff_maxScore(diff)           == 17);

			assert(treeDiff.size() == 1);                                                             // 0
			assert(treeDiff[0].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0].size() == 1);                                              //   1
			assert(treeDiff[0].subDiffs[0][0].subDiffs.size() == 2);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0].size() == 1);                               //     2
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0][0].subDiffs.size() == 2);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0][0].subDiffs[0].size() == 0);                //       3
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0][0].subDiffs[1].size() == 0);                //       3
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1].size() == 1);                               //     2
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs.size() == 3);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[0].size() == 0);                //       3
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[1].size() == 1);                //       3
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[1][0].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[1][0].subDiffs[0].size() == 0); //         -4
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[2].size() == 1);                //       3
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[2][0].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[2][0].subDiffs[0].size() == 0); //         +4

			assert(treeDiff[0].op == DiffEntry::Equal && treeDiff[0].seq == QList<function_t>({1}));
			assert(treeDiff[0].subDiffs[0][0].op == DiffEntry::Equal && treeDiff[0].subDiffs[0][0].seq == QList<function_t>({2, 2}));
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0][0].op == DiffEntry::Equal && treeDiff[0].subDiffs[0][0].subDiffs[0][0].seq == QList<function_t>({3, 3}));
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].op == DiffEntry::Equal && treeDiff[0].subDiffs[0][0].subDiffs[1][0].seq == QList<function_t>({3, 3, 3}));
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[1][0].op == DiffEntry::Delete && treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[1][0].seq == QList<function_t>({4}));
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[2][0].op == DiffEntry::Insert && treeDiff[0].subDiffs[0][0].subDiffs[1][0].subDiffs[2][0].seq == QList<function_t>({4}));

			assert(TreeDiff_score(treeDiff) == 15);
		} else if (traceFileName.contains("b.otf")) {
			assert(diff.size() == 5);
			assert(diff[0].op == DiffEntry::Equal  && diff[0].seq == QList<function_t>({1}));
			assert(diff[1].op == DiffEntry::Delete && diff[1].seq == QList<function_t>({2}));
			assert(diff[2].op == DiffEntry::Equal  && diff[2].seq == QList<function_t>({3}));
			assert(diff[3].op == DiffEntry::Delete && diff[3].seq == QList<function_t>({2}));
			assert(diff[4].op == DiffEntry::Equal  && diff[4].seq == QList<function_t>({1, 4, 5, 4, 1}));

			assert(Diff_minScore(diff)           == 0);
			assert(Diff_score(diff)              == 7);
			assert(Diff_maxAchievableScore(diff) == 7);
			assert(Diff_maxScore(diff)           == 9);

			assert(treeDiff.size() == 1);
			assert(treeDiff[0].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0].size() == 3);
			assert(treeDiff[0].subDiffs[0][0].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0].size() == 1);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0][0].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0][0].subDiffs[0].size() == 0);
			assert(treeDiff[0].subDiffs[0][1].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0][1].subDiffs[0].size() == 0);
			assert(treeDiff[0].subDiffs[0][2].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0][2].subDiffs[0].size() == 1);
			assert(treeDiff[0].subDiffs[0][2].subDiffs[0][0].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0][2].subDiffs[0][0].subDiffs[0].size() == 0);

			assert(treeDiff[0].op == DiffEntry::Equal && treeDiff[0].seq == QList<function_t>({1}));
			assert(treeDiff[0].subDiffs[0][0].op == DiffEntry::Delete && treeDiff[0].subDiffs[0][0].seq == QList<function_t>({2}));
			assert(treeDiff[0].subDiffs[0][1].op == DiffEntry::Insert && treeDiff[0].subDiffs[0][1].seq == QList<function_t>({3}));

			assert(treeDiff[0].subDiffs[0][2].op == DiffEntry::Equal  && treeDiff[0].subDiffs[0][2].seq == QList<function_t>({4}));
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0][0].op == DiffEntry::Delete && treeDiff[0].subDiffs[0][0].subDiffs[0][0].seq == QList<function_t>({3}));
			assert(treeDiff[0].subDiffs[0][2].subDiffs[0][0].op == DiffEntry::Equal && treeDiff[0].subDiffs[0][2].subDiffs[0][0].seq == QList<function_t>({5}));

			assert(TreeDiff_score(treeDiff) == 6);
		} else if (traceFileName.contains("c.otf")) {
			assert(diff.size() == 7);
			assert(diff[0].op == DiffEntry::Equal  && diff[0].seq == QList<function_t>({1, 4, 1, 4, 1}));
			assert(diff[1].op == DiffEntry::Insert && diff[1].seq == QList<function_t>({2}));
			assert(diff[2].op == DiffEntry::Equal  && diff[2].seq == QList<function_t>({1, 1}));
			assert(diff[3].op == DiffEntry::Delete && diff[3].seq == QList<function_t>({3}));
			assert(diff[4].op == DiffEntry::Insert && diff[4].seq == QList<function_t>({2}));
			assert(diff[5].op == DiffEntry::Equal  && diff[5].seq == QList<function_t>({1, 1}));
			assert(diff[6].op == DiffEntry::Delete && diff[6].seq == QList<function_t>({2}));

			assert(Diff_minScore(diff)           ==  0);
			assert(Diff_score(diff)              ==  9);
			assert(Diff_maxAchievableScore(diff) == 11);
			assert(Diff_maxScore(diff)           == 11);

			assert(treeDiff.size() == 7);
			assert(treeDiff[0].subDiffs.size() == 1);                   //  1
			assert(treeDiff[0].subDiffs[0].size() == 1);
			assert(treeDiff[0].subDiffs[0][0].subDiffs.size() == 2);    //    4,  4
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0].size() == 0);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1].size() == 0);
			assert(treeDiff[1].subDiffs.size() == 1);                   // +2
			assert(treeDiff[1].subDiffs[0].size() == 0);
			assert(treeDiff[2].subDiffs.size() == 2);                   //  1,  1
			assert(treeDiff[2].subDiffs[0].size() == 0);
			assert(treeDiff[2].subDiffs[1].size() == 0);
			assert(treeDiff[3].subDiffs.size() == 1);                   // -3
			assert(treeDiff[3].subDiffs[0].size() == 0);
			assert(treeDiff[4].subDiffs.size() == 1);                   // +2
			assert(treeDiff[4].subDiffs[0].size() == 0);
			assert(treeDiff[5].subDiffs.size() == 2);                   //  1,  1
			assert(treeDiff[5].subDiffs[0].size() == 0);
			assert(treeDiff[5].subDiffs[1].size() == 0);
			assert(treeDiff[6].subDiffs.size() == 1);                   // -2
			assert(treeDiff[6].subDiffs[0].size() == 0);

			assert(treeDiff[0].op == DiffEntry::Equal  && treeDiff[0].seq == QList<function_t>({1}));
			assert(treeDiff[0].subDiffs[0][0].op == DiffEntry::Equal && treeDiff[0].subDiffs[0][0].seq == QList<function_t>({4, 4}));
			assert(treeDiff[1].op == DiffEntry::Insert && treeDiff[1].seq == QList<function_t>({2}));
			assert(treeDiff[2].op == DiffEntry::Equal  && treeDiff[2].seq == QList<function_t>({1, 1}));
			assert(treeDiff[3].op == DiffEntry::Delete && treeDiff[3].seq == QList<function_t>({3}));
			assert(treeDiff[4].op == DiffEntry::Insert && treeDiff[4].seq == QList<function_t>({2}));
			assert(treeDiff[5].op == DiffEntry::Equal  && treeDiff[5].seq == QList<function_t>({1, 1}));
			assert(treeDiff[6].op == DiffEntry::Delete && treeDiff[6].seq == QList<function_t>({2}));

			assert(TreeDiff_score(treeDiff) == 9);
		} else if (traceFileName.contains("d.otf")) {
			assert(diff.size() == 2);
			assert(diff[0].op == DiffEntry::Delete && diff[0].seq == QList<function_t>({1, 2, 1, 2, 1}));
			assert(diff[1].op == DiffEntry::Insert && diff[1].seq == QList<function_t>({3}));

			assert(Diff_minScore(diff)           == 0);
			assert(Diff_score(diff)              == 0);
			assert(Diff_maxAchievableScore(diff) == 1);
			assert(Diff_maxScore(diff)           == 5);

			assert(treeDiff.size() == 2);
			assert(treeDiff[0].subDiffs.size() == 1);
			assert(treeDiff[0].subDiffs[0].size() == 1);
			assert(treeDiff[0].subDiffs[0][0].subDiffs.size() == 2);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[0].size() == 0);
			assert(treeDiff[0].subDiffs[0][0].subDiffs[1].size() == 0);
			assert(treeDiff[1].subDiffs.size() == 1);
			assert(treeDiff[1].subDiffs[0].size() == 0);

			assert(treeDiff[0].op == DiffEntry::Delete && treeDiff[0].seq == QList<function_t>({1}));
			assert(treeDiff[0].subDiffs[0][0].op == DiffEntry::Delete && treeDiff[0].subDiffs[0][0].seq == QList<function_t>({2, 2}));
			assert(treeDiff[1].op == DiffEntry::Insert && treeDiff[1].seq == QList<function_t>({3}));

			assert(TreeDiff_score(treeDiff) == 0);
		} else if (traceFileName.contains("e.otf")) {
			assert(diff.size() == 5);
			assert(diff[0].op == DiffEntry::Delete && diff[0].seq == QList<function_t>({1}));
			assert(diff[1].op == DiffEntry::Equal  && diff[1].seq == QList<function_t>({2, 4, 5, 4}));
			assert(diff[2].op == DiffEntry::Delete && diff[2].seq == QList<function_t>({5, 4}));
			assert(diff[3].op == DiffEntry::Equal  && diff[3].seq == QList<function_t>({2, 4, 2}));
			assert(diff[4].op == DiffEntry::Insert && diff[4].seq == QList<function_t>({3}));

			assert(Diff_minScore(diff)           ==  0);
			assert(Diff_score(diff)              ==  7);
			assert(Diff_maxAchievableScore(diff) ==  8);
			assert(Diff_maxScore(diff)           == 10);

			assert(treeDiff.size() == 3);
			assert(treeDiff[0].subDiffs.size() == 1);                                   // -1
			assert(treeDiff[0].subDiffs[0].size() == 0);
			assert(treeDiff[1].subDiffs.size() == 1);                                   //  2
			assert(treeDiff[1].subDiffs[0].size() == 1);
			assert(treeDiff[1].subDiffs[0][0].subDiffs.size() == 2);                    //    4
			assert(treeDiff[1].subDiffs[0][0].subDiffs[0].size() == 2);
			assert(treeDiff[1].subDiffs[0][0].subDiffs[0][0].subDiffs.size() == 1);     //       5
			assert(treeDiff[1].subDiffs[0][0].subDiffs[0][0].subDiffs[0].size() == 0);
			assert(treeDiff[1].subDiffs[0][0].subDiffs[0][1].subDiffs.size() == 1);     //      -5
			assert(treeDiff[1].subDiffs[0][0].subDiffs[0][1].subDiffs[0].size() == 0);
			assert(treeDiff[1].subDiffs[0][0].subDiffs[1].size() == 0);
			assert(treeDiff[2].subDiffs.size() == 1);                                   // +3
			assert(treeDiff[2].subDiffs[0].size() == 0);

			assert(treeDiff[0].op == DiffEntry::Delete && treeDiff[0].seq == QList<function_t>({1}));
			assert(treeDiff[1].op == DiffEntry::Equal  && treeDiff[1].seq == QList<function_t>({2}));
			assert(treeDiff[1].subDiffs[0][0].subDiffs[0][0].op == DiffEntry::Equal && treeDiff[1].subDiffs[0][0].subDiffs[0][0].seq == QList<function_t>({5}));
			assert(treeDiff[1].subDiffs[0][0].subDiffs[0][1].op == DiffEntry::Delete && treeDiff[1].subDiffs[0][0].subDiffs[0][1].seq == QList<function_t>({5}));
			assert(treeDiff[1].subDiffs[0][0].op == DiffEntry::Equal && treeDiff[1].subDiffs[0][0].seq == QList<function_t>({4, 4}));
			assert(treeDiff[2].op == DiffEntry::Insert && treeDiff[2].seq == QList<function_t>({3}));

			assert(TreeDiff_score(treeDiff) == 7);
		}
	}

	anchorPointsTest();

	return 0;
}

static void anchorPointsTest() {
	TreeNode a; a.id = 1;
	TreeNode b; b.id = 2;
	TreeNode c; c.id = 3;
	TreeNode d; d.id = 4;

	FlatSequence s1({   1, 2, 3, 4    });
	FlatSequence s2({1, 1, 2, 3, 4    });
	FlatSequence s3({   1, 2, 3, 4, 4 });

	FlatSequence s4({1, 2, 1, 3, 1, 2, 1});
	FlatSequence s5({1, 2, 1            });

	QList<QPair<int, int>> p00 = {                              }; // s1 s1 equal
	QList<QPair<int, int>> p01 = {{0, 0}, {1, 1}, {2, 2}, {3, 3}}; // s1 s1 equal
	QList<QPair<int, int>> p02 = {{0, 0},         {2, 2}        }; // s1 s1 equal
	QList<QPair<int, int>> p03 = {        {1, 1}, {2, 2}        }; // s1 s1 equal
	QList<QPair<int, int>> p04 = {        {1, 1},         {3, 3}}; // s1 s1 equal

	QList<QPair<int, int>> p05 = {{0, 0}}; // s1 s2
	QList<QPair<int, int>> p06 = {{0, 1}}; // s1 s2
	QList<QPair<int, int>> p07 = {{3, 3}}; // s1 s3
	QList<QPair<int, int>> p08 = {{3, 4}}; // s1 s3

	QList<QPair<int, int>> p09 = {{0, 2}        }; // s1 s1 wrong anchors
	QList<QPair<int, int>> p10 = {{3, 0}        }; // s1 s1 wrong anchors
	QList<QPair<int, int>> p11 = {{1, 0}, {3, 1}}; // s1 s1 wrong anchors

	QList<QPair<int, int>> p12 = {{0, 0}, {2, 2}}; // s4 s5
	QList<QPair<int, int>> p13 = {{2, 0}, {4, 2}}; // s4 s5
	QList<QPair<int, int>> p14 = {{4, 0}, {6, 2}}; // s4 s5

	Diff d00 = {DiffEntry(DiffEntry::Equal, {1, 2, 3, 4})};
	Diff d05 = {DiffEntry(DiffEntry::Equal, {1}), DiffEntry(DiffEntry::Insert, {1}), DiffEntry(DiffEntry::Equal, {2, 3, 4})};
	Diff d06 = {DiffEntry(DiffEntry::Insert, {1}), DiffEntry(DiffEntry::Equal, {1, 2, 3, 4})};

	Diff d07 = {DiffEntry(DiffEntry::Equal, {1, 2, 3, 4}), DiffEntry(DiffEntry::Insert, {4})}; // s1 s3
	Diff d08 = {DiffEntry(DiffEntry::Equal, {1, 2, 3}), DiffEntry(DiffEntry::Insert, {4}), DiffEntry(DiffEntry::Equal, {4})};

	Diff d09 = {DiffEntry(DiffEntry::Delete, {1, 2, 3}), DiffEntry(DiffEntry::Insert, {1, 2, 3}), DiffEntry(DiffEntry::Equal, {4})};
	Diff d10 = {DiffEntry(DiffEntry::Delete, {1, 2, 3, 4}), DiffEntry(DiffEntry::Insert, {1, 2, 3, 4})};

	Diff d12 = {DiffEntry(DiffEntry::Equal, {1, 2, 1}), DiffEntry(DiffEntry::Delete, {3, 1, 2, 1})};
	Diff d13 = {DiffEntry(DiffEntry::Delete, {1, 2}), DiffEntry(DiffEntry::Equal, {1}), DiffEntry(DiffEntry::Delete, {3}), DiffEntry(DiffEntry::Insert, {2}), DiffEntry(DiffEntry::Equal, {1}), DiffEntry(DiffEntry::Delete, {2, 1})};
	Diff d14 = {DiffEntry(DiffEntry::Delete, {1, 2, 1, 3}), DiffEntry(DiffEntry::Equal, {1, 2, 1})};

	struct AnchorCase {
		FlatSequence a;
		FlatSequence b;
		QList<QPair<int, int>> p;
		Diff d;
	};

	QList<AnchorCase> cases = {
			AnchorCase({s1, s1, p00, d00}),
			AnchorCase({s1, s1, p01, d00}),
			AnchorCase({s1, s1, p02, d00}),
			AnchorCase({s1, s1, p03, d00}),
			AnchorCase({s1, s1, p04, d00}),

			AnchorCase({s1, s2, p05, d05}),
			AnchorCase({s1, s2, p06, d06}),
			AnchorCase({s1, s3, p07, d07}),
			AnchorCase({s1, s3, p08, d08}),

			AnchorCase({s1, s1, p09, d09}),
			AnchorCase({s1, s1, p10, d10}),
			AnchorCase({s1, s1, p11, d10}),

			AnchorCase({s4, s5, p12, d12}),
			AnchorCase({s4, s5, p13, d13}),
			AnchorCase({s4, s5, p14, d14})
	};

	foreach (const AnchorCase& c, cases) {
		Diff d;

		// qerr << "tree sequence 1:    " << FlatSequence_print(c.a);
		// qerr << "tree sequence 2:    " << FlatSequence_print(c.b);
		// qerr << "with anchor points: " << AnchorPoints_print(c.p) << "\n";
		// qerr << "compared should be: " << Diff_print(c.d);
		// qerr << "\n";
		// qerr.flush();

		FlatSequence_compare2(c.a, c.b, c.p, &d);

		if (d != c.d) {
			qerr << "error:\n";
			qerr << "tree sequence 1:    " << FlatSequence_print(c.a);
			qerr << "tree sequence 2:    " << FlatSequence_print(c.b);
			qerr << "with anchor points: " << AnchorPoints_print(c.p) << "\n";
			qerr << "compared should be: " << Diff_print(c.d);
			qerr << "not:                " << Diff_print(d);
			qerr << "\n";
			qerr.flush();

			assert(d == c.d);
		}
	}
}
