#include "common/otf/otf.hpp"

#include <otf.h>
#include <otf2/otf2.h>

static int handleDefProcess(void* userData, uint32_t stream, uint64_t id, const char* name, uint32_t parent) {
	Q_UNUSED(stream);
	Q_UNUSED(parent);
	auto processNames = ((QPair<QMap<uint32_t, QString>, QMap<process_t, QString>*>*)userData)->second;
	if (processNames->contains(id)) {
		qerr << "process " << id << " \"" << name << "\" has already been defined. aborting.\n";
		exit(-1);
	} else {
		(*processNames)[id] = QString("%1").arg(name);
	}
	return OTF_RETURN_OK;
}

static int handleOtfDefProcess(void* userData, uint32_t stream, uint32_t id, const char* name, uint32_t parent) {
	return handleDefProcess(userData, stream, (uint64_t) id, name, parent);
}

/* note: string ref needs to be resolved later */
static OTF2_CallbackCode handleOtf2DefProcess(void* userData, OTF2_LocationRef location, OTF2_StringRef name, OTF2_LocationType locationType, uint64_t numberOfEvents, OTF2_LocationGroupRef locationGroup ) {
	Q_UNUSED(numberOfEvents); Q_UNUSED(locationGroup);
	if (locationType == OTF2_LOCATION_TYPE_METRIC) { return OTF2_CALLBACK_SUCCESS; }
	if (handleDefProcess(userData, 0, location, QString("%1 %2").arg((uint32_t)name).arg(locationGroup).toStdString().c_str(), 0) == OTF_RETURN_OK) {
		return OTF2_CALLBACK_SUCCESS;
	} else {
		return OTF2_CALLBACK_ERROR;
	}
}

static int handleOtfDefFunction(void* userData, uint32_t stream, uint32_t id, const char* name, uint32_t group, uint32_t source) {
	Q_UNUSED(stream); Q_UNUSED(group); Q_UNUSED(source);
	auto functionNames = ((QPair<QMap<uint32_t, QString>, QMap<function_t, QString>*>*)userData)->second;
	if(functionNames->contains(id)) {
		qerr << "function " << id << " \"" << name << "\" has already been defined. aborting.\n";
		exit(-1);
	} else {
		(*functionNames)[id] = QString("%1").arg(name);
	}
	return OTF_RETURN_OK;
}

static int handleOtfDefFunction2(void* userData, uint32_t stream, uint32_t id, const char* name, uint32_t group, uint32_t source) {
	Q_UNUSED(stream); Q_UNUSED(source);
	auto* f2g = (QMap<function_t, functiongroup_t>*)userData;
	if(f2g->contains(id)) {
		qerr << "function " << id << " \"" << name << "\" has already been defined. aborting.\n";
		exit(-1);
	} else {
		(*f2g)[id] = group;
	}
	return OTF_RETURN_OK;
}

/* note: string ref needs to be resolved later */
static OTF2_CallbackCode handleOtf2DefFunction(void *userData, OTF2_RegionRef self, OTF2_StringRef name, OTF2_StringRef canonicalName, OTF2_StringRef description, OTF2_RegionRole regionRole, OTF2_Paradigm paradigm, OTF2_RegionFlag regionFlags, OTF2_StringRef sourceFile, uint32_t beginLineNumber, uint32_t endLineNumber) {
	Q_UNUSED(canonicalName); Q_UNUSED(description); Q_UNUSED(regionRole); Q_UNUSED(paradigm); Q_UNUSED(regionFlags); Q_UNUSED(sourceFile); Q_UNUSED(beginLineNumber); Q_UNUSED(endLineNumber); 
	if (handleOtfDefFunction(userData, 0, self, QString("%1").arg((uint32_t)name).toStdString().c_str(), 0, 0) == OTF_RETURN_OK) {
		return OTF2_CALLBACK_SUCCESS;
	} else {
		return OTF2_CALLBACK_ERROR;
	}
}

