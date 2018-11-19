#include <functional>

template<typename Object, typename Attribute>
QString ConceptLattice_print(const ConceptLattice<Object, Attribute>& l, const QString& indent) {

	auto attributeSetToAllObjects = [](QSet<Attribute>* set, const ConceptLattice<Object, Attribute>& l) -> QSet<Object> {

		std::function<QSet<Object>(QSet<Attribute>*, QSet<QSet<Attribute>*>*, const ConceptLattice<Object, Attribute>&)> traverse = [&traverse](QSet<Attribute>* set, QSet<QSet<Attribute>*>* visited, const ConceptLattice<Object, Attribute>& l) -> QSet<Object> {
			if (visited->contains(set)) { return QSet<Object>(); }
			visited->insert(set);

			QSet<Object> ret;

			if (l.attributesToObjects.contains(set)) { ret.unite(l.attributesToObjects[set]); }

			if (l.attributeSupersets.contains(set)) {
				foreach (QSet<Attribute>* superset, l.attributeSupersets[set]) {
					ret.unite(traverse(superset, visited, l));
				}
			}

			return ret;
		};

		QSet<Object> ret;

		if (l.attributeSets.contains(set) == false) {
			qerr << "you are not allowed to query for sets not contained in the call matrix. aborting.\n";
			qerr.flush();
			exit(-1);
		}

		QSet<QSet<Attribute>*> visited;

		return traverse(set, &visited, l);
	};

	QString ret;
	QTextStream s(&ret);
	
	foreach (QSet<Attribute>* set, l.attributeSets) {

		s << indent << set;
		if (set == l.minimumAttributeSet) { s << " min"; }
		if (set == l.maximumAttributeSet) { s << " max"; }
		s << " {\n";

		if (l.attributesToObjects.contains(set)) {
			QList<Object> directObjects = l.attributesToObjects[set].toList();
			std::sort(directObjects.begin(), directObjects.end());

			s << indent + "\t" << "direct objects: ";
			for (int j = 0; j < directObjects.size(); j += 1) {
				s << directObjects[j];
				if (j + 1 < directObjects.size()) { s << ", "; }
			}
			s << "\n";
		}

		QList<Object> allObjects = attributeSetToAllObjects(set, l).toList();
		if (allObjects.isEmpty() == false) {
			std::sort(allObjects.begin(), allObjects.end());

			s << indent + "\t" << "all objects: ";
			for (int j = 0; j < allObjects.size(); j += 1) {
				s << allObjects[j];
				if (j + 1 < allObjects.size()) { s << ", "; }
			}
			s << "\n";
		}

		if (l.attributeSubsets.contains(set)) {
			s << indent + "\t" << "direct attribute subsets: ";
			foreach (QSet<Attribute>* set, l.attributeSubsets[set]) {
				s << set << ", ";
			}
			s << "\n";
		}

		if (l.attributeSupersets.contains(set)) {
			s << indent + "\t" << "direct attribute supersets: ";
			foreach (QSet<Attribute>* set, l.attributeSupersets[set]) {
				s << set << ", ";
			}
			s << "\n";
		}

		if (set->size() > 0) {
			s << indent + "\t" << "attributes: (" << set->size() << "): " << *set << "\n";
		}

		s << indent << "}\n";
	}

	return ret;
}

