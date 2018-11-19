#include "diff/diff.hpp"

static QString Diff_operationToString(const DiffEntry::Operation& op);

/* *** Diff **************************************************************** */
QString Diff_print(const Diff& diff, const QMap<function_t, QString>& functionNames) {
	Q_UNUSED(functionNames);

	QString ret;
	QTextStream s(&ret);

	s << "[";
	for (int i = 0; i < diff.size(); i += 1) {
		for (int j = 0; j < diff[i].seq.size(); j += 1) {
			s << Diff_operationToString(diff[i].op) << diff[i].seq[j];
			if (i < diff.size() - 1 || j < diff[i].seq.size() - 1) { s << ", "; }
		}
	}
	s << "]";

	return ret;
}

// static QString DiffEntry_print(const DiffEntry& e, const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), const QString& indent = "") {
// 	Diff d;
// 	d.append(e);
// 	return Diff_print(d, functionNames, indent);
// }

int Diff_minScore(const Diff& d) {
	/* only correct for matchScore = 1, diffScore = 0 */
	Q_UNUSED(d);
	return 0;
}

static void Diff_sequenceLengths(const Diff& d, int* aLen, int* bLen) {
	*aLen = 0;
	*bLen = 0;

	foreach (const DiffEntry& e, d) {
		if (e.op == DiffEntry::Delete) {
			*aLen += e.seq.size();
		} else if (e.op == DiffEntry::Insert) {
			*bLen += e.seq.size();
		} else { /* e.op == DiffEntry::Equal */
			*aLen += e.seq.size();
			*bLen += e.seq.size();
		}
	}
}

int Diff_maxAchievableScore(const Diff& d) {
	/* only correct for matchScore = 1, diffScore = 0 */
	int aLen = 0;
	int bLen = 0;
	Diff_sequenceLengths(d, &aLen, &bLen);
	return std::min(aLen, bLen);
}

int Diff_maxScore(const Diff& d) {
	/* only correct for matchScore = 1, diffScore = 0 */
	int aLen = 0;
	int bLen = 0;
	Diff_sequenceLengths(d, &aLen, &bLen);
	return std::max(aLen, bLen);
}

int Diff_score(const Diff& diff) {
	const int matchScore = 1;
	const int diffScore  = 0;
	int score = 0;
	for (int i = 0; i < diff.size(); i += 1) {
		if (diff[i].op == DiffEntry::Delete || diff[i].op == DiffEntry::Insert) {
			if (i + 1 < diff[i].seq.size()) {
				if ((diff[i].op == DiffEntry::Delete && diff[i+1].op == DiffEntry::Insert) || (diff[i].op == DiffEntry::Insert && diff[i+1].op == DiffEntry::Delete)) {
					score += diffScore * std::max(diff[i].seq.size(), diff[i+1].seq.size());
					i += 1;
				} else { /* no matching operation is next */
					score += diffScore * diff[i].seq.size();
				}
			} else { /* i + 1 >= diff[i].seq.size() */
				score += diffScore * diff[i].seq.size();
			}
		} else { /* diff[i].op == DiffEntry::Equal */
			score += matchScore * diff[i].seq.size();
		}
	}
	return score;
}

/* *** Flat **************************************************************** */

QString FlatSequence_print(const FlatSequence& sequence, const QMap<function_t, QString>& functionNames) {
	Q_UNUSED(functionNames);

	QString ret;
	QTextStream s(&ret);

	s << "[";
	for (int i = 0; i < sequence.size(); i += 1) {
		s << sequence[i];
		if (i < sequence.size() - 1) { s << ", "; }
	}
	s << "]";

	return ret;
}

