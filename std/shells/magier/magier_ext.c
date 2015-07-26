// $Id: magier_ext.c 7499 2010-03-09 22:08:23Z Zesstra $
#pragma strict_types
#pragma save_types
//#pragma range_check
#pragma no_clone
#pragma pedantic

#include <wizlevels.h>
#include <logging.h>
#include <magier.h>
#include <snooping.h>
#define NEED_PROTOTYPES
#include <player/telnetneg.h>
#undef NEED_PROTOTYPES
#include <properties.h>
#include <files.h>

inherit "/std/shells/magier/parsing";
inherit "/std/shells/magier/upd";
inherit "/std/shells/magier/fileview";
inherit "/std/shells/magier/objects";
inherit "/std/shells/magier/fileedit";
//inherit "/std/shells/magier/todo";
inherit "/std/shells/magier/players";
inherit "/std/shells/magier/admin";
inherit "/std/shells/magier/moving";
inherit "/std/shells/magier/comm";



//                              #######
//############################### SET #################################
//                              #######


//
// _set(): Shell-Befehl 'set'
// args:   Kommandozeilenargumente
//

static int _set(mixed args)
{
  int pos;
  string var;
  mixed val;
  mapping vars;

  if(!args=_unparsed_args())
  {
    if(!sizeof(vars=QueryProp(P_VARIABLES))) 
      printf("Du hast noch keine Variablen definiert!\n");
    else
    {
      printf("Du hast die folgenden Variablen definiert:\n");
      walk_mapping(vars,((: printf(" %-20s = %s\n",$1,
                                 pointerp($2)?implode($2,":"):$2) :)));

    }
    return 1;
  }
  pos = member(args,' ');
  if(pos == -1)
    if(strlen(args))
      {
        efun::m_delete(QueryProp(P_VARIABLES), args);
        printf("Variable %s wurde geloescht.\n",args);
        return 1;
      }
    else return USAGE("set <variable> <wert>");
  var = args[0..pos-1];
  val = (member(args[pos+1..],':')>-1?
         explode(args[pos+1..], ":") :
         args[pos+1..]);
  vars=QueryProp(P_VARIABLES);
  vars+= ([var : val]);
  printf("Variable gesetzt: %s = %s\n",var,args[pos+1..]);
  return 1;
}

//                            ############
//############################# CD + PWD ###################################
//                            ############

static int _pwd()
{
  printf("Aktuelles Verzeichnis: %s\n",QueryProp(P_CURRENTDIR));
  return 1;
}

static int _cd2(string cmdline)
{
  string dest,tmp;
  int flags;
  mixed *args;
  
  args=parseargs(cmdline,&flags,CD_OPTS,1);
  if (flags==-1) return 0;
  if (sizeof(args)>1)
    return notify_fail("Man kann sich (leider) nur in einem "
                       "Verzeichnis gleichzeitig befinden.\n"),0;
  if (!sizeof(args))
  {
    if (IS_WIZARD(this_object()))
      dest="/players/"+getuid(this_object());
    else
      dest="/doc";
  }
  else
  {
    dest=args[0];
    if (dest!="/")
    {
      args=file_list(({dest}),MODE_CD,0,"/");
      args = filter(args, function int (mixed arr)
          { return arr[FILESIZE] == FSIZE_DIR; } );

      if (!sizeof(args))
        return notify_fail("cd: "+dest+
                         ": Kein solches Verzeichnis gefunden.\n"),0;
      if (sizeof(args)>1)
        return notify_fail("cd: "+dest+": Maske ist nicht eindeutig.\n"),0;
      dest=args[0][FULLNAME];
    }
    if (!IS_WIZARD(this_object())&&dest[0..3]!="/doc") dest="/doc";
  }
  printf("Aktuelles Verzeichnis ist jetzt: %s\n",SetProp(P_CURRENTDIR,dest));
  tmp="";
  if (((flags&CD_L)||
       ((!m_contains(&tmp,QueryProp(P_VARIABLES),"CD_SHORT")||
         tmp!="1")&&(!(flags&CD_S)))))
    tell_object(this_object(),
                read_file(dest+(dest[<1]=='/'?".readme":"/.readme"))||"");
  return 1;
}

static int _cd(string cmdline)
{
  return _cd2(_unparsed_args());
}

