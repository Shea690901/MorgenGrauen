// MorgenGrauen MUDlib
//
// thing/commands.c -- thing description
//
// $Id: commands.c 9186 2015-03-29 08:44:53Z Zesstra $
//
// Aus Regenbogen MUDLib
// aus Gueldenland/Wunderland MUDlib (Morgengrauen MUDlib)
//
// P_COMMANDS data-structure:
//
// AddCmd(verb,fun1,1);
// AddCmd(verb+syn1a|syn1b&syn2a|syn2b|syn2c,fun2,
//	   error1_notify|error2_notify^error2_write);
// -->
// ([verb:({fun1,fun2});					// funs
//	  ({1,({error1_notify, error2_write^error2_say, 1})});  // flags
//        ({0,({({syn1a,syn1b}),({syn2a,syn2b,syn2c})})});	// rules
//        0])							// IDs
//
// Rules: ({<Regelsatz fuer fun1>, ({<1. Synonymgruppe>,
//				     <2. Synonymgruppe, ...}), ...})
// Flags: ({<Flag fuer fun1>, ({<Fehlermeldung 1. Synonymgruppe>, ... ,
//				[, Index fuer write anstatt notify_fail]}),
//	    ... })
// IDs:   0 oder ({<ID fuer fun1>}) oder ({0, <ID fuer fun2>}) ...
//
// IDEA: save no 0s in rules/flags if possible (as in IDs)
//       (adressing especially old-style-AddCmd-MUDs)
#pragma strict_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

#include <moving.h>
#include <thing/language.h>
#include <exploration.h>

#define NEED_PROTOTYPES
#include <thing/description.h>
#include <thing/commands.h>
#undef NEED_PROTOTYPES

#ifdef DBG
#undef DBG
#endif
#define DBG(x) printf("Object %O tmpstr=%s\n", explode(object_name(this_object()),"#")[1], x);

private nosave mapping added_cmds;

protected void create()
{
}

protected void create_super() {
  set_next_reset(-1);
}     

varargs void AddCmd(mixed cmd, mixed func, mixed flag, mixed cmdid) {
 int i,j;
 closure cl;
 mixed *rule;

 // potentielle AddCmd mit Regel?
 if(stringp(cmd)) {
  // eine Regel? - aufsplitten
  if((i=member(cmd,'&'))>0) {
   // ... in Array mit Verknuepfungselementen
   rule=explode(cmd[(i+1)..],"&");
   j=sizeof(rule);
   // ... in Array mit Arrays mit Alternativelementen:
   // "p|q&r|s" -> ({ ({"p","q"}), ({"r","s"}} })
   while(j--)
    rule[j]=explode(rule[j],"|");

   // Regeln von Kommandoverben abschneiden
   cmd=cmd[0..(i-1)];
  }
  // Kommandoverben extrahieren
  cmd=explode(cmd,"|");

  // Satz von Regeln existiert: Aufsplitten von Fehlermeldungen
  if(rule)
   if(stringp(flag)) {
    mixed *fail;
    // in einfaches Array mit jeweiligen Fehlermeldungen
    fail=explode(flag,"|");
    j=0;
    i=sizeof(fail);
    while(j<i) {
     // write - Fehlermeldung entdeckt - Position ggf. eintragen
     if(member(fail[j],'^')>=0 && !intp(fail[<1]))
      fail+=({j});
     if(member(fail[j],'@')>=0) {
      int s;
      flag=regexplode(fail[j], "@WE[A-SU]*[0-9]");
      s=sizeof(flag);
      while((s-=2)>0) {
       int tmpint;
       tmpint=flag[s][<1]-'1';
       if(tmpint<0 || tmpint>j)
        raise_error(sprintf(
         "AddCmd: error-message %d contains out-of-bounds @WExx-rule.\n",j+1));
      }
     }
     j++;
    }
    // "Was?|Wie das?" -> ({"Was?","Wie das?"})
    // "Was?|Wie das?^|Womit das?|Worauf das?^@WER1 macht was." ->
    //  ({"Was?",
    //    "Wie das?^Womit das?",
    //	  "Worauf das?^@WER1 macht was.",1})
    flag=sizeof(fail);
    if(flag && flag<sizeof(rule))
     raise_error(
      "AddCmd: number of error-messages does not match number of rules.\n");
    flag=fail; // ueberschreiben mit den parsefreundlichen Format
  } else if(flag)
   raise_error("AddCmd: rules exist but flags are not an error-string.\n");
 } // end if(stringp(cmd)) ... kein Regelstring vorhanden

 // kein Kommandoarray gewesen noch erzeugt?
 if(!pointerp(cmd))
   raise_error("AddCmd: missing string/pointer-parameter for command.\n");

 // Closure aus einem String erzeugen, wenn moeglich und sicher
 // (function_exists() filtert unnoetigerweise auch reine "static" funs,
 //  die genaue Pruefung ueber functionlist() kostet jedoch zuviel)
 if(stringp(func) &&
    (!extern_call() || function_exists(func,this_object())) &&
    closurep(cl=symbol_function(func,this_object())))
  func=cl;

 // jedes einzelne Verb mit seinen Regeln und Funktionen eintragen
 i=sizeof(cmd);
 if(!added_cmds) added_cmds=m_allocate(i,4);
 while(i--) {
  string str;
  str=cmd[i];
  if(!func)
   if(extern_call()) func=previous_object();
   else func=this_object();
  if(!member(added_cmds,str))
   added_cmds+=([str:allocate(0);allocate(0);allocate(0);0]);
  // existierendes Verb ergaenzen
  added_cmds[str,0]+=({func});
  added_cmds[str,1]+=({flag});
  added_cmds[str,2]+=({rule});
  // ggf. id in das ID-Mapping eintragen
  if(cmdid) {
   mixed *tmp;
   j=sizeof((string*)added_cmds[str,0]);
   tmp=added_cmds[str,3]||allocate(j);
   if(sizeof(tmp)<j) tmp+=allocate(j-sizeof(tmp));
   tmp[<1]=cmdid;
   added_cmds[str,3]=tmp;
  }
 }
}