static void FlatSequence_compareRecursively(const FlatSequence& a_, const FlatSequence& b_, Diff* diff) {

	auto split = [](const FlatSequence& a, const FlatSequence& b, int aPos, int bPos, Diff* diff) {
		auto aLeft = a.mid(0, aPos);
		auto bLeft = b.mid(0, bPos);
		auto aRight = (aPos < a.size()) ? a.mid(aPos, -1) : FlatSequence();
		auto bRight = (bPos < b.size()) ? b.mid(bPos, -1) : FlatSequence();

		FlatSequence_compareRecursively(aLeft, bLeft, diff);
		FlatSequence_compareRecursively(aRight, bRight, diff);
	};

	auto commonPrefixLength = [](const FlatSequence& a, const FlatSequence& b) {
		for (int i = 0; i < std::min(a.size(), b.size()); i += 1) {
			if (a[i] != b[i]) { return i; }
		}
		return std::min(a.size(), b.size());
	};

	auto commonSuffixLength = [](const FlatSequence& a, const FlatSequence& b) {
		for (int i = 1; i <= std::min(a.size(), b.size()); i += 1) {
			if (a[a.size() - i] != b[b.size() - i]) { return i - 1; }
		}
		return std::min(a.size(), b.size());
	};

	auto appendDiff = [](Diff *diff_, const DiffEntry& e) {
		if (e.seq.size() > 0) { diff_->append(e); }
	};

	assert(diff != nullptr);

	FlatSequence a = a_, b = b_;

	// trim common prefix
	int commonPrefixLength_   = commonPrefixLength(a, b);
	FlatSequence commonPrefix = a.mid(0, commonPrefixLength_);
	a = a.mid(commonPrefixLength_);
	b = b.mid(commonPrefixLength_);
	appendDiff(diff, DiffEntry(DiffEntry::Equal, commonPrefix));

	// trim common suffix
	int commonSuffixLength_   = commonSuffixLength(a, b);
	FlatSequence commonSuffix = a.mid(a.size() - commonSuffixLength_, -1);
	a = a.mid(0, a.size() - commonSuffixLength_);
	b = b.mid(0, b.size() - commonSuffixLength_);

	// ronny does not understand the following code. copied/adapted from code.google.com/p/google-diff-match-patch

	const int max_d    = (a.size() + b.size() + 1) / 2;
	const int v_offset = max_d;
	const int v_length = 2 * max_d;

	auto v1 = new int[v_length];
	auto v2 = new int[v_length];
	for (int i = 0; i < v_length; i += 1) {
		v1[i] = -1;
		v2[i] = -1;
	}
	const bool collision = ((a.size() - b.size()) % 2 != 0); // if the difference in sequence length is odd, the front path will collide with the reverse path
	// offsets for start and end of k loop prevents mapping of space beyond the grid
	int k1start = 0;
	int k1end = 0;
	int k2start = 0;
	int k2end = 0;

	if (a.size() == 0 || b.size() == 0) {
		if (a.size() == 0 && b.size() > 0) {
			appendDiff(diff, DiffEntry(DiffEntry::Insert, b));
		} else if (a.size() > 0 && b.size() == 0) {
			appendDiff(diff, DiffEntry(DiffEntry::Delete, a));
		}
		goto appendCommonSuffix;
	} else if (a.size() == 1 && b.size() == 1) {
		if (a[0] == b[0]) {
			appendDiff(diff, DiffEntry(DiffEntry::Equal, a));
		} else {
			appendDiff(diff, DiffEntry(DiffEntry::Delete, a));
			appendDiff(diff, DiffEntry(DiffEntry::Insert, b));
		}
		goto appendCommonSuffix;
	}

	v1[v_offset + 1] = 0;
	v2[v_offset + 1] = 0;

	for (int d = 0; d < max_d; d += 1) {
		// walk the front path one step
		for (int k1 = -d + k1start; k1 <= d - k1end; k1 += 2) {
			const int k1_offset = v_offset + k1;
			int x1;
			if (k1 == -d || (k1 != d && v1[k1_offset - 1] < v1[k1_offset + 1])) {
				x1 = v1[k1_offset + 1];
			} else {
				x1 = v1[k1_offset - 1] + 1;
			}
			int y1 = x1 - k1;
			while (x1 < a.size() && y1 < b.size() && a[x1] == b[y1]) {
				x1 += 1;
				y1 += 1;
			}
			v1[k1_offset] = x1;
			if (x1 > a.size()) {
				// ran off the right of the graph
				k1end += 2;
			} else if (y1 > b.size() ){
				// ran off the bottom of the graph
				k1start += 2;
			} else if (collision == true) {
				const int k2_offset = v_offset + (a.size() - b.size()) - k1;
				if ( k2_offset >= 0 && k2_offset < v_length && v2[k2_offset] != -1 ) {
					// mirror x2 onto top-left coordinate system
					const int x2 = a.size() - v2[k2_offset];
					if (x1 >= x2) {
						// overlap detected
						split(a, b, x1, y1, diff);
						goto appendCommonSuffix;
					}
				}
			}
		}

		// walk the reverse path one step
		for (int k2 = -d + k2start; k2 <= d - k2end; k2 += 2) {
			const int k2_offset = v_offset + k2;
			int x2;
			if (k2 == -d || (k2 != d && v2[k2_offset - 1] < v2[k2_offset + 1])) {
				x2 = v2[k2_offset + 1];
			} else {
				x2 = v2[k2_offset - 1] + 1;
			}
			int y2 = x2 - k2;
			while (x2 < a.size() && y2 < b.size() && a[a.size() - x2 - 1] == b[b.size() - y2 - 1]) {
				x2 += 1;
				y2 += 1;
			}
			v2[k2_offset] = x2;
			if (x2 > a.size()) {
				// ran off the left of the graph
				k2end += 2;
			} else if (y2 > b.size()) {
				// ran off the top of the graph
				k2start += 2;
			} else if (collision == false) {
				const int k1_offset = v_offset + (a.size() - b.size()) - k2;
				if (k1_offset >= 0 && k1_offset < v_length && v1[k1_offset] != -1) {
					const int x1 = v1[k1_offset];
					const int y1 = v_offset + x1 - k1_offset;
					// mirror x2 onto top-left coordinate system
					x2 = a.size() - x2;
					if (x1 >= x2) {
						// overlap detected
						split(a, b, x1, y1, diff);
						goto appendCommonSuffix;
					}
				}
			}
		}
	}

	// a and b have nothing in common
	appendDiff(diff, DiffEntry(DiffEntry::Delete, a));
	appendDiff(diff, DiffEntry(DiffEntry::Insert, b));

	appendCommonSuffix:
	appendDiff(diff, DiffEntry(DiffEntry::Equal, commonSuffix));

	delete[] v1;
	delete[] v2;
}