static OTF2_CallbackCode handleOtf2DefFunction2(void *userData, OTF2_RegionRef self, OTF2_StringRef name, OTF2_StringRef canonicalName, OTF2_StringRef description, OTF2_RegionRole regionRole, OTF2_Paradigm paradigm, OTF2_RegionFlag regionFlags, OTF2_StringRef sourceFile, uint32_t beginLineNumber, uint32_t endLineNumber) {
	Q_UNUSED(name);Q_UNUSED(canonicalName); Q_UNUSED(description); Q_UNUSED(regionRole); Q_UNUSED(regionFlags); Q_UNUSED(sourceFile); Q_UNUSED(beginLineNumber); Q_UNUSED(endLineNumber);

	uint32_t group;

	if (paradigm == OTF2_PARADIGM_MPI) {
		group = 2;
	} else if (paradigm == OTF2_PARADIGM_OPENMP) {
		group = 3;
	} else if (paradigm == OTF2_PARADIGM_PTHREAD) {
		group = 4;
	} else if (paradigm == OTF2_PARADIGM_CUDA) {
		group = 5;
	} else {
		group = 1;
	}

	if (handleOtfDefFunction2(userData, 0, self, "", group, 0) == OTF_RETURN_OK) {
		return OTF2_CALLBACK_SUCCESS;
	} else {
		return OTF2_CALLBACK_ERROR;
	}
}

/* note: no otf2 equivalent exists */
static int handleOtfDefFunctionGroup(void* userData, uint32_t stream, uint32_t id, const char* name) {
	Q_UNUSED(stream);
	auto& functionGroupNames = *((QMap<functiongroup_t, QString>*)userData);
	if(functionGroupNames.contains(id)) {
		qerr << "function group " << id << " \"" << name << "\" has already been defined. aborting.\n";
		exit(-1);
	} else {
		functionGroupNames[id] = QString(name);
	}
	return OTF_RETURN_OK;
}

static int handleEnter(void* userData, uint64_t time, uint32_t id, uint64_t process_, uint32_t source) {
	Q_UNUSED(source);
	auto& process = (*((Trace*)userData))[process_];

	FunctionCall f;
	f.id = id;
	f.begin = time;
	f.end = -1;

	if (process.isEmpty()) {
		process.append(f);
	} else {
		FunctionCall* last = &(process.last());
		FunctionCall* previous = nullptr;
		FunctionCall* wanted;

		for (;;) {
			if (last->calls.isEmpty() == false && last->end == -1) {
				previous = last;
				last = &(last->calls.last());
			} else if (last->end != -1) {
				wanted = previous;
				break;
			} else /* if (last->calls.isEmpty() && last->end == -1) */ {
				wanted = last;
				break;
			}
		}

		if (wanted == nullptr) {
			process.append(f);
		} else {
			assert(wanted->end == -1);
			wanted->calls.append(f);
		}
	}

	return OTF_RETURN_OK;
}

static int handleOtfEnter(void* userData, uint64_t time, uint32_t id, uint32_t process_, uint32_t source) {
	return handleEnter(userData, time, id, (uint64_t) process_, source);
}

static OTF2_CallbackCode handleOtf2Enter(OTF2_LocationRef location, OTF2_TimeStamp time, void* userData, OTF2_AttributeList* attributes, OTF2_RegionRef region) {
	Q_UNUSED(attributes);
	if (handleEnter(userData, time, region, location, 0) == OTF_RETURN_OK) {
		return OTF2_CALLBACK_SUCCESS;
	} else {
		return OTF2_CALLBACK_ERROR;
	}
}

