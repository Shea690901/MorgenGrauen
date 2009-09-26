// MorgenGrauen MUDlib
//
// npc/info.c -- Behandeln von Fragen an den NPC
//
// $Id: info.c 7208 2009-04-06 20:39:37Z Zesstra $

/* Letzte Aenderungen von Wim 8.1.99
 *
 * AddInfo( schluessel, antwort [, indent [, [silent [, casebased] ] ] )
 *  Wenn ein Spieler dieses Monster nach "schluessel" fragt, so gib die
 *  Programmierte Antwort aus.
 *  Erweiterung von Wim: ist silent gesetzt, so erfolgt eine "persoenliche"
 *    Antwort. d.h. umstehende Personen bekommen bei silent==1 keinen, bei
 *    stringp(silent), den String ausgegeben, dabei kann er auch die Schluessel-
 *    Worte @WER @WESSEN @WEM @WEN enthalten.
 *  - Ergaenzt um @CAP_WER... fuer zwangs-capitalisierte Namen an Satzanfaengen.
 *    ist bei fragenden NPCs und PCs mit Tarnkappe wichtig! (Silvana)
 *  - Auch in der Antwort des fragenden wird nun ersetzt (Vanion)
 *  Enthaelt casedbased einen Funktionsnamen oder verweist auf eine closure, so
 *    wird die Beantwortung der Frage von dem return-Wert abhaengig gemacht.
 *    Bei 0 wird die Frage normal beantwortet, bei 1 erfolgt die Ausgabe des
 *    unter DEFAULT_NOINFO gespeicherten Textes.
 *    Wird ein String zurueckgegeben, so wird er unter Beachtung von ident an
 *    Stelle der urspruenglichen Information ausgegeben.
 *
 * RemoveInfo( schluessel )
 *  Das Monster antwortet nicht mehr auf diesen Schluessel.
 *
 * SetProp( P_DEFAULT_INFO, antwort [, indent ] )
 *  Setze die Antwort, die das Monster auf unverstaendliche Fragen geben
 *  soll. (Diese Funktion ist obsolet! Benutze stattdessen
 *  AddInfo( "\ndefault info", antwort [, indent ] );
 *
 * GetInfo( [schluessel] )
 *  Wenn Schluessel gesetzt ist, so wird die dazugehoerige Info,
 *  ansonsten werden alle Infos zurueckgegeben.
 *
 * Die Antworten sollten wie emote - kommandos aussehen.
 * Der optionale Indent wird zum Umbrechen von langen Infos benutzt.
 * (Typischerweise sollte indent="sagt: " sein.)
 *
 * In den Infos darf mit process_string gearbeitet werden. Das Ergebnis von
 * process_string wird dann mit umgebrochen!
 *
 *---------------------------------------------------------------------------
 */
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

#define NEED_PROTOTYPES
#include <thing/description.h>
#include <thing/properties.h>
#include <npc.h>
#undef NEED_PROTOTYPES

#include <properties.h>
#include <language.h>
#include <defines.h>
#include <config.h>
#include <exploration.h>

// TODO: langfristig waer hier private schoen.
nosave mapping infos;

protected void create()
{
    // Initialisierung nur wenn noetig, damit beim virtuellen Erben von
    // /std/npc in npc1 und npc2 dann in npc3 beim npc1::create();
    // npc2:create(); im zweiten create() die Infos nicht
    // ueberschrieben/geloescht werden.
    if (!mappingp(infos)) {
        infos = ([
            DEFAULT_INFO:"schaut Dich fragend an.\n";0;
            "schaut @WEN fragend an.\n";0,
            DEFAULT_NOINFO:"moechte Dir nicht antworten.\n";0;
            "verweigert @WEM die Antwort.\n";1
            ]);
    }
}


void init() {
  add_action( "frage", "frag", 1 );
}


static void smart_npc_log(string str)
{
  string creat, creat_det;

  if (!stringp(creat=QueryProp(P_LOG_INFO))) {
    creat = MASTER->creator_file(this_object());
    if (creat == ROOTID)
      creat = "ROOT";
    else if( !creat || creat[0]==' ' )
      creat="STD";
    creat_det="report/"+explode(creat, ".")[<1]+"_INFO.rep";
    creat="report/"+explode(creat, ".")[<1]+".rep";
  }
  log_file(creat,
           sprintf("INFO von %s [%s] (%s):\n%s\n",
                   getuid(this_interactive()),
                   explode(object_name(this_object()),"#")[0],
                   dtime(time())[5..<11],
                   str));
  log_file(creat_det,
           sprintf("INFO von %s [%s] (%s):\n%s\n",
                   getuid(this_interactive()),
                   explode(object_name(this_object()),"#")[0],
                   dtime(time())[5..<11],
                   str));
}

public int frage(string str) {
  string myname, text;

  str=(extern_call()?this_player()->_unparsed_args():str);
  if( !str || sscanf( str, "%s nach %s", myname, text ) != 2 ) {
    _notify_fail( "WEN willst Du nach WAS fragen?\n" );
    return 0;
  }

  if( !id( lower_case(myname) )
    || QueryProp(P_INVIS) ) {
    _notify_fail( "So jemanden findest Du hier nicht.\n" );
    return 0;
  }
  say( capitalize(this_player()->name(WER))+" fragt " +
    name(WEN,2)+" nach "+capitalize(text)+".\n",
      this_player() );

  text = lower_case(text);
  GiveEP(EP_INFO, text);

  return do_frage( text );
}

static mixed _query_pre_info()
{
  return QueryProp(P_PRE_INFO);
}


