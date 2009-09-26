/*  MorgenGrauen MUDlib
    /p/daemon/errord.c
    speichert Fehler und Warnungen
    Autor: Zesstra
    $Id: errord.c 7155 2009-02-25 21:20:50Z Zesstra $
    ggf. Changelog:
*/

#pragma strict_types
#pragma no_clone
#pragma no_shadow
#pragma no_inherit
#pragma pedantic
#pragma range_check
#pragma warn_deprecated

#include <properties.h>
#include <wizlevels.h>
#include <defines.h>
#include <debug_info.h>
#include <commands.h>
#include <wizlevels.h>
#include <mail.h>
#define __NEED_IMPLEMENTATION__
#include "errord.h"
#undef __NEED_IMPLEMENTATION__

#define SAVEFILE  (__DIR__+"ARCH/errord")

#define TI this_interactive()

mapping errors;       //alle ungeloesten Fehler
mapping resolved;     //alle geloesten Fehler

nosave mapping lasterror;   // die letzen 5 jeder Art.

//Fehler registrieren
//Diese Funktion darf nicht mehr als 200k Ticks verbrauchen und wird nur vom
//Master gerufen!
string LogError(string msg,string prg,string curobj,int line,mixed culprit,
    int caught) {
    mixed stacktrace, cli, tienv;
    string uid, hashkey, loadname, titp, cverb;
    mapping errs, err;
    //DEBUG(sprintf("LogError: Prog: %O, Obj: %O,\n",prg,curobj));

    //darf nur vom Master gerufen werden
    if (!extern_call() || 
	(previous_object() && previous_object() != master()))
	return 0;

    //UID bestimmen
    uid=(string)master()->creator_file(curobj);
    //DEBUG(sprintf("LogError: UID: %s\n",uid));

    //Loadname (besser als BP, falls rename_object() benutzt wurde) bestimmen
    if (!stringp(curobj) || !strlen(curobj))
	loadname = curobj = "<Unbekannt>";
    else
	//load_name nimmt Strings und Objects und konstruiert den loadname,
	//wie er sein sollte, wenn das Objekt nicht mehr existiert.
	loadname=load_name(curobj);

    if (!stringp(loadname) || !strlen(loadname))
	//hier sollte man reinkommen, falls curobj ein 'kaputter' Name ist.
	loadname="<Illegal object name>";

    //Hashkey bestimmen, testweise Name der Blueprint des buggenden Objekts 
    //+ Zeilennr.
    //TODO: evtl. sha1() statt md5()?
    hashkey=md5(sprintf("%s%d",loadname,line));
    DEBUG(sprintf("LogError: Hashkey: %s",hashkey));

    // schonmal Speichern anstossen
    if (find_call_out("save_me")==-1)
	//Zeitverzoegert, falls viele Fehler direkt hintereinander kommen.
	call_out("save_me",10);

    // ggf. vorhandenen Fehler suchen. Dieser muss leider in allen UIDs
    // gesucht werden. Falls ein Fehler von seiner urspruenglichen in eine
    // neue UID verschoben wurde, wuerde es sonst ggf. 2 Fehler mit dem
    // gleichen hashkey in verschiedenen UIDs geben.
    err = getError(hashkey, T_RTERROR);
    errs = errors[T_RTERROR];
    if (!member(errs,uid)) errs+=([uid:([])]);
    if (!mappingp(errs[uid])) errs[uid]=([]);

    if (mappingp(err) && sizeof(err)) {
	//wenn dieser Hashkey / Fehler schon drin ist im Mapping und nicht als
	//geloescht markiert, zaehlen wir nur den Counter hoch und
	//aktualisieren den Zeitstempel, sonst wird komplett geloescht.
	if (err[F_STATE] & STAT_DELETED)
	  efun::m_delete(errs[err[F_UID]],hashkey);
	else {
	  err[F_COUNT]++;
	  err[F_MODSTAMP]=time();
	  return hashkey; // und fertig
	}
    }
    //sonst fuegen wir einen neuen Eintrag hinzu
    //DEBUG(sprintf("LogError: OBJ: %s, BP: %s",curobj,loadname));
    // Wenn Fehler im HB, Objektnamen ermitteln
    if (objectp(culprit))
	culprit=object_name(culprit);
    else culprit=0;	
    //stacktrace holen
    if (caught)
	stacktrace=debug_info(DINFO_TRACE,DIT_ERROR);
    else
	stacktrace=debug_info(DINFO_TRACE,DIT_UNCAUGHT_ERROR);
    //gibt es einen TI/TP? Name mit erfassen
    if(objectp(this_interactive())) {
	titp=getuid(this_interactive());
	tienv=environment(this_interactive());	
    }
    else if (objectp(PL) && query_once_interactive(PL)) {
        titp=getuid(PL);
        tienv=environment(PL);	
    }
    else if (objectp(PL)) {
        titp=object_name(PL);
        tienv=environment(PL);
    }
    else
        titp="";
    if (objectp(tienv)) tienv=object_name(tienv);

    // Mal schauen, ob der Commandstack auch was fuer uns hat. ;-)
    if (pointerp(cli=command_stack()) && sizeof(cli)) {
        cverb=cli[0][CMD_VERB];
        cli=cli[0][CMD_TEXT];
    }
    else cli=0; //nicht unwichtig, sonst u.U. Array! ;-)

    err=([F_TYPE: T_RTERROR,
          F_MODSTAMP: time(),
          F_CREATESTAMP: time(),		     
          F_PROG: prg,
          F_OBJ: curobj,
	  F_LOADNAME: loadname,
          F_LINE: line,
          F_MSG: msg,
          F_HB_OBJ: culprit,
          F_CAUGHT: caught,
          F_TITP: titp,
          F_STACK: stacktrace,
          F_CLI: (string)cli,
          F_VERB: cverb,
          F_COUNT: 1,
          F_TIENV: (string)tienv,
          F_UID: uid,
          F_HASHKEY: hashkey,
	  ]);
    // Keys ohne Werte ausfiltern
    foreach(string key, mixed val: err) {
      if (!val) efun::m_delete(err,key);
    }
    errs[uid][hashkey]=err;

    lasterror[T_RTERROR]=hashkey;
//    DEBUG(sprintf("LogError: Fehlereintrag:\n%O\n",
//	  errors[uid][hashkey]));
//    DEBUG(sprintf("LogError: Verbrauchte Ticks: %d\n",
//	  200000-get_eval_cost()));
    return(hashkey);
}

