/* /obj/tools/fehlerteufel.c
   Fehlerteufel - Magiertool zur Abfrage des Fehler-Daemons der Mudlib
   Autor: Zesstra
   Changelog:
*/

#pragma strict_types
#pragma pedantic
#pragma range_check
#pragma no_shadow
#pragma no_inherit

inherit "/std/secure_thing";

#include <defines.h>
#include <wizlevels.h>
#include <properties.h>
#include <moving.h>
#include <errord.h>
#include <config.h>
#include <debug_info.h>

#define TI this_interactive()

// variables
private string owner;  // UID vom Eigentuemer
private string *uids=({}); // UIDs, auf die man schreibrechte hat
private string *filteruids=({}); // wenn nicht-leer, nur diese UIDs zeigen
private string *monitoruids=({}); // diese UIDs in die Liste einschliessen
private int filterstatus=0;       // Filter an oder aus?
private mapping fehler=([]);      // Cache fuer Fehlerdaten
private int modus=T_RTERROR;      //Welche Art von Fehler anzeigen?
private string *xmonitoruids = ({}); // expanded Monitor-UIDs
private string lfehler="";        // letzter 'benutzter' Fehler.
private int altefehlerzahl;       // fehlerzahl im letzten Reset.

// ************** public interface ****************
public string format_stacktrace(mixed bt);
public string format_error(string hashvalue, mapping fehler);
public string format_notes(mixed notes);

// ************** private functions  **************
private varargs void get_errors(int lmodus);
private void get_uids();

public int CmdFehlerZeigen(string arg) {
  mapping fehler;
  string txt;

  if (!stringp(arg) || !strlen(arg))
      arg = lfehler;
  
  notify_fail("Einen Eintrag mit dieser ID gibt es nicht!\n");

  if (!mappingp(fehler=(mapping)ERRORD->QueryError(arg)) ||
       !sizeof(fehler)) {
      return 0;
  }
  // letzten Fehler merken.
  lfehler = arg;
  
  switch (fehler[F_TYPE]) {
    case T_RTERROR:
    case T_RTWARN:
      txt=format_error(arg,fehler);
      break;
  }
  
  if (!stringp(txt) || !strlen(txt))
      return 0;

  tell_object(PL,txt+"\n");
  /*tell_room(environment(PL),break_string(PL->Name(WESSEN)
      +" kleiner Fehlerteufel fluestert "
      +PL->QueryPronoun(WEM)
      +" eindringlich etwas ins Ohr. ",78),({PL}));
  */
  return 1;
}

// lokale gespeicherter Fehler?
private mapping QueryLocal(string hashkey) {
  foreach(string uid, mapping errs: fehler) {
    if (member(errs, hashkey)) {
      return errs[hashkey];
    }
  }
  return 0;
}

public int CmdFehlerLoeschen(string arg) {
  int res;

  if (!stringp(arg) || !strlen(arg))
      arg = lfehler;

  notify_fail("Einen Eintrag mit dieser ID gibt es nicht!\n");
  
  mapping err = QueryLocal(arg);
  int eigener = mappingp(err);
  if (!eigener)
    err = (mapping)ERRORD->QueryError(arg, 0, 0);
  if (!sizeof(err))
    return 0;

  res=(int)ERRORD->ToggleDeleteError(arg, 0, err[F_TYPE], err[F_UID]);
  if (res==1) {
      if (eigener) {
	// Fehler aus lokaler Liste loeschen
	altefehlerzahl--;
	efun::m_delete(fehler[err[F_UID]], arg);
      }
      tell_object(PL,
	"Fehler/Warnung wurde zum Loeschen markiert und wird in Kuerze "
	"geloescht.\n");
  }
  else if (res==2) {
      if (member(uids + xmonitoruids, err[F_UID]) != -1) {
	// Fehler wird in die lokale Liste einfuegen
	altefehlerzahl++;
	fehler[err[F_UID]][arg] = (mapping)ERRORD->QueryError(arg,
	                                    err[F_TYPE], err[F_UID]);
      }
      tell_object(PL,"Loeschmarkierung wurde entfernt.\n");
  }
  else if (res==-1)
    return 0;
  else if (res==-2)
    tell_object(PL,"Du hast leider keinen Schreibzugriff auf diesen "
	"Eintrag.\n");

  // letzten Fehler merken.
  lfehler = arg;
 
  /*
  tell_room(environment(PL),break_string(PL->Name(WER)
      +" fluestert "+PL->QueryPossPronoun(MALE,WEM,SINGULAR)
      +" Fehlerteufel leise etwas ins Ohr.",78),
      ({PL}));*/
  //get_errors(); // Fehler neu einlesen. TODO: nur den geaenderten Fehler.
  return 1;
}

