#include "call-tree/call-tree.hpp"

#include <functional>

static QString CallTree_printRecursively(const CallTreeNode& n, const QMap<function_t, QString>& functionNames, const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	if (n.children.size() == 0 && n.statistics.isNull() == true) {
		s << indent << QString("%1: %2\n").arg(n.id).arg(functionNames[n.id]);
	} else {
		s << indent << QString("%1: %2 {\n").arg(n.id).arg(functionNames[n.id]);

		if (n.statistics.isNull() == false) {
			s << indent + "\t" + "inv " << n.statistics->invocationCount << "\n";
			s << indent + "\t" + "ex: " << Measure_print(n.statistics->exclusiveTime) << "\n";
			s << indent + "\t" + "in: " << Measure_print(n.statistics->inclusiveTime) << "\n";
		}

		foreach (const auto* m, n.children) {
			s << CallTree_printRecursively(*m, functionNames, indent + "\t");
		}

		s << indent + "}\n";
	}

	return ret;
};

QString CallTree_print(const CallTree& t, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames, const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	QMapIterator<process_t, CallTreeNode*> i (t.processes);
	while (i.hasNext()) {
		i.next();
		s << indent << QString("%1: %2 {\n").arg(i.key()).arg(processNames[i.key()]);

		s << CallTree_printRecursively(*i.value(), functionNames, indent + "\t");

		s << indent << "}\n";
	}

	s << "node count: " << CallTree_nodeCount(t) << "\n";

	return ret;
}

static QString CallTree_toDotRecursively(const CallTreeNode* n, const CallTreeNode* parent, const QMap<function_t, QString>& functionNames, bool withIdentifiers, QSet<const CallTreeNode*>* visited, const QString& indent) {

	QString ret;
	QTextStream s(&ret);

	QString nodeLabel;
	if (withIdentifiers) {
		nodeLabel = QString("%1: %2").arg(n->id).arg(functionNames[n->id]);
	} else if (functionNames.contains(n->id) == false) {
		nodeLabel = QString("%1").arg(n->id);
	} else {
		nodeLabel = QString("%1").arg(functionNames[n->id]);
		// nodeLabel = QString("%1; %2").arg(functionNames[n->id]).arg(n->subTreeEdgeCount); // with subTreeEdgeCount
	}

	QString parentNodeName = QString("n%1").arg((qulonglong) parent, 0, 16);
	QString nodeName       = QString("n%1").arg((qulonglong) n     , 0, 16);

	s << indent << QString("%1 -> %2;\n").arg(parentNodeName).arg(nodeName);

	if (visited->contains(n) == false) {
		s << indent << QString("%1[label=\"%2\"];\n").arg(nodeName).arg(nodeLabel);
		// DEBUG
		// foreach (const CallTreeNode* m, n->parents) {
		// 	s << indent << QString("%1 -> n%2;\n").arg(nodeName).arg((qulonglong) m, 0, 16);
		// }
	}
	s << "\n";

	if (visited->contains(n) == false) {
		visited->insert(n);
		foreach (CallTreeNode* m, n->children) {
			s << CallTree_toDotRecursively(m, n, functionNames, withIdentifiers, visited, indent + "\t");
		}
	}

	return ret;
}

QString CallTree_toDot(const CallTree& tree, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames, bool withIdentifiers) {
	QString ret;
	QTextStream s(&ret);

	s << "digraph G {\n";

	s << "\n";

	QMapIterator<process_t, CallTreeNode*> i(tree.processes);
	while (i.hasNext()) {
		i.next();
		QString processLabel;
		if (processNames.contains(i.key())) {
			if (withIdentifiers) {
				processLabel = QString("%1: %2").arg(i.key()).arg(processNames[i.key()]);
			} else {
				processLabel = QString("%1").arg(processNames[i.key()]);
				// processLabel = QString("%1; %2").arg(processNames[i.key()]).arg(i.value()->subTreeEdgeCount); // with subTreeEdgeCount
			}
		} else {
				processLabel = QString("p%1").arg(i.key());
		}
		s << "\t" << QString("n%1[label=\"%2\", shape=box];\n").arg((qulonglong) i.value(), 0, 16).arg(processLabel);
	}
	s << "\n";

	QSet<const CallTreeNode*> visited;

	foreach (CallTreeNode* n, tree.processes) {
		assert(visited.contains(n) == false); // process nodes are unique
		visited.insert(n);
		foreach (CallTreeNode *m, n->children) {
			s << CallTree_toDotRecursively(m, n, functionNames, withIdentifiers, &visited, "\t\t");
		}
		s << "\n";
	}

	s << "}\n";

	return ret;
}