// Auswertung fuer ein Verb loeschen
varargs int RemoveCmd(mixed cmd, int del_norule, mixed onlyid) {
 int ret;

 // Falls Magier das RemoveCmd falsch nutzen (z.B. analog zu AddCmd)
 // wird das Regelsystem verwirrt. Da das del_norule nur int sein darf,
 // gibt es hier eine gute Chance den Fehler abwaertskompatibel zu ent-
 // decken. Damit Spieler den Fehler nicht mitbekommen, wird hier auf
 // ein raise_error verzichtet, und statt dessen in ein Logfile ge-
 // schrieben.
 if (!intp(del_norule))
 {
   log_file("REMOVE_CMD",
     sprintf("\n-- %s --\nIllegal RemoveCommand() in Object [%O]:\n %O\n",
       dtime(time()), this_object(), cmd));
   del_norule=0;
   onlyid=0;
 }
 
 if(!added_cmds || (!cmd && !del_norule && !onlyid))
  added_cmds=(mapping)0; 
 else {
  int i, j;
  mixed *rule, *flag, *fun, *delrule, *ids;

  if(stringp(cmd)) {
   // Regeln entdeckt - Zerlegen (wie AddCmd)
   if((i=member(cmd,'&'))>0) {
    delrule=explode(cmd[(i+1)..],"&");
    j=sizeof(delrule);
    while(j--)
     delrule[j]=explode(delrule[j],"|");
    cmd=cmd[0..(i-1)];
   }
   cmd=explode(cmd,"|");
  } else if(del_norule || onlyid) cmd=m_indices(added_cmds);

  if(!pointerp(cmd))
   raise_error("RemoveCmd: missing string/pointer-parameter.\n");
  i=sizeof(cmd);

  while(i--) {
   // keine Regeln da und Regeln loeschen erlaubt: alles loeschen
   if(!delrule && !del_norule && !onlyid) m_delete(added_cmds,cmd[i]);
   else if(m_contains(&fun, &flag, &rule, &ids, added_cmds, cmd[i])) {
    j=sizeof(fun);
    while(j--) {
     int k;
     // DBG(rule[j]);
    	// Regeln nicht l�schen und Regel?
     if(!(del_norule && pointerp(rule[j])) &&
        // nur bestimmte ID l�schen und ID passt nicht?
        !(onlyid && (!pointerp(ids) || sizeof(ids)<=j || ids[j]!=onlyid)) &&
        // L�schregel existiert und passt nicht auf Regel?
        !(delrule && (k=sizeof(rule[j]))!=sizeof(delrule))) {
      // partielles Testen einer L�schregel ...
      if(delrule) {
       while(k--)
        if(!sizeof(rule[j][k]&delrule[k])) break;
       if(k>=0) continue;
      }
      // alles korrekt: L�schen!
      // (Arraybereich durch leeres Array loeschen)
      flag[j..j]  = allocate(0);
      fun[j..j]   = allocate(0);
      rule[j..j]  = allocate(0);
      if(ids) {
       ids[j..j] = allocate(0);
       if(!sizeof(ids-allocate(1))) ids=(mixed*)0;
      }
      ret++;
     }
    } // end while(j--) {
   }
   // Funktions/Regelliste update oder ggf. Kommando voellig loeschen
   if(sizeof(rule)) {
    added_cmds[cmd[i],0]=fun;
    added_cmds[cmd[i],1]=flag;
    added_cmds[cmd[i],2]=rule;
    added_cmds[cmd[i],3]=ids;
   } else m_delete(added_cmds,cmd[i]);
  }
  if(!sizeof(added_cmds)) added_cmds=(mapping)0;
 }
 return ret;
}

