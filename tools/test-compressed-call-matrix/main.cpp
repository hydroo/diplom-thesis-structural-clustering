#include "common/prereqs.hpp"
#include "call-matrix/call-matrix.hpp"
#include "call-matrix/compressed-call-matrix.hpp"
#include "common/unifier.hpp"

typedef CompressedCallMatrixCell Cell;

int main(int argc, char** args) {
	Q_UNUSED(argc); Q_UNUSED(args);

	QList<QString> traces;
	traces.append(QFileInfo(args[0]).path() + "/a.otf");
	traces.append(QFileInfo(args[0]).path() + "/b.otf");
	traces.append(QFileInfo(args[0]).path() + "/c.otf");
	traces.append(QFileInfo(args[0]).path() + "/d.otf");
	traces.append(QFileInfo(args[0]).path() + "/e.otf");
	traces.append(QFileInfo(args[0]).path() + "/f.otf");
	traces.append(QFileInfo(args[0]).path() + "/g.otf");
	traces.append(QFileInfo(args[0]).path() + "/h.otf");
	traces.append(QFileInfo(args[0]).path() + "/i.otf");
	traces.append(QFileInfo(args[0]).path() + "/j.otf");
	traces.append(QFileInfo(args[0]).path() + "/k.otf");
	traces.append(QFileInfo(args[0]).path() + "/l.otf");

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

		CompressedCallMatrix m;

		foreach (process_t p, processes) {
			ProcessTrace t;
			Otf_processTrace(traceFileName, p, &t);

			CallMatrix n;
			CallMatrix_fromProcessTrace(t, unifier, (trace_t) 1, false, &n);
			CallMatrix_finalize(&n);

			CompressedCallMatrix o;
			CompressedCallMatrix_merge(n, p, &m);
		}

		CompressedCallMatrix_finalize(&m);
		CompressedCallMatrix_removeEmptyNodes(&m);

		// qerr << traceFileName << ":\n";
		// qerr << CompressedCallMatrix_print(m, processNames, functionNames);
		// qerr.flush();

		if (traceFileName.contains("a.otf")) {
			/* simple insertion below
			 *
			 * 1 A                1 A
			 *      + 2 A B  -->  |
			 *                    2 B
			 *
			 */
			assert(m.attributesToObjects.size() == 2);
			assert(m.objectToAttributes.size()  == 2);
			assert(m.attributeSupersets.size()  == 1);
			assert(m.attributeSubsets.size()    == 1);
			assert(m.attributeSets.size()       == 2);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2));

			assert(m.maximumAttributeSet == m.objectToAttributes[2]);
			assert(m.minimumAttributeSet == m.objectToAttributes[1]);

			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 0);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 1);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 0);

			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[1]));
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.objectToAttributes[2]));

			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[1]->contains(Cell(0, 1)));
			assert(m.objectToAttributes[2]->contains(Cell(0, 2)));
		} else if (traceFileName.contains("b.otf")) {
			/* simple insertion above
			 *
			 *                     2 A
			 *         + 2 A  -->  |
			 *  1 A B              1 B
			 *
			 */
			assert(m.attributesToObjects.size() == 2);
			assert(m.objectToAttributes.size()  == 2);
			assert(m.attributeSupersets.size()  == 1);
			assert(m.attributeSubsets.size()    == 1);
			assert(m.attributeSets.size()       == 2);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2));

			assert(m.maximumAttributeSet == m.objectToAttributes[1]);
			assert(m.minimumAttributeSet == m.objectToAttributes[2]);

			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 0);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 0);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 1);

			assert(m.attributeSubsets[m.objectToAttributes[1]].contains(m.objectToAttributes[2]));
			assert(m.attributeSupersets[m.objectToAttributes[2]].contains(m.objectToAttributes[1]));

			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[1]->contains(Cell(0, 2)));
			assert(m.objectToAttributes[2]->contains(Cell(0, 1)));
		} else if (traceFileName.contains("c.otf")) {
			/* simple insert in the middle of a chain
			 *
			 * 2 A                  2 A
			 * |                    |
			 * |      + 3 A B  -->  3 B
			 * |                    |
			 * 1 B C                1 C
			 *
			 */
			assert(m.attributesToObjects.size() == 3);
			assert(m.objectToAttributes.size()  == 3);
			assert(m.attributeSupersets.size()  == 2);
			assert(m.attributeSubsets.size()    == 2);
			assert(m.attributeSets.size()       == 3);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2) && m.objectToAttributes.contains(3));

			assert(m.maximumAttributeSet == m.objectToAttributes[1]);
			assert(m.minimumAttributeSet == m.objectToAttributes[2]);

			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 0);
			assert(m.attributeSubsets[m.objectToAttributes[3]].size()   == 1);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 0);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[3]].size() == 1);

			assert(m.attributeSubsets[m.objectToAttributes[1]].contains(m.objectToAttributes[3]));
			assert(m.attributeSupersets[m.objectToAttributes[3]].contains(m.objectToAttributes[1]));
			assert(m.attributeSubsets[m.objectToAttributes[3]].contains(m.objectToAttributes[2]));
			assert(m.attributeSupersets[m.objectToAttributes[2]].contains(m.objectToAttributes[3]));

			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[3]->size() == 1);
			assert(m.objectToAttributes[2]->contains(Cell(0, 1)));
			assert(m.objectToAttributes[3]->contains(Cell(0, 2)));
			assert(m.objectToAttributes[1]->contains(Cell(0, 3)));
		} else if (traceFileName.contains("d.otf")) {
			/* simple insert in the middle of a chain
			 *
			 * 2 A                2, 4
			 * |                  |
			 * 3 B  + 4 A    -->  3, 5
			 * |    + 5 A B  -->  |
			 * 1 C                1
			 *
			 */
			assert(m.attributesToObjects.size() == 3);
			assert(m.objectToAttributes.size()  == 5);
			assert(m.attributeSupersets.size()  == 2);
			assert(m.attributeSubsets.size()    == 2);
			assert(m.attributeSets.size()       == 3);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2) && m.objectToAttributes.contains(3) && m.objectToAttributes.contains(4) && m.objectToAttributes.contains(5));

			assert(m.maximumAttributeSet == m.objectToAttributes[1]);
			assert(m.minimumAttributeSet == m.objectToAttributes[2]);

			assert(m.objectToAttributes[1] != m.objectToAttributes[2]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[2] == m.objectToAttributes[4]);
			assert(m.objectToAttributes[3] == m.objectToAttributes[5]);

			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 0);
			assert(m.attributeSubsets[m.objectToAttributes[3]].size()   == 1);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 0);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[3]].size() == 1);

			assert(m.attributeSubsets[m.objectToAttributes[1]].toList()[0]   == m.objectToAttributes[3]);
			assert(m.attributeSupersets[m.objectToAttributes[3]].toList()[0] == m.objectToAttributes[1]);
			assert(m.attributeSubsets[m.objectToAttributes[3]].toList()[0]   == m.objectToAttributes[2]);
			assert(m.attributeSupersets[m.objectToAttributes[2]].toList()[0] == m.objectToAttributes[3]);

			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[3]->size() == 1);
			assert(m.objectToAttributes[2]->contains(Cell(0, 1)));
			assert(m.objectToAttributes[3]->contains(Cell(0, 2)));
			assert(m.objectToAttributes[1]->contains(Cell(0, 3)));
		} else if (traceFileName.contains("e.otf")) {
			/* simple horizontal insert
			 *
			 *                      o
			 *                     / \
			 *  1 A  + 2 B -->  A 1   2 B
			 *                     \ /
			 *                      o
			 *
			 */
			assert(m.attributesToObjects.size() == 2);
			assert(m.objectToAttributes.size()  == 2);
			assert(m.attributeSupersets.size()  == 3);
			assert(m.attributeSubsets.size()    == 3);
			assert(m.attributeSets.size()       == 4);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2));

			assert(m.maximumAttributeSet != m.objectToAttributes[1] && m.maximumAttributeSet != m.objectToAttributes[2]);
			assert(m.minimumAttributeSet != m.objectToAttributes[1] && m.minimumAttributeSet != m.objectToAttributes[2]);

			assert(m.objectToAttributes[1] != m.objectToAttributes[2]);

			assert(m.attributeSubsets[m.minimumAttributeSet].size()     == 0);
			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 1);
			assert(m.attributeSubsets[m.maximumAttributeSet].size()     == 2);
			assert(m.attributeSupersets[m.minimumAttributeSet].size()   == 2);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 1);
			assert(m.attributeSupersets[m.maximumAttributeSet].size()   == 0);

			assert(m.attributeSubsets[m.objectToAttributes[1]].contains(m.minimumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.minimumAttributeSet));
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.maximumAttributeSet));
			assert(m.attributeSupersets[m.objectToAttributes[2]].contains(m.maximumAttributeSet));
			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[1]));
			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[2]));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[1]));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[2]));

			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[1]->contains(Cell(0, 1)));
			assert(m.objectToAttributes[2]->contains(Cell(0, 2)));
		} else if (traceFileName.contains("f.otf")) {
			/* simple split insertion
			 *
			 *  1 C                    1 C
			 *  |                     / \
			 *  |    + 3 B C  -->  B 3   2 A
			 *  |                     \ /
			 *  2 A                    o
			 *
			 */
			assert(m.attributesToObjects.size() == 3);
			assert(m.objectToAttributes.size()  == 3);
			assert(m.attributeSupersets.size()  == 3);
			assert(m.attributeSubsets.size()    == 3);
			assert(m.attributeSets.size()       == 4);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2) && m.objectToAttributes.contains(3));

			assert(m.maximumAttributeSet != m.objectToAttributes[1] && m.maximumAttributeSet != m.objectToAttributes[2] && m.maximumAttributeSet != m.objectToAttributes[3]);
			assert(m.minimumAttributeSet == m.objectToAttributes[1]);

			assert(m.objectToAttributes[1] != m.objectToAttributes[2]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[3]);

			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 2);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[3]].size() == 1);
			assert(m.attributeSupersets[m.maximumAttributeSet].size()   == 0);
			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 0);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[3]].size()   == 1);
			assert(m.attributeSubsets[m.maximumAttributeSet].size()     == 2);

			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.objectToAttributes[2]));
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.objectToAttributes[3]));
			assert(m.attributeSupersets[m.objectToAttributes[2]].contains(m.maximumAttributeSet));
			assert(m.attributeSupersets[m.objectToAttributes[3]].contains(m.maximumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[1]));
			assert(m.attributeSubsets[m.objectToAttributes[3]].contains(m.objectToAttributes[1]));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[2]));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[3]));

			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[3]->size() == 1);
			assert(m.maximumAttributeSet->size()   == 0);
			assert(m.objectToAttributes[1]->contains(Cell(0, 3)));
			assert(m.objectToAttributes[2]->contains(Cell(0, 1)));
			assert(m.objectToAttributes[3]->contains(Cell(0, 2)));
		} else if (traceFileName.contains("g.otf")) {
			/* twofold split insertion
			 *
			 *                            o C
			 *                            |\
			 *    1 C D                   | 1 D
			 *    |                       | |
			 *    |      + 3 A C  -->     | |
			 *    |                     A 3 |
			 *    |                        \|
			 *    2 A B                     2 B
			 *
			 */
			assert(m.attributesToObjects.size() == 3);
			assert(m.objectToAttributes.size()  == 3);
			assert(m.attributeSupersets.size()  == 3);
			assert(m.attributeSubsets.size()    == 3);
			assert(m.attributeSets.size()       == 4);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2) && m.objectToAttributes.contains(3));

			assert(m.maximumAttributeSet == m.objectToAttributes[2]);
			assert(m.minimumAttributeSet != m.objectToAttributes[1] && m.minimumAttributeSet != m.objectToAttributes[2] && m.minimumAttributeSet != m.objectToAttributes[3]);

			assert(m.objectToAttributes[1] != m.objectToAttributes[2]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[3]);

			assert(m.attributeSupersets[m.minimumAttributeSet].size()   == 2);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 0);
			assert(m.attributeSupersets[m.objectToAttributes[3]].size() == 1);
			assert(m.attributeSubsets[m.minimumAttributeSet].size()     == 0);
			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 2);
			assert(m.attributeSubsets[m.objectToAttributes[3]].size()   == 1);

			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[1]));
			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[3]));
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.objectToAttributes[2]));
			assert(m.attributeSupersets[m.objectToAttributes[3]].contains(m.objectToAttributes[2]));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[1]));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[3]));
			assert(m.attributeSubsets[m.objectToAttributes[1]].contains(m.minimumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[3]].contains(m.minimumAttributeSet));

			assert(m.minimumAttributeSet->size()   == 1);
			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[3]->size() == 1);
			assert(m.minimumAttributeSet->contains(  Cell(0, 3)));
			assert(m.objectToAttributes[1]->contains(Cell(0, 4)));
			assert(m.objectToAttributes[2]->contains(Cell(0, 2)));
			assert(m.objectToAttributes[3]->contains(Cell(0, 1)));
		} else if (traceFileName.contains("h.otf")) {
			/* approx building on g.otf
			 *
			 *                            o
			 *                            |\
			 *       o                    | \
			 *      / \                 B 3  1 A
			 * B C 2   1 A  + 3 B  -->    |  |
			 *      \ /                   |  |
			 *       o                  C 2  |
			 *                             \ |
			 *                              \|
			 *                               o
			 *
			 */
			assert(m.attributesToObjects.size() == 3);
			assert(m.objectToAttributes.size()  == 3);
			assert(m.attributeSupersets.size()  == 4);
			assert(m.attributeSubsets.size()    == 4);
			assert(m.attributeSets.size()       == 5);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2) && m.objectToAttributes.contains(3));

			assert(m.maximumAttributeSet != m.objectToAttributes[1] && m.maximumAttributeSet != m.objectToAttributes[2] && m.maximumAttributeSet != m.objectToAttributes[3]);
			assert(m.minimumAttributeSet != m.objectToAttributes[1] && m.minimumAttributeSet != m.objectToAttributes[2] && m.minimumAttributeSet != m.objectToAttributes[3]);

			assert(m.objectToAttributes[1] != m.objectToAttributes[2]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[3]);

			assert(m.attributeSupersets[m.minimumAttributeSet].size()   == 2);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[3]].size() == 1);
			assert(m.attributeSupersets[m.maximumAttributeSet].size()   == 0);
			assert(m.attributeSubsets[m.minimumAttributeSet].size()     == 0);
			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[3]].size()   == 1);
			assert(m.attributeSubsets[m.maximumAttributeSet].size()     == 2);

			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[1]));
			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[3]));
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.maximumAttributeSet));
			assert(m.attributeSupersets[m.objectToAttributes[2]].contains(m.maximumAttributeSet));
			assert(m.attributeSupersets[m.objectToAttributes[3]].contains(m.objectToAttributes[2]));
			assert(m.attributeSubsets[m.objectToAttributes[1]].contains(m.minimumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[3]));
			assert(m.attributeSubsets[m.objectToAttributes[3]].contains(m.minimumAttributeSet));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[1]));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[2]));

			assert(m.minimumAttributeSet->size()   == 0);
			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[3]->size() == 1);
			assert(m.maximumAttributeSet->size()   == 0);
			assert(m.objectToAttributes[1]->contains(Cell(0, 1)));
			assert(m.objectToAttributes[2]->contains(Cell(0, 3)));
			assert(m.objectToAttributes[3]->contains(Cell(0, 2)));
		} else if (traceFileName.contains("i.otf")) {
			/* approx building on h.otf
			 *
			 *    o                          o
			 *    |\                        / \
			 *    | \                      /   \
			 * BC 3  1 A  + 4 B D  -->  B o     \
			 *    |  |                    |\     \
			 *    |  |                    | \     \
			 * DE 2  |                  D 4  3 C   1 A
			 *     \ |                     \ |     |
			 *      \|                      \|     |
			 *       o                     E 2     |
			 *                                \    |
			 *                                 \   |
			 *                                  \  |
			 *                                   \ |
			 *                                    \|
			 *                                     o
			 *
			 */
			assert(m.attributesToObjects.size() == 4);
			assert(m.objectToAttributes.size()  == 4);
			assert(m.attributeSupersets.size()  == 6);
			assert(m.attributeSubsets.size()    == 6);
			assert(m.attributeSets.size()       == 7);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2) && m.objectToAttributes.contains(3) && m.objectToAttributes.contains(4));

			assert(m.maximumAttributeSet != m.objectToAttributes[1] && m.maximumAttributeSet != m.objectToAttributes[2] && m.maximumAttributeSet != m.objectToAttributes[3] && m.maximumAttributeSet != m.objectToAttributes[4]);
			assert(m.minimumAttributeSet != m.objectToAttributes[1] && m.minimumAttributeSet != m.objectToAttributes[2] && m.minimumAttributeSet != m.objectToAttributes[3] && m.minimumAttributeSet != m.objectToAttributes[4]);

			assert(m.objectToAttributes[1] != m.objectToAttributes[2]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[4]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[4]);
			assert(m.objectToAttributes[3] != m.objectToAttributes[4]);

			QSet<Cell>* B = m.attributeSubsets[m.objectToAttributes[3]].toList()[0];

			assert(m.attributeSupersets[m.minimumAttributeSet].size()   == 2);
			assert(m.attributeSupersets[B].size()                       == 2);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[3]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[4]].size() == 1);
			assert(m.attributeSupersets[m.maximumAttributeSet].size()   == 0);
			assert(m.attributeSubsets[m.minimumAttributeSet].size()     == 0);
			assert(m.attributeSubsets[B].size()                         == 1);
			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 2);
			assert(m.attributeSubsets[m.objectToAttributes[3]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[4]].size()   == 1);
			assert(m.attributeSubsets[m.maximumAttributeSet].size()     == 2);

			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[1]));
			assert(m.attributeSupersets[m.minimumAttributeSet].contains(B));
			assert(m.attributeSupersets[B].contains(m.objectToAttributes[3]));
			assert(m.attributeSupersets[B].contains(m.objectToAttributes[4]));
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.maximumAttributeSet));
			assert(m.attributeSupersets[m.objectToAttributes[3]].contains(m.objectToAttributes[2]));
			assert(m.attributeSupersets[m.objectToAttributes[4]].contains(m.objectToAttributes[2]));
			assert(m.attributeSupersets[m.objectToAttributes[2]].contains(m.maximumAttributeSet));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[1]));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[2]));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[3]));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[4]));
			assert(m.attributeSubsets[m.objectToAttributes[3]].contains(B));
			assert(m.attributeSubsets[m.objectToAttributes[4]].contains(B));
			assert(m.attributeSubsets[B].contains(m.minimumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[1]].contains(m.minimumAttributeSet));

			assert(m.minimumAttributeSet->size()   == 0);
			assert(B->size()                       == 1);
			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[3]->size() == 1);
			assert(m.objectToAttributes[4]->size() == 1);
			assert(m.maximumAttributeSet->size()   == 0);
			assert(m.objectToAttributes[1]->contains(Cell(0, 1)));
			assert(m.objectToAttributes[2]->contains(Cell(0, 5)));
			assert(m.objectToAttributes[3]->contains(Cell(0, 3)));
			assert(m.objectToAttributes[4]->contains(Cell(0, 4)));
			assert(B->contains(                      Cell(0, 2)));
		} else if (traceFileName.contains("j.otf")) {
			/* approx building on f.otf
			 *
			 *      o                        o
			 *     / \                      / \
			 *  D 3   1 C  + 4 B C  -->  D 3   1 C
			 *     \ /                     |  /|
			 *      2 A                    | / |
			 *                             |/  |
			 *                           A 2   4 B
			 *                              \ /
			 *                               o
			 */
			assert(m.attributesToObjects.size() == 4);
			assert(m.objectToAttributes.size()  == 4);
			assert(m.attributeSupersets.size()  == 5);
			assert(m.attributeSubsets.size()    == 5);
			assert(m.attributeSets.size()       == 6);

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2) && m.objectToAttributes.contains(3) && m.objectToAttributes.contains(4));

			assert(m.maximumAttributeSet != m.objectToAttributes[1] && m.maximumAttributeSet != m.objectToAttributes[2] && m.maximumAttributeSet != m.objectToAttributes[3] && m.maximumAttributeSet != m.objectToAttributes[4]);
			assert(m.minimumAttributeSet != m.objectToAttributes[1] && m.minimumAttributeSet != m.objectToAttributes[2] && m.minimumAttributeSet != m.objectToAttributes[3] && m.minimumAttributeSet != m.objectToAttributes[4]);

			assert(m.objectToAttributes[1] != m.objectToAttributes[2]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[4]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[4]);
			assert(m.objectToAttributes[3] != m.objectToAttributes[4]);

			assert(m.attributeSupersets[m.minimumAttributeSet].size()   == 2);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 2);
			assert(m.attributeSupersets[m.objectToAttributes[3]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[4]].size() == 1);
			assert(m.attributeSupersets[m.maximumAttributeSet].size()   == 0);
			assert(m.attributeSubsets[m.minimumAttributeSet].size()     == 0);
			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[3]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 2);
			assert(m.attributeSubsets[m.objectToAttributes[4]].size()   == 1);
			assert(m.attributeSubsets[m.maximumAttributeSet].size()     == 2);

			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[1]));
			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[3]));
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.objectToAttributes[2]));
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.objectToAttributes[4]));
			assert(m.attributeSupersets[m.objectToAttributes[3]].contains(m.objectToAttributes[2]));
			assert(m.attributeSupersets[m.objectToAttributes[2]].contains(m.maximumAttributeSet));
			assert(m.attributeSupersets[m.objectToAttributes[4]].contains(m.maximumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[1]].contains(m.minimumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[3]].contains(m.minimumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[1]));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[3]));
			assert(m.attributeSubsets[m.objectToAttributes[4]].contains(m.objectToAttributes[1]));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[2]));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[4]));

			assert(m.minimumAttributeSet->size()   == 0);
			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			assert(m.objectToAttributes[3]->size() == 1);
			assert(m.objectToAttributes[4]->size() == 1);
			assert(m.maximumAttributeSet->size()   == 0);
			assert(m.objectToAttributes[1]->contains(Cell(0, 3)));
			assert(m.objectToAttributes[2]->contains(Cell(0, 1)));
			assert(m.objectToAttributes[3]->contains(Cell(0, 4)));
			assert(m.objectToAttributes[4]->contains(Cell(0, 2)));
		} else if (traceFileName.contains("k.otf")) {
			/* approx building on f.otf
			 *
			 *      o                          o
			 *     / \                        / \
			 *  D 3   1 C  + 4 B C D  -->  D 3   1 C
			 *     \ /                        \ /
			 *      2 A                        o
			 *                                / \
			 *                             A 2   4 B
			 *                                \ /
			 *                                 o
			 */
			assert(m.attributesToObjects.size() == 4);
			assert(m.objectToAttributes.size()  == 4);
			//assert(m.attributeSupersets.size()  == 6);
			assert(m.attributeSupersets.size()  == 5); // removed empty nodes
			//assert(m.attributeSubsets.size()    == 6);
			assert(m.attributeSubsets.size()    == 5); // removed empty nodes
			//assert(m.attributeSets.size()       == 7);
			assert(m.attributeSets.size()       == 6); // removed empty nodes

			assert(m.objectToAttributes.contains(1) && m.objectToAttributes.contains(2) && m.objectToAttributes.contains(3) && m.objectToAttributes.contains(4));

			assert(m.maximumAttributeSet != m.objectToAttributes[1] && m.maximumAttributeSet != m.objectToAttributes[2] && m.maximumAttributeSet != m.objectToAttributes[3] && m.maximumAttributeSet != m.objectToAttributes[4]);
			assert(m.minimumAttributeSet != m.objectToAttributes[1] && m.minimumAttributeSet != m.objectToAttributes[2] && m.minimumAttributeSet != m.objectToAttributes[3] && m.minimumAttributeSet != m.objectToAttributes[4]);

			assert(m.objectToAttributes[1] != m.objectToAttributes[2]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[1] != m.objectToAttributes[4]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[3]);
			assert(m.objectToAttributes[2] != m.objectToAttributes[4]);
			assert(m.objectToAttributes[3] != m.objectToAttributes[4]);

			// QSet<Cell>* middle = m.attributeSupersets[m.objectToAttributes[1]].toList()[0];

			assert(m.attributeSupersets[m.minimumAttributeSet].size()   == 2);
			//assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[1]].size() == 2); // removed empty nodes
			//assert(m.attributeSupersets[m.objectToAttributes[3]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[3]].size() == 2); // removed empty nodes
			//assert(m.attributeSupersets[middle].size()                  == 2);
			assert(m.attributeSupersets[m.objectToAttributes[2]].size() == 1);
			assert(m.attributeSupersets[m.objectToAttributes[4]].size() == 1);
			assert(m.attributeSupersets[m.maximumAttributeSet].size()   == 0);
			assert(m.attributeSubsets[m.minimumAttributeSet].size()              == 0);
			assert(m.attributeSubsets[m.objectToAttributes[1]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[3]].size()   == 1);
			//assert(m.attributeSubsets[middle].size()                    == 2);
			//assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[2]].size()   == 2); // removed empty nodes
			//assert(m.attributeSubsets[m.objectToAttributes[4]].size()   == 1);
			assert(m.attributeSubsets[m.objectToAttributes[4]].size()   == 2); // removed empty nodes
			assert(m.attributeSubsets[m.maximumAttributeSet].size()     == 2);

			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[1]));
			assert(m.attributeSupersets[m.minimumAttributeSet].contains(m.objectToAttributes[3]));
			//assert(m.attributeSupersets[m.objectToAttributes[1]].contains(middle));
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.objectToAttributes[2])); // removed empty nodes
			assert(m.attributeSupersets[m.objectToAttributes[1]].contains(m.objectToAttributes[4])); // removed empty nodes
			//assert(m.attributeSupersets[m.objectToAttributes[3]].contains(middle));
			assert(m.attributeSupersets[m.objectToAttributes[3]].contains(m.objectToAttributes[2])); // removed empty nodes
			assert(m.attributeSupersets[m.objectToAttributes[3]].contains(m.objectToAttributes[4])); // removed empty nodes
			//assert(m.attributeSupersets[middle].contains(m.objectToAttributes[4]));
			//assert(m.attributeSupersets[middle].contains(m.objectToAttributes[2]));
			assert(m.attributeSupersets[m.objectToAttributes[2]].contains(m.maximumAttributeSet));
			assert(m.attributeSupersets[m.objectToAttributes[4]].contains(m.maximumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[1]].contains(m.minimumAttributeSet));
			assert(m.attributeSubsets[m.objectToAttributes[3]].contains(m.minimumAttributeSet));
			//assert(m.attributeSubsets[middle].contains(m.objectToAttributes[1]));
			//assert(m.attributeSubsets[middle].contains(m.objectToAttributes[3]));
			//assert(m.attributeSubsets[m.objectToAttributes[2]].contains(middle));
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[1])); // removed empty nodes
			assert(m.attributeSubsets[m.objectToAttributes[2]].contains(m.objectToAttributes[3])); // removed empty nodes
			//assert(m.attributeSubsets[m.objectToAttributes[4]].contains(middle));
			assert(m.attributeSubsets[m.objectToAttributes[4]].contains(m.objectToAttributes[1])); // removed empty nodes
			assert(m.attributeSubsets[m.objectToAttributes[4]].contains(m.objectToAttributes[3])); // removed empty nodes
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[2]));
			assert(m.attributeSubsets[m.maximumAttributeSet].contains(m.objectToAttributes[4]));

			assert(m.minimumAttributeSet->size()   == 0);
			assert(m.objectToAttributes[1]->size() == 1);
			assert(m.objectToAttributes[2]->size() == 1);
			//assert(middle->size()                  == 0);
			assert(m.objectToAttributes[3]->size() == 1);
			assert(m.objectToAttributes[4]->size() == 1);
			assert(m.maximumAttributeSet->size()   == 0);
			assert(m.objectToAttributes[1]->contains(Cell(0, 3)));
			assert(m.objectToAttributes[2]->contains(Cell(0, 1)));
			assert(m.objectToAttributes[3]->contains(Cell(0, 4)));
			assert(m.objectToAttributes[4]->contains(Cell(0, 2)));
		} else if (traceFileName.contains("l.otf")) {
			assert(m.attributesToObjects.size() == 3);
			assert(m.objectToAttributes.size()  == 3);
			assert(m.attributeSupersets.size()  == 7);
			assert(m.attributeSubsets.size()    == 7);
			assert(m.attributeSets.size()       == 8);

			assert(m.minimumAttributeSet->size()   == 0);
			assert(m.objectToAttributes[1]->size() == 0);
			assert(m.objectToAttributes[2]->size() == 0);
			assert(m.objectToAttributes[3]->size() == 1);
			assert(m.maximumAttributeSet->size()   == 0);
		}

		CompressedCallMatrix_delete(&m);
	}

	return 0;
}