template<typename Object, typename Attribute>
QString ConceptLattice_toDot(const ConceptLattice<Object, Attribute>& l) {
	QSet<QSet<Attribute>*> visited;

	std::function<QString(QSet<Attribute>*, QSet<QSet<Attribute>*>*, const QString&, const ConceptLattice<Object, Attribute>&)> traverse = [&traverse](QSet<Attribute>* set, QSet<QSet<Attribute>*>* visited, const QString& indent, const ConceptLattice<Object, Attribute>& l) -> QString {

		const int maxBytesPerLinePerNode = 40;

		QString ret;
		QTextStream s(&ret);

		if (visited->contains(set)) { return ret; }
		visited->insert(set);

		QString setLabel;

		if (set == l.minimumAttributeSet) {
			setLabel += "min";
			if (l.attributesToObjects.contains(set) || set->size() > 0) {
				setLabel += "<br /> ";
			}
		}

		if (set == l.maximumAttributeSet) {
			setLabel += "max";
			if (l.attributesToObjects.contains(set) || set->size() > 0) {
				setLabel += "<br /> ";
			}
		}

		if (l.attributesToObjects.contains(set)) {
			int len = setLabel.size();
			setLabel += QString("objects (%1): ").arg(l.attributesToObjects[set].size());
			auto objects = l.attributesToObjects[set].toList();
			std::sort(objects.begin(), objects.end());
			for (int i = 0; i < objects.size(); i += 1) {
				setLabel += QString("%1").arg(objects[i]);

				if (setLabel.size() - len > maxBytesPerLinePerNode && i < objects.size() - 1) {
					setLabel += "...";
					break;
				} else {
					if (i < objects.size() - 1) { setLabel += ", "; }
				}
			}

			if (set->size() > 0) {
				setLabel += "<br /> ";
			}
		}

		if (set->size() > 0) {
			int len = setLabel.size();
			setLabel += QString("attributes (%1): ").arg(set->size());
			int i = 0;
			foreach (const Attribute& a, *set) {

				QString attributeString;
				QTextStream ss(&attributeString);
				ss << a;
				attributeString.replace(">", "&gt;");

				setLabel += attributeString;

				if (setLabel.size() - len > maxBytesPerLinePerNode && i < set->size() - 1) {
					setLabel += "...";
					break;
				} else {
					if (i < set->size() - 1) { setLabel += ", "; }
				}

				i += 1;
			}

		}

		s << indent << "n" << set << QString("[label=<%1 >];\n").arg(setLabel);;

		if (l.attributeSupersets.contains(set)) {
			foreach (QSet<Attribute>* superset, l.attributeSupersets[set]) {
				s << indent << "n" << set << " -> n" << superset;
				s << traverse(superset, visited, indent + "\t", l);
			}
		}

		return ret;
	};

	return QString("digraph G {\n\n%1}\n").arg(traverse(l.minimumAttributeSet, &visited, "\t", l));
}

template<typename Object, typename Attribute>
QString ConceptLattice_toCxt(const ConceptLattice<Object, Attribute>& l) {

	QString ret;
	QTextStream s(&ret);

	QSet<Attribute> attributeSet;
	QList<Attribute> attributes;
	ConceptLattice_attributes(l, &attributeSet);
	attributes = attributeSet.toList();
	std::sort(attributes.begin(), attributes.end());

	QList<Object> objects = l.objectToAttributes.keys();
	std::sort(objects.begin(), objects.end());

	s << "B\n";

	s << "\n";

	s << objects.size() << "\n";
	s << attributes.size() << "\n";

	s << "\n";

	for (int i = 0; i < objects.size(); i += 1) {
		s << objects[i] << "\n";
	}

	for (int i = 0; i < attributes.size(); i += 1) {
		s << attributes[i] << "\n";
	}

	for (int i = 0; i < objects.size(); i += 1) {

		QSet<QSet<Attribute>*> sets = ConceptLattice_reachableAttributeSubsets(l.objectToAttributes[objects[i]], l);
		QSet<Attribute> set;
		foreach (QSet<Attribute>* s, sets) {
			set.unite(*s);
		}

		for (int j = 0; j < attributes.size(); j += 1) {

			if (set.contains(attributes[j])) {
				s << "X";
			} else {
				s << ".";
			}
		}
		s << "\n";
	}

	return ret;
}