public int CmdRefresh(string arg) {
    reset();
    tell_object(PL,"Fehlerdaten wurden neu eingelesen.\n");
    return 1;
}

public int CmdFehlerListe(string arg) {
  string txt;
  string *luids;
  int lmodus; // modus fuer diese Liste
  mapping fehlerbackup;

  if (stringp(arg) && strlen(arg)) {
    switch(arg) {
      case "fehler":
      case "error":
      case "errors":
	lmodus=T_RTERROR;
	break;
      case "warnungen":
      case "warnung":
      case "warning":
      case "warnings":
	lmodus=T_RTWARN;
        break;
      default:
	lmodus=modus;
    }
    if (lmodus != modus) {
      fehlerbackup=fehler; // Fehlerliste von 'modus' aufheben
      get_errors(lmodus);  // neue Fehlerliste holen
    }
  }
  else lmodus=modus;

  if (!sizeof(fehler)) {
    txt="Fuer Deine UIDs sind keine Fehler/Warnungen bekannt.\n";
    tell_object(PL,txt);
    return 1;
  }
  else
    txt="";
  
  // UIDs ermitteln, die a) im Cache sind und b) entweder die eigenen sind
  // oder auf der Monitorliste stehen, also Schnittmenge bilden
  // Schnittmenge: (m_indices(fehler) & uids) 
  //             + (m_indices(fehler) & xmonitoruids)
  luids = m_indices(fehler);
  luids = (luids & uids ) + ( luids & xmonitoruids);
  // ggf. noch die ausgefiltern raussortieren
  if (filterstatus) {
    luids=m_indices(fehler)-filteruids;
  }

  foreach(string uid: luids) {
    string txt2="";
    foreach(string hashvalue, mapping err: fehler[uid]) {
      // geloeschte oder gefixte Fehler nicht anzeigen.
      if (err[F_STATE] & (STAT_DELETED | STAT_RESOLVED) ) 
	  continue;
      if (err[F_TYPE]==T_RTERROR || err[F_TYPE]==T_RTWARN)
	txt2+=sprintf("%:32s: %.38-s, (%.4-d)\n",
	    hashvalue,err[F_LOADNAME],err[F_COUNT]);
      else
	txt2+=sprintf("%:32s: %.45-s\n",hashvalue,err[F_LOADNAME]);
    }
    if (txt2!="")
      txt+=sprintf("\n%s:\n%s",uid,txt2);
  }
  if (txt=="") {
      tell_object(PL,"Fuer Deine UIDs sind keine Fehler/Warnungen "
	  "bekannt. :-)\n");
      return 1;
  }

  txt="Folgende Fehler/Warnungen sind fuer Deine UIDs bekannt "
    +(filterstatus ? "(Filter an):\n" : "(Filter aus):\n") + txt;

  tell_object(PL,txt);
  /*
  tell_room(environment(PL),break_string(PL->Name(WESSEN)
      +" kleiner Fehlerteufel erzaehlt "
      +PL->QueryPronoun(WEM)
      +" ganz leise etwas.",78),
      ({PL}));
*/
  if (mappingp(fehlerbackup) && modus!=lmodus)
    fehler=fehlerbackup; // fehlerliste fuer 'modus' restaurieren
  return 1;
}