// Ausfuehren samt geparstem Inputstring und getriggerten Parserergebnissen
static int _execute(mixed fun, string str, mixed *parsed) {
 if(closurep(fun))
  return ((int)funcall(fun,str,&parsed));
 if(stringp(fun))
  return ((int)call_other(this_object(),fun,str,&parsed));
 return 0;
}

#define CHECK_PRESENT     1
#define CHECK_ID          2
#define CHECK_PUTGETNONE  4
#define CHECK_PUTGETDROP  8
#define CHECK_PUTGETTAKE  16
#define CHECK_PUTGET      (CHECK_PUTGETNONE|CHECK_PUTGETDROP|CHECK_PUTGETTAKE)

// Wert fuer Fehlschlag, Fallback-Wert der benutzten while-- - Schleifen
#define NOMATCHFOUND      -1

// Regeln fuer ein (nun unwichtiges) Verb triggern
static int _process_command(string str, string *noparsestr,
			     mixed fun, mixed flag, mixed rule) {
 mixed *parsed, *objmatches;

 // eine Regel ... auswerten ...
 if(pointerp(rule)) {
  int nrul;
  parsed=objmatches=allocate(0);
  int lastmatchpos=NOMATCHFOUND;

  // Abgleichen der gesplitteten Eingabe mit Regeln:
  // vorwaerts durch die Synonymgruppen
  int rs=sizeof(rule);
  while(nrul<rs) {
   int matchpos;
   string *synonym;
   mixed matchstr;

   matchpos=NOMATCHFOUND;
   matchstr=0;

   // Synonyme extrahieren
   int nrsynonyms=sizeof(synonym=rule[nrul]);

   // egal wie durch Synonyme bis Match - Abgleich mit Eingabe
   while(nrsynonyms--) {
    int tmppos = member(noparsestr,synonym[nrsynonyms]);
    // ist Synonym im Eingabestring und kommt spaeter als vorheriges Synonym?
    if(tmppos>=0 && tmppos>lastmatchpos) {
     // Erfolg: merken der Position im Eingabestring und den matchenden String
     matchpos=tmppos;
     matchstr=noparsestr[tmppos];
     break;
    }
   }

   // kein Match durch Synonyme? Pruefe die @-Spezialvariablen.
   if(matchpos == NOMATCHFOUND) {
    int check_present;
    // ist Abpruefen von ID/PRESENT in der Synonymgruppe verlangt
    // bei present()/find_obs gleich Voraussetzung gueltiger TP mitprufen
    if(member(synonym,"@ID")>=0) check_present=CHECK_ID;
    if(this_player()) {
     if(member(synonym,"@PRESENT")>=0) check_present|=CHECK_PRESENT;
     else if(member(synonym,"@PUT_GET_NONE")>=0)
		check_present|=CHECK_PUTGETNONE;
     else if(member(synonym,"@PUT_GET_TAKE")>=0)
		check_present|=CHECK_PUTGETDROP;
     else if(member(synonym,"@PUT_GET_DROP")>=0)
		check_present|=CHECK_PUTGETTAKE;
    }

    if(check_present) {
     // wir fangen hinter dem letzten Match an
     int q_start=lastmatchpos+1;
     int r_end=sizeof(noparsestr)-1;

     int range;
     while((range=r_end-q_start)>=0) {
      mixed tmpobj;

      // wie weit wollen wir zur�ckgehen?
      if(range)
        if(!(check_present&CHECK_PUTGET))
          range=range>2?2:range; // 3 Fragmente fuer @ID/@PRESENT (Adverb/Nr)
        else if(range>4)
          range=4;               // 5 Fragmente fuer @PUT_XXX 

      // und jetzt die Substrings pruefen
      while(range>=0 && !matchstr) {
       string tmpstr;

       // zu pruefenden String aus den Teilen zusammensetzen
       if(range) tmpstr=implode(noparsestr[q_start..(q_start+range)]," ");
       else tmpstr=noparsestr[q_start];

       //DBG(tmpstr);
       if(check_present&CHECK_PRESENT &&			// PRESENT ?
          ((tmpobj=present(tmpstr,this_player())) ||
           (tmpobj=present(tmpstr,environment(this_player())))))
        matchstr=tmpobj;
       else if(check_present&CHECK_ID && id(tmpstr))	// ID ?
        matchstr=this_object();
       else if((check_present&CHECK_PUTGET) &&	// PUT_GET_??? ?
               (tmpobj=(object*)
                  this_player()->find_obs(tmpstr,
                      ([CHECK_PUTGETNONE:PUT_GET_NONE,
                        CHECK_PUTGETDROP:PUT_GET_TAKE,
                        CHECK_PUTGETTAKE:PUT_GET_DROP])
                                        [CHECK_PUTGET&check_present])) &&
               sizeof(tmpobj)) {
        if(sizeof(tmpobj)==1) matchstr=tmpobj[0];
        else {	// Arrays werden zwischengespeichert ...
         objmatches+=({sizeof(parsed),tmpobj});
         matchstr=tmpstr;
        }
       } else { // dieser Substring hat nicht gematcht
        // ab weniger als 3 Teilen ist das Nichtmatching eines Substrings mit
        // beendendem Numeral Kriterium fuer Abbruch ("objekt 2" soll matchen)
        string numeralcheck;
        if(range && range<=2 &&
           sizeof(numeralcheck=noparsestr[q_start+range]) &&
           to_string(to_int(numeralcheck))==numeralcheck)
          break;

        // Substringlaenge verkuerzen und weiter
        range--;
       }
      }

      // Match gefunden!
      if(matchstr) {
       matchpos=range+q_start;
       // DBG(matchpos);
       break;
      }
      q_start++;
     } // end while
    }
   }

   // Fehlermeldung fuer diese fehlgeschlagene Synonymgruppe setzen
   if(matchpos == NOMATCHFOUND) {
    // Fehlermeldungen und ein Eintrag an der Fehlerstelle?
    if(pointerp(flag) && sizeof(flag)>nrul) {

     matchstr=flag[nrul];

     if(stringp(matchstr) && sizeof(matchstr)) {
      if(member(matchstr,'@')>=0) {
       matchstr=replace_personal(&matchstr,({this_player()})+parsed,1);
       string stamm=((query_verb()[<1]^'e')?query_verb():query_verb()[0..<2]);
       matchstr=regreplace(matchstr,"@VERB",capitalize(stamm),1);
       matchstr=regreplace(matchstr,"@verb",stamm,1);
      }

      // ist Fehlermeldung ein WRITE?
      // dann return 1 !
      if(intp(flag[<1]) && flag[<1]<=nrul) {
       if(member(matchstr,'^')>=0) {
        matchstr=explode(matchstr,"^");
        write(capitalize(break_string(matchstr[0],78,0,1)));
        if(sizeof(matchstr[1]))
         say(capitalize(break_string(matchstr[1],78,0,1)),({this_player()}) );
       } else write(capitalize(break_string(matchstr,78,0,1)));
       return 1;
      } else notify_fail(capitalize(break_string(matchstr,78,0,1)));
     }
    }
    return 0;
   }

   // Updaten der Hilfsvariablen
   parsed+=({matchstr});
   lastmatchpos=matchpos;
   nrul++;
  } // end while(nrul<rs) ... Erfolg ... ab zum naechsten Regelteil
 }

 // Arrays der @-Objectmatches in Parameter fuer Methode resubstituieren
 int objsize;
 if((objsize=sizeof(objmatches)))
  while((objsize-=2)>=0)
   parsed[objmatches[objsize]]=objmatches[objsize+1];

 // erfolgreich Methode gefunden/eine Regel bis zum Ende durchgeparst:
 return(_execute(&fun,&str,&parsed));
}