template<typename Object, typename Attribute>
void ConceptLattice_addObject(const Object& object, const QSet<Attribute>& set_, ConceptLattice<Object, Attribute>* to) {

	/* algorithm from bibliography: van04 */
	std::function<QSet<Attribute>*(QSet<Attribute>*, QSet<Attribute>*, ConceptLattice<Object, Attribute>*)> traverse = [&traverse](QSet<Attribute>* insert, QSet<Attribute>* max, ConceptLattice<Object, Attribute>* l) {

		/* returns the smallest set containing 'insert' */
		auto findMaximumSet = [](QSet<Attribute>* insert, QSet<Attribute>* maybeMaximum, const ConceptLattice<Object, Attribute>& l) {
			Q_ASSERT(maybeMaximum->contains(*insert));

			bool found = false;
			while (found == false) {
				found = true;

				if (l.attributeSubsets.contains(maybeMaximum) == false) {
					break;
				}

				foreach (QSet<Attribute>* subset, l.attributeSubsets[maybeMaximum]) {
					if (subset->contains(*insert)) {
						maybeMaximum = subset;
						found = false;
						break;
					}
				}
			}

			return maybeMaximum;
		};

		max = findMaximumSet(insert, max, *l);

		if (*max == *insert) {
			delete insert;
			return max;
		}

		QSet<QSet<Attribute>*> newSubsets;

		if (l->attributeSubsets.contains(max)) {
			foreach (QSet<Attribute>* subset, l->attributeSubsets[max]) {
				if (insert->contains(*subset) == false) {
					QSet<Attribute>* insert2 = new QSet<Attribute>;
					*insert2 = *insert & *subset;
					subset = traverse(insert2, subset, l);
				}

				bool addSubset = true;
				foreach (QSet<Attribute>* subset2, newSubsets) {
					if (subset2->contains(*subset)) {
						addSubset = false;
						break;
					} else if (subset->contains(*subset2)) {
						newSubsets.remove(subset2);
					}
				}

				if (addSubset == true) {
					newSubsets.insert(subset);
				}
			}
		}

		l->attributeSets.insert(insert);

		foreach (QSet<Attribute>* newSubset, newSubsets) {
			if (l->attributeSubsets.contains(max))         { l->attributeSubsets[max].remove(newSubset); }
			if (l->attributeSupersets.contains(newSubset)) { l->attributeSupersets[newSubset].remove(max); }
			l->attributeSubsets[insert].insert(newSubset);
			l->attributeSupersets[newSubset].insert(insert);
		}

		l->attributeSubsets[max].insert(insert);
		l->attributeSupersets[insert].insert(max);

		if (max == l->minimumAttributeSet) {
			l->minimumAttributeSet = insert;
		}

		return insert;
	};

	auto set = new QSet<Attribute>;
	*set = set_;

	if (to->attributeSets.isEmpty()) {
		assert(to->attributesToObjects.isEmpty());
		assert(to->objectToAttributes.isEmpty());
		assert(to->attributeSupersets.isEmpty());
		assert(to->attributeSubsets.isEmpty());

		to->attributesToObjects[set].insert(object);
		to->objectToAttributes[object] = set;
		to->attributeSets.insert(set);
		to->maximumAttributeSet = to->minimumAttributeSet = set;
	} else {
		if (to->maximumAttributeSet->contains(*set) == false) {
			/* add a new maximum if necessary */

			if (to->attributesToObjects.contains(to->maximumAttributeSet) == false) {
				to->maximumAttributeSet->unite(*set);
			} else {
				QSet<Attribute>* newMaximum = new QSet<Attribute>;
				newMaximum->unite(*to->maximumAttributeSet);
				newMaximum->unite(*set);

				to->attributeSets.insert(newMaximum);

				to->attributeSubsets[newMaximum].insert(to->maximumAttributeSet);
				to->attributeSupersets[to->maximumAttributeSet].insert(newMaximum);

				to->maximumAttributeSet = newMaximum;
			}
		}

		QSet<Attribute>* labelMe = traverse(set, to->maximumAttributeSet, to);

		to->objectToAttributes[object] = labelMe;
		to->attributesToObjects[labelMe].insert(object);
	}
}