public int CmdFilter(string arg) {

  arg=(string)PL->_unparsed_args(0);  
 
  if (!stringp(arg) || !strlen(arg)) {
    tell_object(PL,break_string(
	  "Momentan interessieren Dich folgende UIDs nicht"
	  +(filterstatus ? " (Filter an):\n" : " (Filter aus):\n")
	  +CountUp(filteruids)+"\n", 78,"",BS_LEAVE_MY_LFS));
    return 1;
  }

  if (arg=="keine") {
    filteruids=({});
    filterstatus=1;
    tell_object(PL,break_string(
	"Dein Fehlerteufel wird Dir nun nur noch ausgewaehlte "
	"Fehler berichten. Momentan hast Du keine UIDs ausgeblendet. "
	"(Filter an)",78));
  }
  else if (arg=="alle") {
    filterstatus=1;
    filteruids=uids;
    tell_object(PL,break_string(
	"Dein Fehlerteufel wird Dir nun nur noch ausgewaehlte "
	"Fehler berichten. Du blendest momentan alle UIDs aus. "
	"(Filter an)",78));
  }
  else if (arg=="aus") {
    filterstatus=0;
    tell_object(PL,break_string(
	"Dein Fehlerteufel wird Dir nun wieder alle Fehler berichten. ",
	78));
  }
  else if (arg=="an" || arg=="ein") {
    filterstatus=1;
    tell_object(PL,break_string(
	"Dein Fehlerteufel wird Dir nun nur noch ausgewaehlte "
	"Fehler berichten.",78));
  }
  else {
    foreach(string uid: explode(arg," ")-({""})) {
      if (strlen(uid)>1 && uid[0]=='+') {
	if (member(filteruids,uid[1..])==-1)
	  filteruids+=({uid[1..]});
      }
      else if (strlen(uid)>1 && uid[0]=='-') {
	  filteruids-=({uid[1..]});
      }
      else {
	if (member(filteruids,uid)==-1)
	  filteruids+=({uid});
	else
	  filteruids-=({uid});
      }
    }
  }

  tell_object(PL,break_string(
	  "Momentan interessieren Dich folgende UIDs nicht"
	  +(filterstatus ? " (Filter an):\n" : " (Filter aus):\n")
	  +CountUp(filteruids)+"\n", 78,"",BS_LEAVE_MY_LFS));

  return 1;
}

public int CmdMonitor(string arg) {

  arg=(string)PL->_unparsed_args(0);  
 
  if (!stringp(arg) || !strlen(arg)) {
    tell_object(PL,break_string(
	  "Momentan interessieren Dich folgende UIDs zusaetzlich zu Deinen: \n"
	  +(sizeof(monitoruids) ? CountUp(monitoruids) : "")
	  +"\n", 78,"",BS_LEAVE_MY_LFS));
    return 1;
  }

  if (arg=="keine") {
    monitoruids=({});
    xmonitoruids=({});
    tell_object(PL,break_string(
	"Dein Fehlerteufel wird Dir nun nur noch "
	"Fehler Deiner eigenen UIDs berichten.",78));
    return 1;
  }
  else {
    foreach(string uid: explode(arg," ")-({""})) {
      if (strlen(uid)>1 && uid[0]=='+') {
	if (member(monitoruids,uid[1..])==-1)
	  monitoruids+=({uid[1..]});
      }
      else if (strlen(uid)>1 && uid[0]=='-') {
	  monitoruids-=({uid[1..]});
      }
      else {
	if (member(monitoruids,uid)==-1)
	  monitoruids+=({uid});
	else
	  monitoruids-=({uid});
      }
    }
  }
  get_uids();
  tell_object(PL,break_string(
	"Momentan interessieren Dich folgende UIDs zusaetzlich zu Deinen: \n"
	+(sizeof(monitoruids) ? CountUp(monitoruids) : "")
	+"\n", 78,"",BS_LEAVE_MY_LFS));

  return 1;
}

public int CmdModus(string arg) {
  string txt;
  
  // Argument verwursten
  if (stringp(arg) && strlen(arg)) {
    switch(arg) {
      case "fehler":
      case "error":
      case "errors":
	modus=T_RTERROR;
	break;
      case "warnungen":
      case "warnung":
      case "warning":
      case "warnings":
	modus=T_RTWARN;
        break;
    }
    reset();  // neue Fehlerliste holen
  }
  // aktuelle Einstellung ausgeben.
  switch(modus) {
    case T_RTERROR:
      txt="Fehler (Laufzeit)";
      break;
    case T_RTWARN:
      txt="Warnungen (Laufzeit)";
      break;
  }
  tell_object(PL, break_string(
      "Deiner Fehlerteufel wird Dir nun ueber aufgetretene "
      +txt+" Bericht erstatten.",78));
  return(1);
}