static int handleLeave (void* userData, uint64_t time, uint32_t id, uint64_t process_, uint32_t source) {
	Q_UNUSED(source);
	auto& process = (*((Trace*)userData))[process_];

	FunctionCall* last = &(process.last());
	FunctionCall* previous = nullptr;
	FunctionCall* wanted = nullptr;
	for (;;) {
		if (last->calls.isEmpty() == false && last->end == -1) {
			previous = last;
			last = &(last->calls.last());
		} else if (last->end != -1) {
			wanted = previous;
			break;
		} else /* if (last->calls.isEmpty() && last->end == -1) */ {
			wanted = last;
			break;
		}
	}

	assert(id == 0 || wanted->id == (int32_t) id);
	assert(wanted->end == -1);

	wanted->end = time;

	return OTF_RETURN_OK;
}

static int handleOtfLeave (void* userData, uint64_t time, uint32_t id, uint32_t process_, uint32_t source) {
	return handleLeave (userData, time, id, (uint64_t) process_, source);
}

static OTF2_CallbackCode handleOtf2Leave(OTF2_LocationRef location, OTF2_TimeStamp time, void* userData, OTF2_AttributeList* attributes, OTF2_RegionRef region) {
	Q_UNUSED(attributes);
	if (handleLeave(userData, time, region, location, 0) == OTF_RETURN_OK) {
		return OTF2_CALLBACK_SUCCESS;
	} else {
		return OTF2_CALLBACK_ERROR;
	}
}

/* note: only used in otf2 */
static OTF2_CallbackCode handleOtf2DefStringProcess(void *userData, OTF2_StringRef self, const char *string) {
	auto& strings = ((QPair<QMap<uint32_t, QString>, QMap<process_t, QString>*>*) userData)->first;
	if (strings.contains(self)) {
		qerr << "string " << self << " \"" << string << "\" has already been defined. aborting.\n";
		exit(-1);
	} else {
		strings[self] = QString("%1").arg(string);
	}
	return OTF2_CALLBACK_SUCCESS;
}

static OTF2_CallbackCode handleOtf2DefStringFunction(void *userData, OTF2_StringRef self, const char *string) {
	auto& strings = ((QPair<QMap<uint32_t, QString>, QMap<function_t, QString>*>*) userData)->first;
	if (strings.contains(self)) {
		qerr << "string " << self << " \"" << string << "\" has already been defined. aborting.\n";
		exit(-1);
	} else {
		strings[self] = QString("%1").arg(string);
	}
	return OTF2_CALLBACK_SUCCESS;
}

struct Otf {
	enum class Which { Unknown, Otf1, Otf2 } which;

	OTF_HandlerArray *h;
	OTF_FileManager *f;
	OTF_Reader *r;

	OTF2_GlobalDefReaderCallbacks *hd2;
	OTF2_GlobalEvtReaderCallbacks *he2;
	OTF2_Reader *r2;
};

static void Otf_init(Otf *otf) {
	otf->which = Otf::Which::Unknown;

	otf->f = OTF_FileManager_open(900);
	assert(otf->f != nullptr);
	otf->h = OTF_HandlerArray_open();
	assert(otf->h != nullptr);
	otf->r = nullptr;

	otf->hd2 = OTF2_GlobalDefReaderCallbacks_New();
	otf->he2 = OTF2_GlobalEvtReaderCallbacks_New();
	otf->r2  = nullptr;
}

static void Otf_open(const QString& traceFileName, Otf *otf) {
	assert(otf->which == Otf::Which::Unknown); /* make sure nothing has been opened already */

	otf->r = OTF_Reader_open(traceFileName.toStdString().c_str(), otf->f);
	if (otf->r == nullptr) {
		otf->r2 = OTF2_Reader_Open(traceFileName.toStdString().c_str());
		if (otf->r2 == nullptr) {
			qout << "could not open \"" << traceFileName << "\". aborting.\n";
			exit(-1);
		} else {
			otf->which = Otf::Which::Otf2;
		}
	} else {
		otf->which = Otf::Which::Otf1;
	}
}

