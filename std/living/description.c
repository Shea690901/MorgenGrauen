// MorgenGrauen MUDlib
//
// living/description.c -- description for living objects
//
// $Id: description.c 7123 2009-02-14 22:13:54Z Zesstra $
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

inherit "/std/container/description";

#define NEED_PROTOTYPES

#include <living/skills.h>
#include <living/clothing.h>
#include <thing/properties.h>
#include <wizlevels.h>
#include <new_skills.h>
#include <properties.h>
#include <language.h>
#include <defines.h>
#include <class.h>
#include <sys_debug.h>

public string _query_internal_extralook() {
  mixed xl;
  int zeit;
  string res, look="";

  xl=Query(P_INTERNAL_EXTRA_LOOK,F_VALUE);
  if (!mappingp(xl))
    return(0);

  foreach(string key, mapping xld: xl) {
    if (intp(zeit=xld["xlduration"])) {
      //hat offenbar nen Ablaufdatum
      if ( (zeit > 0 && zeit < time()) ||
           (zeit < 0 && abs(zeit) < object_time(ME)) ) {
	// Zeit abgelaufen oder
	// negative "Ablaufzeit" und das Objekt ist neuer als die
	// Eintragzeit, also loeschen und weiter (ja, das geht. ;-) und xld
	// hat das Eintragsmapping ja noch, weitere Benutzung also moeglich.)
	efun::m_delete(xl,key);
	// ggf. Meldung ausgeben
	if (interactive(ME)) {
	  if (strlen(xld["xlende"])) {
	    tell_object(ME,xld["xlende"]);
	  }
	  //kein einfacher String, aber Objekt+Funktion gegeben?
	  else if (strlen(xld["xlendefun"]) && strlen(xld["xlobjectname"]) &&
	    (!catch(res=call_other(xld["xlobjectname"],xld["xlendefun"],ME)
		    ;publish))) {
	      if (stringp(res) && strlen(res))
		tell_object(ME,res);
	    }
	}
	continue;
      }
    }
    // Der Eintrag ist offenbar noch gueltig, Meldung anhaengen, bzw. via
    // Funktionsaufruf beschaffen.
    if (strlen(xld["xllook"]))
      look+=xld["xllook"];
    else if (strlen(xld["xlfun"]) && strlen(xld["xlobjectname"]) &&
	(!catch(res=call_other(xld["xlobjectname"],xld["xlfun"],ME);publish))) {
      if (!stringp(res) || !strlen(res)) {
	// keinen String oder leeren String gekriegt -> ueberspringen.
	continue;
      }
      else
	look+=res;
    }
  }
  // fertig. Wenn look nicht leer ist, zurueckgeben, sonst 0.
  if (strlen(look))
    return(look);
  else
    return(0);
}