int CmdAddNote(string str) {
  string *arr, hashkey;
  
  notify_fail("Bitte eine ID und einen Text angeben!\n");
  if(!objectp(TI)) return(0);
 
  str=(string)PL->_unparsed_args(0);
  if (!stringp(str) || !strlen(str))
    return(0);
  
  arr=explode(str," ")-({""});
  if (sizeof(arr)<2)
    return(0);
  hashkey=arr[0];

  str=implode(arr[1..]," ");  //text wiederherstellen, aber ohne Key

  switch((int)ERRORD->AddNote(hashkey,str)) {
    case -1:
      tell_object(PL,
	  sprintf("Es gibt keinen Fehler mit der ID: %s\n",hashkey));
      return(1);
    case -3:
      return(0); //offenbar keine Notiz angegeben.
  }
  // letzten Fehler merken.
  lfehler = hashkey;

  tell_object(PL,
      sprintf("Deine Notiz wurde zu %s hinzugefuegt.\n",
	hashkey));
  return(1);
}

int CmdFix(string str) {
  string *arr, hashkey;
  int fixing, res;

  notify_fail("Bitte eine ID und optional eine Notiz angeben!\n");
  if(!objectp(TI)) return(0);
 
  str=(string)PL->_unparsed_args(0);
  if (!stringp(str) || !strlen(str))
    return(0);
  
  arr=explode(str," ")-({""});
  if (!sizeof(arr)) return 0;
 
  hashkey=arr[0];
  if (sizeof(arr)>1)
    str=implode(arr[1..]," ");  //text wiederherstellen, aber ohne Key
  else str=0;

  notify_fail("Einen Fehler mit dieser ID gibt es leider nicht.\n");
  mapping err = QueryLocal(hashkey);
  int eigener = mappingp(err);
  if (!eigener)
    err = (mapping)ERRORD->QueryError(hashkey, 0, 0);
  if (!sizeof(err))
    return 0;

  if (query_verb()=="ffix" || query_verb()=="fehlerfix") {
      fixing=1;
      res=(int)ERRORD->ResolveError(hashkey,str, err[F_TYPE], err[F_UID]);
  }
  else {
      res=(int)ERRORD->ReOpenError(hashkey, str);
  }

  if (res==-1) {
      tell_object(PL,
	  sprintf("Es gibt keinen Fehler mit der ID: %s\n",hashkey));
  }
  else if (res==-2) {
      tell_object(PL,
	"Du hast leider keinen Schreibzugriff diesen Fehler.\n"
	"Aber vielleicht moechtest Du mit fnotiz eine Notiz anhaengen?\n");
  }
  else if (res==-3) {
    if (fixing)
      tell_object(PL,"Dieser Fehler ist bereits gefixt.\n");
    else
      tell_object(PL,"Dieser Fehler noch nicht gefixt.\n");
  }
  else if (res>0) {
    if (fixing) {
      // fehler in lokaler Liste ggf. loeschen.
      if (eigener) {
	efun::m_delete(fehler[err[F_UID]], hashkey);
	altefehlerzahl--;
      }
      tell_object(PL,
	  sprintf("Fehler %s als gefixt markiert.\n",hashkey));
    }
    else {
      // eigene UID oder beobachtete? -> fehler in lokaler Liste
      if (member(uids + xmonitoruids, err[F_UID]) != -1) {
	altefehlerzahl++;
	fehler[err[F_UID]][hashkey] = (mapping)ERRORD->QueryError(hashkey,
	                                       err[F_TYPE], err [F_UID]);
      }
      tell_object(PL,
	  sprintf("Fehler %s als nicht gefixt markiert.\n",hashkey));
    }
  }
  // letzten Fehler merken.
  lfehler = hashkey;
  //get_errors(); // TODO: nur den geaenderten Fehler. ;-)
  return(1);
}