template<typename Object, typename Attribute>
void ConceptLattice_group(const ConceptLattice<Object, Attribute>& l, double minimumSimilarity, QList<QSet<QSet<Attribute>*>>* groups) {
	assert(minimumSimilarity >= 0.0);
	assert(minimumSimilarity <= 1.0);
	assert(groups->isEmpty());

	QMap<QSet<Attribute>*, QMap<QSet<Attribute>*, double>> similarityMatrix;

	/* sort sets by minimum process identifier contained. this makes the grouping stable and repeatable. */
	QMap<QSet<Attribute>*, Object> minimumObject;
	foreach (QSet<Attribute>* set, l.attributesToObjects.keys()) {
		Q_ASSERT(l.attributesToObjects[set].size() > 0);
		minimumObject[set] = *std::min_element(l.attributesToObjects[set].begin(), l.attributesToObjects[set].end());
	}

	QList<QSet<Attribute>*> sets = l.attributesToObjects.keys();
	std::sort(sets.begin(), sets.end(), [&](QSet<Attribute>* a, QSet<Attribute>* b) { return minimumObject[a] < minimumObject[b]; });

	QMap<QSet<Attribute>*, int> s2g;
	QList<QSet<QSet<Attribute>*>>& g2s = *groups;

	for (int i = 0; i < sets.size(); i += 1) {
		similarityMatrix[sets[i]][sets[i]] = 1.0;
		for (int j = i + 1; j < sets.size(); j += 1) {
			double sim = ConceptLattice_simpleConceptSimilarity(sets[i], sets[j], l);
			similarityMatrix[sets[i]][sets[j]] = sim;
			similarityMatrix[sets[j]][sets[i]] = sim;
		}
	}

	/* calculate similarity to the given set for each set in the group and weight the result by the number of processes contained
	 *
	 * note: this is a spot where one could try out different strategies. minimum/maximum similarity, non-weighted.
	 * i expect it to not matter, though.
	 */
	auto groupSimilarityFunction = [&](QSet<Attribute>* set, const QSet<QSet<Attribute>*>& group) {
		double summedSimilarity = 0;
		int processCount = 0;
		foreach (QSet<Attribute>* s, group) {
			Q_ASSERT(similarityMatrix.contains(set));
			Q_ASSERT(similarityMatrix[set].contains(s));
			summedSimilarity += similarityMatrix[set][s] * l.attributesToObjects[s].size();
			processCount += l.attributesToObjects[s].size();
		}
		return summedSimilarity / processCount;
	};

	/* for each set, find the best group to throw it in. if no group is found or the minimum similarity is not meat, create a new group */
	for (int i = 0; i < sets.size(); i += 1) {
		if (s2g.contains(sets[i])) continue;

		double bestSim = -1.0;
		int bestGroup = -1;

		for (int j = 0; j < g2s.size(); j += 1) {
			double sim = groupSimilarityFunction(sets[i], g2s[j]);
			if (sim > bestSim) {
				bestSim = sim;
				bestGroup = j;
			}
			// qDebug() << QString("%1 vs %2 = %3, best %4").arg(i+1).arg(j+1).arg(sim).arg(bestGroup);
		}

		if (bestGroup == -1 || bestSim < minimumSimilarity) {
			g2s.append(QSet<QSet<Attribute>*>());
			g2s.last().insert(sets[i]);
			s2g[sets[i]] = g2s.size()-1;
		} else {
			g2s[bestGroup].insert(sets[i]);
			s2g[sets[i]] = bestGroup;
		}
	}

	/* make sure: no empty groups */
	for (int i = 0; i < groups->size(); i += 1) {
		Q_ASSERT((*groups)[i].isEmpty() == false);
	}

	/* make sure: no set is in two groups */
	QSet<QSet<Attribute>*> setsCheck;
	for (int i = 0; i < groups->size(); i += 1) {
		foreach (QSet<Attribute>* set, (*groups)[i]) {
			Q_ASSERT(setsCheck.contains(set) == false);
		}
		setsCheck.unite((*groups)[i]);
	}

	/* make sure: every set is part of a group */
	Q_ASSERT(setsCheck == sets.toSet());
}

template<typename Object, typename Attribute>
void ConceptLattice_finalize(ConceptLattice<Object, Attribute>* l) {

	auto subtractImplicitSets = [](QList<QSet<Attribute>*>* q, QSet<QSet<Attribute>*>* visited, ConceptLattice<Object, Attribute>* l) {
		QSet<Attribute>* set = q->first();
		q->removeFirst();

		if (visited->contains(set)) { return; }

		if (l->attributeSupersets.contains(set)) {
			if (visited->contains(l->attributeSupersets[set]) == false) {
				if (q->contains(set) == false) {
					q->append(set);
				}
				return;
			}
		}

		visited->insert(set);

		if (l->attributeSubsets.contains(set)) {
			foreach (QSet<Attribute>* subset, l->attributeSubsets[set]) {
				set->subtract(*subset);
				if (q->contains(subset) == false) {
					q->append(subset);
				}
			}
		}
	};

	if (l->attributeSets.size() == 0) return;

	QList<QSet<Attribute>*> q;
	q.append(l->maximumAttributeSet);
	QSet<QSet<Attribute>*> visited;
	while (q.isEmpty() == false) {
		subtractImplicitSets(&q, &visited, l);
	}
}

template<typename Object, typename Attribute>
void ConceptLattice_removeEmptyNodes(ConceptLattice<Object, Attribute>* l) {
	auto traverseBreadthFirst = [](QList<QSet<Attribute>*>* q, QSet<QSet<Attribute>*>* visited, ConceptLattice<Object, Attribute>* l) {
		QSet<Attribute>* set = q->first();
		q->removeFirst();

		if (visited->contains(set)) { return; }

		if (l->attributeSupersets.contains(set)) {
			if (visited->contains(l->attributeSupersets[set]) == false) {
				if (q->contains(set) == false) {
					q->append(set);
				}
				return;
			}
		}

		visited->insert(set);

		if (l->attributeSubsets.contains(set)) {
			foreach (QSet<Attribute>* subset, l->attributeSubsets[set]) {
				if (q->contains(subset) == false) {
					q->append(subset);
				}
			}
		}

		if (set->size() == 0 && l->attributesToObjects.contains(set) == false) {
			if (l->attributeSubsets.contains(set) && l->attributeSupersets.contains(set)) {
				foreach (QSet<Attribute>* subset, l->attributeSubsets[set]) {
					foreach (QSet<Attribute>* superset, l->attributeSupersets[set]) {
						l->attributeSupersets[subset].remove(set);
						l->attributeSupersets[subset].insert(superset);
						l->attributeSubsets[superset].remove(set);
						l->attributeSubsets[superset].insert(subset);
					}
				}
				l->attributeSubsets.remove(set);
				l->attributeSupersets.remove(set);
				l->attributeSets.remove(set);
				delete set;
			}
		}
	};

	if (l->attributeSets.size() == 0) return;

	QList<QSet<Attribute>*> q;
	q.append(l->maximumAttributeSet);
	QSet<QSet<Attribute>*> visited;

	while (q.isEmpty() == false) {
		traverseBreadthFirst(&q, &visited, l);
	}
}

