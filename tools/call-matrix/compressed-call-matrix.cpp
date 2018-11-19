#include "call-matrix/compressed-call-matrix.hpp"

typedef CompressedCallMatrixCell Cell;

QTextStream& operator<<(QTextStream& s, const CompressedCallMatrixCell& c) {
	s << QString("%1 -> %2").arg(c.from).arg(c.to);
	return s;
}

QString CompressedCallMatrix_print(const CompressedCallMatrix& m, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames, const QString& indent) {
	Q_UNUSED(processNames);
	Q_UNUSED(functionNames);
	return ConceptLattice_print(m, indent);
}

QString CompressedCallMatrix_printAdditional(const CompressedCallMatrix& m, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames, const QString& indent) {
	Q_UNUSED(processNames);
	Q_UNUSED(functionNames);

	QString ret;
	QTextStream s(&ret);

	QSet<function_t> allCallers;
	QSet<function_t> allCallees;

	foreach (QSet<CompressedCallMatrixCell>* set, m.attributeSets) {

		QSet<function_t> differentCallers;
		QSet<function_t> differentCallees;

		foreach (const CompressedCallMatrixCell& c, *set) {
			differentCallers.insert(c.from);
			differentCallees.insert(c.to);
		}

		s << indent << set << ": ";
		s << "different callers " << differentCallers.size();
		s << ", different callees " << differentCallees.size();
		s << ", both " << (differentCallers + differentCallees).size() << "\n";

		allCallers.unite(differentCallers);
		allCallees.unite(differentCallees);
	}

	s << "all: callers " << allCallers.size();
	s << ", callees " << allCallees.size();
	s << ", both " << (allCallers + allCallees).size() << "\n";

	return ret;
}

QString CompressedCallMatrix_toDot(const CompressedCallMatrix& m, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames) {
	Q_UNUSED(processNames);
	Q_UNUSED(functionNames);
	return ConceptLattice_toDot(m);
}

QString CompressedCallMatrix_toCxt(const CompressedCallMatrix& m, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames) {
	Q_UNUSED(processNames);
	Q_UNUSED(functionNames);
	return ConceptLattice_toCxt(m);
}

void CompressedCallMatrix_merge(const CallMatrix& from, process_t process, CompressedCallMatrix* into) {

	auto setFromCallMatrix = [](const CallMatrix& n, QSet<Cell>* set) {
		Q_ASSERT(set->isEmpty());

		QMapIterator<function_t, QMap<function_t, CallMatrixCell>> i(n);
		while (i.hasNext()) {
			i.next();
			function_t from = i.key();

			QMapIterator<function_t, CallMatrixCell> j(i.value());
			while (j.hasNext()) {
				j.next();
				function_t to = j.key();

				set->insert(Cell(from, to));
			}
		}
	};

	assert(into->objectToAttributes.contains(process) == false);

	QSet<Cell> set;
	setFromCallMatrix(from, &set);

	ConceptLattice_addObject(process, set, into);
}

void CompressedCallMatrix_group(const CompressedCallMatrix& m, double minimumSimilarity, QList<QSet<QSet<Cell>*>>* groups) {
	ConceptLattice_group(m, minimumSimilarity, groups);
}

void CompressedCallMatrix_finalize(CompressedCallMatrix* m) {
	ConceptLattice_finalize(m);
}

void CompressedCallMatrix_removeEmptyNodes(CompressedCallMatrix* m) {
	ConceptLattice_removeEmptyNodes(m);
}

void CompressedCallMatrix_delete(CompressedCallMatrix* m) {
	ConceptLattice_delete(m);
}

/* *** low level functions ************************************************* */

/* maybe it is dissallowed to insert new items into a map while iterating over it.
 * if so, FIXME
 */
void CompressedCallMatrix_addTransitiveCells(CallMatrix *m) {

	bool addedSomething = true;
	while (addedSomething) {
		addedSomething = false;

		QMapIterator<function_t, QMap<function_t, CallMatrixCell>> i(*m);
		while (i.hasNext()) {
			i.next();
			function_t from = i.key();

			QMapIterator<function_t, CallMatrixCell> j(i.value());
			while (j.hasNext()) {
				j.next();
				function_t to = j.key();

				if (m->contains(to)) {
					QMapIterator<function_t, CallMatrixCell> k((*m)[to]);
					while (k.hasNext()) {
						k.next();
						function_t to2 = k.key();

						if ((*m)[from].contains(to2) == false) {
							(*m)[from].insert(to2, CallMatrixCell());
							addedSomething = true;
						}
					}
				}
			}
		}
	}
}

double CompressedCallMatrix_simpleSimilarity(QSet<Cell>* a, QSet<Cell>* b, const CompressedCallMatrix& m) {
	return ConceptLattice_simpleConceptSimilarity(a, b, m);
}

double CompressedCallMatrix_subsumptionSimilarity(QSet<Cell>* a, QSet<Cell>* b, const CompressedCallMatrix& m) {
	return ConceptLattice_subsumptionConceptSimilarity(a, b, m);
}

/* *** misc analysis *** *************************************************** */

void CompressedCallMatrix_cells(const CompressedCallMatrix& m, QSet<CompressedCallMatrixCell>* cells) {
	return ConceptLattice_attributes(m, cells);
}
