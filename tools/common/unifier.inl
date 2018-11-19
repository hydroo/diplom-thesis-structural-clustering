template<typename TokenType>
void Unifier<TokenType>::insert(trace_t trace, const QString& traceName, TokenType token, const QString& name) {
	if (traceName.contains(trace) == false) {
		traceNames.insert(trace, traceName);
	}
	
	if (tokenMapping[trace].contains(token) == false) {
		if (namesReverse.contains(name)) {
			tokenMapping[trace][token] = namesReverse[name];
		} else {
			while(names.contains(lastToken) == true) {
				lastToken += 1;
			}
			names[lastToken] = name;
			namesReverse[name] = lastToken;
			tokenMapping[trace][token] = lastToken;
		}
	} else {
		assert(names.contains(tokenMapping[trace][token]));
		assert(name == names[tokenMapping[trace][token]]);
	}
}

template<typename TokenType>
void Unifier<TokenType>::insert(trace_t trace, const QString& traceName, const QMap<TokenType, QString>& names) {
	foreach (TokenType f, names.keys()) {
		this->insert(trace, traceName, f, names[f]);
	}
}

template<typename TokenType>
TokenType Unifier<TokenType>::map(trace_t trace, TokenType token) const {
	auto i = this->tokenMapping.find(trace);
	if (i == this->tokenMapping.end()) {
		qerr << QString("No unifier mapping exists for trace %1. Aborting.").arg(trace) << endl;
		assert(0);
	}
	auto j = i->find(token);
	if (j == i->end()) {
		qerr << QString("No unifier mapping exists for function %1 in trace %2. Aborting.").arg(token).arg(trace) << endl;
		assert(0);
	}

	return j.value();
}

template<typename TokenType>
QMap<TokenType, QString> Unifier<TokenType>::mappedNames() const {
	QMap<TokenType, QString> ret;
	foreach (auto f, names.keys()) {
		ret.insert(f, names.find(f).value());
	}
	return ret;
}

template<typename TokenType>
QString Unifier<TokenType>::dump(const QString& indent) const {
	QString ret;
	QTextStream s(&ret);

	s << indent << "Unifier {\n";

	s << indent << "traces:\n";

	s << indent << "functions: { ";
	foreach (auto f, names.keys()) {
		s << QString("%1 : %2, ").arg(f).arg(names[f]);
	}
	s << " }\n";

	s << indent << "mapping:\n";
	foreach (auto t, tokenMapping.keys()) {
		s << indent + "\t" << QString("trace %1, \"%2\": {").arg(t).arg(traceNames[t]);
		foreach (auto f, tokenMapping[t].keys()) {
			s << QString("%1 : %2, ").arg(f).arg(tokenMapping[t][f]);
		}
		s << " }\n";
	}

	s << indent << "} Unifier\n";

	return ret;
}
