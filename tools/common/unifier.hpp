#ifndef UNIFIER_HPP
#define UNIFIER_HPP

#include "common/prereqs.hpp"

typedef int trace_t;

/* unifies (function) token inside and accross traces
 *
 * also makes sure that function identifiers start with 1. otf2 traces typically start with 0, therefore to every function identifier will be increased by one. (the call matrix e.g. uses 0 as a special function id)
 *
 * there exist traces (e.g. sweep) where functions with the same name are used with different identifiers.
 * using the unifier statistics will be gathered for function which names match. not original otf/vampirtrace tokens.
 */
template<typename TokenType>
class Unifier {
public:

	void insert(trace_t trace, const QString& traceName, TokenType token, const QString& name);
	void insert(trace_t trace, const QString& traceName, const QMap<TokenType, QString>& names);

	TokenType map(trace_t trace, TokenType token) const;

	QMap<TokenType, QString> mappedNames() const;

	QString dump(const QString& indent = "") const;

private:
	QHash<trace_t, QString> traceNames;

	QHash<TokenType, QString> names;
	QHash<QString, TokenType> namesReverse;

	QHash<trace_t, QHash<TokenType, TokenType>> tokenMapping;

	TokenType lastToken = 1;
};

#include "common/unifier.inl"

#endif /* UNIFIER_HPP */
