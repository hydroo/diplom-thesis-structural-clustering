#ifndef COMPRESSED_CALL_LIST_HPP
#define COMPRESSED_CALL_LIST_HPP

#include "common/conceptlattice.hpp"
#include "common/prereqs.hpp"
#include "call-list/call-list.hpp"

typedef ConceptLattice<process_t, function_t> CompressedCallList;

QString CompressedCallList_print(const CompressedCallList& l, const QMap<process_t, QString>& processNames = QMap<process_t, QString>(), const QMap<function_t, QString>& functionNames = QMap<function_t, QString>(), const QString& indent = "");

QString CompressedCallList_toDot(const CompressedCallList& l, const QMap<process_t, QString>& processNames = QMap<process_t, QString>(), const QMap<function_t, QString>& functionNames = QMap<function_t, QString>());

QString CompressedCallList_toCxt(const CompressedCallList& l, const QMap<process_t, QString>& processNames = QMap<process_t, QString>(), const QMap<function_t, QString>& functionNames = QMap<function_t, QString>());

/* note: attributeSubsets are safed redundantly for the sake of merging more matrices later */
void CompressedCallList_merge(const CallList& from, process_t p, CompressedCallList *into);

void CompressedCallList_group(const CompressedCallList& l, double minimumSimilarity, QList<QSet<QSet<function_t>*>>* groups);

/* call this after merging all call matrices */
void CompressedCallList_finalize(CompressedCallList* l);

// note: this breaks the formal concept lattice property that every set of nodes has a unique supremum and infimum
void CompressedCallList_removeEmptyNodes(CompressedCallList* l);

void CompressedCallList_delete(CompressedCallList* l);

/* *** low level functions ************************************************* */

double CompressedCallList_simpleSimilarity(QSet<function_t>* a, QSet<function_t>* b, const CompressedCallList& l);

double CompressedCallList_subsumptionSimilarity(QSet<function_t>* a, QSet<function_t>* b, const CompressedCallList& l);

/* *** misc analysis *** *************************************************** */

void CompressedCallList_functions(const CompressedCallList& l, QSet<function_t>* functions);

#endif // COMPRESSED_CALL_LIST_HPP
