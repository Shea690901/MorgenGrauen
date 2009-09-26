// MorgenGrauen MUDlib
//
// /std/hook_provider.c  - Hooksystem
//
// $Id: hook_provider.c 6365 2007-07-15 21:05:18Z Zesstra $

#pragma strong_types
#pragma save_types
#pragma no_clone
#pragma range_check
#pragma pedantic

#define NEED_PROTOTYPES
#include <hook.h>
#undef NEED_PROTOTYPES

/* hook mapping
   the list of all consumers in the following structure:
   ([hookid:
            ([
                H_HOOK_SURVEYOR:    ({({object,prio,time})}),
                H_HOOK_MODIFICATOR: ({({object,prio,time})}),
                H_DATA_MODIFICATOR: ({({object,prio,time})}),
                H_LISTENER:         ({({object,prio,time})}),
            )]
   )]
*/
nosave mapping hookMapping=([]);

/* Interfaces */
// in /sys/hooks.h

/* implementation */

// Debugging - ggf. ueberschreiben
protected status h_dbg() {return 0;}

void testOffer(int id,status stat){
    if(h_dbg()) {
        offerHook(id,stat);
    }
}

void testTrigger(int id,mixed data){
    if(h_dbg()) {
        HookFlow(id,data);
    }
}

nomask mapping HCopyHookMapping(){
    cleanHookMapping();
    return deep_copy(hookMapping);
}

nomask protected void cleanHookMapping(){
    int* hooks; // Die verschiedenen Sorten von Hooks
    int i; // Die Anzahl Hooktypen (Defend/DIE/MOVE/...)
    int k; // Zaehlvariable ueber die vier Sorten von Hooks
    int j; // Die Anzahl Hooks einer Sorte eines Typs.
    int* ar; // Die Infos eines (!) Hooks
    int stime;

    stime=time();
    hooks=m_indices(hookMapping);
    i=sizeof(hooks);
    i--;

    for(i;i>=0;i--){
        for(k=H_LISTENER;k>=H_HOOK_SURVEYOR;k--){
            j=sizeof(hookMapping[hooks[i]][k]);
            j--;
            for(j;j>=0;j--){
                ar=hookMapping[hooks[i]][k][j];
                if(pointerp(ar) 
		    && (!objectp(ar[H_OBJECT])
		      || (ar[H_TIME]>0 && ar[H_TIME]<stime))){
                    ar=hookMapping[hooks[i]][k][j]=0;
                }
            }

            // Bambi, Bereinigung der Hookliste von 0-Eintraegen, die sonst
            // in HookFlow() buggen.
            hookMapping[hooks[i]][k] -= ({0});
        }
    }
}

nomask protected void offerHook(int hookid, status offerstate)
{
    H_DMSG(sprintf("offerHook hookid %d offerstate %d\n",hookid,offerstate));
    if(hookid>0)
    {
        if(offerstate){
            if(!member(hookMapping,hookid)){
                hookMapping+=([hookid:([
                    H_HOOK_SURVEYOR:    allocate(MAX_SURVEYOR),
                    H_HOOK_MODIFICATOR: allocate(MAX_HOOK_MODIFICATOR),
                    H_DATA_MODIFICATOR: allocate(MAX_DATA_MODIFICATOR),
                    H_LISTENER:         allocate(MAX_LISTENER),
                ])]);
            }
        }
        else{
            if(member(hookMapping,hookid)){
                hookMapping-=([hookid]);
            }
        }
    }
    H_DMSG(sprintf("  result %O\n",hookMapping));
}

status HConsumerTypeIsAllowed(int type, object consumer){
    return 1;
}

status HPriorityIsAllowed(int prio, object consumer){
    return 1;
}

nomask protected int findObjectInTypeArr(mixed ar, object ob)
{
    int i,stime;

    stime=time();
    i=sizeof(ar)-1;

    for(i;i>=0;i--) {
        if(pointerp(ar[i]) && ar[i][H_OBJECT]==ob &&
	    (ar[i][H_TIME]<0 || ar[i][H_TIME]>stime)) {
	    return i;
        }
    }

    return -1;
}