// Auswertung - add_action-Fun 
public int _cl(string str) {
 int nindex;
 string *keys;
 mixed *flag;
 // Verb existiert, Kommandos auch, Eintrag fuer Verb gefunden
 if(mappingp(added_cmds) && query_verb()) {
  mixed *fun, *rule, *ids;

  // ist das Verb ein Key im Kommandomapping?
  if(m_contains(&fun, &flag, &rule, &ids, added_cmds, query_verb())) {
   string *noparsestr;
   nindex=sizeof(fun);
   noparsestr=explode((str||"")," ")-({""});
   // dann matche ungeparsten Input gegen etwaige Regeln
   // -- nicht aendern: neue Kommandos sollen alte "ueberschreiben" koennen
   while(nindex--)
    if(_process_command(&str, noparsestr,
		       fun[nindex], flag[nindex], rule[nindex]))
    {
      GiveEP(EP_CMD, query_verb());
      return 1;
    }
  }

  // keine Regel passte, unscharfe Auswertung auf alle
  // AddCmdverben ausdehnen
  keys=m_indices(added_cmds);
  nindex=sizeof(keys);
  while(nindex--)
   if(!strstr(query_verb(),keys[nindex]) &&
      member((flag=added_cmds[keys[nindex],1]),1)>=0) {
    int i,ret;
    i=sizeof(flag);
    // Reihenfolge nicht aendern !
    while(i--)
     if(flag[i]==1 && _execute(added_cmds[keys[nindex],0][i],str,0))
     {
       GiveEP(EP_CMD, query_verb());
       return 1;
     }
   }
 }
 return 0;
}

