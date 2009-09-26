// MorgenGrauen MUDlib
//
// restriction_checker.c -- Beschraenkungen in Gilden, Spells und Skills
//
// $Id: restriction_checker.c 6829 2008-04-13 19:39:10Z Zesstra $
#pragma strong_types
#pragma save_types
#pragma no_clone
#pragma pedantic
//#pragma range_check
#pragma no_shadow

#define NEED_PROTOTYPES
#include <thing/properties.h>
#include <properties.h>
#include <new_skills.h>
#include <wizlevels.h>

// TODO: mit execute_anything aus /std/util/executer.c zusammenfuehren.
static mixed execute_anything(mixed fun, object pl, mixed args) {
  if (!fun) return 0;
  if (closurep(fun)
      && objectp(query_closure_object(fun)))
    return funcall(fun,pl,args);
  if (stringp(fun))
    return call_other(this_object(),fun,pl,args);
  if (pointerp(fun)
      && sizeof(fun)>=2
      && stringp(fun[1])
      && (stringp(fun[0]) || objectp(fun[0])))
    return call_other(fun[0],fun[1],pl,fun[2..],args);
  return 0;
}

mapping race_modifier(object pl, mapping map_ldfied) {
  mapping rmap,help;

  if (mappingp(map_ldfied) && member(map_ldfied,SM_RACE) && objectp(pl) && mappingp(rmap=map_ldfied[SM_RACE]))
//    if (mappingp(help=rmap[pl->_query_real_race()]))
    if (mappingp(help=rmap[pl->QueryProp(P_REAL_RACE)]))
	return map_ldfied+help;
    else if(member(rmap,"*") && mappingp(help=rmap["*"]))
	return map_ldfied+help;
  return map_ldfied;
}

string
check_restrictions(object pl, mapping restr) {
  /* Folgende Einschraenkungen koennen geprueft werden:
   * - Mindestwerte bei allen Attributen und Properties Level, QP, XP
   * - bestimmte Rassen koennen ausgeschlossen werden bzw. erlaubt werden
   *   (SR_EXCLUDE_RACE ist ein Array von ausgeschlossenen Rassen,
   *    SR_INCLUDE_RACE eines mit eingeschlossenen)
   * - SR_RACE kann ein Mapping sein, dass Rassen ein eigenes
   *   Restriction-mapping zuordnet. "*" steht fuer alle anderen Rassen.
   */
  int i;
  closure cl,cl2;
  string race, guild, *ind;

  if (!mappingp(restr) || !sizeof(restr)) return 0; // Keine Einschraenkungen
  if (!objectp(pl)) return "";
  if (pl->QueryDisguise())
    return "Zieh erst mal den Tarnhelm aus.\n";

  restr=race_modifier(&pl,&restr);

  i=sizeof(ind=m_indices(restr));
  cl=symbol_function("QueryProp",pl);
  cl2=symbol_function("QueryAttribute",pl);

  while(i--)
   switch(ind[i]) {
     case P_LEVEL:
	if (funcall(cl,P_LEVEL) < restr[P_LEVEL]) 
	  return "Deine Stufe reicht dafuer nicht aus.\n";
	break;
     case P_GUILD_LEVEL:
	if (funcall(cl,P_GUILD_LEVEL) < restr[P_GUILD_LEVEL])
	  return "Deine Gildenstufe reicht dafuer nicht aus.\n";
	break;
     case SR_SEER: // das macht nun wahrlich nur bei interactives sinn!!!
	if (restr[SR_SEER] && query_once_interactive(pl) && !IS_SEER(pl))
	  return "Du musst dazu erst Seher werden.\n";
	break;
     case P_XP:
	if (funcall(cl,P_XP) < restr[P_XP])
	  return "Du hast nicht genuegend Erfahrung dafuer.\n";
	break;
     case P_QP:
	if (funcall(cl,P_QP) < restr[P_QP])
	  return "Du hast nicht genuegend Aufgaben geloest.\n";
	break;
     case P_ALCOHOL:
	if (funcall(cl,P_ALCOHOL) >= restr[P_ALCOHOL])
	  return "Du bist zu besoffen.\n";
	break;
     case P_DRINK:
	if (funcall(cl,P_DRINK) >= restr[P_DRINK])
	  return "Du hast zuviel getrunken.\n";
	break;
     case P_FOOD:
	if (funcall(cl,P_FOOD) >= restr[P_FOOD])
	  return "Du hast zuviel gegessen.\n";
	break;
     case P_DEAF:
	if (funcall(cl,P_DEAF))
	  return "Du kannst nichts hoeren.\n";
	break;
     case P_BLIND:
	if (funcall(cl,P_BLIND))
	  return "Du kannst nichts sehen.\n";
	break;
     case P_FROG:
	if (funcall(cl,P_FROG))
	  return "Als Frosch kannst Du das leider nicht.\n";
	break;
     case A_INT:
	if (funcall(cl2,A_INT) < restr[A_INT])
	  return "Deine Intelligenz reicht dafuer nicht aus.\n";
	break;
     case A_DEX:
	if (funcall(cl2,A_DEX) < restr[A_DEX])
	  return "Deine Geschicklichkeit reicht dafuer nicht aus.\n";
	break;
     case A_CON:
	if (funcall(cl2,A_CON) < restr[A_CON])
	  return "Deine Ausdauer reicht dafuer nicht aus.\n";
	break;
     case A_STR:
	if (funcall(cl2,A_STR) < restr[A_STR])
	  return "Deine Staerke reicht dafuer nicht aus.\n";
	break;
     case SR_BAD:
	if (funcall(cl,P_ALIGN)<restr[SR_BAD])
	  return "Du bist zu boese.\n";
	break;
     case SR_GOOD:
	if (funcall(cl,P_ALIGN)>restr[SR_GOOD])
	  return "Du bist nicht boese genug.\n";
	break;
     case SR_MIN_SIZE:
	if (funcall(cl,P_SIZE)<restr[SR_MIN_SIZE])
	  return "Du bist dafuer zu klein.\n";
	break;
     case SR_MAX_SIZE:
	if (funcall(cl,P_SIZE)>restr[SR_MAX_SIZE])
	  return "Du bist dafuer zu gross.\n";	      
	break;
     case SR_FREE_HANDS:
	if (restr[SR_FREE_HANDS] > 
	    (funcall(cl,P_MAX_HANDS)-funcall(cl,P_USED_HANDS)))
	return "Du hast nicht genug freie Haende dafuer.\n";
	break;
     case SR_EXCLUDE_RACE:
	if (IS_LEARNER(pl)) race=funcall(cl,P_RACE); else race=funcall(cl,P_REAL_RACE); //race=pl->_query_real_race();
	if (pointerp(restr[SR_EXCLUDE_RACE]) &&
	    member(restr[SR_EXCLUDE_RACE],race)>=0)
	  return ("Als "+race+" kannst Du das nicht.\n");
	break;
     case SR_INCLUDE_RACE:
	if (IS_LEARNER(pl)) race=funcall(cl,P_RACE); else race=funcall(cl,P_REAL_RACE); //race=pl->_query_real_race();
	if (pointerp(restr[SR_INCLUDE_RACE]) &&
	    member(restr[SR_INCLUDE_RACE],race)<0)
	  return ("Als "+race+" kannst Du das nicht.\n");
	break;
     case SR_EXCLUDE_GUILD:
	guild=funcall(cl,P_GUILD);
	if (pointerp(restr[SR_EXCLUDE_GUILD]) &&
	    member(restr[SR_EXCLUDE_GUILD],guild)>=0)
	  return ("Mit Deiner Gildenzugehoerigkeit kannst Du das nicht.\n");
	break;
     case SR_INCLUDE_GUILD:
	guild=funcall(cl,P_GUILD);
	if (pointerp(restr[SR_INCLUDE_GUILD]) &&
	    member(restr[SR_INCLUDE_GUILD],guild)<0)
	  return ("Mit Deiner Gildenzugehoerigkeit kannst Du das nicht.\n");
	break;
   }

  if(member(restr,SR_FUN))
   return execute_anything(restr[SR_FUN],pl,0);

  return 0;
}

