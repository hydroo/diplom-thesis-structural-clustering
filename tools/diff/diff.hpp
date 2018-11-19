#ifndef DIFF_HPP
#define DIFF_HPP

#include <functional>

#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"
#include "common/unifier.hpp"

typedef QList<function_t> FlatSequence;

class DiffEntry {
public:
	enum Operation { Delete = -1, Insert = 1, Equal = 0 };

	Operation op;
	QList<function_t> seq;

	DiffEntry() {}
	DiffEntry(Operation o, const QList<function_t>& s) : op(o), seq(s) {}
	virtual ~DiffEntry() {}

	bool operator==(const DiffEntry &d) const { return this->op == d.op && this->seq == d.seq; }
	bool operator!=(const DiffEntry &d) const { return (*this == d) == false; }
};

typedef QList<DiffEntry> Diff;

class TreeDiffEntry;

typedef QList<TreeDiffEntry> TreeDiff;

class TreeDiffEntry : public DiffEntry {
public:
	QList<TreeDiff> subDiffs;

	TreeDiffEntry() : DiffEntry() {}
	explicit TreeDiffEntry(const DiffEntry& e) : DiffEntry(e) {}
};

struct TreeNode;

typedef QList<TreeNode*> TreeSequence;

struct TreeNode {
	function_t id;
	TreeSequence calls;
	bool operator==(const TreeNode& t) const;
};

/* *** Diff *** */
QString Diff_print(const Diff& d, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>());

int Diff_minScore(const Diff& d);
int Diff_score(const Diff& d);              // >= Diff_minScore
int Diff_maxAchievableScore(const Diff& d); // >= Diff_score
int Diff_maxScore(const Diff& d);           // >= Diff_maxScore

/* *** FlatSequence *** */
QString FlatSequence_print(const FlatSequence& f, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>());

void FlatSequence_compare(const FlatSequence& before, const FlatSequence& after, Diff* diff);
void FlatSequence_compare2(const FlatSequence& a, const FlatSequence& b, QList<QPair<int, int>> anchorPoints, Diff* diff);

void FlatSequence_fromProcessTrace(const ProcessTrace& processTrace, const Unifier<function_t>& u, trace_t traceIdentifier, FlatSequence* sequence);
/* depth = INT_MAX, use the whole tree, 1 = use only the direct calls */
void FlatSequence_fromTreeSequence(const TreeSequence& t, FlatSequence* sequence, int depth = INT_MAX);

/* *** TreeDiff *** */
QString TreeDiff_print(const TreeDiff& d, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), const QString& indent = "");

int TreeDiff_minScore(const TreeDiff& d);
int TreeDiff_score(const TreeDiff& d);              // >= TreeDiff_minScore
int TreeDiff_maxAchievableScore(const TreeDiff& d); // >= TreeDiff_score
int TreeDiff_maxScore(const TreeDiff& d);           // >= TreeDiff_maxAchievableScore

/* *** TreeSequence *** */

/* returns a list of indizes for before / after.
 * indizes may be -1 if no matching index is present on the other sequence (gap).
 * an anchor point pair may pair two different function identifiers together (diff).
 */
typedef std::function<void(const TreeSequence& before, const TreeSequence& after, QList<QPair<int, int>>*)> AnchorPointsFunction;

QString TreeSequence_print(const TreeSequence& f, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), const QString& indent = "");

// no anchor points, no attempt at reducing the to-be-diffed length
void TreeSequence_compareSimple(const TreeSequence& before, const TreeSequence& after, TreeDiff* diff);
// always use anchors, use more intelligence when anchors are not sufficient to bring the to-be-diffed length below 'longSequenceLength'
void TreeSequence_compareSteve(const TreeSequence& before, const TreeSequence& after, int longSequenceLength, TreeDiff* diff);
// always use anchors, use more intelligence when anchors are not sufficient to bring the to-be-diffed length below 'longSequenceLength'
void TreeSequence_compareRonny1(const TreeSequence& before, const TreeSequence& after, int longSequenceLength, TreeDiff* diff);

bool TreeSequence_equals(const TreeSequence& a, const TreeSequence& b);

void TreeSequence_delete(TreeSequence* s);

void TreeSequence_fromProcessTrace(const ProcessTrace& processTrace, const Unifier<function_t>& u, trace_t traceIdentifier, TreeSequence* sequence);

// note: uses the simple tree comparison variant
void TreeSequence_extractLongSubsequences(const QString& traceFileName, const TreeSequence& a, const TreeSequence& b, process_t aIdentifier, process_t bIdentifier, const QString& aName, const QString bName, const QMap<function_t, QString>& functionNames, int minimumAddedSequenceLengths);

/* *** anchor points *** */

QString AnchorPoints_print(const QList<QPair<int, int>>& a);

#endif /* DIFF_HPP */