static void Otf_finalize(Otf *otf) {
	if (otf->r != nullptr) { OTF_Reader_close(otf->r);       otf->r = nullptr; }
	if (otf->h != nullptr) { OTF_HandlerArray_close(otf->h); otf->h = nullptr; }
	if (otf->f != nullptr) { OTF_FileManager_close(otf->f);  otf->f = nullptr; }

	if (otf->r2  != nullptr) { OTF2_Reader_Close(otf->r2);                     otf->r2  = nullptr; }
	if (otf->he2 != nullptr) { OTF2_GlobalEvtReaderCallbacks_Delete(otf->he2); otf->he2 = nullptr; }
	if (otf->hd2 != nullptr) { OTF2_GlobalDefReaderCallbacks_Delete(otf->hd2); otf->hd2 = nullptr; }
}

void Otf_processNames(const QString& traceFileName, QMap<process_t, QString>* processNames) {
	Otf otf;
	Otf_init(&otf);
	Otf_open(traceFileName, &otf);

	QPair<QMap<uint32_t, QString>, QMap<process_t, QString>*> userData; // strings, processNames
	userData.second = processNames;

	if (otf.which == Otf::Which::Otf1) {
		OTF_HandlerArray_setHandler(otf.h, (OTF_FunctionPointer*) handleOtfDefProcess, OTF_DEFPROCESS_RECORD);
		OTF_HandlerArray_setFirstHandlerArg(otf.h, &userData, OTF_DEFPROCESS_RECORD);

		OTF_Reader_readDefinitions(otf.r, otf.h);
	} else {
		OTF2_GlobalDefReaderCallbacks_SetLocationCallback(otf.hd2, &handleOtf2DefProcess);
		OTF2_GlobalDefReaderCallbacks_SetStringCallback(otf.hd2, &handleOtf2DefStringProcess);

		OTF2_Reader_RegisterGlobalDefCallbacks(otf.r2, OTF2_Reader_GetGlobalDefReader(otf.r2), otf.hd2, &userData);

		uint64_t dummyEventsRead;
		OTF2_Reader_ReadAllGlobalDefinitions(otf.r2, OTF2_Reader_GetGlobalDefReader(otf.r2), &dummyEventsRead);

		foreach (auto p, processNames->keys()) {
			unsigned long int stringId;
			unsigned int locationGroup;
			sscanf((*processNames)[p].toStdString().c_str(),"%lu %u", &stringId, &locationGroup);
			(*processNames)[p] = QString("%1:%2").arg(userData.first[(uint32_t) stringId]).arg(locationGroup);
		}
	}

	Otf_finalize(&otf);
}

void Otf_functionNames(const QString& traceFileName, QMap<function_t, QString>* functionNames) {
	Otf otf;
	Otf_init(&otf);
	Otf_open(traceFileName, &otf);

	QPair<QMap<uint32_t, QString>, QMap<function_t, QString>*> userData; // strings, functionNames
	userData.second = functionNames;

	if (otf.which == Otf::Which::Otf1) {
		OTF_HandlerArray_setHandler(otf.h, (OTF_FunctionPointer*) handleOtfDefFunction, OTF_DEFFUNCTION_RECORD);
		OTF_HandlerArray_setFirstHandlerArg(otf.h, &userData, OTF_DEFFUNCTION_RECORD);

		OTF_Reader_readDefinitions(otf.r, otf.h);
	} else {
		OTF2_GlobalDefReaderCallbacks_SetRegionCallback(otf.hd2, &handleOtf2DefFunction);
		OTF2_GlobalDefReaderCallbacks_SetStringCallback(otf.hd2, &handleOtf2DefStringFunction);

		OTF2_Reader_RegisterGlobalDefCallbacks(otf.r2, OTF2_Reader_GetGlobalDefReader(otf.r2), otf.hd2, &userData);

		uint64_t dummyEventsRead;
		OTF2_Reader_ReadAllGlobalDefinitions(otf.r2, OTF2_Reader_GetGlobalDefReader(otf.r2), &dummyEventsRead);

		foreach (auto f, functionNames->keys()) {
			unsigned long int stringId;
			sscanf((*functionNames)[f].toStdString().c_str(),"%lu", &stringId);
			(*functionNames)[f] = userData.first[(uint32_t) stringId];
		}
	}

	Otf_finalize(&otf);
}