varargs mixed
GetData(string dname, mapping map_ldfied, object pl) {
  mixed dat,res;

  if (!dname || !mappingp(map_ldfied)) return 0;
  if (closurep(dat=map_ldfied[dname]) && (res=funcall(dat,pl,map_ldfied)))
	return res;
  return dat;
  
}

varargs int
GetValue(string vname, mapping map_ldfied, object pl) {
  mixed dat,res;

  // printf("GetValue(%O): %O\n",vname,map_ldfied);
  if (!vname || !map_ldfied) return 0;
  if ((dat=map_ldfied[vname]) && (res=execute_anything(dat,pl,map_ldfied)))
	return res;
  // printf("Value: %O\n",dat);
  return intp(dat) ? dat : 0;
}

varargs int
GetFactor(string fname, mapping map_ldfied, object pl) {
  mixed res;

  // printf("GetFactor(%O):\n",fname);
  if (!fname  || !(res=GetValue(FACTOR(fname),map_ldfied,pl)))
	return 100;
  if (res<10) res=10;
  else if (res>1000) res=1000;
  // printf("Factor: %O\n",res);
  return res;
}

varargs int
GetOffset(string oname, mapping map_ldfied, object pl) {
  mixed res;

  // printf("GetOffset(%O):\n",oname);
  if (!oname  || !(res=GetValue(OFFSET(oname),map_ldfied,pl)))
	return 0;
  if (res<-10000) res=-10000;
  else if (res>10000) res=10000;
  // printf("Offset: %O\n",res);
  return res;
}

varargs int
GetFValue(string vname, mapping map_ldfied, object pl) {
  return (GetValue(vname,map_ldfied,pl)*GetFactor(vname,map_ldfied,pl))/100;
}

varargs int
GetValueO(string vname, mapping map_ldfied, object pl) {
  return GetValue(vname,map_ldfied,pl)+GetOffset(vname,map_ldfied,pl);
}

varargs int
GetFValueO(string vname, mapping map_ldfied, object pl) {
  return ((GetValue(vname,map_ldfied,pl)*GetFactor(vname,map_ldfied,pl))/100
		  + GetOffset(vname,map_ldfied,pl));
}

varargs int
GetRandFValueO(string vname, mapping map_ldfied, object pl) {
  return (random(GetValue(vname,map_ldfied,pl)*GetFactor(vname,map_ldfied,pl))/100
	  + GetOffset(vname,map_ldfied,pl));
}

mapping AddSkillMappings(mapping oldmap, mapping newmap) {
  mapping res,t1,t2;
  mixed *inx,ind;
  int i;

  if (!mappingp(oldmap)) return newmap;
  if (!mappingp(newmap)) return oldmap;
  inx=({SI_SKILLRESTR_LEARN,SI_SKILLRESTR_USE,SM_RACE});
  res=oldmap+newmap;
  for (i=sizeof(inx);i--;) {
	ind=inx[i];
	t1=oldmap[ind];
	t2=newmap[ind];
	if (t1) {
	  if (t2)
		res[ind]=t1+t2;
	  else
		res[ind]=t1;
	} else {
	  if (t2)
		res[ind]=t2;
	}
  }
  return res;
}

