#ifndef PROFILE_HPP
#define PROFILE_HPP

#include <functional>

#include "common/measure.hpp"
#include "common/otf/otf.hpp"
#include "common/prereqs.hpp"

template<typename Identifier>
struct Profile {
	QMap<Identifier, Measure<int64_t>> invocations;
	QMap<Identifier, Measure<int64_t>> exclusiveTime;
	QMap<Identifier, Measure<int64_t>> inclusiveTime;
};

typedef QPair<function_t, function_t> FunctionPair;
typedef QList<function_t> FunctionTuple;

bool operator<(const FunctionTuple& a, const FunctionTuple& b);

typedef Profile<functiongroup_t> FunctionGroupProfile;
typedef Profile<function_t>      CallListProfile;
typedef Profile<FunctionPair>    CallMatrixProfile;
typedef Profile<FunctionTuple>   CallTreeProfile;

template<typename Identifier>
void Profile_add(const Identifier& i, int64_t inv, int64_t exc, int64_t inc, Profile<Identifier>* p);

inline void FunctionGroupProfile_add(functiongroup_t g     , int64_t inv, int64_t exc, int64_t inc, FunctionGroupProfile* p) { Profile_add<functiongroup_t>(g , inv, exc, inc, p); }
inline void CallListProfile_add     (function_t f          , int64_t inv, int64_t exc, int64_t inc, CallListProfile*      p) { Profile_add<function_t>     (f , inv, exc, inc, p); }
inline void CallMatrixProfile_add   (const FunctionPair& p_, int64_t inv, int64_t exc, int64_t inc, CallMatrixProfile*    p) { Profile_add<FunctionPair>   (p_, inv, exc, inc, p); }
inline void CallTreeProfile_add     (const FunctionTuple& t, int64_t inv, int64_t exc, int64_t inc, CallTreeProfile*      p) { Profile_add<FunctionTuple>  (t , inv, exc, inc, p); }

template<typename Identifier>
void Profile_finalize(Profile<Identifier>* p);

inline void FunctionGroupProfile_finalize(FunctionGroupProfile* p) { Profile_finalize<functiongroup_t>(p); }
inline void CallListProfile_finalize     (CallListProfile*      p) { Profile_finalize<function_t>     (p); }
inline void CallMatrixProfile_finalize   (CallMatrixProfile*    p) { Profile_finalize<FunctionPair>   (p); }
inline void CallTreeProfile_finalize     (CallTreeProfile*      p) { Profile_finalize<FunctionTuple>  (p); }

template<typename Identifier>
QString Profile_print(const Profile<Identifier>& p, std::function<QString(const Identifier&)> printIdentifier, const QString& indent);

inline QString FunctionGroupProfile_print(const FunctionGroupProfile& p, std::function<QString(functiongroup_t)> printIdentifier     , const QString& indent) { return Profile_print<functiongroup_t>(p, printIdentifier, indent); }
inline QString CallListProfile_print     (const CallListProfile& p     , std::function<QString(function_t)> printIdentifier          , const QString& indent) { return Profile_print<function_t>     (p, printIdentifier, indent); }
inline QString CallMatrixProfile_print   (const CallMatrixProfile& p   , std::function<QString(const FunctionPair&)> printIdentifier , const QString& indent) { return Profile_print<FunctionPair>   (p, printIdentifier, indent); }
inline QString CallTreeProfile_print     (const CallTreeProfile& p     , std::function<QString(const FunctionTuple&)> printIdentifier, const QString& indent) { return Profile_print<FunctionTuple>  (p, printIdentifier, indent); }

#include "profile/profile.inl"

#endif /* PROFILE_HPP */