public varargs int AddExtraLook(string look, int duration, string key, 
                                string lookende, object ob) {
  mapping xl;
  string oname;
  if (!stringp(key) || !strlen(key)) {
    // Automatisch erzeugen, wenn moeglich
    if (!objectp(previous_object()) || 
	!stringp(key=object_name(previous_object())) || !strlen(key))
      return(-1);
  }
  
  if (!stringp(look) || !strlen(look))
    return(-2);
  if (!intp(duration))
    return(-3);
  
  xl=Query(P_INTERNAL_EXTRA_LOOK,F_VALUE); // dran denken: liefert referenz zurueck
  if (!mappingp(xl)) {
    Set(P_INTERNAL_EXTRA_LOOK, xl=([]) );
  }

  // kein Automatisches Ueberschreiben.
  if (member(xl,key))
    return(-4);

  // neg. Werte: "bis Ende/reboot", abs(duration) == Eintragzeit
  // 0: "fuer ewig", >0: Zeitdauer in Sekunden
  if (duration > 0)
    duration+=time();  // hoffentlich gibt es reichtzeitig 64bit-Ints
  else if (duration < 0)
    duration=negate(time());
  // 0 bleibt, wie es ist.

  if (objectp(ob)) {
    // Funktionsname und Objektname (als Name, damit es auch noch geht, wenn
    // das Objekt entladen wurde, Crash/reboot war etc.) speichern
    if (!oname=object_name(blueprint(ob)))
	oname=BLUE_NAME(ob); //BLUE_NAME nimmt explode()...
    xl[key]=(["xlobjectname":oname,
	      "xlfun": look,
	     ]);
    // ggf. Name der Funktion speichern, die bei Ablauf aufgerufen wird.
    if (stringp(lookende) && strlen(lookende))
	xl[key]["xlendefun"]=lookende;
  }
  else {
    // Einfacher Eintrag, nur den bearbeiteten String merken. ;-)
    xl[key]=(["xllook": break_string(replace_personal(look,({ME}),1),78,
	                             "",BS_LEAVE_MY_LFS),
	     ]);
    // ggf. Meldung speichern, die bei Ablauf ausgegeben werden soll.
    if (stringp(lookende) && strlen(lookende)) {
      xl[key]["xlende"]=break_string(replace_personal(lookende,({ME}),1),78,
	                             "",BS_LEAVE_MY_LFS);
    }
  }
  // Endezeit vermerken.
  if (duration != 0)
    xl[key]["xlduration"]=duration;

  // Kein Set noetig, weil Query das Mapping ja als Referenz lieferte.
  return(1);
}

public int RemoveExtraLook(string key) {
  mapping xl;
  if (!stringp(key) || !strlen(key)) {
    // Automatisch erzeugen, wenn moeglich
    if (!objectp(previous_object()) ||
	!stringp(key=object_name(previous_object())) || !strlen(key))
      return(-1);
  }
  xl=Query(P_INTERNAL_EXTRA_LOOK,F_VALUE); // dran denken: liefert referenz zurueck
  if (!mappingp(xl))
    return (-2);
  if (!member(xl,key))
    return(-2);

  efun::m_delete(xl,key);
  // Kein Set noetig, weil Query das Mapping ja als Referenz lieferte.
  return(1);
}

void create()
{ 
  ::create();
  Set(P_GENDER, SAVE, F_MODE);
  // Extralook-Property speichern und vor manueller Aenderung schuetzen
  // EMs duerfen, die wissen hoffentlich, was sie tun.
  Set(P_INTERNAL_EXTRA_LOOK, SAVE|PROTECTED, F_MODE_AS);
  SetProp(P_LIGHT_TRANSPARENCY, 0);
  SetProp(P_CLOTHING,({}));
  AddId("Living");
}

string condition()
{
  int hpnt, max_hpnt, perc;

  hpnt        = QueryProp( P_HP );
  max_hpnt    = QueryProp( P_MAX_HP );

  if(max_hpnt>0 && hpnt>0)
    perc=100*hpnt/max_hpnt;
 
  switch(perc) {
    case 0..9:
	return capitalize(QueryPronoun(WER))+" steht auf der Schwelle des Todes.\n";
    case 10..19:
	return capitalize(QueryPronoun(WER))+" braucht dringend einen Arzt.\n";
    case 20..29:
	return capitalize(QueryPronoun(WER))+" ist in keiner guten Verfassung.\n";
    case 30..39:
	return capitalize(QueryPronoun(WER))+" wankt bereits bedenklich.\n";
    case 40..49:
	return capitalize(QueryPronoun(WER))+" macht einen mitgenommenen Eindruck.\n";
    case 50..59:
	return capitalize(QueryPronoun(WER))+" sieht nicht mehr taufrisch aus.\n";
    case 60..69:
	return capitalize(QueryPronoun(WER))+" ist leicht angeschlagen.\n";
    case 70..79:
	return capitalize(QueryPronoun(WER))+" fuehlte sich heute schon besser.\n";
    case 80..89:
	return capitalize(QueryPronoun(WER))+" ist schon etwas geschwaecht.\n";
  }
  //fall-through
  return capitalize(QueryPronoun(WER))+" ist absolut fit.\n";
}