nomask status HIsHookConsumer(int hookid, object consumer) {
    cleanHookMapping();
    if(hookConsumerInfo(hookid,consumer)!=0){
        return 1;
    }

    return 0;
}

nomask protected int* hookConsumerInfo(int hookid, object consumer)
{
    int* ret;
    int index;
    ret=0;

    H_DMSG(sprintf("hookConsumerInfo hookid %d consumer %O\n",hookid,consumer));

    if(consumer!=0 && member(hookMapping,hookid)){
        index=findObjectInTypeArr(hookMapping[hookid][H_HOOK_SURVEYOR],
	    consumer);
        if(index!=-1 && hookMapping[hookid][H_HOOK_SURVEYOR]){
            ret=({H_HOOK_SURVEYOR,index});
        }
        else{
            index=findObjectInTypeArr(hookMapping[hookid][H_HOOK_MODIFICATOR],
		consumer);
            if(index!=-1){
                ret=({H_HOOK_MODIFICATOR,index});
            }
            else{
                index=findObjectInTypeArr(hookMapping[hookid][H_DATA_MODIFICATOR],
		    consumer);
                if(index!=-1){
                    ret=({H_DATA_MODIFICATOR,index});
                }
                else{
                    // Bambi: hookMapping[hookid][H_LISTENER][H_OBJECT]
                    //        =>
                    //        hookMapping[hookid][H_LISTENER]
                    index=findObjectInTypeArr(hookMapping[hookid][H_LISTENER],
			consumer);
                    if(index!=-1){
                        ret=({H_LISTENER,index});
                    }
                }
            }
        }
    }
    H_DMSG(sprintf("  result %O\n",ret));
    return ret;
}

nomask mapping HListHookConsumer(int hookid){

    cleanHookMapping();

    if(member(hookMapping,hookid)) {
        return deep_copy(hookMapping[hookid]);
    }

    return 0;
}

nomask int* HListHooks(){

    cleanHookMapping();
    if(sizeof(hookMapping)) {
        return m_indices(hookMapping);
    }
    return ({});
}

status HUnregisterFromHook(int hookid, object consumer){
    int* info;
    status ret=0;

    H_DMSG(sprintf("HUnregisterFromHook hookid %d consumer %O\n",hookid,consumer));
    info=hookConsumerInfo(hookid,consumer);
    if(pointerp(info)) {
        hookMapping[hookid][info[H_TYPE]][info[H_INDEX]]=0;
	ret=1;
    }
    cleanHookMapping();
    H_DMSG(sprintf("  result %O\n",hookMapping));
    return ret;
}

nomask protected status askSurveyorsForRegistrationAllowance(mixed* surveyors,
    object consumer,int hookid,int hookprio,int consumertype) {
    int i;
    status tmp;

    H_DMSG(sprintf("askSurveyorsForRegistrationAllowance surveyors %O, "
	  "consumer %O, hookid %d, hookprio %d, consumertype %d\n",
	  surveyors,consumer,hookid,hookprio,consumertype));
    
    foreach(mixed surveyor: surveyors) {
        if (objectp(surveyor[H_OBJECT])) {
            H_DMSG(sprintf("Calling surveyor %O\n",surveyor[H_OBJECT]));
            tmp=surveyor[H_OBJECT]->HookRegistrationCallback(consumer, 
		hookid, this_object(), hookprio, consumertype);
            H_DMSG(sprintf("Result is %d\n",tmp));
            if(tmp==0){
                return 0;
            }
        }
    }
    return 1;
}