//Warnungen registrieren
//Diese Funktion darf nicht mehr als 200k Ticks verbrauchen und wird nur vom
//Master gerufen!
string LogWarning(string msg,string prg,string curobj,int line, int in_catch) {
    string uid,hashkey,loadname,titp, cverb;
    mixed tienv, cli;

    //DEBUG(sprintf("LogWarning: Prog: %O, Obj: %O,\n",prg,curobj));

    //darf nur vom Master gerufen werden
    if (!extern_call() || 
	(previous_object() && previous_object() != master()))
	return 0;

    //UID bestimmen
    uid=(string)master()->creator_file(curobj);
    //DEBUG(sprintf("LogWarning UID: %s\n",uid));

    //Loadname (besser als BP, falls rename_object() benutzt wurde) bestimmen
    if (!stringp(curobj) || !strlen(curobj))
	loadname = curobj = "<Unbekannt>";
    else
	//load_name nimmt Strings und Objects und konstruiert den loadname,
	//wie er sein sollte, wenn das Objekt nicht mehr existiert.
	loadname=load_name(curobj);

    if (!stringp(loadname) || !strlen(loadname))
	//hier sollte man reinkommen, falls curobj ein 'kaputter' Name ist.
	loadname="<Illegal object name>";

    //DEBUG(sprintf("LogWarning: OBJ: %s, BP: %s\n",curobj,blue));

    //Hashkey bestimmen, testweise Name der Blueprint des buggenden Objekts 
    //+ Zeilennr.
    hashkey=md5(sprintf("%s%d",loadname,line));
    //DEBUG(sprintf("LogWarning: Hashkey: %s",hashkey));

    // schonmal Speichern anstossen
    if (find_call_out("save_me")==-1)
	//Zeitverzoegert, falls viele Fehler direkt hintereinander kommen.
	call_out("save_me",10);	

    // ggf. vorhandene Warnung suchen. Dieser muss leider in allen UIDs
    // gesucht werden. Falls eine Warnung von ihrer urspruenglichen in eine
    // neue UID verschoben wurde, wuerde es sonst ggf. 2 Warnungen mit dem
    // gleichen hashkey in verschiedenen UIDs geben.
    mapping err = getError(hashkey, T_RTWARN);
    mapping warnings=errors[T_RTWARN];
    if (!member(warnings,uid)) warnings+=([uid:([])]);
    if (!mappingp(warnings[uid])) warnings[uid]=([]);
    
    if (mappingp(err) && sizeof(err)) {
	//wenn dieser Hashkey / Fehler schon drin ist im Mapping und nicht als
	//geloescht markiert, zaehlen wir nur den Counter hoch und
	//aktualisieren den Zeitstempel, sonst erstmal komplett loeschen.
	if (err[F_STATE] & STAT_DELETED)
	  efun::m_delete(warnings[err[F_UID]],hashkey);
	else {
	  err[F_COUNT]++;
	  err[F_MODSTAMP]=time();
	  return hashkey; // und fertig.
	}
    }
    //sonst fuegen wir einen neuen Eintrag hinzu
    //gibt es einen TI/TP? Name mit erfassen
    if(objectp(this_interactive())) {
        titp=getuid(this_interactive());
        tienv=environment(this_interactive());
    }
    else if (objectp(PL) && query_once_interactive(PL)) {
        titp=getuid(PL);
        tienv=environment(PL);
    }
    else if (objectp(PL)) {
        titp=object_name(PL);
        tienv=environment(PL);
    }
    else
        titp="";
    if (objectp(tienv)) tienv=object_name(tienv);
    // Mal schauen, ob der Commandstack auch was fuer uns hat. ;-)
    if (pointerp(cli=command_stack()) && sizeof(cli)) {
        cverb=cli[0][CMD_VERB];
        cli=cli[0][CMD_TEXT];
    }
    else cli=0; //nicht unwichtig, sonst u.U. Array! ;-)

    err=([F_TYPE: T_RTWARN,
          F_MODSTAMP: time(),
          F_CREATESTAMP: time(),		     
          F_PROG: prg,
          F_OBJ: curobj,
          F_LOADNAME: loadname,
          F_LINE: line,
          F_MSG: msg,
          F_CAUGHT: in_catch,
          F_TITP: titp,
          F_CLI: (string)cli,
          F_VERB: cverb,
          F_COUNT: 1,
          F_TIENV: (string)tienv,
          F_UID: uid,
          F_HASHKEY: hashkey,
          ]);
    // Keys ohne Werte ausfiltern
    foreach(string key, mixed val: err) {
      if (!val) efun::m_delete(err,key);
    }
    warnings[uid][hashkey]=err;

    lasterror[T_RTWARN]=hashkey;
//    DEBUG(sprintf("LogWarning: Warnungseintrag:\n%O\n",
//	  warnings[uid][hashkey]));
//    DEBUG(sprintf("LogWarning: Verbrauchte Ticks: %d\n",
//	  200000-get_eval_cost()));
    return(hashkey);
}