varargs string long() {
  string str, cap_pronoun;
  string descr, invl,tmp,exl;
  int hpnt, max_hpnt;
  mixed filter_ldfied;
  object ob;

  str = process_string( QueryProp(P_LONG) );
  if(!stringp(str)) str = "";

  str += condition();

  // Extralook
  if(stringp(tmp = QueryProp(P_EXTRA_LOOK)))
    str += tmp;
  if (stringp(tmp = QueryProp(P_INTERNAL_EXTRA_LOOK)))
    str += tmp;
  for(ob = first_inventory(ME); ob; ob = next_inventory(ob))
    if(exl = ob->QueryProp(P_EXTRA_LOOK)) 
      str += exl;
    else if(exl = ob->extra_look()) 
      str += exl; // TO BE REMOVED

  
  if(filter_ldfied = QueryProp(P_TRANSPARENT))
  {
    invl = make_invlist(PL, all_inventory(ME));
    if(invl != "")
      str += capitalize(QueryPronoun(WER))+" traegt bei sich:\n" + invl;
  }
  return str;
}

varargs string name(int casus, int demonst)
{
  string s;
  
  if( QueryProp( P_INVIS ) )
  {
    if( casus == RAW ) return "Jemand";
    return ({"Jemand","Jemands","Jemandem","Jemanden"})[casus];
  }
  if (QueryProp(P_FROG) && casus != RAW )
  {
    SetProp(P_ARTICLE,1);
    s=QueryArticle(casus,0)+"Frosch";
    if (casus==WESSEN) s += "es";
    SetProp(P_ARTICLE,0);
    return s;
  }
  return ::name( casus, demonst );
}

static int _query_gender()
{
  if (QueryProp(P_FROG)) return 1;
  return Query(P_GENDER);
}

// NPC sollen aus Kompatibilitaetsgruenden auch eine "echte" Rasse haben.
// Default ist hier die Rasse, man kann aber hiermit auch einen NPC faken,
// der sich tarnt, indem man P_REAL_RACE per Hand setzt.
static string _query_real_race()
{
  return Query(P_REAL_RACE,F_VALUE)||QueryProp(P_RACE);
}

static mixed _set_name(mixed nm )
{
  string lvnam;
  lvnam = nm;
  if(pointerp(nm)) lvnam = nm[0];
  set_living_name(lower_case(lvnam));
  return Set(P_NAME, nm);
}

int _query_container()
{
  return 0;
}

int is_class_member(mixed str) {
  mixed cl;
  if (::is_class_member(str))
    return 1;

  if (stringp(str))
    str = ({str});

  if (!pointerp(str))
    return 0;

  if ((member(str, CL_UNDEAD) >= 0) && 
      ::is_class_member( ({CL_ZOMBIE, CL_SKELETON, CL_GHOUL}) ))
    return 1;
  else if ((member(str, CL_GOBLIN) >= 0) &&
	   ::is_class_member( ({ CL_HOBGOBLIN }) ))
    return 1;
  else if ( (cl = QueryProp(P_RACE)) && member( str, lower_case(cl) ) > -1 )
    return 1;
  else
    return 0;
}

mapping _query_material() {
  mixed res;
 
  if (mappingp(res=Query(P_MATERIAL)))
    return res;
  return ([MAT_MISC_LIVING:100]);
}

static int _query_player_light()
{  return _query_int_light()+QueryProp(P_LIGHT_MODIFIER);  }

varargs int CannotSee(int silent)
{
   string is_blind;
   if (is_blind = QueryProp(P_BLIND)) {
      if (!silent) {
         if (stringp(is_blind))
            tell_object(this_object(), is_blind);
         else tell_object(this_object(), "Du bist blind!\n");
      }
      return 2;
   }
   if (UseSkill(SK_NIGHTVISION)<=0 &&
       environment() && QueryProp(P_PLAYER_LIGHT)<=0 &&
       (!IS_LEARNER(this_object()) || !Query(P_WANTS_TO_LEARN)))
   {
       if (!silent) tell_object(this_object(), "Es ist zu dunkel!\n");
       return 1;
   }
   return 0;
}