void Otf_functionGroupNames(const QString& traceFileName, QMap<functiongroup_t, QString>* functionGroupNames) {
	Otf otf;
	Otf_init(&otf);
	Otf_open(traceFileName, &otf);

	if (otf.which == Otf::Which::Otf1) {
		OTF_HandlerArray_setHandler(otf.h, (OTF_FunctionPointer*) handleOtfDefFunctionGroup, OTF_DEFFUNCTIONGROUP_RECORD);
		OTF_HandlerArray_setFirstHandlerArg(otf.h, functionGroupNames, OTF_DEFFUNCTIONGROUP_RECORD);

		OTF_Reader_readDefinitions(otf.r, otf.h);
	} else {
		functionGroupNames->insert(1, "Application");
		functionGroupNames->insert(2, "MPI");
		functionGroupNames->insert(3, "OpenMP");
		functionGroupNames->insert(4, "Pthreads");
		functionGroupNames->insert(5, "CUDA");
	}

	Otf_finalize(&otf);
}

void Otf_functionGroupMembers(const QString& traceFileName, QMap<function_t, functiongroup_t>* f2g, QMap<functiongroup_t, QSet<function_t>>* g2f) {
	Otf otf;
	Otf_init(&otf);
	Otf_open(traceFileName, &otf);

	QMap<function_t, functiongroup_t>* userData = f2g;

	if (otf.which == Otf::Which::Otf1) {
		OTF_HandlerArray_setHandler(otf.h, (OTF_FunctionPointer*) handleOtfDefFunction2, OTF_DEFFUNCTION_RECORD);
		OTF_HandlerArray_setFirstHandlerArg(otf.h, userData, OTF_DEFFUNCTION_RECORD);

		OTF_Reader_readDefinitions(otf.r, otf.h);
	} else {
		OTF2_GlobalDefReaderCallbacks_SetRegionCallback(otf.hd2, &handleOtf2DefFunction2);

		OTF2_Reader_RegisterGlobalDefCallbacks(otf.r2, OTF2_Reader_GetGlobalDefReader(otf.r2), otf.hd2, userData);

		uint64_t dummyEventsRead;
		OTF2_Reader_ReadAllGlobalDefinitions(otf.r2, OTF2_Reader_GetGlobalDefReader(otf.r2), &dummyEventsRead);
	}

	QMapIterator<function_t, functiongroup_t> i(*f2g);
	while (i.hasNext()) {
		i.next();
		(*g2f)[i.value()].insert(i.key());
	}

	Otf_finalize(&otf);
}

void Otf_unifyFunctionGroupMembers(trace_t t, const Unifier<function_t>& functionUnifier, const Unifier<functiongroup_t>& functionGroupUnifier, QMap<function_t, functiongroup_t>* f2g, QMap<functiongroup_t, QSet<function_t>>* g2f) {

	QMap<function_t, functiongroup_t> f2g_ = *f2g;
	QMap<functiongroup_t, QSet<function_t>> g2f_ = *g2f;

	f2g->clear();
	g2f->clear();

	QMapIterator<function_t, functiongroup_t> i(f2g_);
	while (i.hasNext()) {
		i.next();

		function_t f = i.key();
		function_t f2 = functionUnifier.map(t, f);
		functiongroup_t g = i.value();
		functiongroup_t g2 = functionGroupUnifier.map(t, g);

		if (f2g->contains(f2) && (*f2g)[f2] != g2) {
			// no other behavior is possible with this implementation
			// function with the same name string are supposed to be the exact same function and therefore shouldn't be defined to be in two distinct groups
			qerr << QString("warning: unified function %1 is a member of two different function groups. assigning the same group %2.\n").arg(f2).arg(g2);
			qerr.flush();
		}

		f2g->insert(f2, g2);
		(*g2f)[g2].insert(f2);
	}

	Q_ASSERT(f2g_.size() >= f2g->size());
	Q_ASSERT(g2f_.size() >= g2f->size());
}