int CmdLock(string str) {
  string *arr, hashkey;
  int locking;
  mixed res;

  notify_fail("Bitte eine ID und optional eine Notiz angeben!\n");
  if(!objectp(TI)) return(0);
 
  str=(string)PL->_unparsed_args(0);
  if (!stringp(str) || !strlen(str))
    return(0);
  
  arr=explode(str," ")-({""});
  if (!sizeof(arr)) return 0;
 
  hashkey=arr[0];
  if (sizeof(arr)>1)
    str=implode(arr[1..]," ");  //text wiederherstellen, aber ohne Key
  else str=0;
 
  if (query_verb()=="flock" || query_verb()=="fehlerlock") {
      locking=1;
      res=(mixed)ERRORD->LockError(hashkey,str);
  }
  else {
      res=(int)ERRORD->UnlockError(hashkey,str);
  }

  if (res==-1) {
      tell_object(PL,
	  sprintf("Es gibt keinen Fehler mit der ID: %s\n",hashkey));
  }
  else if (res==-2) {
       tell_object(PL,
	   "Du hast leider keinen Schreibzugriff diesen Fehler.\n");
  }
  else if (res==-3) {
      if (locking)
	  tell_object(PL,
	      "Dieser Fehler ist bereits vor autom. Loeschen geschuetzt.\n");
      else
	  tell_object(PL,
	      "Dieser Fehler ist bereits zum autom. Loeschen freigegeben.\n");
  }
  else if (res==-4) {
       tell_object(PL,
	   "Dieser Fehler ist bereits gefixt und wird bald geloescht.\n");
  }
  else if (pointerp(res) && sizeof(res)==2) {
    if (locking)
      tell_object(PL,
	  sprintf("Fehler %s vor autom. Loeschen geschuetzt.\n",hashkey));
  }
  else if (res>0) {
    if (!locking)
      tell_object(PL,
	  sprintf("Fehler %s zum autom. Loeschen freigegeben.\n",hashkey));
  }
  // letzten Fehler merken.
  lfehler = hashkey;

  return(1);
}

int CmdReassign(string str) {

  notify_fail("Bitte eine ID, die neue UID und ggf. eine Notiz angeben!\n");
  if(!objectp(TI)) return(0);
 
  str=(string)PL->_unparsed_args(0);
  if (!stringp(str) || !strlen(str))
    return(0);
  
  string *arr=explode(str," ")-({""});
  if (sizeof(arr)<2)
    return(0);
  string hashkey=arr[0];
  string newuid=arr[1];

  //text wiederherstellen, aber ohne Key und UID
  if (sizeof(arr) > 2)
    str = implode(arr[2..]," ");
  else
    str = 0;

  switch((int)ERRORD->ReassignError(hashkey, newuid, str)) {
    case -1:
      tell_object(PL,
	  sprintf("Es gibt keinen Fehler mit der ID: %s\n",hashkey));
      return(1);
    case -2:
      tell_object(PL,
	  sprintf("Du hast keine Schreibrechte auf Fehler %s\n",hashkey));
      return 1;
    case -3:
      return(0); //offenbar keine Notiz angegeben.
    case -4:
      tell_object(PL,break_string(
	  sprintf("In %s gibt es bereits einen Fehler der ID %s\n",
	    newuid,hashkey),78));
      return 1;
  }
  // letzten Fehler merken.
  lfehler = hashkey;
  // neue Fehlerliste holen (ja, schneller waere, den geaenderten Fehler
  // lokal zu aendern.
  get_errors();

  tell_object(PL,break_string(
      sprintf("Der Fehler der ID %s wurde an die UID %s "
	"uebertragen.\n", hashkey, newuid),78));
  return(1);
}