//Warnungen und Fehler beim Kompilieren  registrieren
//Diese Funktion darf nicht mehr als 200k Ticks verbrauchen und wird nur vom
//Master gerufen!
string LogCompileProblem(string file,string msg,int warn) {
    string uid,hashkey;
    mapping mapp, err;

    //DEBUG(sprintf("LogCompileProblem: Prog: %O, Obj: %O,\n",file,msg));

    //darf nur vom Master gerufen werden
    if (!extern_call() || 
	(previous_object() && previous_object() != master()))
	return 0;

    //loggen wir fuer das File ueberhaupt?
    if (member(BLACKLIST,explode(file,"/")[<1])>=0)
	return 0;

    //UID bestimmen
    uid=(string)master()->creator_file(file);
    //DEBUG(sprintf("LogCompileProblem UID: %s\n",uid));

    //Hashkey bestimmen, in diesem Fall einfach, wir koennen die
    //Fehlermeldunge selber nehmen.
    hashkey=md5(msg);
    //DEBUG(sprintf("LogCompileProblem: Hashkey: %s",hashkey));

    // ggf. vorhandene Warnung suchen. Dieser muss leider in allen UIDs
    // gesucht werden. Falls eine Warnung von ihrer urspruenglichen in eine
    // neue UID verschoben wurde, wuerde es sonst ggf. 2 Warnungen mit dem
    // gleichen hashkey in verschiedenen UIDs geben.
    // ausserdem das richtige Mapping finden.
    if (warn) {
      err = getError(hashkey, T_CTWARN);
      mapp=errors[T_CTWARN];
    }
    else {
      err = getError(hashkey, T_CTERROR);
      mapp=errors[T_CTERROR];
    }

    // schonmal speichern anstossen
    if (find_call_out("save_me")==-1)
	//Zeitverzoegert, falls viele Fehler direkt hintereinander kommen.
	call_out("save_me",10);
    
    if (mappingp(err) && sizeof(err)) {
        //wenn dieser hashkey / Fehler schon drin ist im Mapping und nicht
        //als geloescht markiert, zaehlen wir nur den Counter hoch und
	//aktualisieren den Zeitstempel, sonst loeschen.
	if (err[F_STATE] & STAT_DELETED)
	    efun::m_delete(mapp[err[F_UID]], hashkey);
	else {
	  err[F_COUNT]++;
	  err[F_MODSTAMP]=time();
	  return hashkey; // und fertig
	}
    }
    // neuen Eintrag
    if (!member(mapp,uid)) mapp+=([uid:([])]);
    if (!mappingp(mapp[uid])) mapp[uid]=([]);
    err=([F_TYPE: (warn ? T_CTWARN : T_CTERROR),
          F_MODSTAMP: time(),
          F_CREATESTAMP: time(),		     
          F_LOADNAME: file,
          F_MSG: msg,
          F_COUNT: 1,
          F_UID: uid,
          F_HASHKEY: hashkey,
          ]);
    mapp[uid][hashkey]=err;

    if (warn) lasterror[T_CTWARN]=hashkey;
    else lasterror[T_CTERROR]=hashkey;

//    DEBUG(sprintf("LogCompileProblem: Eintrag:\n%O\n",
//	  (warn ? ctwarnings[uid][hashkey] : cterrors[uid][hashkey])));
//   DEBUG(sprintf("LogCompileProblem: Verbrauchte Ticks: %d\n",
//	  200000-get_eval_cost()));
   return(hashkey);
}

/* ****************  Public Interface ****************** */

// den letzten Eintrag den jeweiligen Typ liefern.
mapping QueryLastError(int type) {

    if (!member(lasterror,type)) return ([]);
    //einfach den kompletten letzten Eintrag zurueckliefern
    return(QueryError(lasterror[type],type));
}

//Einen bestimmten Fehler suchen, Hashkey ist zwar eindeutig, aber Angabe der
//der UID ist deutlich schneller. Weglassen des Typs nochmal langsamer. ;-)
//Potentiell also sehr teuer, wenn man UID oder UID+Typ weglaesst.
varargs mapping QueryError(string hashkey, int type, string uid) {
    mapping err;

    // Wenn Fehler bei den erledigten steht, einfach den zurueckgeben.
    if (member(resolved, hashkey))
      return(filter_private(hashkey,resolved[hashkey]));

    err=getError(hashkey,type,uid);
    if (!mappingp(err)) return(([]));

    err[F_READSTAMP]=time();
    //filter_private macht ne Kopie des Fehlereintrags.
    return(filter_private(hashkey, err));
    return(([]));
}