QSet<process_t> Otf_processes(const QString& traceFileName) {
	QSet<process_t> ret;

	Otf otf;

	Otf_init(&otf);
	Otf_open(traceFileName, &otf);

	if (otf.which == Otf::Which::Otf1) {
		OTF_MasterControl *c = OTF_Reader_getMasterControl(otf.r);

		int streamCount = (int) OTF_MasterControl_getCount(c);

		for (int i = 0; i < streamCount; i += 1) {
			OTF_MapEntry* e = OTF_MasterControl_getEntryByIndex(c, i);
			for (int j = 0; j < (int) e->n; j += 1) {
				ret.insert(e->values[j]);
			}
		}
	} else {
		QMap<process_t, QString> processes;
		QPair<QMap<uint32_t, QString>, QMap<process_t, QString>*> userData; // strings, processNames
		userData.second = &processes;

		OTF2_GlobalDefReaderCallbacks_SetLocationCallback(otf.hd2, &handleOtf2DefProcess);

		OTF2_Reader_RegisterGlobalDefCallbacks(otf.r2, OTF2_Reader_GetGlobalDefReader(otf.r2), otf.hd2, &userData);

		uint64_t dummyEventsRead;
		OTF2_Reader_ReadAllGlobalDefinitions(otf.r2, OTF2_Reader_GetGlobalDefReader(otf.r2), &dummyEventsRead);

		ret = processes.keys().toSet();
	}

	Otf_finalize(&otf);

	return ret;
}

void Otf_trace(const QString& traceFileName, Trace* trace) {

	Otf otf;

	Otf_init(&otf);
	Otf_open(traceFileName, &otf);

	if (otf.which == Otf::Which::Otf1) {
		OTF_HandlerArray_setHandler(otf.h, (OTF_FunctionPointer*) handleOtfEnter, OTF_ENTER_RECORD);
		OTF_HandlerArray_setFirstHandlerArg(otf.h, trace, OTF_ENTER_RECORD);
		OTF_HandlerArray_setHandler(otf.h, (OTF_FunctionPointer*) handleOtfLeave, OTF_LEAVE_RECORD);
		OTF_HandlerArray_setFirstHandlerArg(otf.h, trace, OTF_LEAVE_RECORD);

		OTF_Reader_readEvents(otf.r, otf.h);
	} else {

		QSet<process_t> processes = Otf_processes(traceFileName);

		foreach (process_t p, processes) { OTF2_Reader_SelectLocation(otf.r2, p); }

		bool successfullyOpenedDefinitions = OTF2_Reader_OpenDefFiles(otf.r2) == OTF2_SUCCESS;

		OTF2_Reader_OpenEvtFiles(otf.r2);

		foreach (process_t p, processes) {
			if (successfullyOpenedDefinitions) { /* needed so otf2 can apply a mapping to map local id's to global ones */
				OTF2_DefReader* dr = OTF2_Reader_GetDefReader(otf.r2, p);
				if (dr != nullptr) {
					uint64_t dummy = 0;
					OTF2_Reader_ReadAllLocalDefinitions(otf.r2, dr, &dummy);
					OTF2_Reader_CloseDefReader(otf.r2, dr);
				}
			}
			OTF2_Reader_GetEvtReader(otf.r2, p); /* discard return value */
		}

		if (successfullyOpenedDefinitions) { OTF2_Reader_CloseDefFiles(otf.r2); }

		OTF2_GlobalEvtReaderCallbacks_SetEnterCallback(otf.he2, &handleOtf2Enter);
		OTF2_GlobalEvtReaderCallbacks_SetLeaveCallback(otf.he2, &handleOtf2Leave);

		auto er = OTF2_Reader_GetGlobalEvtReader(otf.r2);

		OTF2_Reader_RegisterGlobalEvtCallbacks(otf.r2, er, otf.he2, &trace);

		uint64_t dummyEventsRead;
		OTF2_Reader_ReadAllGlobalEvents(otf.r2, er, &dummyEventsRead);

		OTF2_Reader_CloseGlobalEvtReader(otf.r2, er);
		OTF2_Reader_CloseEvtFiles(otf.r2);
	}

	Otf_finalize(&otf);
}

