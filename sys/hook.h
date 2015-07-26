// MorgenGrauen MUDlib
//
// /sys/hooks.h  - Hooksystem
//
// $Id: hook.h 7325 2009-10-27 21:24:49Z Zesstra $

#ifndef _HOOKS_H_
#define _HOOKS_H_

// global hooks
#define H_HOOK_MOVE           1
#define H_HOOK_DIE            2
#define H_HOOK_DEFEND         3
#define H_HOOK_ATTACK         4
#define H_HOOK_HP             5
#define H_HOOK_KP             6
#define H_HOOK_ATTACK_MOD     7
#define H_HOOK_ALCOHOL        8
#define H_HOOK_FOOD           9
#define H_HOOK_DRINK          10
#define H_HOOK_POISON         11
#define H_HOOK_CONSUME        12

// the possible consumer types
#define H_HOOK_SURVEYOR       1
#define H_HOOK_MODIFICATOR    2
#define H_DATA_MODIFICATOR    3
#define H_LISTENER            4
#define H_CONSUMERCHECK(x)    ((intp(x)&& x>0 && x<=H_LISTENER)?x:-1)

// priorities
#define H_HOOK_PRIORITYRANGE  3
#define H_HOOK_LIBBASEPRIO    0
#define H_HOOK_GUILBASEPRIO   (H_HOOK_LIBBASEPRIO+H_HOOK_PRIORITYRANGE)
#define H_HOOK_OTHERBASEPRIO  (H_HOOK_GUILBASEPRIO+H_HOOK_PRIORITYRANGE)
#define H_HOOK_PRIOCHECK(x)   ((intp(x) && x>0 && x<=H_HOOK_PRIORITYRANGE)?x:-1)
#define H_HOOK_LIBPRIO(x)     ((H_HOOK_PRIOCHECK(x)!=-1)?(H_HOOK_LIBBASEPRIO+x):-1)
#define H_HOOK_GUILDPRIO(x)   ((H_HOOK_PRIOCHECK(x)!=-1)?(H_HOOK_GUILBASEPRIO+x):-1)
#define H_HOOK_OTHERPRIO(x)   ((H_HOOK_PRIOCHECK(x)!=-1)?(H_HOOK_OTHERBASEPRIO+x):-1)
#define H_HOOK_VALIDPRIO(x)   ((intp(x) && x>0 && x<(H_HOOK_OTHERBASEPRIO+H_HOOK_PRIORITYRANGE))?x:-1)

// maximum consumer per type
#define MAX_SURVEYOR          1
#define MAX_HOOK_MODIFICATOR  2
#define MAX_DATA_MODIFICATOR  3
#define MAX_LISTENER          5

// data indices
#define H_OBJECT              0
#define H_PRIO                1
#define H_TIME                2
#define H_TYPE                0
#define H_INDEX               1
#define H_RETCODE             0
#define H_RETDATA             1

// return codes for HookFlow
#define H_NO_MOD              0
#define H_CANCELLED           1
#define H_ALTERED             2

// debugging
#define H_DMSG(x)             ((find_player("zesstra") && h_dbg()) ? \
    tell_object(find_player("zesstra"),x):0)

#endif //_HOOKS_H_

// prototypes 
#ifdef NEED_PROTOTYPES
// provider
#ifndef __HOOK_PROVIDER_PROTO
#define __HOOK_PROVIDER_PROTO

// list of offered hooks
nomask int* HListHooks();

// register to hook
status HRegisterToHook(int hookid,object consumer, int hookprio, 
    int consumertype, int timeInSeconds);

// unregister from hook
status HUnregisterFromHook(int hookid, object consumer);

// list consumers in hook mapping structure
nomask mapping HListHookConsumer(int hookid);

// check wether a consumertype is allowed
status HConsumerTypeIsAllowed(int type, object consumer);

// check wether a priority is allowed
status HPriorityIsAllowed(int prio, object consumer);

// check wether object is a consumer for a given hook
nomask status HIsHookConsumer(int hookid, object consumer);

// give copy of hook mapping
nomask mapping HCopyHookMapping();


/* internals */

// offer a hook or stop offering it
nomask protected void offerHook(int hookid, status offerstate);

// trigger a hook
nomask protected mixed HookFlow(int hookid, mixed hookdata);

// find an entry for an object in an consumer array
nomask protected int findObjectInTypeArr(mixed ar, object ob);

// check wether object is a consumer for a given hook and return
// array containing hook type and index in that type or 0
nomask protected int* hookConsumerInfo(int hookid, object consumer);

// clean internal mapping
nomask protected void cleanHookMapping();

// sort function for hook entries
nomask int internalEntrySort(mixed a, mixed b);

// surveyors are asked for cancellation permittance
nomask protected status askSurveyorsForCancelAllowance(mixed surveyors,
    object modifiyingOb,mixed data,int hookid,int prio,object hookOb);

// surveyors are asked for data change permittance
nomask protected status askSurveyorsForModificationAllowance(mixed surveyors,
    object modifiyingOb,mixed data,int hookid,int prio,object hookOb);

// surveyors are asked for registration permittance
nomask protected status askSurveyorsForRegistrationAllowance(mixed surveyors,
    object consumer,int hookid,int hookprio,int consumertype);

#endif // __HOOK_PROVIDER_PROTO
#endif // NEED_PROTOTYPES