void CallTree_fromProcessTrace(const ProcessTrace& trace, process_t p, const Unifier<function_t>& u, trace_t traceId, bool withStatistics, CallTree* t) {

	auto newSubCall = [](CallTreeNode* n, CallTree *t, function_t id, bool withStatistics) -> CallTreeNode* {

		CallTreeNode *m;

		if (n->children.contains(id)) {
			m = n->children[id];
		} else {
			m = n->children[id] = new CallTreeNode;
			m->id = id;
			m->parents.insert(n);
			m->subTreeEdgeCount = 0;

			t->nodes.insert(m);

			if (withStatistics) {
				m->statistics.reset(new CallTreeNode::Statistics);
				// f->invocationCount intentionally not initialized
				Measure_init(&m->statistics->exclusiveTime);
				Measure_init(&m->statistics->inclusiveTime);
			}

			Q_ASSERT(n->parents.size() <= 1);

			CallTreeNode *parent = n;
			while (parent != nullptr) {

				parent->subTreeEdgeCount += 1;

				if (parent->parents.size() == 1) {
					parent = parent->parents.toList()[0];
				} else {
					parent = nullptr;
				}
			}

		}

		return m;
	};

	std::function<void(const FunctionCall&, const Unifier<function_t>&, trace_t, bool, CallTreeNode*, CallTree*)> traverse = [&traverse, &newSubCall](const FunctionCall& c, const Unifier<function_t>& u, trace_t trace, bool withStatistics, CallTreeNode* n, CallTree* t) {

		if (n->statistics.isNull() == false) {
			int64_t accumulatedSubCallTime = 0;
			foreach (const auto s, c.calls) { accumulatedSubCallTime += s.end - s.begin; }

			int64_t inclusiveTime = c.end - c.begin;
			int64_t exclusiveTime = inclusiveTime - accumulatedSubCallTime;

			Measure_record(&n->statistics->exclusiveTime, exclusiveTime);
			Measure_record(&n->statistics->inclusiveTime, inclusiveTime);
		}

		foreach (const auto& s, c.calls) {
			traverse(s, u, trace, withStatistics, newSubCall(n, t, u.map(trace, s.id), withStatistics), t);
		}
	};

	if (t->processes.contains(p) == false) {
		CallTreeNode* n = t->processes[p] = new CallTreeNode;
		n->id = 0; // magic number, shouldn't be used/queried anywhere
		n->parents = QSet<CallTreeNode*>();
		t->nodes.insert(n);
		t->reverseProcesses[n] = p;

		if (withStatistics) { // not used, but all nodes are supposed to have this structure
			n->statistics.reset(new CallTreeNode::Statistics);
			// f->invocationCount intentionally not initialized
			Measure_init(&n->statistics->exclusiveTime);
			Measure_init(&n->statistics->inclusiveTime);
		}
	}

	foreach (const auto& c, trace) {
		traverse(c, u, traceId, withStatistics, newSubCall(t->processes[p], t, u.map(traceId, c.id), withStatistics), t);
	}
}

void CallTree_finalize(CallTree* t) {

	bool withStatistics = false;

	foreach (CallTreeNode* n, t->nodes) {

		if (withStatistics == false && n->statistics.isNull() == false) {
			withStatistics = true;
		}

		if (withStatistics == true) {

			Q_ASSERT(n->statistics.isNull() == false);

			Measure_finalize(&n->statistics->exclusiveTime);
			Measure_finalize(&n->statistics->inclusiveTime);
			n->statistics->invocationCount = n->statistics->exclusiveTime.dataPointCount;

			Q_ASSERT(n->statistics->exclusiveTime.dataPointCount == n->statistics->inclusiveTime.dataPointCount);
		}
	}
}

void CallTree_merge(CallTree* from, CallTree* into) {
	assert((from->nodes + into->nodes).size() == from->nodes.size() + into->nodes.size()); // from and to do not share nodes
	const auto& fromProcesses = from->processes.keys().toSet();
	const auto& intoProcesses = into->processes.keys().toSet();
	assert((fromProcesses + intoProcesses).size() == fromProcesses.size() + intoProcesses.size()); // from and to do not share processes

	/* move all processes/nodes of 'from' into 'into' */

	into->processes.unite(from->processes);
	into->reverseProcesses.unite(from->reverseProcesses);
	into->nodes.unite(from->nodes);

	from->processes.clear();
	from->reverseProcesses.clear();
	from->nodes.clear();
}