template<typename Object, typename Attribute>
void ConceptLattice_delete(ConceptLattice<Object, Attribute>* l) {
	foreach (QSet<Attribute>* set, l->attributeSets) {
		delete set;
	}
	l->attributesToObjects.clear();
	l->objectToAttributes.clear();
	l->attributeSupersets.clear();
	l->attributeSubsets.clear();
	l->attributeSets.clear();
}

/* *** low level functions *** ********************************************* */

template<typename Object, typename Attribute>
QSet<QSet<Attribute>*> ConceptLattice_reachableAttributeSubsets(QSet<Attribute>* set, const ConceptLattice<Object, Attribute>& l) {

	std::function<QSet<QSet<Attribute>*>(QSet<Attribute>*, QSet<QSet<Attribute>*>*, const ConceptLattice<Object, Attribute>&)> traverse = [&traverse](QSet<Attribute>* set, QSet<QSet<Attribute>*>* visited, const ConceptLattice<Object, Attribute>& l) -> QSet<QSet<Attribute>*> {

		if (visited->contains(set)) { return QSet<QSet<Attribute>*>(); }
		visited->insert(set);

		QSet<QSet<Attribute>*> ret;
		ret.insert(set);

		if (l.attributeSubsets.contains(set)) {
			foreach (QSet<Attribute>* subset, l.attributeSubsets[set]) {
				ret.unite(traverse(subset, visited, l));
			}
		}

		return ret;
	};

	QSet<QSet<Attribute>*> visited;
	return traverse(set, &visited, l);
}

template<typename Object, typename Attribute>
double ConceptLattice_simpleConceptSimilarity(QSet<Attribute>* a, QSet<Attribute>* b, const ConceptLattice<Object, Attribute>& l) {
	assert(l.attributeSets.contains(a));
	assert(l.attributeSets.contains(b));

	QSet<QSet<Attribute>*> as = ConceptLattice_reachableAttributeSubsets(a, l);
	QSet<QSet<Attribute>*> bs = ConceptLattice_reachableAttributeSubsets(b, l);

	int equal = 0;
	auto intersection = as & bs;
	foreach (QSet<Attribute>* set, intersection) {
		equal += set->size();
	}

	int unequal = 0;
	auto symmetricDifference = (as - bs) + (bs - as);
	foreach (QSet<Attribute>* set, symmetricDifference) {
		unequal += set->size();
	}

	int sum = 0;
	auto union_ = as + bs;
	foreach (QSet<Attribute>* set, union_) {
		sum += set->size();
	}

	Q_ASSERT(equal + unequal == sum);

	return (double) equal / ((double) equal + (double) unequal);
}

template<typename Object, typename Attribute>
double ConceptLattice_subsumptionConceptSimilarity(QSet<Attribute>* a, QSet<Attribute>* b, const ConceptLattice<Object, Attribute>& l) {
	assert(l.attributeSets.contains(a));
	assert(l.attributeSets.contains(b));

	QSet<QSet<Attribute>*> as = ConceptLattice_reachableAttributeSubsets(a, l);
	QSet<QSet<Attribute>*> bs = ConceptLattice_reachableAttributeSubsets(b, l);

	int correct = 0;
	auto both = as & bs;
	foreach (QSet<Attribute>* set, both) {
		correct += set->size();
	}

	int wrong = 0;
	auto bWithoutA = bs - as;
	foreach (QSet<Attribute>* set, bWithoutA) {
		wrong += set->size();
	}

	int sum = 0;
	foreach (QSet<Attribute>* set, bs) {
		sum += set->size();
	}

	Q_ASSERT(correct + wrong == sum);

	return (double) correct / ((double) correct + (double) wrong);
}

/* *** misc analysis *** *************************************************** */

template<typename Object, typename Attribute>
void ConceptLattice_attributes(const ConceptLattice<Object, Attribute>& l, QSet<Attribute>* attributes) {
	foreach (QSet<Attribute>* attributeSet, l.attributeSets) {
		attributes->unite(*attributeSet);
	}
}