void FlatSequence_compare2(const FlatSequence& a, const FlatSequence& b, QList<QPair<int, int>> anchorPoints, Diff* diff) {

	assert(diff!= nullptr);
	assert(diff->size() == 0);

	auto appendSubDiff = [](const FlatSequence& a, const FlatSequence& b, int aFrom, int bFrom, int aTo, int bTo, Diff* diff) {

		Diff dSub;
		FlatSequence aSub = a.mid(aFrom, aTo - aFrom + 1);
		FlatSequence bSub = b.mid(bFrom, bTo - bFrom + 1);

		FlatSequence_compareRecursively(aSub, bSub, &dSub);

		diff->append(dSub);
	};

	auto appendAnchor = [](const FlatSequence& a, const FlatSequence& b, int ap, int bp, Diff* diff) {
		if (a[ap] == b[bp]) {
			diff->append(DiffEntry(DiffEntry::Equal, {a[ap]}));
		} else {
			diff->append(DiffEntry(DiffEntry::Delete, {a[ap]}));
			diff->append(DiffEntry(DiffEntry::Insert, {b[bp]}));
		}
	};

	if (anchorPoints.size() > 0) {
		appendSubDiff(a, b, 0, 0, anchorPoints[0].first - 1, anchorPoints[0].second - 1, diff);
		appendAnchor(a, b, anchorPoints[0].first, anchorPoints[0].second, diff);
	} else {
		FlatSequence_compareRecursively(a, b, diff);
	}

	int aFrom, bFrom, aTo, bTo;
	for (int i = 0; i < anchorPoints.size(); i += 1) {

		aFrom = anchorPoints[i].first + 1;
		bFrom = anchorPoints[i].second + 1;

		if (i + 1 < anchorPoints.size()) {
			aTo = anchorPoints[i+1].first - 1;
			bTo = anchorPoints[i+1].second - 1;
		} else {
			aTo = a.size() - 1;
			bTo = b.size() - 1;
		}

		appendSubDiff(a, b, aFrom, bFrom, aTo, bTo, diff);

		if (i + 1 < anchorPoints.size()) {
			appendAnchor(a, b, anchorPoints[i+1].first, anchorPoints[i+1].second, diff);
		}
	}

	// make diff canonical
	bool somethingChanged = true;
	while (somethingChanged == true) {
		somethingChanged = false;
		for (int i = 1; i < diff->size(); i += 1) {
			// unite two adjacent operations that are the same
			if ((*diff)[i-1].op == (*diff)[i].op) {
				(*diff)[i-1].seq.append((*diff)[i].seq);
				diff->removeAt(i);
				somethingChanged = true;
				break;
			}
			// delete is always in front of an adjacent insert
			if ((*diff)[i-1].op == DiffEntry::Insert && (*diff)[i].op == DiffEntry::Delete) {
				DiffEntry tmp = (*diff)[i-1];
				(*diff)[i-1] = (*diff)[i];
				(*diff)[i] = tmp;
				somethingChanged = true;
				break;
			}
		}
	}
}

void FlatSequence_compare(const FlatSequence& a, const FlatSequence& b, Diff* diff) {
	FlatSequence_compare2(a, b, QList<QPair<int, int>>(), diff);
}

void FlatSequence_fromProcessTrace(const ProcessTrace& processTrace, const Unifier<function_t>& u, trace_t t, FlatSequence* sequence) {
	assert(sequence != nullptr);
	assert(sequence->size() == 0);

	std::function<void(const FunctionCall&, const Unifier<function_t>&, trace_t, FlatSequence*)> traverse = [&traverse](const FunctionCall& c, const Unifier<function_t>& u, trace_t t, FlatSequence* sequence) {
		sequence->append(u.map(t, c.id));
		foreach (const FunctionCall& s, c.calls) {
			traverse(s, u, t, sequence);
			sequence->append(u.map(t, c.id));
		}
	};

	foreach (const FunctionCall& f, processTrace) {
		traverse(f, u, t, sequence);
	}
}