// ************** public 'internal' functions **************
public string QueryOwner() {return owner;}
public mapping QueryErrors() {return fehler;}

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
	                 "   Zeile: %.8d [%.50s]\n"
			 "   Evalcosts: %d",
	                 frame[TRACE_NAME],frame[TRACE_PROGRAM],
			 frame[TRACE_LOC],frame[TRACE_OBJECT],
			 (sizeof(frame) > TRACE_EVALCOST) ? 
			              frame[TRACE_EVALCOST] : 0)
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

  txt=sprintf( "\nDaten fuer %s mit ID '%s':\n"
               "Zeit: %25s (Erstmalig: %25s)\n"
	       "Programm:   %.60s\n"
	       "Zeile:      %.60d\n"
	       "Objekt:     %.60s\n"
	       "Loadname:   %.60s\n"
	       "UID:        %.60s\n",
	       label[0], hashvalue,
	       dtime(abs(fehler[F_MODSTAMP])),dtime(fehler[F_CREATESTAMP]),
	       fehler[F_PROG] || "<Unbekannt>", 
	       fehler[F_LINE], fehler[F_OBJ],
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
   
  if (!stringp(fehler[F_CLI]) || fehler[F_CLI]=="Bitte EM fragen") {
      // in diesem Fall zumindest das Verb ausgeben, so vorhanden
      if (fehler[F_VERB])
	  txt+=sprintf(
	      "Verb:        %.60s\n",fehler[F_VERB]);
  }
  else if (stringp(fehler[F_CLI]))
      txt+=sprintf(     
	  "Befehl:     %.60s\n",fehler[F_CLI]);
  if (fehler[F_CAUGHT])
      txt+=label[1]+" trat in einem 'catch()' auf.\n";
 
  if (fehler[F_STATE] & STAT_DELETED)
      txt+=label[1]+" wurde als geloescht markiert.\n";
  if (fehler[F_STATE] & STAT_LOCKED)
      txt+=break_string(
	  sprintf("%s wurde von %s am %s vor automatischem Loeschen "
	  "geschuetzt (locked).\n",
	  label[1],fehler[F_LOCK][1],dtime(fehler[F_LOCK][0])),78);
  if (fehler[F_STATE] & STAT_RESOLVED)
      txt+=label[1]+" wurde als erledigt markiert.\n";
 
  txt+=sprintf("%s trat bisher %d Mal auf.\n",
               label[1],fehler[F_COUNT]);
  
  if (pointerp(fehler[F_STACK]))
      txt+="Backtrace:\n"+format_stacktrace(fehler[F_STACK])+"\n";

  if (pointerp(fehler[F_NOTES]) && sizeof(fehler[F_NOTES]))
      txt+="Bemerkungen:\n"+format_notes(fehler[F_NOTES])+"\n";

  return txt;
}

void create() {
    if (!clonep(ME))
        return;
    ::create(); 

    SetProp(P_SHORT,"Der Fehlerteufel");
    SetProp(P_LONG,break_string(
	  "Dein Fehlerteufel soll Dir helfen, Informationen "
	  "ueber aufgetretene Fehler zu erhalten. Hierzu fragt er die "
	  "in \"Deinen\" UIDs aufgetretenen Fehler und deren Details vom "
	  "Fehlerspeicher der Mudlib ab. Folgende Kommandos kennt er:",78)
    +"fehlerabfrage <id>  - Fragt Details ueber Fehler mit der ID ab.\n"
    "fehlerloeschen <id> - Fehler zum Loeschen markieren.\n"
    "fehlerliste         - Fehlerliste der eigenen UIDs anzeigen\n"
    "fehlerrefresh       - Fehlerdaten und UIDs neu einlesen\n"
    "fehlerfilter        - UIDs fuer den Filter angeben (s. manpage!)\n"
    "fehlermodus         - Fehler oder Warnungen ausgeben? (s. manpage)\n"
    "fehlermonitor       - zus. UIDs beobachten (s. manpage)\n"
    "fnotiz <id> <note>  - eine Notiz anhaengen\n"
    "flock <id>  <note>  - Fehler vor autom. Loeschen schuetzen\n"
    "funlock <id> <note> - Fehler zum autom. Loeschen freigeben\n"
    "ffix <id> <note>    - Fehler als gefixt kennzeichnen\n"
    "funfix <id> <note>  - gefixten Fehler als nicht-gefixt markieren\n"
    "fuebertrage <id> <newuid> <note>\n"
    "                    - Fehler an die UID uebertragen\n"
    );
    SetProp(P_NAME,"Fehlerteufel");
    SetProp(P_GENDER,MALE);
    SetProp(P_WEIGHT,0);
    SetProp(P_VALUE,0);
    SetProp(P_SIZE,10);
    SetProp(P_NODROP,"Den Fehlerteufel behaelst Du lieber bei Dir.\n");
    SetProp(P_NEVERDROP,1);

    AddId( ({"fehlerteufel","teufel"}) );

    AddCmd(({"fehlerabfrage","fabfrage"}), "CmdFehlerZeigen" );
    AddCmd(({"fehlerloeschen","floeschen"}), "CmdFehlerLoeschen");
    AddCmd(({"fehlerliste","fliste", "fehleruebersicht","fuebersicht"}), 
	"CmdFehlerListe");
    AddCmd(({"fehlerrefresh","frefresh"}),"CmdRefresh");
    AddCmd(({"fehlerfilter","ffilter"}),"CmdFilter");
    AddCmd(({"fehlermodus","fmodus"}),"CmdModus");
    AddCmd(({"fehlermonitor","fmonitor"}),"CmdMonitor");
    AddCmd(({"fehlernotiz","fnotiz"}),"CmdAddNote");
    AddCmd(({"fehlerlock","flock","fehlerunlock","funlock"}),
	"CmdLock");
    AddCmd(({"fehlerfix","ffix","fehlerunfix","funfix"}),
	"CmdFix");
    AddCmd(({"fehleruebertrage","fuebertrage"}),"CmdReassign");
}

