// MorgenGrauen MUDlib
//
// container/description.c -- standard description for containers
//
// $Id: description.c 6986 2008-08-22 21:32:15Z Zesstra $

inherit "std/thing/description";

#pragma strict_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

#define NEED_PROTOTYPES

#include <container.h>
#include <properties.h>
#include <defines.h>
#include <wizlevels.h>

private nosave int last_light_calc;

void create()
{
  ::create();
  SetProp(P_TRANSPARENT, 1);
  // wieviel Licht schluckt der Container _muss_ > 0 sein!
  SetProp(P_LIGHT_TRANSPARENCY, 2);
  AddId("Container");
  last_light_calc=-1;
}

varargs string long(int mode) {
  string descr, inv_descr;

  descr = process_string(QueryProp(P_LONG));
  if(!QueryProp(P_TRANSPARENT)) return descr;

  inv_descr = make_invlist(PL, all_inventory(ME), mode );
  if ( inv_descr != "" )
    descr += capitalize(QueryPronoun(WER)) + " enthaelt:\n" + inv_descr;
  return descr;
}

// flags: 1 - wizard, 2 - don't collect equal objects '
// flags: 4 - don't append infos for wizards
private void stringenize(mixed obj, int flags, mixed objs, mixed info)
{
  string id, tmp;
  int idx;
  
  if ( (!(flags & 4) && (flags & 1))) {
    //wenn Magier und Magierinfos angehaengt werden sollen:
    tmp = capitalize(obj->short()||"")[0..<2]
        + " ["+object_name(obj)+"]";
  }
  else if (obj->QueryProp(P_INVIS)) {
    //keine Ausgabe bzw. leerer String, wird dann von collect rausgeschmissen
    tmp="";
  }
  else {
    //Spieler, Magier ohne P_WANTS_TO_LEARN etc.
    tmp = capitalize(obj->short()||"")[0..<2];
  }
  //wenn wizard und 'dont collect equal objects': id ist object_name()
  if(flags & 3 || living(obj)) 
    id = object_name(obj);
  //sonst nehmen wir den Namen der Blueprint ->zusammenfassen der Objekte
  else
    id = BLUE_NAME(obj) + tmp;
  // Anzahl des jeweiligen Objekts (anhand von id) zaehlen.
  if((idx = member(objs, id)) == -1)
  {
    objs += ({ id });
    info += ({ ({ tmp, 1, obj}) });
  }
  else
    info[idx][1]++;
}

private string collect(mixed obj)
{
  //bekommt ein Array ({name,anzahl,objekt})
  //Objekte ohne Namen (P_SHORT==0 oder P_INVIS) wegwerfen
  if(!strlen(obj[0])) return 0;
  // Objektname + (Anzahl) zurueckgeben.
  return obj[0] + (obj[1] > 1 ? " ("+obj[1]+")" : "");
}

// flags: 1 - return array, 2 - don't collect equal objects '
// flags: 4 - don't append infos for wizards
varargs mixed make_invlist(object viewer, mixed inv, int flags)
{
  int iswiz;
  mixed objs, info;

  iswiz = IS_LEARNER( viewer ) && viewer->QueryProp(P_WANTS_TO_LEARN);
  objs = ({}); info = ({});
  map(inv, #'stringenize/*'*/, iswiz | (flags & 2) | (flags & 4), &objs, &info);
  if(flags & 1) return info;
  inv = map(info, #'collect/*'*/) - ({ 0 });
  if(!sizeof(inv)) return "";
  return sprintf("%"+(sizeof(inv) > 6 ? "#" : "=")+"-78s",
                 implode(inv, "\n")) + "\n";
}

static mixed _query_light_transparency()
{
   if (QueryProp(P_TRANSPARENT)) return Query(P_LIGHT_TRANSPARENCY);
   return 999;
}

static int _query_total_light()
{
   int i, tmp, light, dark;
   object *inv;

   if ( _query_last_content_change() == last_light_calc )
       return Query(P_TOTAL_LIGHT);

   for (i=sizeof(inv=all_inventory())-1; i>=0; i--) {
      tmp=(int)inv[i]->QueryProp(P_TOTAL_LIGHT);
      if (tmp>0 && light<tmp)
          light=tmp;
      else if (dark>tmp) dark=tmp;
   }

   // P_INT_LIGHT berechnen...
   tmp=QueryProp(P_LIGHT);
   if (tmp>0 && light<tmp)
      SetProp(P_INT_LIGHT, ({ tmp, dark, tmp+dark }));
   else if (dark>tmp) SetProp(P_INT_LIGHT, ({ light, tmp, light+tmp }));
   else SetProp(P_INT_LIGHT, ({ light, dark, light+dark }));

   // Container schlucken ggf. Licht nach aussen hin...
   light+=dark; // Licht verrechnen
   if (light>=0)
      i=light-QueryProp(P_LIGHT_TRANSPARENCY); // Lichtdurchlaessigkeit
   else i=light+QueryProp(P_LIGHT_TRANSPARENCY);
   // wenn sich das Vorzeichen geaendert hat auf 0 setzen.
   light=((i^light) & 0x80000000 ? 0 : i);
   // wenn das eigene Licht dasselbe vorzeichen hat nur das staerkere nehmen
   light=((tmp^light) & 0x80000000 ? light+tmp : (tmp>light ? tmp : light));

   last_light_calc = _query_last_content_change();
   return SetProp(P_TOTAL_LIGHT, light);
}

static int _query_int_light()
{
   int tmp;
   if ( _query_last_content_change() != last_light_calc )
       _query_total_light();

   // P_INT_LIGHT des environments kann sich natuerlich aendern _ohne_ das
   // etwas an einem container geaendert wird. Daher Auswertung jedes mal
   // neu aktualisieren.
   if (!environment() || !(tmp=(int)environment()->QueryProp(P_INT_LIGHT)))
      return Query(P_INT_LIGHT)[2];
   else {
      int light, dark, reduce;

      light=Query(P_INT_LIGHT)[0];
      dark =Query(P_INT_LIGHT)[1];
      // bei einem offenen Container kann natuerlich das Licht des
      // environments den Container auch ausleuchten....
      reduce=QueryProp(P_LIGHT_TRANSPARENCY); // Lichtdurchlaessigkeit
      if (tmp>0 && light<tmp-reduce)
         light=tmp-reduce;
      else if (dark>tmp+reduce) dark=tmp+reduce;
      else if (light+dark==0) return tmp;
      return light+dark;
   }
}

static int _set_light(int light)
{
  object env;
  last_light_calc=-1;
  env=ME;
  while (objectp(env)) {
      // Ja. Man ruft die _set_xxx()-Funktionen eigentlich nicht direkt auf.
      // Aber das Lichtsystem ist schon *so* rechenintensiv und gerade der
      // P_LAST_CONTENT_CHANGE-Cache wird *so* oft benoetigt, dass es mir
      // da um jedes bisschen Rechenzeit geht.
      // Der Zweck heiligt ja bekanntlich die Mittel. ;-)
      //
      // Tiamak
      env->_set_last_content_change();
     env=environment(env);
  }
  return Set(P_LIGHT, light);
}