void FlatSequence_fromTreeSequence(const TreeSequence& t, FlatSequence* sequence, int depth) {
	assert(sequence != nullptr);
	assert(sequence->size() == 0);

	std::function<void(const TreeNode&, FlatSequence*, int)> traverse = [&traverse](const TreeNode& n, FlatSequence* sequence, int depth) {
		if (depth >= 1) {
			sequence->append(n.id);
		}

		if (depth - 1 >= 1) {
			foreach (const TreeNode* m, n.calls) {
				traverse(*m, sequence, depth - 1);
				sequence->append(n.id);
			}
		}
	};

	foreach (const TreeNode* f, t) {
		traverse(*f, sequence, depth);
	}
}

/* *** TreeDiff *** */
QString TreeDiff_print(const TreeDiff& d, const QMap<function_t, QString>& functionNames, const QString& indent) {
	Q_UNUSED(functionNames);

	QString ret;
	QTextStream s(&ret);

	bool haveChildren = false;
	foreach (const TreeDiffEntry& e, d) {
		if (e.subDiffs.size() > 0) {
			for (int i = 0; i < e.subDiffs.size(); i += 1) {
				haveChildren |= e.subDiffs[i].size() > 0;
			}
		}
	}

	if (haveChildren) {
		foreach (const TreeDiffEntry& e, d) {
			for (int i = 0; i < e.seq.size(); i += 1) {
				s << indent << QString("%1%2").arg(Diff_operationToString(e.op)).arg(e.seq[i]);
				if (e.subDiffs[i].size() > 0) {
					s << ":";

					bool haveChildren2 = false;
					foreach (const TreeDiffEntry& f, e.subDiffs[i]) {
						foreach (const TreeDiff& g, f.subDiffs) {
							haveChildren2 |= g.size() > 0;
						}
					}
					if (haveChildren2) { s << "\n"; }
					else               { s << "\t"; }

					s << TreeDiff_print(e.subDiffs[i], functionNames, indent + "\t");
				} else {
					s << "\n";
				}
			}
		}
	} else {
		for (int i = 0; i < d.size(); i += 1) {
			for (int j = 0; j < d[i].seq.size(); j += 1) {
				s << QString("%1%2").arg(Diff_operationToString(d[i].op)).arg(d[i].seq[j]);
				if (i < d.size() - 1 || j < d[i].seq.size() - 1) { s << ", "; }
			}
		}
	}

	s << "\n";

	return ret;
}

int TreeDiff_minScore(const TreeDiff& d) {
	/* only correct for matchScore = 1, diffScore = 0 */
	Q_UNUSED(d);
	return 0;
}

static void TreeDiff_flatSequenceLengths(const TreeDiff& d, int* aLen, int* bLen) {
	std::function<void(const TreeDiff&, const DiffEntry::Operation&, bool, int*, int*)> traverse = [&traverse](const TreeDiff& d, const DiffEntry::Operation& op, bool isProcessNode, int* aLen, int* bLen) {

		if (isProcessNode == false) {
			if (op == DiffEntry::Delete) { // depending on the previous operation add one
				*aLen += 1;
			} else if (op == DiffEntry::Insert) {
				*bLen += 1;
			} else { /* op == DiffEntry::Equal */
				*aLen += 1;
				*bLen += 1;
			}
		}

		foreach (const TreeDiffEntry& e, d) {
			Q_ASSERT(e.subDiffs.size() == e.seq.size());

			for (int i = 0; i < e.subDiffs.size(); i += 1) {
				traverse(e.subDiffs[i], e.op, false, aLen, bLen);
			}

			if (isProcessNode == false) {
				if (e.op == DiffEntry::Delete) {
					*aLen += e.seq.size();
				} else if (e.op == DiffEntry::Insert) {
					*bLen += e.seq.size();
				} else { /* e.op == DiffEntry::Equal */
					*aLen += e.seq.size();
					*bLen += e.seq.size();
				}
			}
		}
	};

	*aLen = 0;
	*bLen = 0;
	traverse(d, DiffEntry::Equal, true, aLen, bLen);
}

int TreeDiff_maxAchievableScore(const TreeDiff& d) {
	/* only correct for matchScore = 1, diffScore = 0 */
	int aLen = 0;
	int bLen = 0;
	TreeDiff_flatSequenceLengths(d, &aLen, &bLen);
	return std::min(aLen, bLen);
}