void init() {
    if (find_call_out("remove") != -1) return;
    // pruefung auf env nicht noetig, move geht nur in ein env und ohne env
    // auch kein init().
    if ( !query_once_interactive(environment()) || 
	!IS_LEARNER(environment())) {
	// in interactive, aber kein magier -> direkt weg.
	call_out("remove",0,1);
	return;
    }
    else if (!strlen(owner))
	// Env ist Interactiv und Magier (sonst waer man oben rausgeflogen)
	owner=getuid(environment());
    else if (owner!=getuid(environment())) {
       //ok, nicht der Eigentuemer, direkt weg.
       call_out("remove",0);
       return;
   }
   SetProp(P_EXTRA_LOOK,break_string(
	 "Auf "+environment()->Name(WESSEN)+" Schulter sitzt ein kleiner "
	 "Fehlerteufel, der "
	 +environment()->QueryPronoun(WEM)
	 +" immer wieder etwas ins Ohr fluestert.",78));
   call_out("reset",1);
   ::init();
}

mapping _query_autoloadobj() {
    // auch ein leeres Array ist zum Glueck != 0
    return (["filteruids":filteruids,
	     "filterstatus":filterstatus,
	     "modus":modus,
	     "monitoruids":monitoruids,
	     "fehlerzahl": altefehlerzahl]);
}

mapping _set_autoloadobj(mixed data) {

    if (mappingp(data)) {
      if (member(data,"filteruids") && pointerp(data["filteruids"]))
	filteruids=data["filteruids"];
      if (member(data,"filterstatus") && intp(data["filterstatus"]))
	filterstatus=data["filterstatus"];
      if (member(data,"modus") && intp(data["modus"]))
	modus=data["modus"];
      if (member(data,"monitoruids") && pointerp(data["monitoruids"]))
	monitoruids=data["monitoruids"];
      if (member(data,"fehlerzahl") && intp(data["fehlerzahl"]))
	altefehlerzahl=data["fehlerzahl"];
    }
    return(_query_autoloadobj());
}


void reset() {
    int neuefehlerzahl;

    get_uids();
    get_errors();
    
    // neue Fehler zaehlen, dabei aber nur die nicht-geloeschten und
    // nicht-geloesten beruecksichtigen.
    foreach(string uid, mapping errs: fehler) {
      neuefehlerzahl+=sizeof(filter(errs, function int (string key, mapping e)
	    { if (e[F_STATE] & (STAT_DELETED | STAT_RESOLVED))
	        return 0;
	      return 1;
	    }
	      ));
    }
    if (altefehlerzahl < neuefehlerzahl)
      tell_object(environment(ME), break_string(
	"Deine Fehlerliste ist soeben laenger geworden.",78,
	"Dein Fehlerteufel teilt Dir mit: "));
    else if (altefehlerzahl > neuefehlerzahl)
      tell_object(environment(ME), break_string(
	"Deine Fehlerliste ist soeben kuerzer geworden.",78,
	"Dein Fehlerteufel teilt Dir mit: "));
    altefehlerzahl = neuefehlerzahl;
}

// ******** private functions *********************
private void get_uids() {
    uids=(string *)master()->QueryUIDsForWizard(owner);
    xmonitoruids=({});
    if (sizeof(monitoruids)) {
      closure cl=symbol_function("QueryUIDAlias", master());
      foreach(string uid: monitoruids) {
	xmonitoruids += (string*)funcall(cl, uid);
      }
    }
}

private varargs void get_errors(int lmodus) {
    if (!lmodus)
      lmodus=modus;
    fehler=m_allocate(10,1);
    foreach(string uid : uids + xmonitoruids) {
      fehler+=([uid:(mapping)ERRORD->QueryErrors(lmodus,uid)]);
    }
}