static void CallTree_replaceNodes(CallTreeNode* n, const QMap<CallTreeNode*, CallTreeNode*>& mapping, QSet<CallTreeNode*>* visited) {
	if (visited->contains(n)) { return; }
	visited->insert(n);

	foreach (CallTreeNode* m, n->children) {
		m->parents.remove(n);
		n->children[m->id] = mapping[n->children[m->id]];
		n->children[m->id]->parents.insert(n);
		CallTree_replaceNodes(n->children[m->id], mapping, visited);

	}
}

static void CallTree_compressRecursively(CallTreeNode* n, CallTree* tree, QMap<CallTreeNode*, CallTreeNode*>* mapping) {

	foreach (CallTreeNode *m ,n->children) { /* post-order traversal is necessary */
		CallTree_compressRecursively(m, tree, mapping);
	}

	if (mapping->contains(n)) { return; }

	bool found = false;
	foreach (CallTreeNode *m, *mapping) {
		bool isEqual = true;
		if (n->id != m->id || n->children.size() != m->children.size()) {
			isEqual = false;
		} else {
			foreach (CallTreeNode *o, n->children) {
				Q_ASSERT(mapping->contains(n->children[o->id]));
				if (m->children.contains(o->id) == true) {
					Q_ASSERT(mapping->contains(o));
					isEqual &= ((*mapping)[m->children[o->id]] == (*mapping)[o]);
				} else {
					isEqual = false;
					break;
				}
			}
		}

		if (isEqual) {
			mapping->insert(n, m);
			found = true;
			break;
		}
	}
	if (found == false) {
		mapping->insert(n, n);
	}
}

static void CallTree_reachableNodes(CallTreeNode* n, QSet<CallTreeNode*>* set) {
	if (set->contains(n)) return;
	set->insert(n);

	foreach (CallTreeNode *m, n->children) {
		CallTree_reachableNodes(m, set);
	}
}

void CallTree_compress(CallTree* tree) {
	QMap<CallTreeNode*, CallTreeNode*> mapping;

	/* generate mapping */
	foreach (CallTreeNode *n, tree->processes) {
		/* intuitively this foreach is not necessary and you could just pass tree->tree[p], but that would lead to the problem of comparing process ids to function ids */
		foreach (CallTreeNode* m, n->children) {
			if (n->statistics.isNull() == false) {
				qDebug() << "compressing trees with statistics is in principle doable, but not supported right now. aborting";
				exit(-1);
			}
			CallTree_compressRecursively(m, tree, &mapping);
		}
	}

	/* validate mapping */
	QSet<CallTreeNode*> nodesWithoutProcesses = tree->nodes - tree->processes.values().toSet();

	foreach (CallTreeNode* n, nodesWithoutProcesses) { Q_ASSERT(mapping.contains(n))    ; }
	foreach (CallTreeNode* n, mapping.keys()       ) { Q_ASSERT(tree->nodes.contains(n)); }
	foreach (CallTreeNode* n, mapping.values()     ) { Q_ASSERT(tree->nodes.contains(n)); }

	/* replace nodes according to mapping*/
	QSet<CallTreeNode*> visited2;
	foreach (CallTreeNode *n, tree->processes) {
		CallTree_replaceNodes(n, mapping, &visited2);
	}

	/* free duplicate nodes */
	QSet<CallTreeNode*> remainingNodes;
	foreach (CallTreeNode *n, tree->processes) {
		CallTree_reachableNodes(n, &remainingNodes);
	}

	foreach (CallTreeNode *n, tree->nodes - remainingNodes) {
		delete n;
	}
	tree->nodes = remainingNodes;
}

int CallTree_nodeCount(const CallTree& tree) {
	return tree.nodes.size();
}

