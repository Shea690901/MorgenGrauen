// MorgenGrauen MUDlib
//
// master.c -- master object
//
// $Id: master.c 7486 2010-02-21 19:14:29Z Zesstra $
#pragma strict_types
#pragma no_clone
#pragma no_shadow
#pragma no_inherit
#pragma verbose_errors
#pragma combine_strings
#pragma pedantic
#pragma range_check
#pragma warn_deprecated

#include "/sys/files.h"

inherit "/secure/master/misc";
inherit "/secure/master/userinfo";
inherit "/secure/master/network";
inherit "/secure/master/domain";
inherit "/secure/master/guild";
inherit "/secure/master/file_access";
inherit "/secure/master/players_deny";

#include "/secure/master.h"

//for limited, set_limit(), etc. Abs. Pfad noetig, da noch kein Include-Hook
#include "/sys/rtlimits.h"
#include "/sys/debug_info.h"
#include "/sys/debug_message.h"

// Module des Master einfuegen. Per #include, damits geringfuegig schneller.
// Da die Module ja nirgendwo sonst geerbt werden koennten, ist dies
// ausnahmsweise OK. ;)
#include "/secure/master/destruct.c"
#include "/secure/master/autoinclude.c"

// Fuer Logfile(-Rotation)
#define RESETINT   3600                      // jede Stunde
#define LOGROTATE  (24*2)                    // alle 48h
#define LOGFILEAGE (RESETINT*LOGROTATE)      // alle 2 tage wechseln
#define LOGNUM     3                         // Anzahl der Logfiles

static int logrotate_counter; //READ_FILE alle LOGROTATE Resets rotieren


//                       #####################
//######################## Start des Masters ############################
//                       #####################

