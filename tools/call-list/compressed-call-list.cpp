#include "call-list/compressed-call-list.hpp"

QString CompressedCallList_print(const CompressedCallList& l, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames, const QString& indent) {
	Q_UNUSED(processNames);
	Q_UNUSED(functionNames);
	return ConceptLattice_print(l, indent);
}

QString CompressedCallList_toDot(const CompressedCallList& l, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames) {
	Q_UNUSED(processNames);
	Q_UNUSED(functionNames);
	return ConceptLattice_toDot(l);
}

QString CompressedCallList_toCxt(const CompressedCallList& l, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames) {
	Q_UNUSED(processNames);
	Q_UNUSED(functionNames);
	return ConceptLattice_toCxt(l);
}

void CompressedCallList_merge(const CallList& from, process_t process, CompressedCallList* into) {
	assert(into->objectToAttributes.contains(process) == false);

	QSet<function_t> set = from.keys().toSet();

	ConceptLattice_addObject(process, set, into);
}

void CompressedCallList_group(const CompressedCallList& l, double minimumSimilarity, QList<QSet<QSet<function_t>*>>* groups) {
	ConceptLattice_group(l, minimumSimilarity, groups);
}

void CompressedCallList_finalize(CompressedCallList* l) {
	ConceptLattice_finalize(l);
}

void CompressedCallList_removeEmptyNodes(CompressedCallList* l) {
	ConceptLattice_removeEmptyNodes(l);
}

void CompressedCallList_delete(CompressedCallList* l) {
	ConceptLattice_delete(l);
}

/* *** low level functions ************************************************* */

double CompressedCallList_simpleSimilarity(QSet<function_t>* a, QSet<function_t>* b, const CompressedCallList& l) {
	return ConceptLattice_simpleConceptSimilarity(a, b, l);
}

double CompressedCallList_subsumptionSimilarity(QSet<function_t>* a, QSet<function_t>* b, const CompressedCallList& l) {
	return ConceptLattice_subsumptionConceptSimilarity(a, b, l);
}

/* *** misc analysis *** *************************************************** */

void CompressedCallList_functions(const CompressedCallList& l, QSet<function_t>* functions) {
	return ConceptLattice_attributes(l, functions);
}