static string _set_currentdir(string path)
{
    string promptstring;
    mapping TN;

    if( !(promptstring = QueryProp(P_PROMPT)) || promptstring == "" )
        promptstring = "> ";
    else
        promptstring = implode( explode(promptstring, "\\w"), path );

    TN=query_telnet_neg();

    if ( mappingp(TN) && mappingp(TN["received"])
         && (TN["received"][TELOPT_EOR,1] == DO) )
        set_prompt(lambda(({}),
      ({ #'efun::binary_message,
         ({ #'+,
            ({ #'to_array,
               ({ #'implode,
                    ({ #'explode,promptstring,"\\t" }),
                        ({ #'[..],({ #'ctime }),11, 15 })
               })
            }),'({ IAC, EOR })
         })
      })), this_object());
  else
    set_prompt(
               lambda(({}),
                       ({ #'implode,
                               ({ #'explode,promptstring, "\\t" }),
                               ({ #'[..],({ #'ctime }),11, 15 })
                       })), this_object());

  return Set(P_CURRENTDIR, path);
}

private string _query_currentdir()
{
  string c;
  if(!c=Query(P_CURRENTDIR))
    return Set(P_CURRENTDIR, "/players/"+getuid(this_object()));
  return c;
}

//                           ##########
//############################ PROMPT #################################
//                           ##########

//
// renew_prompt: Prompt erneuern
// Noetig, damit telnetneg.c den Prompt auch von Ausserhalb setzen kann
//

void renew_prompt()
{
    _set_currentdir(_query_currentdir());
}


//
// _prompt_subst: Mudnamen, Usernamen, Zeilenvorschub in Eingabeauf-
//                forderung einbauen
// str:           Uebergebener Kommandozeilenabschnitt
//

private string _prompt_subst(string str)
{
  switch(str)
  {
    case "\\h": return strlen(MUDNAME)?MUDNAME:"Mud ohne Namen";
    case "\\u": return capitalize(getuid(this_object()));
    case "\\n": return "\n";
  }
  return str;
}


//
// _prompt: Eingabeaufforderung setzen
// arg:     Kommandozeile
//

static int _prompt(string args)
{
  string *pargs;

  args=(extern_call()?_unparsed_args():args);
  if (!strlen(args)) args="> ";
  if (args[0]=='"') args=args[1..<2];     //");
  if(!sizeof(pargs = regexplode(args, "\\\\[huwn]")))
    return USAGE("prompt \"<Eingabeaufforderungsdefinition>\"");
  pargs=map(pargs,#'_prompt_subst);
  SetProp(P_PROMPT,implode(pargs,""));
  _set_currentdir(_query_currentdir());  // Pfad einfuegen
  return 1;
}

//                           ##############
//############################ SHOWPRESAY ###############################
//                           ##############

static int _showpresay(string cmdline)
{
  int presay;
  presay=QueryProp(P_CAN_FLAGS)&CAN_PRESAY;
  cmdline=_unparsed_args(0);
  if (!strlen(cmdline))
  {
    printf("Dein Presay wird im Moment %sangezeigt.\n"
           "%sschalten mit \'showpresay %s\'.\n",
           presay?"":"nicht ",presay?"Aus":"Ein",
           presay?"aus":"ein");
    return 1;
  }
  if (cmdline=="ein"||cmdline=="an")
  {
    printf("Dein Presay wird jetzt angezeigt.\n");
    SetProp(P_CAN_FLAGS,QueryProp(P_CAN_FLAGS)|CAN_PRESAY);
    return 1;
  }
  if (cmdline=="aus")
  {
    printf("Dein Presay wird jetzt nicht mehr angezeigt.\n");
    SetProp(P_CAN_FLAGS,QueryProp(P_CAN_FLAGS)&~CAN_PRESAY);
    return 1;
  }
  return USAGE("showpresay [ein|an|aus]");
}

//                              ########
//############################### EXEC ###############################
//                              ########

static int _exec(string filename)
{
  object ob;
  if (!IS_LORD(this_object())) return 0;
  if (this_player()!=this_interactive()) return 0;
  if (this_player()!=this_object()) return 0;
  if (!(filename=_unparsed_args())) return USAGE("exec <objektname>");
  filename=(string)"secure/master"->_get_path(filename,getuid());
  if (file_size(filename)<0&&(!to_filename(filename+".c"))||
      file_size(to_filename(filename+".c"))<0)
  {
    printf("exec: %s: Datei nicht vorhanden oder ein Verzeichnis.\n",filename);
    return 1;
  }
  if (catch(call_other(filename,"????"))) 
  {
    printf("exec: Fehler beim Laden von %s.\n",filename);
    return 1;
  }
  if (!(ob=clone_object(filename))) return 0;
  if (getuid(ob) != getuid(this_object()))
  {
    printf("exec: UID-Konflikt: %s <-> %s.\n",getuid(ob),
           getuid(this_object()));
    destruct(ob);
    return 1;
  }
  log_file(SHELLLOG("EXEC"),
         sprintf("%12.12s %40.40s %25.25s\n",
                 capitalize(getuid(this_object())),filename,dtime(time())[4..]));
  disable_commands();
  exec(ob,this_object());
  if (interactive(this_object())||!interactive(ob))
  {
    enable_commands();
    printf("Fehler bei EXEC: Uebertragung der Shell "
           "nicht moeglich.\n");
    return 1;
  }
  ob->start_player(capitalize(getuid(this_object())));
  remove();
  return 1;
}
//                            ############
//############################# SNOOPING ###############################
//                            ############

private static string snoop_allowed;
private static object snoopee;

nomask int QueryAllowSnoop(object who)
{
  return (getuid(who) == snoop_allowed);
}

static int _sallow(string str)
{
  object ob;

  if (!strlen(str))
  {
    if (!snoop_allowed) return USAGE("sallow [<name>]\n");
    str=snoop_allowed;
    snoop_allowed=0;
    printf("Du entziehst %s die Erlaubnis zum Snoopen.\n",capitalize(str));
    ob=query_snoop(this_player());
    if (!ob||(getuid(ob)!=str)) return 1;
    tell_object(ob,break_string(sprintf("%s entzieht Dir die "
            "Erlaubnis zum snoopen und zwingt Dich so dazu, mit "
            "dem Snoopen aufzuhoeren.\n",capitalize(getuid()))));
    snoop(ob);
    return 1;
  }
  if (snoop_allowed) _sallow(""); // Erstmal abschalten
  ob=find_player(lower_case(str));
  str=capitalize(str);
  if (!ob) return
      printf("sallow: Spieler %s konnte nicht gefunden werden.\n",str),1;
  if (query_wiz_grp(ob)>query_wiz_grp(this_player()))
    return printf("sallow: %s hat einen hoeheren Rang als Du und kann "
                  "Dich daher ohnehin snoopen.\n",str),1;
  snoop_allowed=getuid(ob);
  return printf("sallow: Du erlaubst %s, Dich zu snoopen.\n",str),1;
}

static int _snoop(string cmdline)
{
  object ob;
  int flags;
  string *args;

  if (!strlen(cmdline=_unparsed_args())||
      sizeof(args=parseargs(cmdline,&flags,SNOOP_OPTS,0))!=1||flags==-1)
  {
    if (!snoop(this_object())) return USAGE("snoop [-" SNOOP_OPTS
                                            "] [<spieler>]\n");
    if (snoopee)
      printf("Du snoopst %s jetzt nicht mehr.\n",capitalize(getuid(snoopee)));
    else
    {
      printf("Du hoerst auf zu snoopen.\n");
    // evtl. irgendetwas loggen ... sollte eigentlich nicht passieren.
    }
    snoopee=(object)0;
    return 1;
  }
  SetProp(P_SNOOPFLAGS,flags); // FUNKTIONIERT NUR, WENN magier.h und
                               // snooping.h abgeglichen sind
  if (!(ob = find_player(args[0])))
    return printf("snoop: Konnte keinen Spieler '%s' finden.\n",
                  capitalize(args[0])),1;
  if (!snoop(this_player(),ob))
    return printf("snoop: Der Versuch, %s zu snoopen, ist "
                  "fehlgeschlagen.\n%s",capitalize(args[0]),
                  ((~flags)&SNOOP_F)?"Eventuell funktioniert es mit dem "
                  "Flag '-f'.\n":""),1;
  snoopee=ob;
  return printf("Du snoopst jetzt %s.\n",capitalize(getuid(ob))),1;
}


//                           ##########
//############################ MSCHAU #################################
//                           ##########

static int _mschau(string str)
{
  if (this_interactive()!=this_object()) return 0;
  if (str=="ja"||str=="an"||str=="ein")
  {
    if ( QueryProp(P_WANTS_TO_LEARN) )
      printf("Du hast den Magier-Modus doch schon eingeschaltet!\n");
    else
    {
      printf("Du hast jetzt den Magier-Modus eingeschaltet.\n" );
      SetProp(P_WANTS_TO_LEARN, 1);
    }
    return 1;
  }
  if (str=="nein"||str=="aus")
  {
    if (QueryProp(P_WANTS_TO_LEARN))
    {
      printf( "Du schaltest den Magier-Modus aus.\n");
      set_heart_beat(1);
      SetProp(P_WANTS_TO_LEARN,0);
    }
    else
      printf("Du hast den Magier-Modus doch schon ausgeschaltet.\n");
    return 1;
  }
  return USAGE("mschau [an|ein|ja|aus|nein]\n");
}

//                           ###########
//############################ PROTECT ################################
//                           ###########

static int _protect(string str)
{
 
  if (this_object()!=this_interactive()) return 0;
  if (!strlen(str))
    return USAGE("protect <propertyname>\n");
  Set(str, PROTECTED, F_MODE);
  return printf("Deine Property %s ist jetzt %sgeschuetzt.\n",
                str,(Query(str,F_MODE) & PROTECTED?"":"nicht mehr ")),1;
}


//                         ###############
//########################## VIS + INVIS ##############################
//                         ###############

static int _hbstop;
static int _age;

static int _invis(string inform)
{

  if (QueryProp(P_INVIS))
    return printf("Du bist doch schon unsichtbar!\n"),1;
  tell_room(environment(),sprintf("%s %s.\n",capitalize(Name()),
            QueryProp(P_MMSGOUT)),({ this_object() }));
  if (inform=="e") "/secure/merlin"->notify_player_leave(getuid());
  SetProp(P_INVIS, QueryProp(P_AGE));
  printf("Du bist jetzt unsichtbar.\n");
  return 1;
}

static int _vis(string inform)
{
  if (!QueryProp(P_INVIS))
    return printf("Du bist doch schon sichtbar.\n"),1;
   tell_room(environment(),sprintf("%s %s.\n",capitalize(Name()),
            QueryProp(P_MMSGIN)),({ this_object() }));
  SetProp(P_INVIS, 0);
  if (inform=="e") "/secure/merlin"->notify_player_enter(getuid());
  printf("Du bist jetzt sichtbar.\n");
  return 1;
}

//                          ############
//########################### LOCALCMD ###############################
//                          ############


static int _localcmd()
{
  int size,more;
  string *verbs,result;

  // Per Umweg ueber Mapping doppelte Werte einstampfen
  size=sizeof(verbs=m_indices(mkmapping(query_actions(this_object()))));
  verbs-=({""});
  more=(size>sizeof(verbs));
  if (!sizeof(verbs))
  {
    if (more)
      printf("Die vorhandenen Befehle koennen nicht angezeigt werden, "
             "da sie per 'add_action(\"funktion\",\"\",1)' definiert "
             "wurden.\n");
    else
      printf("Dir stehen keine Befehle zur Verfuegung ... eigenartig!\n");
    return 1;
  }
  verbs=sort_array(verbs,#'>);
  result=break_string(sprintf("\'%s\'",implode(verbs,"' '")),78);
  return printf("\n%'-'78.78s\nDie fuer Dich aktuell verfuegbaren "
                "Befehle sind:\n\n%s\n%s%'-'78.78s\n","",result,
                more?"Zudem wurden Befehle per 'add_action("
                "\"funktion\",\"\",1)' definiert.\n":"",""),1;
}

//                           ###########
//############################ TRAENKE ################################
//                           ###########

static int _traenke(string str)
{
  if(SetProp(P_TRANK_FINDEN, !QueryProp(P_TRANK_FINDEN)))
    write("Du kannst jetzt Zaubertraenke finden.\n");
  else
    write("Du findest jetzt keine Zaubertraenke mehr.\n");
  return 1;
}

static int norm_rusage() {
	mixed* r;
  r = rusage();
  return r[0]/100 + r[1]/100;
}

//                           #############
//############################ ASYNCHRON #################################
//                           #############

//
// asynchron(): Asynchrone Abarbeitung eines Arrays
// array:     Zu bearbeitendes Array
// cmd:       Auszufuehrende Closure
// data:      Extraargumente
// flags:     Zusatzdaten (normalerweise Flags)
// c:         schon asynchron?
//

static varargs void asynchron(mixed* array, closure cmd, mixed data, mixed flags,int c)
{
  int i, j, k;
  mixed ret_val;
  string cmd_string;

  k = norm_rusage()+5;
  j = sizeof(array);
  i=0;
  
  switch (cmd_string=explode(sprintf("%O",cmd),"->")[1])
  {
    case "_make":     cmd_string=(data&UPD_LOAD?"load":"upd");break;
    case "cp_file":   cmd_string =(data?"mv":"cp"); break;
    case "grep_file": cmd_string = "grep"; break;
    case "rm_file":   cmd_string = "rm"; break;
    default: break;
  }
  
  while(i < j && get_eval_cost() > 200000 && norm_rusage()<k)
    // Sowohl Too-Long-Eval als auch Lag verhindern
  {
    ret_val=apply(cmd,array[i],data, flags);
    if (closurep(ret_val))
    {
      if(c) tell_object(this_object(),
               sprintf("%s: Verlasse Asynchronen Modus.\n",cmd_string));
      funcall(ret_val,array[i..],data,flags);
      return;
    }
    if (ret_val==RET_DELETE)
    {
      array[i+array[i][SUBDIRSIZE]+1..i+array[i][SUBDIRSIZE]]=
        ({ (array[i][0..5]+({0})) });
    }
    if (ret_val==RET_JUMP) i+=array[i][SUBDIRSIZE];
    i++;
  }
  if(sizeof(array = array[i..])) 
  {  
    if(!c) tell_object(this_object(),
              sprintf("%s: Asynchroner Modus aktiviert!\n", cmd_string));
    call_out(#'asynchron, 1, array, cmd, data,flags, 1);
  }
  else
  {
    if(c) tell_object(this_object(),
                      sprintf("%s: abgeschlossen.\n",cmd_string));
      else if (query_verb()) tell_object(this_object(),
            sprintf("%s: abgeschlossen.\n",query_verb()));
  }
  return;                                                    
}

//                         ###################
//########################## INITIALISIERUNG #############################
//                         ###################

//
// _query_localcmds: Welche Kommandos werden in dieser Datei implementiert?
//

static mixed *_query_localcmds()
{
  return ({
    ({"set","_set",0,LEARNER_LVL}),
    ({"pwd","_pwd",0,LEARNER_LVL}),
    ({"cd","_cd",0,LEARNER_LVL}),
    ({"prompt","_prompt",0,LEARNER_LVL}),
    ({"showpresay","_showpresay",0,LEARNER_LVL}),
    ({"exec","_exec",0,ARCH_LVL}),
    ({"sallow","_sallow",0,LEARNER_LVL}),
    ({"snoop","_snoop",0,WIZARD_LVL}),
    ({"mschau","_mschau",0,LEARNER_LVL}),
    ({"protect","_protect",0,WIZARD_LVL}),
    ({"invis","_invis",0,LEARNER_LVL}),
    ({"vis","_vis",0,LEARNER_LVL}),
    ({"localcmd","_localcmd",0,LEARNER_LVL}),
    ({"traenke","_traenke",0,DOMAINMEMBER_LVL})})
    +fileview::_query_localcmds()
    +upd::_query_localcmds()
    +objects::_query_localcmds()
    +fileedit::_query_localcmds()
//    +todo::_query_localcmds()
    +players::_query_localcmds()
    +admin::_query_localcmds()
    +moving::_query_localcmds()
    +comm::_query_localcmds();
}

//
// initialize: Initialisierung der Shell
//

static void initialize()
{
  Set(P_PROMPT, SAVE, F_MODE_AS);
  Set(P_VARIABLES,SAVE,F_MODE_AS);
  Set("filesys",SAVE,F_MODE_AD);   // P_FILESYS ist obsolet
  Set("short_cwd",SAVE,F_MODE_AD); // P_SHORT_CWD auch
  if(!mappingp(QueryProp(P_VARIABLES)))
    SetProp(P_VARIABLES, ([]));
  _prompt(QueryProp(P_PROMPT));
//  todo::initialize();
  _cd2("");
  return;
}

static void reconnect() { _cd(QueryProp(P_CURRENTDIR)); }