status HRegisterToHook(int hookid,object consumer, int hookprio, 
    int consumertype, int timeInSeconds) {
    status ret;
    mixed ar;
    mixed sortedSurveyors;
    int superseedeInd;
    int superseedePrio;
    int i;
    int stime;
    int regtime;
    stime=time();

    if(timeInSeconds>0){
        regtime=stime+timeInSeconds;
    }
    else{
        regtime=-1;
    }
    superseedePrio=hookprio;
    superseedeInd=-1;
    ret=0;

    H_DMSG(sprintf("HRegisterToHook hookid %d consumer %O\n hookprio %d "
	  "consumertype %d\n",hookid,consumer,hookprio,consumertype));
    
    cleanHookMapping();
    
    if(member(hookMapping,hookid) && consumer!=0 && 
	!HIsHookConsumer(hookid,consumer)) {
        H_DMSG("First barrier\n");
        if(H_CONSUMERCHECK(consumertype)!=-1 && 
	    HConsumerTypeIsAllowed(consumertype,consumer)){
            H_DMSG("Second barrier\n");
            if(H_HOOK_VALIDPRIO(hookprio)!=-1 && 
		HPriorityIsAllowed(hookprio,consumer)){
                H_DMSG("Third barrier\n");
                sortedSurveyors=
		  sort_array(hookMapping[hookid][H_HOOK_SURVEYOR],#'internalEntrySort)
		  -({0});
                if(askSurveyorsForRegistrationAllowance(sortedSurveyors,
		      consumer,hookid,hookprio,consumertype)){
                    H_DMSG("Fourth barrier\n");

                    ar=hookMapping[hookid][consumertype];
                    i=sizeof(ar)-1;
                    // Bambi: Bei einem leeren Mapping direkt eintragen.
                    if (i < 0)
                    {
                      ar = ({({consumer,hookprio,regtime})});
                      hookMapping[hookid][consumertype]=ar;
                      ret=1;
                    }
                    else
                    for(i;i>=0;i--){
                        // Ersetzbarer Eintrag in der Hook-Liste.
                        if(ar[i]==0 || !pointerp(ar[i][H_OBJECT]) 
			    || (ar[i][H_TIME]>0 && ar[i][H_TIME]<stime)){
                            H_DMSG("Found empty entry\n");
                            ar[i]=({consumer,hookprio,regtime});
                            // nicht noetig, wegen Array->Referenz
			    //hookMapping[hookid][consumertype]=ar;
                            ret=1;
                            break;
                        }
                        else if(ar[i][H_PRIO]>superseedePrio) {
                                superseedePrio=ar[i][H_PRIO];
                                superseedeInd=i;
                        }
                    }
                    if(ret==0 && superseedeInd!=-1) {
                        H_DMSG("Found superseedable entry\n");
                        ar[superseedeInd][H_OBJECT]->superseededHook(hookid,
			    this_object());
                        ar[superseedeInd]=({consumer,hookprio,regtime});
                        // nicht noetig, wegen Array -> Referenz
			//hookMapping[hookid][consumertype]=ar;
                        ret=1;
                    }

                }

            }
        }
    }
    H_DMSG(sprintf("  result %O\n",hookMapping));
    return ret;
}

nomask int internalEntrySort(mixed a, mixed b){
    int ret;

    if(a==0 && b!=0){
        ret=1;
    }
    else if(a==0 && b==0){
        ret=0;
    }
    else if(a!=0 && b==0){
        ret=0;
    }
    else if(a[H_PRIO]<=b[H_PRIO]){    
	ret=0;    
    }    
    else {    
	ret=1;    
    }
    return ret;
}

nomask protected status askSurveyorsForCancelAllowance(mixed surveyors,
    object modifiyingOb,mixed data,int hookid,int prio,object hookOb){

  foreach(mixed surveyor: surveyors) {
    if(objectp(surveyor[H_OBJECT]) &&     
	surveyor[H_OBJECT]->HookCancelAllowanceCallback(modifiyingOb,
	      hookid,hookOb,prio,data)==0) {
        return 0;
    }
  }
  return 1;
}

nomask protected status askSurveyorsForModificationAllowance(mixed surveyors,
    object modifiyingOb,mixed data,int hookid,int prio,object hookOb){
  
  foreach(mixed surveyor: surveyors) {
    if(objectp(surveyor[H_OBJECT]) &&     
	surveyor[H_OBJECT]->HookModificationAllowanceCallback(modifiyingOb,
	      hookid,hookOb,prio,data)==0){
        return 0;
    }
  }
  return 1;
}

nomask protected mixed HookFlow(int hookid, mixed hookdata){
    mixed sortedSurveyors;
    mixed sortedHMods;
    mixed sortedDMods;
    mixed sortedListener;
    status wasCancelled;
    mixed tmp;
    mixed ret;

    wasCancelled=0;
    ret=({H_NO_MOD,hookdata});

    // Bambi: Um die 0-Objekt-Eintraege rauszuwerfen.
    //cleanHookMapping();

    H_DMSG(sprintf("HookFlow hookid %d hookdate %O\n",hookid,hookdata));
    if(member(hookMapping,hookid)) {
        sortedSurveyors=
	  sort_array(hookMapping[hookid][H_HOOK_SURVEYOR],#'internalEntrySort)
	  -({0});

        sortedHMods=
	  sort_array(hookMapping[hookid][H_HOOK_MODIFICATOR],#'internalEntrySort)
	  -({0});

        sortedDMods=
	  sort_array(hookMapping[hookid][H_DATA_MODIFICATOR],#'internalEntrySort)
	  -({0});

	sortedListener=
	  sort_array(hookMapping[hookid][H_LISTENER],#'internalEntrySort) - ({0});

        // notify surveyors
	foreach(mixed surv: sortedSurveyors) {
	    if (!objectp(surv[H_OBJECT])) continue;
            tmp=surv[H_OBJECT]->HookCallback(this_object(),
		hookid,ret[H_RETDATA]);
            if(tmp[H_RETCODE]==H_CANCELLED){
                wasCancelled=1;
                ret[H_RETCODE]=H_CANCELLED;
            }
            else if(tmp[H_RETCODE]==H_ALTERED){
                    ret[H_RETCODE]=H_ALTERED;
                    ret[H_RETDATA]=tmp[H_RETDATA];
            }
	    if (wasCancelled) return ret;
        }

        // notify hmods
	foreach(mixed hmod: sortedHMods) {
	    if (!objectp(hmod[H_OBJECT])) continue;
            tmp=hmod[H_OBJECT]->HookCallback(this_object(),hookid,
		ret[H_RETDATA]);
            if(tmp[H_RETCODE]==H_CANCELLED) {
                if(askSurveyorsForCancelAllowance(sortedSurveyors,
		      hmod[H_OBJECT],hookdata,hookid,
		      hmod[H_PRIO],this_object())) {
                    // ask allowance in surveyors
                    wasCancelled=1;
                    ret[H_RETCODE]=H_CANCELLED;
                }
            }
            else if(tmp[H_RETCODE]==H_ALTERED) {
                    // ask allowance in surveyors
                if(askSurveyorsForModificationAllowance(sortedSurveyors,
		      hmod[H_OBJECT],hookdata,hookid,
		      hmod[H_PRIO],this_object())) {
                    ret[H_RETCODE]=H_ALTERED;
                    ret[H_RETDATA]=tmp[H_RETDATA];
                }
            }
	    if (wasCancelled) return ret;
        }

        // notify dmods
	foreach(mixed dmod: sortedDMods) {
	    if (!objectp(dmod[H_OBJECT])) continue;
            tmp=dmod[H_OBJECT]->HookCallback(this_object(),hookid,
		ret[H_RETDATA]);
            if(tmp[H_RETCODE]==H_ALTERED){
                // ask allowance in surveyors
                if(askSurveyorsForModificationAllowance(sortedSurveyors,
		      dmod[H_OBJECT],hookdata,hookid,
		      dmod[H_PRIO],this_object())){
                    ret[H_RETCODE]=H_ALTERED;
                    ret[H_RETDATA]=tmp[H_RETDATA];
                }
            }
        }

        // notify listener
	foreach(mixed listener: sortedListener) {
	  if (!objectp(listener[H_OBJECT])) continue;
          listener[H_OBJECT]->HookCallback(this_object(),hookid,ret[H_RETDATA]);
        }
    }

    return ret;
}