static string infoDefaultReplace(string pstring, object pl)
{
  pstring=" "+pstring;
  if (strstr(pstring,"@WER",0) >-1 )
    pstring= regreplace(pstring,"@WER",pl->name(WER,1),1);
  if (strstr(pstring,"@WESSEN",0) >-1 )
    pstring= regreplace(pstring,"@WESSEN",pl->name(WESSEN,1),1);
  if (strstr(pstring,"@WEM",0) >-1 )
    pstring= regreplace(pstring,"@WEM",pl->name(WEM,1),1);
  if (strstr(pstring,"@WEN",0) >-1 )
    pstring= regreplace(pstring,"@WEN",pl->name(WEN,1),1);
  if (strstr(pstring,"@CAP_WER",0) >-1 )
    pstring= regreplace(pstring,"@CAP_WER",pl->Name(WER,1),1);
  if (strstr(pstring,"@CAP_WESSEN",0) >-1 )
    pstring= regreplace(pstring,"@CAP_WESSEN",pl->Name(WESSEN,1),1);
  if (strstr(pstring,"@CAP_WEM",0) >-1 )
    pstring= regreplace(pstring,"@CAP_WEM",pl->Name(WEM,1),1);
  if (strstr(pstring,"@CAP_WEN",0) >-1 )
    pstring= regreplace(pstring,"@CAP_WEN",pl->Name(WEN,1),1);

  return pstring[1..];
}

static mixed *GetInfoArr(string str)
{  
   return ({ infos[str, 0], infos[str, 1], infos[str,2], infos[str, 3] });
}

public int do_frage(string text)
{
  string indent,answer;
  mixed silent, preinfo, noanswer;
  mixed *info;

  if (stringp(preinfo = _query_pre_info()))
   {
     tell_object(this_player(),
                 break_string(capitalize(name(WER,2))+" "+preinfo,78));
     say(break_string(capitalize(name(WER,2))+" ist nicht gewillt, "
                     +capitalize(this_player()->name(WEM,2))+" zu antworten.\n",78),
        ({ this_player() }) );
     return 1;
   }
  else
   if (intp(preinfo) && preinfo > 0) return 1;

  info=GetInfoArr(text);
  if (!info[0])
  {
    if( this_interactive() && QueryProp(P_LOG_INFO) )
      smart_npc_log(text);
    text = DEFAULT_INFO;
    info=GetInfoArr(text);
  }

  if (closurep(info[0]) ) {
    answer=funcall(info[0]);
    if( !answer || answer=="") return 1;
  } else {
    answer=process_string(info[0]);
  }

  if (closurep(info[3]) )
   {
    noanswer=funcall(info[3]);
    if ( intp(noanswer) && noanswer > 0)
     {
       text = DEFAULT_NOINFO;
       if (closurep(info[0]) ) {
         answer=funcall(info[0]);
         if( !answer || answer=="") return 1;
       } else {
         answer=process_string(info[0]);
       }
     }
    else if ( stringp(noanswer) )
      answer = noanswer;
   }

  silent=info[2];

  // Replacements gehen auch in der Antwort des NPC. Das gibt den Antworten 
  // eine persoenliche Note, und so teuer is das auch nicht :)
  answer = infoDefaultReplace(answer, this_player());
  
  if( indent=info[1] )
    if (stringp(silent) || (intp(silent) && silent > 0) )
     {  // Persoenliche Antwort mit indent
       tell_object(this_player(),
                   break_string( answer,78,capitalize(name(WER,2))+" "+indent,1) );
       if (stringp(silent))
        {
          silent=infoDefaultReplace(silent, this_player());
          say( capitalize(name(WER,2))+" "+silent, ({ this_player() }) );
        }
     }
    else // "normale Antwort" mit Indent
      say( break_string( answer,78,
             capitalize(name(WER,2))+" "+indent,1) );
  else
    if (stringp(silent) || (intp(silent) && silent > 0) )
     {  // Persoenliche Antwort ohne indent
       tell_object(this_player(),capitalize(name(WER,2))+" "+answer );
       if (stringp(silent))
        {
          silent=infoDefaultReplace(silent, this_player());
          say( capitalize(name(WER,2))+" "+silent, ({ this_player() }) );
        }
     }
    else // "normale Antwort" ohne Indent
      say( capitalize(name(WER,2))+" "+answer );

  return 1;
}

/*
 *---------------------------------------------------------------------------
 * Setzen von Infos
 *---------------------------------------------------------------------------
 */

public varargs void AddInfo(mixed key, mixed info, string indent, 
                            mixed silent, mixed casebased ) {

  if (stringp(casebased))
    casebased=symbol_function(casebased,this_object());

  if( pointerp( key ) ) {
    int i;
    for ( i=sizeof( key )-1; i>=0; i-- )
      infos += ([ key[i]: info; indent; silent; casebased ]);
  }
  else
    infos += ([ key: info; indent; silent; casebased ]);
}

public varargs void AddSpecialInfo(mixed keys, string functionname, 
                      string indent, mixed silent, mixed casebased )
{
  int i;
  closure cl;

  if(!(cl=symbol_function(functionname,this_object()))) return;
  return AddInfo(keys,cl,indent,silent,casebased);
}


public void RemoveInfo( string key )
{
  efun::m_delete(infos,key);
}

static varargs void _set_default_info( mixed info )
{
  if (pointerp(info))
    apply(#'AddInfo/*'*/,DEFAULT_INFO,info);
  else
    AddInfo(DEFAULT_INFO,info);
}

public varargs mixed GetInfo(string str)
{
  if (!str) return deep_copy(infos);
  return infos[str];
}


static mapping _query_npc_info()
{
    return deep_copy(infos);
}


static mapping _set_npc_info( mapping map_ldfied )
{
    if ( !mappingp(map_ldfied) )
        return 0;

    return infos = map_ldfied;
}