int TreeDiff_maxScore(const TreeDiff& d) {
	/* only correct for matchScore = 1, diffScore = 0 */
	int aLen = 0;
	int bLen = 0;
	TreeDiff_flatSequenceLengths(d, &aLen, &bLen);
	return std::max(aLen, bLen);
}

int TreeDiff_score(const TreeDiff& d) {
	/* only correct for matchScore = 1, diffScore = 0 */

	std::function<void(const TreeDiff&, const DiffEntry::Operation&, bool, int*)> traverse = [&traverse](const TreeDiff& d, const DiffEntry::Operation& op, bool isProcessNode, int* score) {

		if (isProcessNode == false) {
			if (op == DiffEntry::Equal) {
				*score += 1;
			}
		}

		for (int i = 0; i < d.size(); i += 1) {
			Q_ASSERT(d[i].subDiffs.size() == d[i].seq.size());

			if (d[i].op != DiffEntry::Equal) {
				if (isProcessNode == false) {
					if (i + 1 < d.size()) {
						if (d[i].op == DiffEntry::Delete && d[i+1].op == DiffEntry::Insert) {
							*score += 1;
							i += 1;
						} else if (d[i].op == DiffEntry::Insert && d[i+1].op == DiffEntry::Delete) {
							*score += 1;
							i += 1;
						}
					}
				}
			} else /* if (d[i].op == DiffEntry::Equal) */ {
				for (int j = 0; j < d[i].subDiffs.size(); j += 1) {
					traverse(d[i].subDiffs[j], d[i].op, false, score);
					if (isProcessNode == false) {
						*score += 1;
					}
				}
			}
		}
	};

	int score = 0;
	traverse(d, DiffEntry::Equal, true, &score);
	return score;
}

/* *** Tree **************************************************************** */

QString TreeSequence_print(const TreeSequence& sequence, const QMap<function_t, QString>& functionNames, const QString& indent) {
	Q_UNUSED(functionNames);

	QString ret;
	QTextStream s(&ret);

	int maximumFunctionIdentifierLength = 0;
	bool haveChildren = false;
	foreach (const TreeNode* n, sequence) {
		haveChildren |= n->calls.size() > 0;
		if (floor(log10(n->id)) + 1 > maximumFunctionIdentifierLength) {
			maximumFunctionIdentifierLength = floor(log10(n->id)) + 1;
		}
	}

	if (haveChildren) {
		foreach (const TreeNode* n, sequence) {
			s << indent << QString("%1").arg(n->id, maximumFunctionIdentifierLength);
			if (n->calls.size() > 0) {
				s << ":";

				bool haveChildren2 = false;
				foreach (const TreeNode* m, n->calls) {
					haveChildren2 |= m->calls.size() > 0;
				}
				if (haveChildren2) { s << "\n"; }
				else               { s << "\t"; }

				s << TreeSequence_print(n->calls, functionNames, indent + "\t");
			} else {
				s << "\n";
			}
		}
	} else {
		for (int i = 0; i < sequence.size(); i += 1) {
			s << QString("%1").arg(sequence[i]->id, maximumFunctionIdentifierLength);
			if (i < sequence.size() - 1) { s << ", "; }
		}
	}

	s << "\n";

	return ret;
}