mapping QueryErrors(int type, string uid) {
//liefert alle Fehler eines Typs und einer UID
    mapping mapp;
    if (!type || !stringp(uid) || !strlen(uid))
	return(([]));
    mapp=errors[type];
    if (!member(mapp,uid) 
	|| !mappingp(mapp[uid]))
	return(([]));
    //filter_private macht Kopien der Fehlereintraege
    return(map(mapp[uid],#'filter_private));
}

varargs string *QueryErrorIDs(int type, string uid) {
  //alle Errorkeys (einer UID) liefern
    mapping mapp;
    string *uids,*ids;

    if (!type)
	return(({}));
    //fuer eine UID
    if (stringp(uid) && strlen(uid)) {
      mapp=errors[type];
      if (!member(mapp,uid) || !mappingp(mapp[uid]))
	  return(({}));
      return(m_indices(mapp[uid]));
    }
    //fuer alle UIDs
    uids=QueryUIDsForType(type);
    ids=({});
    foreach(uid: uids) {
	ids+=QueryErrorIDs(type,uid);
    }
    return(ids);
}

varargs string *QueryUIDsForType(int type) {
    //liefert alle UIDs fuer einen Fehlertyp oder fuer alle Fehlertypen
    string *uids;

    if (type) {
	return(m_indices(errors[type]));
    }

    uids=({});
    foreach(type: m_indices(errors)) {
	uids+=QueryUIDsForType(type);
    }
    return(uids);
}

//Wieviele unterschiedliche Fehler in diesem Typ?
varargs int QueryUniqueErrorNo(int type, string uid) {
    int zahl; 
    
    if (type && stringp(uid) || strlen(uid))
	return(queryUniqueErrorNoForUID(type,uid));

    //wenn kein Typ gegeben, wirds aufwaendig...
    if (!type) {
	foreach(type: m_indices(errors)) {
	    //rekursion ist 'toll'... :-/
	    zahl+=QueryUniqueErrorNo(type);
	}
	return(zahl); //puuh.
    }

    //ok, zaehlen wir ueber alle UIDs.
    foreach(uid: QueryUIDsForType(type)) {
	zahl+=queryUniqueErrorNoForUID(type,uid);
    }
    return(zahl);
}

//Wieviele Fehler insgesamt in diesem Typ=?
varargs int QueryErrorNo(int type,string uid) {
    //For now
    return(QueryUniqueErrorNo(type,uid));
}

//Einen bestimmten Fehler loeschen, Hashkey ist zwar eindeutig, aber Angabe der
//der UID ist deutlich schneller. Weglassen des Typs nochmal langsamer. ;-)
//Potentiell also sehr teuer, wenn man UID oder UID+Typ weglaesst.
varargs int ToggleDeleteError(string hashkey, string note, int type, 
                              string uid) {
    mapping err;

    err=getError(hashkey,type,uid);
    if (!mappingp(err)) return(-1);

    if (!access_check(err[F_UID], M_WRITE))
	//zugriff zum Schreiben nicht gestattet
	return(-2);

    if (!stringp(note)) note=0;

    call_out(#'save_me,2);

    if (!pointerp(err[F_NOTES]))
	err[F_NOTES]=({});
    err[F_STATE] ^=  STAT_DELETED;
    if (err[F_STATE] & STAT_DELETED) {
      err[F_NOTES]+=({ ({time(),getuid(TI),
	sprintf("Loeschmarkierung gesetzt. (%s)",
	  note ? note: "<kein Kommentar>")}) });
      return(1); // Loeschmarkierung gesetzt.
    }
    else {
      err[F_NOTES]+=({ ({time(),getuid(TI),
	sprintf("Loeschmarkierung entfernt. (%s)",
	  note ? note : "<kein Kommentar>")}) });
      return(2); // Loeschmarkierung entfernt.
    }
    return(-1);
}

// sperrt den Eintrag oder entsperrt ihn.
// Sperre heisst hier, dass der Fehler vom Expire nicht automatisch geloescht
// wird.
varargs mixed LockError(string hashkey, string note, int type, string uid) {
    return set_lock(hashkey, 1, note, type, uid);
}

varargs int UnlockError(string hashkey, string note, int type, string uid) {
    return (int)set_lock(hashkey, 0, note, type, uid);
}

// einen Fehler als gefixt markieren
varargs int ResolveError(string hashkey, string note, int type, string uid) {
    return set_resolution(hashkey, 1, note, type, uid);
}
// einen Fehler als nicht gefixt markieren
varargs int ReOpenError(string hashkey, string note, int type, string uid) {
    return set_resolution(hashkey, 0, note, type, uid);
}

varargs int AddNote(string hashkey, string note, int type, string uid) {
    mapping err;

    if (member(resolved,hashkey))
	err=resolved[hashkey];
    else
	err=getError(hashkey,type,uid);
    if (!mappingp(err))
      return(-1);

    // absichtlich kein Access-Check, weil ja auch Magier, die nicht
    // zustaendig sind und keine Schreibrechte haben, ne gute Idee oder nen
    // Hinweis zu nem Bug haben koennen.
    // Wenn jemand spammt... Naja, social problem. ;-)
    /*if (!access_check(err[F_UID], M_WRITE))
	//zugriff zum Schreiben nicht gestattet
	return(-2);
	*/

    if (!stringp(note) || !strlen(note))
      return(-3);

    if (!pointerp(err[F_NOTES]))
      err[F_NOTES]=({});
    err[F_NOTES]+=({ ({time(),getuid(TI),note}) });
    save_me();
    return(1);
}

//Einen bestimmten Fehler einer anderen UID zuweisen.
//Hashkey ist zwar eindeutig, aber Angabe der
//der UID ist deutlich schneller. Weglassen des Typs nochmal langsamer. ;-)
//Potentiell also sehr teuer, wenn man UID oder UID+Typ weglaesst.
varargs int ReassignError(string hashkey, string newuid, string note, 
                          int type, string uid) {
    mapping err, errs;

    err=getError(hashkey,type,uid); 
    if (!mappingp(err)) return(-1); 
    uid = err[F_UID];
    
    if (!access_check(uid, M_WRITE))
	//zugriff zum Schreiben nicht gestattet
	return(-2);

    if (!stringp(newuid) || !strlen(newuid))
	return(-3);

    errs = errors[err[F_TYPE]];
    if (!member(errs, newuid)) errs+=([newuid:([])]);
    if (!mappingp(errs[newuid])) errs[newuid]=([]);
    
    // momentan erstmal abbrechen, eigentlich darf das nicht vorkommen, weil
    // die Hashes eindeutig sein sollen.
    if (member(errs[newuid],hashkey)) return(-4);

    if (!stringp(note)) note=0;

    call_out(#'save_me,2);

    err[F_UID] = newuid;
    errs[newuid] += ([hashkey: err]);
    efun::m_delete(errs[uid], hashkey);

    if (!pointerp(err[F_NOTES]))
	err[F_NOTES]=({});
    err[F_NOTES]+=({ ({time(),getuid(TI),
	sprintf("Fehler von %s an %s uebertragen. (%s)",
	  uid, newuid,
	  note ? note: "<kein Kommentar>")}) });
    
    return 1;
}

/* ***************  Helpers  *********************** */
public string find_uid_for_hash(string hashkey, int type) {
    //zu einem Hashkey die UID finden, falls es den Hash gibt.
    mapping mapp;
    int i;

    mapp=errors[type];
    foreach(string luid: m_indices(mapp)) {
	if (member(mapp[luid],hashkey))
	    return(luid);
    }
    return(""); //not found
}

public mixed find_global_hash(string hashkey) {
    //sucht in allen Fehlermappings einen Eintrag mit dem Key hash
    //liefert Array mit Typ und UID zurueck, wenn gefunden, sonst leeres Array
    // ACHTUNG: potentiell sehr teuer!
    mapping mapp;
    int i;
    string uid;
    //ueber alle Typen iterieren
    foreach(int type: m_indices(errors)) {
	if (strlen(uid=find_uid_for_hash(hashkey,type)))
	    return(({type,uid}));
    }
    return(({})); //not found
}

/* *********** Eher fuer Debug-Zwecke *********************** */
mixed QueryAll(int type) {
    //das koennte ein sehr sehr grosses Mapping sein, ausserdem wird keine
    //Kopie zurueckgegeben, daher erstmal nur ich...
    if (!this_interactive() || 
	member(MAINTAINER,getuid(this_interactive()))<0)
	return(-1);
    if (process_call()) return(-2);
    if (!type) return(-3);
    return(errors[type]);
}

mixed QueryResolved() {
    //das koennte ein sehr sehr grosses Mapping sein, ausserdem wird keine
    //Kopie zurueckgegeben, daher erstmal nur ich...
    if (!this_interactive() || 
	member(MAINTAINER,getuid(this_interactive()))<0)
	return(-1);
    if (process_call()) return(-2);
    return(resolved);
}

/*
mixed ResetAll(int type) {
    mapping mapp;

    if (!this_interactive() || 
	member(MAINTAINER,getuid(this_interactive()))<0)
	return(-1);
    if (process_call()) return(-2);
    if (!type) return(-3);

    mapp=errors[type];
    mapp=([]);

    return(mapp);
}
*/

/* *****************  Internal Stuff   ******************** */
void create() {
    seteuid(getuid(ME));
    if (!restore_object(SAVEFILE)) {
	errors=([T_RTERROR: ([]),
	         T_RTWARN: ([]),
		 T_CTERROR: ([]),
		 T_CTWARN: ([]) ]);
	resolved=([]);
    } 
    if (!mappingp(errors))       
	errors=([T_RTERROR: ([]),
	         T_RTWARN: ([]),
		 T_CTERROR: ([]),
		 T_CTWARN: ([]) ]);
    if (!mappingp(resolved))
	resolved=([]);

    lasterror=([]);
}

string name() {return("<Error-Daemon>");}

void save_me() {
    save_object(SAVEFILE);
}

varargs int remove(int silent) {
    save_me();
    destruct(ME);
    return(1);
}

// Krams zur Expiren //

//delete empty UIDs
private varargs void _expire_empty_uids(int *types, string *uids) {
    int i,size;
    mapping mapp;

    if (!pointerp(types))
	types=m_indices(errors);

    if (!sizeof(types)) return;
    mapp=errors[types[0]];

    if (!pointerp(uids))
	uids=m_indices(mapp);

    size=sizeof(uids);
    while(i<size && get_eval_cost()>750000) {
	if (!sizeof(mapp[uids[i]])) {
	    efun::m_delete(mapp,uids[i]);
	    //DEBUG(sprintf("_expire_empty_uids: Leere UID %s geloescht.\n",
	    //	  uids[i]));
	}
	i++;
    }

    if (i<size) {
	uids=uids[i..];	//rest mitnehmen in den naechsten Durchlauf
	//lfun closure wegen private
	call_out(#'_expire_empty_uids,10,types,uids);
	//DEBUG(sprintf("Callout auf _expire_empty_uids() mit Types %O "
	//    "und UIDs: %O gestartet.\n",types,uids));
    }
    else if (sizeof(types)>1) {//wenn noch mehr als einer hier drinsteht,
	types=types[1..];      //in den naechsten Durchlauf mitnehmen.
	//lfun closure wegen private
	call_out(#'_expire_empty_uids,10,types);
	//DEBUG(sprintf("Callout auf _expire_empty_uids() mit Types: %O "
	//      "gestartet.\n",types));
    }
    return;
}

//Try to expire some errors from the mappings
private varargs void _expire(int zeit, int *types, string *uids, 
    string *hashes) {
    //in den Arrays steht das aktuell bearbeitete immer ganze vorne drin, das
    //wird auch erst geloescht, wenn ich damit fertig bin.
    int i,size;
    mapping mapp;

    //DEBUG("_expire gerufen.\n");

    //ohne vernuenftige Zeitangabe wird nix gemacht.
    if (!intp(zeit) || zeit<=0)
	return;

    //wenn keine types: alle types holen
    if (!pointerp(types))
	types=m_indices(errors);
    //DEBUG(sprintf("Types: %O\n",types));

    //mapping fuer den aktuellen Typ holen
    if (!sizeof(types)) return;
    mapp=errors[types[0]];

    //alle UIDs des aktuellen Typs holen
    if (!pointerp(uids))
	uids=m_indices(mapp);
    //DEBUG(sprintf("UIDs: %O\n",uids));
    //wenn keine UIDs in dem Mapping sind: mit naechstem Typ weitermachen
    if (!sizeof(uids)) {
	if (sizeof(types)>1)  //ok, noch was uebrig zu bearbeiten
	  call_out(#'_expire,2,zeit,types[1..]);

	return;
    }

    //alle Hashes der aktueller UID holen
    if (!pointerp(hashes))
	hashes=m_indices(mapp[uids[0]]);
    //Pruefung, ob es Eintraege unter dieser UID gibt, sind nicht noetig, ist
    //in der while-Schleife drin.

    size=sizeof(hashes);
    while(i<size && get_eval_cost()>750000) {
	// Fehlereintrag loeschen, wenn nicht gelockt und letzte Aenderung
	// zu lang her war oder wenn als geloescht markiert.
	if ( (mapp[uids[0]][hashes[i]][F_MODSTAMP]<zeit &&
	      !(mapp[uids[0]][hashes[i]][F_STATE] & STAT_LOCKED) ) ||
	    mapp[uids[0]][hashes[i]][F_STATE] & STAT_DELETED) {
	    efun::m_delete(mapp[uids[0]],hashes[i]);
	    //DEBUG(sprintf("_expire: Eintrag %s aus UID %s geloescht.\n",
	//	  hashes[i], uids[0]));
	}
	i++;
    }

    if (i<size) {
	hashes=hashes[i..]; //rest mitnehmen in den naechsten Durchlauf
	call_out(#'_expire,2,zeit,types,uids,hashes);
	//DEBUG(sprintf("Callout auf _expire() mit Types %O "
	//    "und UIDs: %O und Hashes: %O gestartet.\n",types,uids,hashes));
    }
    else if (sizeof(uids)>1) {
	uids=uids[1..];	//rest mitnehmen in den naechsten Durchlauf
	//lfun closure wegen private
	call_out(#'_expire,2,zeit,types,uids);
	//DEBUG(sprintf("Callout auf _expire() mit Types %O "
	//    "und UIDs: %O gestartet.\n",types,uids));
    }
    else if (sizeof(types)>1) {//wenn noch mehr als einer hier drinsteht,
	types=types[1..];      //in den naechsten Durchlauf mitnehmen.
	//lfun closure wegen private
	call_out(#'_expire,2,zeit, types);
	//DEBUG(sprintf("Callout auf _expire mit Types: %O "
	//      "gestartet.\n",types));
    }
    else {
	//scheinbar ist nix mehr weiter zu tun, also mal leere UIDs loswerden.
	call_out(#'_expire_empty_uids,10);
	//DEBUG(sprintf("Callout auf _expire_empty_uids gestartet.\n"));
    }
    return;
}

// sperrt den Eintrag (lock!=0) oder entsperrt ihn (lock==0). 
// Sperre heisst hier, dass der Fehler vom Expire nicht automatisch geloescht
// wird.
// liefert <0 im Fehlerfall, sonst Array mit Lockdaten
private varargs mixed set_lock(string hashkey, int lock, string note, 
                      int type, string uid) {
    mapping err;

    if (member(resolved,hashkey))
	return(-4); // gefixte Fehler koennen nicht gelockt/entsperrt werden.
    err=getError(hashkey,type,uid);
    if (!mappingp(err)) return(-1);

    if (!access_check(err[F_UID], M_WRITE))
	//zugriff zum Schreiben nicht gestattet
	return(-2);

    // falls hier jemand was anderes als String oder 0 uebergeben hat.
    if (!stringp(note)) note=0;

    if (!pointerp(err[F_NOTES]))
	err[F_NOTES]=({});

    call_out(#'save_me,2);

    if (lock) {
      // wenn schon gesperrt: Lockinfos zurueckgeben und Ende
      if ((err[F_STATE] & STAT_LOCKED) &&
	  pointerp(err[F_LOCK]))
	  return (copy(err[F_LOCK]));
      // sonst sperren
      err[F_STATE] |= STAT_LOCKED;
      // wurde gesperrt, Infos eintragen
      err[F_LOCK]=({time(),getuid(TI)});
      err[F_NOTES]+=({ ({time(),getuid(TI),
	  sprintf("Lock gesetzt: %s",
	    note ? note : "<kein Kommentar>") }) });
      return(copy(err[F_LOCK]));
    }
    else {
      // entsperren
      if ((err[F_STATE] & STAT_LOCKED) &&
	  pointerp(err[F_LOCK])) {
	  err[F_STATE] &= ~STAT_LOCKED;
	  efun::m_delete(err,F_LOCK);
	  err[F_NOTES]+=({ ({time(),getuid(TI),
	      sprintf("Lock geloescht: %s",
		note ? note : "<kein Kommentar>") }) });
      }
      return(1);
    }
    return(-3);
}
private varargs mapping getError(string hashkey, int type, string uid) {
    mapping err;
    mixed arr;

    if (!stringp(hashkey) || !strlen(hashkey)) return(0);
    if (!stringp(uid) && !type) {
	arr=find_global_hash(hashkey);
	if (sizeof(arr)==2) {
	    type=arr[0];
	    uid=arr[1];
	}
	else return 0;
    }
    else if (!stringp(uid)) {
	if (!strlen(uid=find_uid_for_hash(hashkey,type)))
	  return 0;
    }
    if (!member(errors[type],uid) ||
	!member(errors[type][uid],hashkey)) 
	return 0;

    if (mappingp(err=errors[type][uid][hashkey])) {
      return(err);
    }

    return 0;
}

//markiert einen Fehler als gefixt, mit 'note' als Bemerkung (res!=0) oder
//markiert einen Fehler wieder als nicht-gefixt (res==0)
//liefert < 0 im Fehlerfall, 1, wenn ok.
private varargs int set_resolution(string hashkey, int res, string note,
                          int type, string uid) {
    mapping err;

    if (member(resolved,hashkey))
      err=resolved[hashkey];
    else {
      //Fehler holen, dabei sollen Infos wie UID, Hashkey mit _in_ den
      //Fehlereintrage geschrieben werden.
      err=getError(hashkey,type,uid,1);
      if (!mappingp(err)) return(-1);
    }

    if (!access_check(err[F_UID], M_WRITE))
	//zugriff zum Schreiben nicht gestattet
	return(-2);

    // falls hier jemand was anderes als String oder 0 uebergeben hat.
    if (!stringp(note)) note=0;

    if (!pointerp(err[F_NOTES]))
	err[F_NOTES]=({});
    // gna, alte Fehler haben den hashkey nicht drinnen.
    if (!member(err,F_HASHKEY))
	err[F_HASHKEY]= hashkey;

    call_out(#'save_me,2);

    if (res && !(err[F_STATE] & STAT_RESOLVED)) {
      // soll als erledigt markiert und ist nicht erledigt.
      err[F_STATE] |= STAT_RESOLVED;
      err[F_NOTES]+= ({ ({time(),getuid(TI),
	  sprintf("Fehler gefixt: %s",
	    note ? note : "<kein Kommentar>") }) });
      // UID und Typ stehen im Fehlereintrag drin.
      resolved[hashkey]=err; // in resolved wegschreiben zur Archivierung
      versende_mail(err);  // per Mail verschicken
      // und aus dem normaler Fehlermapping loeschen
      efun::m_delete(errors[err[F_TYPE]][err[F_UID]],hashkey);
      return(1);
    }
    else if (!res && err[F_STATE] & STAT_RESOLVED) {
      // soll als nicht-erledigt markiert werden und ist erledigt.
      err[F_STATE] &= ~STAT_RESOLVED;
      err[F_NOTES]+= ({ ({time(),getuid(TI),
	  sprintf("Fix zurueckgezogen: %s",
	    note ? note : "<kein Kommentar>") }) });
      if (!member(errors,err[F_TYPE]))
	errors[err[F_TYPE]]=([]);
      if (!member(errors[err[F_TYPE]],err[F_UID]))
	errors[err[F_TYPE]][err[F_UID]]=([]);
      //Fehler wieder in normales Fehlermapping schreiben
      errors[err[F_TYPE]][err[F_UID]][hashkey]=err;
      efun::m_delete(resolved,hashkey);
      return(1);
    }
    // offenbar schon so markiert, wie man gerade wuenscht.
    return(-3);
}

//Wieviele unterschiedliche Fehler hat diese UID in diesem typ?
private int queryUniqueErrorNoForUID(int type,string uid) {
    mapping mapp;
    if (!type || !stringp(uid) || !strlen(uid))
	return(0);
    mapp=errors[type];
    if (!member(mapp,uid)) return(0);
    return(sizeof(mapp[uid]));
}

//wieviele Fehler insg. hat diese UID in diesem Typ?
private int queryErrorNoForUID(int type,string uid) {
    //For now
    return(queryUniqueErrorNoForUID(type,uid));
}

//ist der Zugriff auf uid erlaubt? Geprueft wird TI (wenn kein TI, auch kein
//Schreibzugriff)
//mode gibt an, ob lesend oder schreibend
private int access_check(string uid, int mode) {

    if (mode==M_READ)
	return(1);  //lesen darf jeder

    // In process_string() schonmal gar nicht.
    if (process_call()) return(0);
    // EM+ duerfen alles loeschen.
    if (ARCH_SECURITY) return(1);
    // eigene UIDs darf man auch bearbeiten.
    if (secure_euid()==uid) return(1);

    // Master nach UIDs fragen, fuer die der jew. Magier
    // zustaendig ist.
    if (member((string *)master()->QueryUIDsForWizard(secure_euid()),uid))
	    return(1);

    return(0);	//Fall-through, nein
}

private mapping filter_private(string hashkey, mapping error) {
    mapping errcopy;
    //liefert eine (gefilterte) Kopie des Fehlereintrags, momentan wird F_CLI,
    //also die Spielereingabe vor dem Fehler ausgefiltert, wenn TI kein EM
    //oder man in process_string() ist.

    if (!stringp(hashkey) || !mappingp(error))
	return(([]));

    errcopy=deep_copy(error);
    //Wenn EM und nicht in process_string() oder die Spielereingabe gar nicht
    //im Fehlereintrag drinsteht: ungefiltert zurueck
    if (errcopy[F_CLI]==0 ||
	(!process_call() && ARCH_SECURITY) )
	return(errcopy);

    //sonst F_CLI rausfiltern, also Kopie und in der Kopie aendern.
    errcopy[F_CLI]="Bitte EM fragen";
    return(errcopy);
}

public string format_stacktrace(mixed bt) {
    string *lines;

    if (!pointerp(bt) || !sizeof(bt))
	return("");
    lines=({});
    foreach(mixed frame: bt) {
      if (stringp(frame))
	lines+=({sprintf("Thread-Start: %s",frame)});
      else if (pointerp(frame)) {
	lines+=({sprintf("Fun: %.20O in Prog: %.40s\n"
	                 "   Zeile: %.8d [%.50s]",
	                 frame[TRACE_NAME],frame[TRACE_PROGRAM],
			 frame[TRACE_LOC],frame[TRACE_OBJECT])
	        });
      }
    }

    return(implode(lines,"\n"));
}

public string format_notes(mixed notes) {
  int i;
  string txt="";
  foreach(mixed note: notes) {
    txt+=sprintf("Notiz %d von %.10s am %.30s\n%s",
	++i,capitalize(note[1]),dtime(note[0]),
	break_string(note[2],78,"  "));
  }
  return txt;
}

public string format_error(string hashvalue, mapping fehler) {
  string txt;
  string *label;
  
  if (!mappingp(fehler) || !sizeof(fehler) || !strlen(hashvalue)) { 
    return 0;
  }
  switch(fehler[F_TYPE]) {
    case T_RTERROR:
      label=({"Fehler","Dieser Fehler"});
      break;
    case T_RTWARN:
      label=({"Warnung","Diese Warnung"});
      break;
    default: return 0;
  }

  txt=sprintf( "Daten fuer %s mit ID '%s':\n"
               "Zeit: %25s (Erstmalig: %25s)\n"
	       "Programm:   %.60s\n"
	       "Zeile:      %.60d\n"
	       "Objekt:     %.60s\n"
	       "Loadname:   %.60s\n"
	       "UID:        %.60s\n",
	       label[0], hashvalue,
	       dtime(abs(fehler[F_MODSTAMP])),dtime(fehler[F_CREATESTAMP]),
	       fehler[F_PROG], fehler[F_LINE], fehler[F_OBJ],
	       fehler[F_LOADNAME], fehler[F_UID]);
	      
  txt+=sprintf("%s",break_string(fehler[F_MSG],78,
	          "Meldung:    ",BS_INDENT_ONCE));
  if (stringp(fehler[F_HB_OBJ]))
      txt+=sprintf(
	       "HB-Obj:     %.60s\n",fehler[F_HB_OBJ]);

  if (stringp(fehler[F_TITP])) {
      txt+=sprintf(
	       "TI/TP:      %.60s\n",fehler[F_TITP]);
      if (stringp(fehler[F_TIENV]))
	  txt+=sprintf(
	       "Environm.:  %.60s\n",fehler[F_TIENV]);
  }
   
  if (!stringp(fehler[F_CLI]) || 
      !ARCH_SECURITY || process_call())  {
      // Kommandoeingabe ist Privatsphaere und darf nicht von jedem einsehbar
      // sein.
      // in diesem Fall aber zumindest das Verb ausgeben, so vorhanden
      if (fehler[F_VERB])
	  txt+=sprintf(
	      "Verb:        %.60s\n",fehler[F_VERB]);
  }
  // !process_call() && ARCH_SECURITY erfuellt...
  else if (stringp(fehler[F_CLI]))
      txt+=sprintf(     
	  "Befehl:     %.60s\n",fehler[F_CLI]);

  if (fehler[F_CAUGHT])
      txt+=label[1]+" trat in einem 'catch()' auf.\n";
 /* // Diese Infos interessieren im Changelog nicht.
  if (fehler[F_STATE] & STAT_DELETED)
      txt+=label[1]+" wurde als geloescht markiert.\n";
  
  if (fehler[F_STATE] & STAT_LOCKED)
      txt+=break_string(
	  sprintf("%s wurde von %s am %s vor automatischem Loeschen "
	  "geschuetzt (locked).\n",
	  label[1],fehler[F_LOCK][1],dtime(fehler[F_LOCK][0])),78);
  if (fehler[F_STATE] & STAT_RESOLVED)
      txt+=label[1]+" wurde als erledigt markiert.\n";
 */
  txt+=sprintf("%s trat bisher %d Mal auf.\n",
               label[1],fehler[F_COUNT]);
  
  if (pointerp(fehler[F_STACK]))
      txt+="Backtrace:\n"+format_stacktrace(fehler[F_STACK])+"\n";

  if (pointerp(fehler[F_NOTES]) && sizeof(fehler[F_NOTES]))
      txt+="Bemerkungen:\n"+format_notes(fehler[F_NOTES])+"\n";

  return txt;
}

private int  versende_mail(mapping fehler) {
  // Versendet eine mail mit dem gefixten Fehler.
  mixed *mail;
  string text, *empf;

  empf = (string*)master()->QueryWizardsForUID(fehler[F_UID]);
  if (!sizeof(empf)) return -1; // leider keine Empfaenger ermittelbar

  mail = allocate(9);

  // hier mal kein efun::m_delete(), ich brauch ne Kopie ohne das F_CLI, auch
  // wenn ein EM fixt, sollen die Empfaenger nicht automatisch die
  // Spielereingabe erhalten.
  fehler = m_delete(fehler, F_CLI);

  text = break_string(
      sprintf(STANDARDMAILTEXT,capitalize(getuid(TI)))
      +format_error(fehler[F_HASHKEY],fehler),78,"",BS_LEAVE_MY_LFS);

  mail[MSG_FROM] = "<Fehler-Daemon>";
  mail[MSG_SENDER] = getuid(TI);
  mail[MSG_RECIPIENT] = empf[0];
  if (sizeof(empf)>1)
    mail[MSG_CC] = empf[1..];
  else
    mail[MSG_CC] = 0;
  mail[MSG_BCC] = 0;
  mail[MSG_SUBJECT] = sprintf("Fehler in %s behoben",fehler[F_LOADNAME]);
  mail[MSG_DATE] = dtime(time());
  mail[MSG_ID]=sprintf("MorgenGrauen: %d.%d",time(),random(__INT_MAX__));
  mail[MSG_BODY]=text;

  if (!sizeof("/secure/mailer"->DeliverMail(mail,0)))
    return -2; // an niemanden erfolgreich zugestellt. :-(
    
  return 1;
}


private void archive() {
  string txt="";
  if (!sizeof(resolved)) {
    call_out(#'_expire,2,time()-STDEXPIRE);
    return;
  }
  foreach(string hashkey, mapping fehler: resolved) {
      if (get_eval_cost() < __MAX_EVAL_COST__/2)
	  break; // abbrechen. ;-)
      txt+=format_error(hashkey,fehler)+"\n";
      efun::m_delete(resolved,hashkey);
  }
  write_file(CHANGELOG,txt);
  call_out(#'archive,4);
}

void reset() {
    archive(); // und _expire() wird von archive() gerufen 
    //_expire(time()-(STDEXPIRE)); //21 Tage
    set_next_reset(3600*24);
}

// Nicht jeder Magier muss den ErrorD direkt zerstoeren koennen.
string NotifyDestruct(object caller) {
    if( (caller!=this_object() && !ARCH_SECURITY) || process_call() ) {
      return "Du darfst den Error-Daemon nicht zerstoeren!\n";
    }
}