// Initialisierung des Masters
//arg==0: Mud startet gerade, arg==1: Master wurde reaktiviert,
//arg==2: Master reaktiviert (Variablen weg), arg==3: Master wurde
//neugeladen.
protected void inaugurate_master(int arg) {

  seteuid(ROOTID);
  userinfo::create();
  LoadPLDenylists();

  // Was soll vor jede Datei geschrieben werden?
  set_driver_hook(H_AUTO_INCLUDE, #'autoincludehook);

  //div. Listen einlesen
  ReloadBanishFile();
  ReloadDeputyFile();
  ReloadInsecureFile();
 
  // Bei Neustart wizinfo initialisieren
  if (!arg)
    set_extra_wizinfo(0, allocate(BACKBONE_WIZINFO_SIZE));
  
  //simul_efun.c nach inaugurate_master() starten und initialisieren, 
  //(so richtig seh ich den Sinn hier momentan nicht ein...
  //call_out("start_simul_efun",0);

  // Reset festsetzen (1h)
  set_next_reset(RESETINT);


  // Hooks setzen
  
  // Standardincludeverzeichnisse fuer #include <>
  set_driver_hook(H_INCLUDE_DIRS, ({"secure/","sys/"}) );
  
  //Nach dem Laden/Clonen create() im Objekt rufen
  set_driver_hook(H_CREATE_CLONE,       "create");
  set_driver_hook(H_CREATE_OB,          "create");
  set_driver_hook(H_CREATE_SUPER,       "create_super");

  // Bei Reset reset() im Objekt aufrufen
  set_driver_hook(H_RESET,              "reset");
  
  // Zum Aufraeumen clean_up() im Objekt aufrufen  
  set_driver_hook(H_CLEAN_UP,           "clean_up");

  // Jede Eingabe wird ueber modify_command gelenkt
  set_driver_hook(H_MODIFY_COMMAND,       "modify_command");
  set_driver_hook(H_MODIFY_COMMAND_FNAME, "modify_command");

  // Standard-Fehlermeldung
  //set_driver_hook(H_NOTIFY_FAIL,        "Wie bitte?\n");
  set_driver_hook(H_NOTIFY_FAIL, function string (string cmd, object tp)
      {if (stringp(cmd=(string)tp->QueryProp(P_DEFAULT_NOTIFY_FAIL)))
         return(cmd);
       return("Wie bitte?\n");});

  // Was machen bei telnet_neg
  set_driver_hook(H_TELNET_NEG,"telnet_neg");
  //  set_driver_hook(H_TELNET_NEG,0);
 
  // EUID und UID muessen neue Objekte auch noch kriegen, Hooks dafuer setzen
  set_driver_hook(H_LOAD_UIDS, #'load_uid_hook);
  set_driver_hook(H_CLONE_UIDS, #'clone_uid_hook);

  // Nun der beruechtigte Move-Hook ...
  // (bewegt objekt und ruft alle notwendigen inits auf)
  // Hier wird H_MOVE_OBJECT0 benutzt, d.h. die lambda wird an das aktuelle
  // Objekt vor Ausfuehrung gebunden, nicht an 'item', was move_objet()
  // uebergeben wurde.
  set_driver_hook(H_MOVE_OBJECT0,
     unbound_lambda(({'item,'dest}),
       ({#',,                                      // item!=this_object?
         ({#'?,({#'!=,'item,({#'this_object})}),
           ({#'raise_error,                        // ->Fehler!
             "Illegal to move other object than this_object()\n"}) }),
         ({#'efun::set_environment,'item,'dest}),  // item nach dest bewegen
         ({#'?,
           ({#'living,'item}),                     // living(item)?
           ({#',,
             ({#'efun::set_this_player,'item}),    // set_this_player(item)
             ({#'call_other,'dest,"init"}),        // dest->init()
             ({#'?!, 
               ({#'&&,                             // !objectp(item)||
                 ({#'objectp, 'item}),             // env(item)!=dest?
                 ({#'==,({#'environment, 'item}),'dest})}),
               ({#'return})})})}),                 // -> fertig
         ({#'=,'others,({#'-,({#'all_inventory,'dest}),({#'({,'item})})}),
#ifdef EMACS_NERV 
         })   // Emacs kann ({#'({ nicht parsen    // others=all_inv(dest)-
#endif                                             //        ({item})
         ({#'filter,'others,lambda(({'ob,'item}),
            ({#'?,                                     
              ({#'&&,
                ({#'objectp,'item}),               // objectp(item)&&
                ({#'living,'ob}),                  // living(ob)&&
                ({#'==,({#'environment,'item}),({#'environment,'ob})})}),
              ({#',,                               // env(item)==env(ob)?
                ({#'efun::set_this_player, 'ob}),  // set_this_player(ob)
                ({#'call_other,'item, "init"})     // item->init()
              })})),'item}),
         ({#'?,
           ({#'living,'item}),                     // living(item)?
              ({#',,
                ({#'efun::set_this_player,'item}), // set_this_player(item)
                ({#'filter,'others,lambda(({'ob,'item}),
                    ({#'?,                          
                      ({#'&&,                 
                        ({#'objectp,'item}),       // objectp(item)&&
                        ({#'objectp,'ob}),         // objectp(ob)&&
                        ({#'==,({#'environment,'item}),({#'environment,'ob})})
                      }),                          // env(item)==env(ob)?
                      ({#'call_other,'ob,"init"})  // ob->init()
                    })),'item})})}),
         ({#'?,
           ({#'&&,
             ({#'objectp,'item}),                  // objectp(item)&&
             ({#'living,'dest}),                   // living(dest)&&
             ({#'==,({#'environment,'item}),'dest})// env(item)==dest?
           }),
           ({#',,
             ({#'efun::set_this_player,'dest}),    // set_this_player(dest)
             ({#'call_other,'item,"init"})})})})));// item->init()
  DEBUG("Master inaugurated");
  return;
}

// epilog() gibt die Dateien zurueck, die vorgeladen werden muessen
protected string *epilog(int eflag) {
  string *files, *domains;
  int i;

  seteuid(ROOTID);
  ReloadBanishFile();
  ReloadDeputyFile();
  ReloadInsecureFile();

  if (eflag) {
    write("-e angegeben -> Preloading unterdrueckt ...\n");
    return 0;
  }

  printf("Preloading gestartet: %s\n\n",ctime(time()));

  //Files fuers Preloading und Regionen holen
  files=explode_files("/std/preload_file") + explode_files("/d/preload_file");
  domains=get_domains() + ({"erzmagier"}); // EM haben auch ein Preload File

  for (i=sizeof(domains);i--;)
    files+=explode_files("/d/"+domains[i]+"/preload_file");
  
  write("\n");
 
  return files;
}

// preload() wird fuer jeder Datei im Preload einmal durchlaufen
protected void preload(string file) {
  string err;
  string name;
  int *res, zeit;

  // Kein richtiger Dateiname: fertig
  if(!file || !file[0] || file[0] == ';' || file_size(file+".c") < 0) return;

  // Kein Besitzer -> Meldung ausgeben, fertig
  if (!(name=creator_file(file)))
  {
    printf("Kein Besitzer gefunden fuer Datei %s.\n",file);
    return;
  }

  // Datei und Besitzer ausgeben
  printf("%-50s%-15s",file,name);

  // Euid kann nicht geaendert werden -> Meldung, fertig
  if (!seteuid(name))
  {
    printf("\nEuid konnte nicht auf %s gesetzt werden\n"+
             "Sie ist nun %s.\n",name,geteuid(TO));
    return;
  }

  // Dann mal laden .. dabei Zeit messen
  zeit = apply(#'+,rusage()[0..1]);

  // Waehrend des Preloads sind 1.5MTicks leider nicht soviel, insb. wenn
  // sowas wie der channeld jede menge Objekte laden muss, die wiederum erst
  // das ganze Geraffel aus /std/ laden. Daher hier einfach mal fuers Preload
  // die Grenze hoch.
  err = catch(limited(#'load_object,({5000000}),file));

  if (err != 0)
    printf("\nFehler beim Laden von %s:\n%s\n",file,err);
  else
  {
    zeit=apply(#'+,rusage()[0..1])-zeit;
    printf("(%2d.%:02d s)\n",zeit/1000,zeit%1000);
  }

  // Noch EUID zuruecksetzen
  seteuid(ROOTID);

  return;
}

//simul_efun.c laden oder die spare_simul_efun.c als Fallback
//es ist moeglich, hier ein Array von strings zurueckzugeben. Die
//nachfolgenden gelten als Backup-Objekte, falls eine sefun im Hauptobjekt
//nicht gefunden wird.
protected string *get_simul_efun() {
  string err;

  if (!(err=catch(SIMUL_EFUN_FILE->start_simul_efun())) )
    return ({SIMUL_EFUN_FILE});
  
  write("Failed to load simul efun " + SIMUL_EFUN_FILE + "\n");
  debug_message("Failed to load simul efun " + SIMUL_EFUN_FILE +
      " " + err, DMSG_STDOUT | DMSG_LOGFILE);

  if (!(err=catch(SPARE_SIMUL_EFUN_FILE->start_simul_efun())) )
    return ({SPARE_SIMUL_EFUN_FILE});
  
  write("Failed to load spare simul efun" + SPARE_SIMUL_EFUN_FILE + "\n");
  debug_message("Failed to load spare simul efun " + SPARE_SIMUL_EFUN_FILE +
      " " + err, DMSG_STDOUT | DMSG_LOGFILE);

 
  efun::shutdown();
  
  return 0;
}

// Preload manuell wiederholen; Keine GD-Funktion
void redo_preload()
{
  string *to_preload;
  int i,j;

  to_preload=epilog(0);
  j=sizeof(to_preload);
  for (i=0;i<j;i++) 
      catch(preload(to_preload[i]));
  return;
}

//                         ##################
//########################## Standard-Lfuns #############################
//                         ##################

// Keine GD-Funktion, aber sinnvoll.
public varargs int remove(int silent)
{
  write("Der Master will aber nicht zerstoert werden!\n");
  return 0;
}

// Einige repetitiven Taetigkeiten kann der Reset erledigen
protected void reset()
{
  int i, *date;
  mixed *wl;

  //Darf nicht von Hand aufgerufen werden!
  if (TI||TP) return;
  
  // Naechsten Reset setzen
  set_next_reset(RESETINT);

  // Userinfo aufraeumen
  _cleanup_uinfo();

  // Projekt-Cache loeschen
  _cleanup_projects();

  // Wenn Zeit abgelaufen, dann READ_FILE-Log rotieren
  if (!logrotate_counter--)
  {
    i=time()/LOGFILEAGE;
    date=get_dir(sprintf("%s.%d", READ_FILE,i%LOGNUM), GETDIR_DATES);
    if (pointerp(date)&&sizeof(date)&&
        (date[0]/LOGFILEAGE)==i) return;
    if (file_size(READ_FILE)>0)
      rename(READ_FILE, sprintf("%s.%d", READ_FILE,i%LOGNUM));
    logrotate_counter=LOGROTATE;
  }
  // einmal am Tag die UIDAliase resetten. Hierzu wird der logrotate_counter
  // missbraucht.
  if (!(logrotate_counter%24)) {
    ResetUIDAliase();
  }

  // Wizlist altert
  wl = wizlist_info();
  for (i=sizeof(wl); i--; )
    set_extra_wizinfo(wl[i][WL_NAME], wl[i][WL_EXTRA] * 99 / 100);

  // Dann noch Mails bearbeiten
  mails_last_hour=0;
  mailread();

  return;
}


//                         #################
//########################## ID-Management ##############################
//                         #################

// Magier-ID fuer Datei str (als Objekt oder als String) ermitteln
string creator_file(mixed str) {
  string *strs,tmp;
  int s;

  // full_path_array nach strs
  // TODO: was ist mit clones?
  if(objectp(str))
    strs=full_path_array(object_name(str), 0);
  else if(stringp(str))
    strs=full_path_array(str, 0);
  else return NOBODY;

  // absolute Pfade interessieren hier gerade nicht.
  strs -= ({""});

  s=sizeof(strs);
  if(s<2) return NOBODY;

  switch(strs[0]) {
    case DOMAINDIR:
      // Fuer Nicht-Magier bzw. "Pseudo-"Magier die Regionskennung
      if( s==2 || WIZLVLS[strs[2]] || !IS_LEARNER(strs[2]) )
        return strs[1];

      // in /d/erzmagier gibt es Magier-IDs
      if (strs[1]=="erzmagier") return strs[2];

      // Ansonsten d.region.magiername
      return sprintf("d.%s.%s", strs[1], strs[2]);
 
    case PROJECTDIR:
      // In p.service gibt es ids mit uid
      if (s>2 && strs[1]=="service") return "p.service."+strs[2];

      // Ansonsten nur p.projekt (p.service ist auch hier drin)
      return "p."+strs[1];

    case WIZARDDIR:
      if (s>2)
        return strs[1];
      if (s==2 && file_size(strs[0]+"/"+strs[1])==FSIZE_DIR)
        return strs[1];
      return NOBODY;

    case SECUREDIR:
      return ROOTID;

    case GUILDDIR:
    case SPELLBOOKDIR:
      tmp=strs[1];
      if (tmp[<2..<2]==".") tmp=tmp[0..<3];
      if ((s=member(tmp,'.'))>0) tmp=tmp[s+1..];
      return "GUILD."+tmp;

    case LIBROOMDIR:
      return ROOMID;

    case STDDIR:
    case LIBOBJDIR:
      return BACKBONEID;

    case DOCDIR:
      return DOCID;

    case MAILDIR:
      return MAILID;
    case NEWSDIR:
      return NEWSID;

    /* Fall-Through */
    default:
      return NOBODY;

 }
 return(NOBODY);  //should never be reached.
}

// UID und EUID an Objekt geben (oder eben nicht)
// Keine GD-Funktion, aber von Hooks aufgerufen
protected mixed give_uid_to_object(string datei,object po)
{
  string creator,pouid;

  // Parameter testen
  if (!stringp(datei)||!objectp(po))  return 1;

  // Datei muss Besitzer haben
  if (!(creator=creator_file(datei))) return 1;
  
  // Keine Objekte in /log und /open
  if (datei[0..2]=="log" || datei[0..3]=="open") return 1;

  // Hack, damit simul_efun geladen werden kann
  if (creator==ROOTID && po==TO) return ROOTID;

  // Keine euid, kein Laden ...
  if (!(pouid=geteuid(po))) return 1; // Disallow if no euid in po

  // Root generiert keine Root-Objekte ;-)
  // Nur UID setzen
  if (pouid==ROOTID)
    return ({creator,NOBODY}); // root does not create root objects!

  // EUID mitsetzen, wenn PO==creator oder creator==BACKBONEID
  if (creator==pouid || creator==BACKBONEID)
    return pouid;

  return ({creator,NOBODY});
}

// Die System-IDs muessen bekannt sein
string get_master_uid()          { return ROOTID;}
string get_bb_uid()              { return BACKBONEID; }
// TODO: Bei get_wiz_name koennte man - so moeglich - auf 'wirkliche'
//       Magiernamen gehen (Idee von mandragon)
string get_wiz_name(string file) { return creator_file(file);}

//                     ##########################
//###################### Sonstige GD-Funktionen #########################
//                     ##########################

// Spieler hat sich eingeloggt
protected object connect()
{
  string err;
  int errno;
  object ob,bp;

#if defined(SSLPORT) && __EFUN_DEFINED__(tls_available)
  if (query_mud_port() == SSLPORT) {
    // reject connection of TLS is not available
    if (!tls_available())
      return 0;
    // establish TLS
    errno=tls_init_connection();
    if (errno < 0) {
      // Fehler im Vebindungsaufbau
      printf("Can't establish a TLS/SSL encrypted connection: %s\n",
          tls_error(errno));
      return 0; // this will close the connection to the client.
    }
  }
#endif
  // if err is > 0, the TLS connection is still being established in the
  // background. We must not send any data to the client.
  if (errno == 0) {
    printf("HTTP/1.0 302 Found\n"
         "Location: http://mg.mud.de/\n\n"
         "NetCologne, Koeln, Germany. Local time: %s\n\n"
         MUDNAME" LDmud, NATIVE mode, driver version %s\n\n",
         strftime("%c"), __VERSION__);
  }

  // Blueprint im Environment? Das geht nun wirklich nicht ...
  if ((bp=find_object("/secure/login")) && environment(bp)) 
      catch(destruct(bp);publish);

  // Login-Shell clonen
  err = catch(ob = clone_object("secure/login"); publish);

  if (errno == 0 && err)
      write("Fehler beim Laden von /secure/login.c\n"+err+"\n");

  return ob;
}

// Was machen bei disconnect?
protected void disconnect(object who, string remaining) {
    who->NetDead(); return;
}

// Es gibt kein File 'filename'. VC aktivieren so vorhanden ...
protected object compile_object(string filename)
{
  object ret;
  string *str;
  string compiler;

  if (!strlen(filename)) return 0;
  str=efun::explode(filename,"/");

  if(sizeof(str)<2) return 0;
  compiler=implode(str[0..<2],"/")+"/virtual_compiler";
 
  if (file_size(compiler+".c")>0)
  {
    if(catch(
          ret=(object)call_other(compiler,"compile_object",str[<1]); publish))
      return 0;
  }
  else
      return(0);

  if ( objectp(ret) && efun::explode(creator_file(filename), ".")[<1] !=
       REAL_EUID(ret) ){
      funcall( symbol_function('log_file/*'*/), "ARCH/VC_MISSBRAUCH",
               sprintf( "%s: %s versucht per VC %s zu %s umzubenennen!\n",
                        funcall( symbol_function('dtime/*'*/), time() ),
                        (this_interactive() ? getuid(this_interactive()):
                         object_name(previous_object())), object_name(ret),
                         filename) );

      return 0;
  }
  return ret;
}

//                           ############
//############################ Shutdown #################################
//                           ############

// Spieler bei Shutdown entfernen
protected void remove_player(object victim)
{
  catch(victim->quit());
  if (victim) catch(victim->remove());
  if (victim) destruct(victim);
  return;
}

// Langsamer Shutdown 
protected void slow_shut_down(int minutes)
{
  //sollte nur vom GD gerufen werden. Oder allenfalls Master selbst
  filter(users(),#'tell_object,
  "Der Gamedriver ruft: Der Speicher wird knapp ! Bereitet euch auf das Ende vor !\n");
  "/obj/shut"->shut(minutes);
  return;
}


// In LD varargs void notify_shutdown(string crash_reason)
protected varargs void notify_shutdown (string crash_reason) {

  if (PO && PO != TO)
    return;
  if (crash_reason) {
      //Uhoh... Verdammter Crash. :-( Zumindest mal mitloggen,
      //was der Driver sagt...
      write_file(CRASH_LOG,sprintf(
            "\n%s: The driver crashed! Reason: %s\n",
            ctime(time()),crash_reason));
  }

  filter(users(), #'tell_object,
               "Game driver shouts: LDmud shutting down immediately.\n");
  save_wiz_file();
  return;
}

//                         ##################
//########################## Berechtigungen #############################
//                         ##################

// Darf Verbindung durch name von obfrom nach ob gelegt werden?

//int valid_exec(string name) {return 1;}

int valid_exec(string name, object ob, object obfrom)
{
  // Ungueltige Parameter oder Aufruf durch process_string -> Abbruch
  if (!objectp(ob) || !objectp(obfrom) 
      || !stringp(name) || !strlen(name)
      || funcall(symbol_function('process_call)) )
    return 0;

  // renew_player_object() darf ...
  if (name=="secure/master/misc.c") return 1;

  // Ansonsten darf sich nur die Shell selber aendern ...
  if (previous_object() != obfrom) return 0;

  // Die Login-Shell zu jedem Objekt ...
  if (name=="secure/login.c") return 1;

  // Magier per exec nur, wenn sie damit keine Fremde uid/euid bekommen
  if (this_interactive() == obfrom && getuid(obfrom) == getuid(ob) 
      && geteuid(obfrom) == geteuid(ob)) 
      return 1;

  // Sonst darf niemand was
  return 0;
}


// Darf me you snoopen?
int valid_snoop(object me, object you) { 
    return getuid(PO) == ROOTID; 
}

// Darf wiz wissen, ob er gesnoopt wird?
int valid_query_snoop(object wiz) {
    return getuid(PO) == ROOTID; 
}

// Darf ob seine EUID auf neweuid setzen?
int valid_seteuid(object ob, string neweuid)
{
  return (ob==this_object() ||
          getuid(ob) == ROOTID ||
          getuid(ob) == neweuid ||
          creator_file(object_name(ob)) == neweuid);
}

// Darf getraced werden?
int valid_trace(string what, mixed arg) { 
    return IS_ARCH(TI);
}


// valid_write() und
// valid_read() befinden sich in file_access.c


// Darf PO ob shadowen?
int query_allow_shadow(object ob)
{
  // ROOT darf nicht geshadowed werden
  if(getuid(ob) == ROOTID)
    return 0;
  
  // Access-Rights auch nicht
  if (efun::explode(object_name(ob),"#")[0][<14..]=="/access_rights")
    return 0;
  
  // Ansonsten query_prevent_shadow fragen ...
  return !(int)ob->query_prevent_shadow(PO);
}


/* privilege_violation is called when objects try to do illegal things,
 * or files being compiled request a privileged efun.
 *
 * return values:
 *   1: The caller/file is allowed to use the privilege.
 *   0: The caller was probably misleaded; try to fix the error.
 *  -1: A real privilege violation. Handle it as error.
 */
int privilege_violation(string op, mixed who, mixed arg1, mixed arg2)
{

  if (objectp(who) && 
      (who==this_object() || geteuid(who)==ROOTID))
    return 1; 

  switch(op)
  {
    case "call_out_info":
      return -1;
    case "send_udp":
      return 1;

    case "nomask simul_efun":
    case "get_extra_wizinfo":
      // benutzt von /secure/memory.c und /secure/simul_efun.c
    case "set_extra_wizinfo":
      // wird benutzt von simul_efun.c, welche aber als ROOT-Objekt implizit
      // erlaubt ist (s.o.)
       if (who==SIMUL_EFUN_FILE||
           who == SPARE_SIMUL_EFUN_FILE)
         return 1;
       return -1;

    case "rename_object":
      if (object_name(who)=="/secure/impfetch" ||
          BLUE_NAME(who)=="/secure/login")
        return 1;
      return(-1);

    case "limited":
      // Spielershells duerfen sich zusaetzliche Ticks fuer den Aufruf von
      // ::die() beschaffen, damit der Tod nicht wegen wenig Ticks buggt.
      if (strstr(load_name(who), "/std/shells/") == 0
          && get_type_info(arg1, 2) == who
          && get_type_info(arg1, 3) == "/std/living/life"
//          && get_type_info(arg1, 4) == "die"
          && arg2[LIMIT_EVAL] <= 10000000 ) {
          return 1;
      }
      //DEBUG(sprintf("%O, %O, %O", who, arg1, arg2));
      int *limits=query_limits();
      // LIMIT_EVAL darf verringert werden. Alle anderen Limits duerfen nicht
      // geaendert werden. Achja, LIMIT_EVAL darf nicht 0 (LIMIT_UNLIMITED)
      // sein. *g*
      // LIMIT_COST darf entweder 0 oder -100 sein.
      if (arg2[LIMIT_EVAL]>1000 && (arg2[LIMIT_EVAL] < get_eval_cost())-500
          && arg2[LIMIT_ARRAY] == limits[LIMIT_ARRAY]
          && arg2[LIMIT_MAPPING_KEYS] == limits[LIMIT_MAPPING_KEYS]
          && arg2[LIMIT_MAPPING_SIZE] == limits[LIMIT_MAPPING_SIZE]
          && arg2[LIMIT_BYTE] == limits[LIMIT_BYTE]
          && arg2[LIMIT_FILE] == limits[LIMIT_FILE]
          && arg2[LIMIT_CALLOUTS] == limits[LIMIT_CALLOUTS]
          && (arg2[LIMIT_COST] == 0 || arg2[LIMIT_COST] == -100) )
            return(1);
      //sonst verweigern.
      return(-1);

    case "attach_erq_demon":
    case "bind_lambda": 
    case "enable_telnet":
    case "execute_command":
    case "erq": 
    case "input_to":
    case "mysql":
    case "net_connect":
    case "pgsql":
    case "set_limits":
    case "set_extra_wizinfo_size":
    case "set_driver_hook":
    case "set_max_commands":
    case "set_this_object":
    case "shadow_add_action":
    case "symbol_variable":
    case "variable_list":
    case "wizlist_info":
    default:
      return(-1);
  }
  return -1;
}

//                       ####################
//######################## Fehlerbehandlung #############################
//                       ####################

// Behandlung von Fehler im Heart_Beat
protected int heart_beat_error( object culprit, string error, string program,
                     string current_object, int line, int caught)
{
  if (!objectp(culprit)) return 0;
  if (interactive(culprit))
  {
    tell_object(culprit,"Der GameDriver teilt Dir mit: "
                "Dein Herzschlag hat ausgesetzt!\n");
    if (IS_LEARNER(culprit))
      tell_object(culprit,
                  sprintf("Fehler: %sProgamm: %s, CurrObj: %s, Zeile: %d, "
                    "the error was %scaught at higher level.\n",
                    error,program,current_object,line,
                    caught ? "" : "not"));
  }

  if (query_once_interactive(culprit))
    call_out("restart_heart_beat", 5, culprit);
  else
    catch(culprit->make_immortal(); publish);
  return 0;
}


/**
 * \brief Error handling fuer Fehler beim Compilieren.
 *
 * log_error() wird vom GameDriver aufgerufen, wenn zur Compile-Zeit ein
 * Fehler auftrat. Die Fehlermeldung wird unter /log/error geloggt.
 * Falls this_interactive() ein Magier ist, bekommt er die Fehlermeldung
 * direkt angezeigt.
 * Sollte das Logfile 50kB ueberschreiten, wird es rotiert. Auf [Entwicklung]
 * wird dann eine Fehlermeldung abgesetzt.
 *
 * @param file Die Datei, in der der Fehler auftrat.
 * @param message Die Fehlermeldung.
 * @param warn Warnung (warn!=0) oder Fehler (warn==0)?
 * */

protected int log_error(string file, string message, int warn) {
  string lfile;
  string cr;
  mixed *lfile_size;

 //Fehlerdaten an den Errord zur Speicherung weitergeben, falls wir sowas
 //haben.
#ifdef ERRORD
    catch(limited(#'call_other,({200000}),ERRORD,"LogCompileProblem",
          file,message,warn); publish);
#endif // ERRORD

 // Logfile bestimmen
  cr=creator_file(file);
  if (!cr)                     lfile="NOBODY.err";
  else if (cr==ROOTID)         lfile="ROOT";
  else if (member(cr,' ')!=-1) lfile="STD";
  else                         lfile=cr;
  //je nach Warnung oder Fehler Verzeichnis und Suffix bestimmen
  if (warn) {
      lfile="/log/warning/" + lfile + ".warn";
  }
  else {
      lfile="/log/error/" + lfile + ".err";
  }

  // Bei Bedarf Rotieren des Logfiles
  if ( sizeof(lfile_size = get_dir(lfile,2)) &&
           lfile_size[0] >= MAX_ERRLOG_SIZE )
  {
    catch(rename(lfile, lfile + ".old"); publish); /* No panic if failure */
    DEBUG3_MSG("Logfile rotiert: "+lfile+".");
  }

  write_file(lfile,message);

  // Magier bekommen die Fehlermeldung angezeigt
  if (IS_LEARNER(TI)) write(message);
  return 1;
}

/* Gegenmassnahme gegen 'too long eval' im Handler vom runtime error:
   Aufsplitten des Handlers und Nutzung von limited() */
//keine GD-Funktion
private void handle_runtime_error(string err, string prg, string curobj, 
    int line, mixed culprit, int caught, object po, string hashkey) {
  string code;
  string debug_txt;
  object ob, titp; 

  //DEBUG(sprintf("Callerstack: (handle_runtime_error()): %O\n",
  //        caller_stack(1)));
  // Fehlermeldung bauen
  if (!stringp(err))    err="<NULL>";
  if (!stringp(prg))    prg="<NULL>";
  if (!stringp(curobj)) curobj="<NULL>";
  debug_txt=sprintf("Fehler: %O, Objekt: %s, Programm: %O, Zeile %O (%s), "
                    "ID: %s",
                    err[0..<2], curobj, (prg[0]!='<'?"/":"")+prg, line,
                    (TI?capitalize(getuid(TI)):"<Unbekannt>"),
                    hashkey||"");
  
  titp = TI || TP;
  // Fehlermeldung an Kanaele schicken
  if (titp && (IS_LEARNER(titp) || (titp->QueryProp(P_TESTPLAYER))))
    DEBUG2_MSG(debug_txt,po); // -entw
  else
    DEBUG_MSG(debug_txt,po);  // -Debug
  if (!titp) return;

  // Fehlermeldung an Benutzer
  if (IS_LEARNER(titp))
    tell_object(titp,
                sprintf("%'-'78.78s\nfile: %s line: %d object: %s\n" +
                        "%serror: %s%'-'78.78s\n","",prg,line,curobj,
                        (prg&&(code=read_file("/"+prg,line,1))?
                         "\n"+code+"\n":""),err,""));
  else
    write("Du siehst einen Fehler im Raum-Zeit-Gefuege.\n");

  // Wenn Magierclosure zerstoert wurde: Magier zerstoeren
  // TODO: Ist das gewollt?
  if (err=="Object the closure was bound to has been destructed\n")
  {
    ob=find_object(curobj);
    if (ob && IS_LEARNER(ob) && query_once_interactive(ob))
    {
      ob->quit();
      if (ob) catch(ob->remove(); publish);
      if (ob) destruct(ob);
    }
  }
  return;
}

//Keine GD-Funktion, limitiert die Kosten fuer den handler
private void call_runtime_error(string err, string prg, string curobj, 
    int line, mixed culprit, int caught, object po) {
    string hashkey;

    //Fehlerdaten an den Errord zur Speicherung weitergeben, falls wir sowas
    //haben.
#ifdef ERRORD
    catch(hashkey=limited(#'call_other,({200000}),ERRORD,"LogError",
          err,prg,curobj,line,culprit,caught);publish);
#endif // ERRORD

    //eigenen Errorhandler laufzeitbegrenzt rufen
    limited(#'handle_runtime_error, ({ 200000 }), err, prg, curobj,
        line,culprit,caught, po, hashkey);
}

// Laufzeitfehlerbehandlung, hebt Evalcost-Beschraenkung auf und reicht weiter
// an call_runtime_error, die die Grenze wieder auf einen bestimmten Wert
// festlegt.
protected void runtime_error(string err ,string prg, string curobj, int line,
     mixed culprit, int caught) {

    //DEBUG(sprintf("Callerstack: (runtime_error()): %O\nPO: %O\nPO(0): %O\n",
    //          caller_stack(1),previous_object(),previous_object(0)));

    limited(#'call_runtime_error, ({ LIMIT_UNLIMITED }),
        err,prg,curobj,line,culprit,caught,previous_object());
}

//Warnungen mitloggen
protected void runtime_warning( string msg, string curobj, string prog, int line,
    int inside_catch) {
  string code, hashkey;
  string debug_txt;
  object titp;

  //DEBUG(sprintf("Callerstack: in runtime_warning(): %O\nPO(0): %O\n",
  //        caller_stack(1),previous_object(0)));
  
  //Daten der Warnung an den Errord zur Speicherung weitergeben, falls wir 
  //sowas haben.
#ifdef ERRORD
  catch(hashkey=limited(#'call_other,({200000}),ERRORD,"LogWarning",
        msg,prog,curobj,line,inside_catch); publish);
#endif // ERRORD

  // Fehlermeldung bauen
  if (!stringp(msg))    msg="<NULL>";
  if (!stringp(prog))    prog="<NULL>";
  if (!stringp(curobj)) curobj="<NULL>";
  debug_txt=sprintf("Warnung: %O, Objekt: %s, Programm: %O, Zeile %O (%s) "
                    "ID: %s",
                    msg[0..<2], curobj, (prog[0]!='<'?"/":"")+prog, line,
                    (TI?capitalize(getuid(TI)):"<Unbekannt>"),
                    hashkey||"");
  
  //Fehlermeldungen an -warnungen schicken
  DEBUG4_MSG(debug_txt,previous_object());

  titp = TI || TP;

  if (!titp) return;
  
  // Fehlermeldung an Benutzer
  if (IS_LEARNER(titp))
    tell_object(titp,
                sprintf("%'-'78.78s\nfile: %s line: %d object: %s\n" +
                        "%sWarnung: %s%'-'78.78s\n","",prog,line,curobj,
                        (prog&&(code=read_file("/"+prog,line,1))?
                         "\n"+code+"\n":""),msg,""));
  return;
}


//                       #####################
//######################## Einrichten des ED ############################
//                       #####################

// Setup speichern
protected int save_ed_setup(object who, int code)
{
  string file;

  if (!intp(code)) return 0;
  file = sprintf("/players/%s/.edrc",geteuid(who));
  rm(file);
  return write_file(file,(string)code);
}

// Setup einladen
protected int retrieve_ed_setup(object who)
{
  string file;

  file = sprintf("/players/%s/.edrc",getuid(who));
  if (file_size(file)<1) return 0;
  return (int)read_file(file);
}

// Wo werden Dateien gespeichert?
string get_ed_buffer_save_file_name(string file)
{
  return sprintf("/players/%s/.dead_ed_files/%s",
                 getuid(this_player()),efun::explode(file, "/")[<1]);  
}

//                  ################################
//################### Ungenutzte Pflichtfunktionen ######################
//                  ################################

// Nichts zu tun beim Master-Neustart
protected void external_master_reload()  { return; }

// Keine externen Master-Flags
protected void flag(string str) { return; }

// Wird benoetigt, falls ERQ angeschlossen ist
protected void stale_erq(closure callback) { return; }

// Zerstoerten Master wiederbeleben ...
// nicht viel zu tun, wenn flag gesetzt ist. Wenn aber nicht, sind alle
// Variablen auf 0. Nach dieser Funktion wird wieder inaugurate_master()
// gerufen.
protected void reactivate_destructed_master(int flag) {
    if (flag) {
        //re-init
        //was machen? TODO
    }
    return;
}

// Kein Quota-Demon (potentielles TODO)
//Handle quotas in times of memory shortage.
//is called during the final phase of a garbage collection, if the user
//reserve could not be re-allocated. Last (!) Chance to free (active) objects 
//from the system and prevent call to slow_shut_down()!
protected void quota_demon() {
    //was kann man machen?
    //uebervolle Seherhaeuser leeren? 
    return;
}

// Keine Prepositionen in parse_command
//string *parse_command_prepos_list() { return ({}); }

// Wie lautet das Wort fuer 'alle' ?
//string *parse_command_all_word() { return ({}); }


// Keine Besonderen Objektnamen
string printf_obj_name(object ob) { return 0; }

//                        ###################
//######################### Sonstiges/Hooks #############################
//                        ###################

//TODO: save_wiz_file in shutdown() integrieren?
// Wizliste speichern: Zeile generieren
static string _save_wiz_file_loop(mixed *a)
{
  return sprintf("%s %d %d\n",a[WL_NAME],a[WL_COMMANDS],a[WL_EXTRA]);
}

// Wizliste speichern
protected void save_wiz_file()
{
  rm("/WIZLIST");
  write_file("/WIZLIST",implode(
     map(wizlist_info(),#'_save_wiz_file_loop),""));
}

// TODO: macht der telnet_neg-hook ueberhaupt was?
void telnet_neg(mixed cmd,mixed opt,mixed args)
{
  if (opt==34 && cmd==251)
        binary_message(({255,250,34,1,1,255,240}));
}
  
// EUID und UID werden von give_uid_to_object() vergeben, diese sind in
// inaugurate_master() als driver hooks angemeldet.

/*load_uid_hook() sollte machen, was diese Lambda machte...
  unbound_lambda( ({'printf_obj_name}),
               ({#'give_uid_to_object,'printf_obj_name,
                   ({#'previous_object}),0})));   */
protected mixed load_uid_hook(string datei) {
    return(give_uid_to_object(datei,previous_object()));
}

/* clone_uid_hook sollte machen, was diese Lambda machte...
            unbound_lambda( ({'blueprint, 'new_name}), 
              ({#'give_uid_to_object,'new_name,
                  ({#'previous_object}),1})));      */
protected mixed clone_uid_hook(string blueprint,string new_name) {
    return(give_uid_to_object(new_name,previous_object()));
}