// void traverse2(QList<CallTreeNode*>* queue, QMap<CallTreeNode*, QMap<function_t, QMap<function_t, int>>>* buckets, QMap<CallTreeNode*, QSet<CallTreeNode*>>* unfinishedChildren) {
//
// 	// qDebug() << /*queue->first() <<*/ queue->first()->id << "a" /*<< *queue*/;
//
// 	Q_ASSERT(buckets->contains(queue->first()));
//
// 	CallTreeNode* n = queue->first();
// 	queue->removeFirst();
//
// 	if (n->parents.isEmpty() == false) {
// 		/* no process node */
//
// 		// qDebug() << n->id << "b";
//
// 		if ((*unfinishedChildren)[n].isEmpty()) {
// 			/* a node which does not have dependencies on any children */
//
// 			// qDebug() << n->id << "c";
//
// 			foreach (CallTreeNode* m, n->parents) {
//
// 				// qDebug() << n->id << "d" << m->id;
//
// 				/* merge bucket with the parent node's bucket */
// 				QMapIterator<function_t, QMap<function_t, int>> i((*buckets)[n]);
// 				while (i.hasNext()) {
// 					i.next();
//
// 					QMapIterator<function_t, int> j(i.value());
// 					while (j.hasNext()) {
// 						j.next();
//
// 						if ((*buckets)[m][i.key()].contains(j.key())) {
// 							(*buckets)[m][i.key()][j.key()] += j.value();
// 						} else {
// 							(*buckets)[m][i.key()][j.key()]  = j.value();
// 						}
// 					}
// 				}
//
// 				/* add edge (parent -> this) */
// 				if ((*buckets)[m][m->id].contains(n->id)) {
// 					(*buckets)[m][m->id][n->id] += 1;
// 				} else {
// 					(*buckets)[m][m->id][n->id]  = 1;
// 				}
//
// 				/* add all children, except the one we come from, as unfinished nodes, if this hasn't been already done */
// 				if ((*unfinishedChildren)[m].isEmpty()) {
// 					foreach (CallTreeNode* o, m->children) {
// 						(*unfinishedChildren)[m].insert(o);
// 					}
// 				}
// 				(*unfinishedChildren)[m].remove(n);
//
// 				/* queue this node if it is not already queued */
// 				bool alreadyContained = false;
// 				foreach (CallTreeNode* o, *queue) {
// 					if (o == m) {
// 						alreadyContained = true;
// 						break;
// 					}
// 				}
// 				if (alreadyContained == false) {
// 					queue->prepend(m);
// 				}
// 			}
//
// 			buckets->remove(n);
//
// 			traverse2(queue, buckets, unfinishedChildren);
//
// 		} else {
// 			/* wait for all unfinished children to be processed */
// 			/* no requeuing need since it will be visited anyways */
// 			return;
// 		}
//
// 	} else {
// 		/* process node */
// 		return;
// 	}
//
// }
//
// void CallTree_groupSimilarProcesses(const CallTree& tree, double threshold, QList<QList<process_t>>* groups) {
//
// 	QSet<CallTreeNode*> leaves;
// 	foreach (CallTreeNode* n, tree.nodes) {
// 		if (n->children.isEmpty()) {
// 			leaves.insert(n);
// 		}
// 	}
//
// 	QMap<CallTreeNode* /*whose*/, QMap<function_t /*from*/, QMap<function_t /*to*/, int /*multiplicity*/>>> buckets;
// 	foreach (CallTreeNode* n, leaves) {
// 		buckets.insert(n, QMap<function_t, QMap<function_t, int>>());
// 	}
//
// 	QList<CallTreeNode*> unfinishedQueue = leaves.toList();
// 	QMap<CallTreeNode*, QSet<CallTreeNode*>> unfinishedChildren;
//
// 	while (unfinishedQueue.isEmpty() == false) {
// 		traverse2(&unfinishedQueue, &buckets, &unfinishedChildren);
// 	}
//
// 	Q_ASSERT(buckets.size() == tree.processes.size());
// 	QMapIterator<CallTreeNode*, QMap<function_t, QMap<function_t, int>>> i(buckets);
// 	while (i.hasNext()) {
// 		i.next();
//
// 		qerr << QString("Process %1:\n").arg(tree.reverseProcesses[i.key()]);
//
// 		QMapIterator<function_t, QMap<function_t, int>> j(i.value());
// 		while (j.hasNext()) {
// 			j.next();
//
// 			QMapIterator<function_t, int> k(j.value());
// 			while (k.hasNext()) {
// 				k.next();
//
// 				qerr << "\t" << QString("%1 -> %2 : %3\n").arg(j.key()).arg(k.key()).arg(k.value());
// 			}
// 		}
// 		qerr << "\n";
// 		qerr.flush();
// 	}
// }

void CallTree_delete(CallTree *tree) {
	foreach (CallTreeNode *n, tree->nodes) {
		delete n;
	}
	tree->nodes.clear();
	tree->processes.clear();
}
