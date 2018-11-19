#ifndef OTF_HPP
#define OTF_HPP

#include "common/prereqs.hpp"
#include "common/unifier.hpp"

typedef int32_t function_t;
typedef int32_t functiongroup_t;
typedef int64_t process_t;

struct FunctionCall {
	function_t id;
	int64_t begin, end; // not uint64_t to be able to save -1 into 'end'
	QList<FunctionCall> calls;
};

typedef QList<FunctionCall> ProcessTrace;
typedef QMap<process_t, ProcessTrace> Trace;

void Otf_processNames(const QString& traceFileName, QMap<process_t, QString>* processNames);
void Otf_functionNames(const QString& traceFileName, QMap<function_t, QString>* functionNames);
void Otf_functionGroupNames(const QString& traceFileName, QMap<functiongroup_t, QString>* functionGroupNames);
void Otf_functionGroupMembers(const QString& traceFileName, QMap<function_t, functiongroup_t>* f2g, QMap<functiongroup_t, QSet<function_t>>* g2f);
void Otf_unifyFunctionGroupMembers(trace_t traceIdentifier, const Unifier<function_t>& functionUnifier, const Unifier<functiongroup_t>& functionGroupUnifier, QMap<function_t, functiongroup_t>* f2g, QMap<functiongroup_t, QSet<function_t>>* g2f);
QSet<process_t> Otf_processes(const QString& traceFileName);
void Otf_processTrace(const QString& traceFileName, process_t process, ProcessTrace* processTrace);
void Otf_trace(const QString& traceFileName, Trace* trace);

#endif /* OTF_HPP */