// // note: longAddedSequenceLengths = -1 means don't cope with the long sequence case
// void TreeSequence_compare(const TreeSequence& a, const TreeSequence& b, int longAddedSequenceLengths, const AnchorPointsFunction& anchorPointsFunction, TreeDiff* diff) {
// 
// 	std::function<void(const TreeSequence&, DiffEntry::Operation, TreeDiff*)> traverseDiff = [&traverseDiff](const TreeSequence& s, DiffEntry::Operation op, TreeDiff* diff) {
// 
// 		if (s.size() == 0) { return; }
// 
// 		diff->append(TreeDiffEntry());
// 		TreeDiffEntry& e = diff->last();
// 		e.op = op;
// 		FlatSequence_fromTreeSequence(s, &e.seq, 1);
// 
// 		foreach (const TreeNode* n, s) {
// 			e.subDiffs.append(TreeDiff());
// 			traverseDiff(n->calls, op, &e.subDiffs.last());
// 		}
// 	};
// 
// 	std::function<void(const TreeSequence&, const TreeSequence&, TreeDiff*)> traverseEqual = [&anchorPointsFunction, &longAddedSequenceLengths, &traverseDiff, &traverseEqual](const TreeSequence& a, const TreeSequence& b, TreeDiff* diff) {
// 
// 		if (a.size() == 0 && b.size() == 0) { return; }
// 
// 		FlatSequence af, bf;
// 		Diff flatDiff;
// 		FlatSequence_fromTreeSequence(a, &af, 1);
// 		FlatSequence_fromTreeSequence(b, &bf, 1);
// 
// 		Q_ASSERT(a.size() == af.size());
// 		Q_ASSERT(b.size() == bf.size());
// 
// 		QList<QPair<int ,int>> anchorPoints;
// 
// 		anchorPointsFunction(a, b, &anchorPoints);
// 
// 		FlatSequence_compare2(af, bf, anchorPoints, &flatDiff);
// 
// 		// qDebug() << FlatSequence_print(af) << FlatSequence_print(bf) << Diff_print(flatDiff) << "{";
// 
// 		int aPos = 0, bPos = 0;
// 		foreach (const DiffEntry& e, flatDiff) {
// 			diff->append(TreeDiffEntry(e));
// 
// 			for (int i = 0; i < e.seq.size(); i += 1) {
// 				diff->last().subDiffs.append(TreeDiff());
// 				if (e.op == DiffEntry::Delete) {
// 					traverseDiff(a[aPos]->calls, DiffEntry::Delete, &diff->last().subDiffs.last());
// 					aPos += 1;
// 				} else if (e.op == DiffEntry::Insert) {
// 					traverseDiff(b[bPos]->calls, DiffEntry::Insert, &diff->last().subDiffs.last());
// 					bPos += 1;
// 				} else /* e.op == DiffEntry::Equal */ {
// 					traverseEqual(a[aPos]->calls, b[bPos]->calls, &diff->last().subDiffs.last());
// 					aPos += 1;
// 					bPos += 1;
// 				}
// 			}
// 		}
// 	};
// 
// 	traverseEqual(a, b, diff); // the base node (the process) is always equal
// }

void TreeSequence_compareSimple(const TreeSequence& a, const TreeSequence& b, TreeDiff* diff) {

	// do not attempt to further compare different entries. just copy them with the given operations.
	std::function<void(const TreeSequence&, DiffEntry::Operation, TreeDiff*)> traverseDiff = [&traverseDiff](const TreeSequence& s, DiffEntry::Operation op, TreeDiff* diff) {

		if (s.size() == 0) { return; }

		diff->append(TreeDiffEntry());
		TreeDiffEntry& e = diff->last();
		e.op = op;
		FlatSequence_fromTreeSequence(s, &e.seq, 1);

		foreach (const TreeNode* n, s) {
			e.subDiffs.append(TreeDiff());
			traverseDiff(n->calls, op, &e.subDiffs.last());
		}
	};

	std::function<void(const TreeSequence&, const TreeSequence&, TreeDiff*)> traverseEqual = [&traverseDiff, &traverseEqual](const TreeSequence& a, const TreeSequence& b, TreeDiff* diff) {

		if (a.size() == 0 && b.size() == 0) { return; }

		FlatSequence af, bf;
		Diff flatDiff;
		FlatSequence_fromTreeSequence(a, &af, 1);
		FlatSequence_fromTreeSequence(b, &bf, 1);

		Q_ASSERT(a.size() == af.size());
		Q_ASSERT(b.size() == bf.size());

		FlatSequence_compare(af, bf, &flatDiff);

		// qDebug() << FlatSequence_print(af) << FlatSequence_print(bf) << Diff_print(flatDiff) << "{";

		int aPos = 0, bPos = 0;
		foreach (const DiffEntry& e, flatDiff) {
			diff->append(TreeDiffEntry(e));

			for (int i = 0; i < e.seq.size(); i += 1) {
				diff->last().subDiffs.append(TreeDiff());
				if (e.op == DiffEntry::Delete) {
					traverseDiff(a[aPos]->calls, DiffEntry::Delete, &diff->last().subDiffs.last());
					aPos += 1;
				} else if (e.op == DiffEntry::Insert) {
					traverseDiff(b[bPos]->calls, DiffEntry::Insert, &diff->last().subDiffs.last());
					bPos += 1;
				} else /* e.op == DiffEntry::Equal */ {
					traverseEqual(a[aPos]->calls, b[bPos]->calls, &diff->last().subDiffs.last());
					aPos += 1;
					bPos += 1;
				}
			}
		}
	};

	traverseEqual(a, b, diff); // the base node (the process) is always equal
}

void TreeSequence_compareSteve(const TreeSequence& before, const TreeSequence& after, int longAddedSequenceLengths, TreeDiff* diff) {
	Q_UNUSED(before);
	Q_UNUSED(after);
	Q_UNUSED(longAddedSequenceLengths);
	Q_UNUSED(diff);
	// TODO
	qerr << "comparing tree sequences according to steve's method has not been implemented, yet. aborting.\n";
	qerr.flush();
	exit(-1);
}