void init() {
 add_action("_cl","",1);
}


static void _check_copy_commands(string ind, mixed *fun,
				  mixed *flag, mixed *rule) {
 if(pointerp(fun)) added_cmds[ind,0]=fun+allocate(0);
 else added_cmds[ind,0]=({fun});
 if(pointerp(flag)) added_cmds[ind,1]=flag+allocate(0);
 else if(flag) added_cmds[ind,1]=({flag});
 else added_cmds[ind,1]=allocate(sizeof(added_cmds[ind]));
 if(pointerp(rule)) added_cmds[ind,2]=rule+allocate(0);
 else added_cmds[ind,2]=allocate(sizeof(added_cmds[ind]));

 if(sizeof(added_cmds[ind])!=sizeof(added_cmds[ind,1]) ||
    sizeof(added_cmds[ind])!=sizeof(added_cmds[ind,2])) {
  added_cmds=(mapping)0;
  raise_error("SetProp(P_COMMANDS): corrupt commands-mapping.\n");
 }
}

static mapping _set_commands(mapping commands) {
 if(!commands) added_cmds=(mapping)0;
 else if(mappingp(commands)) {
  added_cmds=m_allocate(sizeof(commands),4);
  walk_mapping(commands,#'_check_copy_commands);
 }
 if (mappingp(added_cmds))
   return(deep_copy(added_cmds));
 else
   return (mapping)0;
}

static mapping _query_commands() {
 if (mappingp(added_cmds))
   return(deep_copy(added_cmds));
 else
   return (mapping)0;
}

