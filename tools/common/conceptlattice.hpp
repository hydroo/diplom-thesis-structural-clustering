#ifndef CONCEPTLATTICE_HPP
#define CONCEPTLATTICE_HPP

#include "common/prereqs.hpp"

template<typename Object, typename Attribute>
struct ConceptLattice {
	QMap<QSet<Attribute>*, QSet<Object>          > attributesToObjects;
	QMap<Object,           QSet<Attribute>*      > objectToAttributes;
	QMap<QSet<Attribute>*, QSet<QSet<Attribute>*>> attributeSupersets;
	QMap<QSet<Attribute>*, QSet<QSet<Attribute>*>> attributeSubsets;

	QSet<QSet<Attribute>*> attributeSets;

	QSet<Attribute> *maximumAttributeSet, *minimumAttributeSet;
};

template<typename Object, typename Attribute>
QString ConceptLattice_print(const ConceptLattice<Object, Attribute>& l, const QString& indent = "");

template<typename Object, typename Attribute>
QString ConceptLattice_toDot(const ConceptLattice<Object, Attribute>& l);

template<typename Object, typename Attribute>
QString ConceptLattice_toCxt(const ConceptLattice<Object, Attribute>& l);

template<typename Object, typename Attribute>
void ConceptLattice_addObject(const Object& o, const QSet<Attribute>& a, ConceptLattice<Object, Attribute>* to);

template<typename Object, typename Attribute>
void ConceptLattice_group(const ConceptLattice<Object, Attribute>& l, double minimumSimilarity, QList<QSet<QSet<Attribute>*>>* groups);

template<typename Object, typename Attribute>
void ConceptLattice_finalize(ConceptLattice<Object, Attribute>* l);

// note: this breaks the formal concept lattice property that every set of nodes has a unique supremum and infimum
template<typename Object, typename Attribute>
void ConceptLattice_removeEmptyNodes(ConceptLattice<Object, Attribute>* l);

template<typename Object, typename Attribute>
void ConceptLattice_delete(ConceptLattice<Object, Attribute>* m);

/* *** low level functions *** ********************************************* */

template<typename Object, typename Attribute>
QSet<QSet<Attribute>*> ConceptLattice_reachableAttributeSubsets(QSet<Attribute>* set, const ConceptLattice<Object, Attribute>& l);

template<typename Object, typename Attribute>
double ConceptLattice_simpleConceptSimilarity(QSet<Attribute>* a, QSet<Attribute>* b, const ConceptLattice<Object, Attribute>& l);

template<typename Object, typename Attribute>
double ConceptLattice_subsumptionConceptSimilarity(QSet<Attribute>* a, QSet<Attribute>* b, const ConceptLattice<Object, Attribute>& l);

/* *** misc analysis *** *************************************************** */

template<typename Object, typename Attribute>
void ConceptLattice_attributes(const ConceptLattice<Object, Attribute>& l, QSet<Attribute>* attributes);

#include "common/conceptlattice.inl"

#endif // CONCEPTLATTICE_HPP