void Otf_processTrace(const QString& traceFileName, process_t process, ProcessTrace* processTrace) {

	Otf otf;

	Otf_init(&otf);
	Otf_open(traceFileName, &otf);

	Trace trace;

	if (otf.which == Otf::Which::Otf1) {
		OTF_Reader_setProcessStatusAll(otf.r, 0);
		OTF_Reader_setProcessStatus(otf.r, process, 1);


		OTF_HandlerArray_setHandler(otf.h, (OTF_FunctionPointer*) handleOtfEnter, OTF_ENTER_RECORD);
		OTF_HandlerArray_setFirstHandlerArg(otf.h, &trace, OTF_ENTER_RECORD);
		OTF_HandlerArray_setHandler(otf.h, (OTF_FunctionPointer*) handleOtfLeave, OTF_LEAVE_RECORD);
		OTF_HandlerArray_setFirstHandlerArg(otf.h, &trace, OTF_LEAVE_RECORD);

		OTF_Reader_readEvents(otf.r, otf.h);
	} else {
		OTF2_Reader_SelectLocation(otf.r2, process);

		bool successfullyOpenedDefinitions = OTF2_Reader_OpenDefFiles(otf.r2) == OTF2_SUCCESS;

		OTF2_Reader_OpenEvtFiles(otf.r2);

		if (successfullyOpenedDefinitions) { /* needed so otf2 can apply a mapping to map local id's to global ones */
			OTF2_DefReader* dr = OTF2_Reader_GetDefReader(otf.r2, process);
			if (dr != nullptr) {
				uint64_t dummy = 0;
				OTF2_Reader_ReadAllLocalDefinitions(otf.r2, dr, &dummy);
				OTF2_Reader_CloseDefReader(otf.r2, dr);
			}
		}
		OTF2_Reader_GetEvtReader(otf.r2, process); /* discard return value */

		if (successfullyOpenedDefinitions) { OTF2_Reader_CloseDefFiles(otf.r2); }

		OTF2_GlobalEvtReaderCallbacks_SetEnterCallback(otf.he2, &handleOtf2Enter);
		OTF2_GlobalEvtReaderCallbacks_SetLeaveCallback(otf.he2, &handleOtf2Leave);

		auto er = OTF2_Reader_GetGlobalEvtReader(otf.r2);

		OTF2_Reader_RegisterGlobalEvtCallbacks(otf.r2, er, otf.he2, &trace);

		uint64_t dummyEventsRead;
		OTF2_Reader_ReadAllGlobalEvents(otf.r2, er, &dummyEventsRead);

		OTF2_Reader_CloseGlobalEvtReader(otf.r2, er);
		OTF2_Reader_CloseEvtFiles(otf.r2);
	}

	if (trace.keys().contains(process) == false) {
		qerr << QString("process %1 was read. but probably no events to read were present. warning.").arg(process) << endl;
		qerr.flush();
		assert(trace.keys().contains(process));
	}
	assert(trace.keys().size() == 1);

	*processTrace = trace[process];

	Otf_finalize(&otf);
}