void TreeSequence_compareRonny1(const TreeSequence& before, const TreeSequence& after, int longAddedSequenceLengths, TreeDiff* diff) {
	Q_UNUSED(before);
	Q_UNUSED(after);
	Q_UNUSED(longAddedSequenceLengths);
	Q_UNUSED(diff);
	// TODO
	qerr << "comparing tree sequences according to ronny's method has not been implemented, yet. aborting.\n";
	qerr.flush();
	exit(-1);
}

bool TreeSequence_equals(const TreeSequence& a, const TreeSequence& b) {
	if (a.size() != b.size()) return false;
	for (int i = 0; i < a.size(); i += 1) {
		if (a[i]->id != b[i]->id) { return false; }
		if (TreeSequence_equals(a[i]->calls, b[i]->calls) == false) { return false; }
	}
	return true;
}

void TreeSequence_delete(TreeSequence* s) {
	foreach (TreeNode* n, *s) {
		TreeSequence_delete(&n->calls);
		delete n;
	}
}

void TreeSequence_fromProcessTrace(const ProcessTrace& processTrace, const Unifier<function_t>& u, trace_t t, TreeSequence* sequence) {
	assert(sequence != nullptr);
	assert(sequence->size() == 0);

	foreach (const FunctionCall& c, processTrace) {
		sequence->append(new TreeNode);
		sequence->last()->id = u.map(t, c.id);
		TreeSequence_fromProcessTrace(c.calls, u, t, &sequence->last()->calls);
	}
}

