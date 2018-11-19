#ifndef COMPRESSED_CALL_MATRIX_HPP
#define COMPRESSED_CALL_MATRIX_HPP

#include "call-matrix/call-matrix.hpp"
#include "common/conceptlattice.hpp"

struct CompressedCallMatrixCell {
	function_t from, to;

	CompressedCallMatrixCell(function_t f, function_t t) : from(f), to(t) {}
	bool operator==(const CompressedCallMatrixCell& c) const { return this->from == c.from && this->to == c.to; }
	bool operator<(const CompressedCallMatrixCell& a) const {
		if (this->from < a.from || (this->from == a.from && this->to < a.to)) { return true; }
		else { return false; }
	}
};

typedef ConceptLattice<process_t, CompressedCallMatrixCell> CompressedCallMatrix;

QString CompressedCallMatrix_print(const CompressedCallMatrix& m, const QMap<process_t, QString>& processNames = QMap<process_t, QString>(), const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), const QString& indent = "");

QString CompressedCallMatrix_printAdditional(const CompressedCallMatrix& m, const QMap<process_t, QString>& processNames = QMap<process_t, QString>(), const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), const QString& indent = "");

QString CompressedCallMatrix_toDot(const CompressedCallMatrix& m, const QMap<process_t, QString>& processNames = QMap<process_t, QString>(), const QMap<function_t, QString>& functionNames = QMap<function_t, QString>());

QString CompressedCallMatrix_toCxt(const CompressedCallMatrix& m, const QMap<process_t, QString>& processNames = QMap<process_t, QString>(), const QMap<function_t, QString>& functionNames = QMap<function_t, QString>());

/* note: attributeSubsets are safed redundantly for the sake of merging more matrices later */
void CompressedCallMatrix_merge(const CallMatrix& from, process_t p, CompressedCallMatrix *into);

void CompressedCallMatrix_group(const CompressedCallMatrix& m, double minimumSimilarity, QList<QSet<QSet<CompressedCallMatrixCell>*>>* groups);

/* call this after merging all call matrices */
void CompressedCallMatrix_finalize(CompressedCallMatrix* m);

// note: this breaks the formal concept lattice property that every set of nodes has a unique supremum and infimum
void CompressedCallMatrix_removeEmptyNodes(CompressedCallMatrix* m);

void CompressedCallMatrix_delete(CompressedCallMatrix* m);

/* *** low level functions ************************************************* */

void CompressedCallMatrix_addTransitiveCells(CallMatrix *m);

double CompressedCallMatrix_simpleSimilarity(QSet<CompressedCallMatrixCell>* a, QSet<CompressedCallMatrixCell>* b, const CompressedCallMatrix& m);

double CompressedCallMatrix_subsumptionSimilarity(QSet<CompressedCallMatrixCell>* superset, QSet<CompressedCallMatrixCell>* subset, const CompressedCallMatrix& m);

/* *** misc analysis *** *************************************************** */

void CompressedCallMatrix_cells(const CompressedCallMatrix& m, QSet<CompressedCallMatrixCell>* cells);

/* *** misc *** ************************************************************ */
QTextStream& operator<<(QTextStream& s, const CompressedCallMatrixCell& c);

inline uint qHash(const CompressedCallMatrixCell& c) {
	return (uint) (((c.from & 0x0000ffff) << 16) | (c.to & 0x0000ffff));
}

#endif /* COMPRESSED_CALL_MATRIX_HPP */
