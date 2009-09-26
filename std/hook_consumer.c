// MorgenGrauen MUDlib
//
// /std/hook_consumer.c  - Basisklasse fuer Hook-Objekte.
//
// $Id: hook_consumer.c 6365 2007-07-15 21:05:18Z Zesstra $


#pragma strong_types
#pragma save_types
#pragma no_clone
#pragma pedantic
#pragma range_check

#include <hook.h>

/* override as wished and needed*/

status HookRegistrationCallback(object registringObject, int hookid, object hookSource, int registringObjectsPriority, int registringObjectsType){
	return 1;
}

status HookCancelAllowanceCallback(object cancellingObject, int hookid, object hookSource, int cancellingObjectsPriority, mixed hookData){
	return 1;
}

status HookModificationAllowanceCallback(object modifyingObject, int hookid, object hookSource, int modifyingObjectsPriority, mixed hookData){
	return 1;
}

mixed HookCallback(object hookSource, int hookid, mixed hookData){
	return ({H_NO_MOD,hookData});
}