void TreeSequence_extractLongSubsequences(const QString& traceFileName_, const TreeSequence& a, const TreeSequence& b, process_t ap, process_t bp, const QString& an, const QString bn, const QMap<function_t, QString>& functionNames, int minimumAddedSequenceLengths) {

	auto writeTrace = [](const QString& traceFileName, const TreeSequence& a, const TreeSequence& b, process_t ap, process_t bp, const QString& an, const QString& bn, const QMap<function_t, QString>& functionNames_) {

		std::function<void(const TreeSequence&, QSet<function_t>*)> usedFunctionIdentifiers = [&usedFunctionIdentifiers](const TreeSequence& s, QSet<function_t>* f) {
			foreach (const TreeNode* n, s) {
				f->insert(n->id);
				usedFunctionIdentifiers(n->calls, f);
			}
		};

		/* remove unused functions from functionNames */
		QSet<function_t> used;
		usedFunctionIdentifiers(a, &used);
		usedFunctionIdentifiers(b, &used);

		QMap<function_t, QString> functionNames;
		QMapIterator<function_t, QString> i(functionNames_);
		while (i.hasNext()) {
			i.next();
			if (used.contains(i.key())) {
				functionNames.insert(i.key(), i.value());
			}
		}

		/* number identifiers starting from 1 by using the unifier */

		Unifier<function_t> unifier;
		unifier.insert((trace_t) 1, traceFileName, functionNames);
		functionNames = unifier.mappedNames();

		/* write the actual trace by hand. less messy code and trace file than with using the otf api. */
		bool opened = true;
		QFile otfFile(traceFileName + ".otf");                               opened &= otfFile.open(QIODevice::WriteOnly);
		QFile defFile(traceFileName + ".0.def");                             opened &= defFile.open(QIODevice::WriteOnly);
		QFile ev1File(traceFileName + QString(".%1.events").arg(ap, 0, 16)); opened &= ev1File.open(QIODevice::WriteOnly);
		QFile ev2File(traceFileName + QString(".%1.events").arg(bp, 0, 16)); opened &= ev2File.open(QIODevice::WriteOnly);

		if (opened == false) {
			qerr << QString("could not open \"%1\" .otf, .0.def, .1.events, .2.events for writing. aborting").arg(traceFileName); qerr.flush();
			otfFile.remove(); defFile.remove(); ev1File.remove(); ev2File.remove();
			assert(false);
		}

		QTextStream otf(&otfFile);
		QTextStream def(&defFile);
		QTextStream ev1(&ev1File);
		QTextStream ev2(&ev2File);

		otf << QString("%1:%1\n").arg(ap, 0, 16);
		otf << QString("%1:%1\n").arg(bp, 0, 16);
		otf.flush(); otfFile.close();

		def << "DTR1\n";
		def << QString("DP%1NM\"%2\"\n").arg(ap, 0, 16).arg(an);
		def << QString("DP%1NM\"%2\"\n").arg(bp, 0, 16).arg(bn);
		def << "DFG3e8NM\"Application\"\n"; // 1000 = 0x3e8 is the usual "Application" function group identifier. Could be changed I guess.

		QMapIterator<function_t, QString> j(functionNames);
		while (j.hasNext()) {
			j.next();
			def << QString("DF%1G3e8NM\"%2\"\n").arg(j.key(), 0, 16).arg(j.value());
		}
		def.flush(); defFile.close();

		ev1 << "0\n";
		ev1 << QString("*%1\n").arg(ap, 0, 16);
		ev2 << "0\n";
		ev2 << QString("*%2\n").arg(bp, 0, 16);

		std::function<void(const TreeSequence&, process_t, int64_t*, const Unifier<function_t>&, QTextStream*)> traverse = [&traverse](const TreeSequence& seq, process_t p, int64_t* t, const Unifier<function_t>& unifier, QTextStream* s) {
			foreach(const TreeNode* n, seq) {
				*s << QString("%1\n").arg(*t, 0, 16);
				*s << QString("*%1\n").arg(p, 0, 16);
				*s << QString("E%1\n").arg(unifier.map((trace_t) 1, n->id), 0, 16);
				*t += 1;

				traverse(n->calls, p, t, unifier, s);

				*s << QString("%1\n").arg(*t, 0, 16);
				*s << QString("*%1\n").arg(p, 0, 16);
				*s << "L\n";
				*t += 1;
			}
		};

		int64_t t = 1;
		traverse(a, ap, &t, unifier, &ev1);
		t = 1;
		traverse(b, bp, &t, unifier, &ev2);

		ev1.flush(); ev1File.close();
		ev2.flush(); ev2File.close();
	};

	// adapted from TreeSequence_compareSimple
	std::function<void(const TreeSequence&, const TreeSequence&, int, int*, const QString&, process_t, process_t, const QString&, const QString&, const QMap<function_t, QString>&)> compare = [&compare, &writeTrace](const TreeSequence& a, const TreeSequence& b, int minimumAddedSequenceLengths, int* currentExtract, const QString& traceFileName, process_t ap, process_t bp, const QString& an, const QString& bn, const QMap<function_t, QString>& functionNames) {

		if (a.size() == 0 && b.size() == 0) { return; }

		FlatSequence af, bf;
		Diff flatDiff;
		FlatSequence_fromTreeSequence(a, &af, 1);
		FlatSequence_fromTreeSequence(b, &bf, 1);
		FlatSequence_compare(af, bf, &flatDiff);

		if (af.size() + bf.size() >= minimumAddedSequenceLengths) {
			writeTrace(traceFileName + QString("-extract-p%1-p%2-%3-%4-%5").arg(ap).arg(bp).arg(*currentExtract).arg(af.size()).arg(bf.size()), a, b, ap, bp, an, bn, functionNames);
			*currentExtract += 1;
		}

		int aPos = 0, bPos = 0;
		foreach (const DiffEntry& e, flatDiff) {
			for (int i = 0; i < e.seq.size(); i += 1) {
				if (e.op == DiffEntry::Delete) {
					aPos += 1;
				} else if (e.op == DiffEntry::Insert) {
					bPos += 1;
				} else /* e.op == DiffEntry::Equal */ {
					compare(a[aPos]->calls, b[bPos]->calls, minimumAddedSequenceLengths, currentExtract, traceFileName, ap, bp, an, bn, functionNames);
					aPos += 1;
					bPos += 1;
				}
			}
		}
	};

	/* strip .otf or .otf2 suffix */
	QString traceFileName = traceFileName_;

	if (traceFileName.endsWith(".otf") == true) {
		traceFileName = traceFileName.left(traceFileName.size() - 4);
	} else if (traceFileName.endsWith(".otf2") == true) {
		traceFileName = traceFileName.left(traceFileName.size() - 5);
	}

	int currentExtract = 1; // number of the extract determines the trace file name later

	compare(a, b, minimumAddedSequenceLengths, &currentExtract, traceFileName, ap, bp, an, bn, functionNames);
}

/* *** anchor points *** *************************************************** */

QString AnchorPoints_print(const QList<QPair<int, int>>& a) {
	QString ret;
	QTextStream s(&ret);

	for (int i = 0; i < a.size(); i += 1) {
		s << QString("(%1, %2)").arg(a[i].first).arg(a[i].second);
		if (i + 1 < a.size()) { s << ", "; }
	}

	return ret;
}

/* *** misc *** ************************************************************ */

static QString Diff_operationToString(const DiffEntry::Operation& op) {
	QString ret;
	if (op == DiffEntry::Delete)      { ret = "-"; }
	else if (op == DiffEntry::Insert) { ret = "+"; }
	else /* op == DiffEntry::Equal */ { ret = " "; }
	return ret;
}

bool TreeNode::operator==(const TreeNode& t) const {
	if (this->id != t.id || this->calls.size() != t.calls.size()) return false;
	for (int i = 0; i < this->calls.size(); i += 1) {
		if ((this->calls[i] == t.calls[i]) == false) {
			return false;
		}
	}
	return true;
}

