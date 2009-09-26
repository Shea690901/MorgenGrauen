// MorgenGrauen MUDlib
//
// master.h -- definitions for the master object
//
// $Id: master.h 6720 2008-02-05 19:40:10Z Zesstra $

#ifndef __MASTER_H__
#define __MASTER_H__


#ifdef P_ARMOURS
#undef P_ARMOURS
#endif
#define P_ARMOURS "armours"

#ifdef P_WEAPON
#undef P_WEAPON
#endif
#define P_WEAPON "weapon"

//fuer den notify_fail-Hook
#ifndef P_DEFAULT_NOTIFY_FAIL
#define P_DEFAULT_NOTIFY_FAIL "default_notify_fail"
#endif

#include "/secure/config.h"

#include "/sys/userinfo.h"
#include "/sys/shells.h"
#include "/sys/player/base.h"
#include "/sys/moving.h"
#include "/sys/defines.h"
#include "/sys/wizlist.h"
#include "/sys/daemon.h"
#include "/sys/mail.h"
#include "/sys/driver_hook.h"
#include "/sys/functionlist.h"

#include "/secure/wizlevels.h"

//Ebenenmeldungen
//das previous_object(3) in den beiden ersten ist so gedacht. ;-)
#define DEBUG_MSG(x,po) catch(CHMASTER->send("Debug", \
	capitalize(objectp(po) ? REAL_UID(po) : ""), x))
#define DEBUG2_MSG(x,po) catch(CHMASTER->send("Entwicklung", \
	capitalize(objectp(po) ? REAL_UID(po) : ""), x))
#define DEBUG3_MSG(x) catch(CHMASTER->send("Entwicklung", \
	"<MasteR>", x))
//hier ist previous_object() der 'Uebeltaeter'
#define DEBUG4_MSG(x,po) catch(CHMASTER->send("Warnungen", \
	capitalize(objectp(po) ? REAL_UID(po) : ""), x))


//#define DEBUG(x)
#ifndef DEBUG
#define DEBUG(x)	if (funcall(symbol_function('find_player),"zesstra"))\
        tell_object(funcall(symbol_function('find_player),"zesstra"),\
        "MDBG: "+x+"\n")
#endif

//Logfile fuer Lesezugriffe
#define READ_FILE  "/log/ARCH/READ_FILE"

//Logfile fuer Crashes
#define CRASH_LOG  "/log/CRASH_LOG"

// MAX_ERRLOG_SIZE definiert, ab welcher Groesse ein Log in
// /log/error rotiert.
#define MAX_ERRLOG_SIZE 51200 

#define log_file(file,str) write_file("/log/"+file, str)
#define NAME(x)            getuid(x)
#define CAP_NAME(x)        capitalize(NAME(x))
#define TP                 efun::this_player()
#define TI                 efun::this_interactive()
#define PO                 efun::previous_object()
#define TO                 efun::this_object()
#define PATH_ARRAY(x) (efun::explode(x, "/")-({"","."}))

// Verzeichnis mit Infomails fuer Magierbefoerderungen
#define WIZ_HELP_MAIL_DIR "/doc/infomails/"

// aus master.c
string        creator_file(mixed str);
mixed         give_uid_to_object(string datei,object po);
void          save_wiz_file();
mixed         load_uid_hook(string datei);
mixed         clone_uid_hook(string blueprint,string new_name);

// aus domains.c
int            domain_master(string user, string domain);
int            domain_member(string user, string domain);
string         *get_domain_homes(string wiz);

// aus file_access.c
void          LoadDeputyFileList();
mixed         valid_read(string path, string euid, string fun, object obj);
mixed         valid_write(string path, string euid, string fun, object obj);

// aus misc.c
string         _get_path(string path, string user);
mixed          QueryBanished(string str);
// sendet bei Befoerderungen Infomail an Magier
protected void SendWizardHelpMail(string name, int level);

// aus userinfo.c
public int       find_userinfo(string user);
protected mixed *get_full_userinfo(string user);
public mixed    *get_userinfo(string user);
public int       get_wiz_level(string user);
public string    query_player_object( string name );
public int       query_wiz_level(mixed player);
int              update_wiz_level(string user,int lev);
protected void   set_guilds(string player, string *guilds);
protected void   set_domains(string player, string *domains);
void             update_late_players();
public string    secure_savefile(string name);
protected void   save_userinfo(string user);
protected string query_secure_euid();
protected int    query_secure_level();
private string*  ExpandUIDAlias(string alias, int rec);
public varargs string* QueryUIDAlias(string alias, int rec);

//Einen Magier als verantwortlich fuer eine bestimmte UID eintragen
public string*   AddWizardForUID(string uid, string wizuid);
//Einen Magier wieder austragen, wenn er nicht mehr zustaendig ist.
public string*   RemoveWizardFromUID(string uid, string wizuid);
//Verantwortliche Magier fuer eine UID ausgeben
public varargs string* QueryWizardsForUID(string uid, int recursive);
//Welche UIDs ist ein Magier implizit verantwortlich? (Ist er RM,
//Gildenmagier, in welchen Regionen hat er ein Verzeichnis, etc.)
public varargs string* QueryUIDsForWizard(string wizuid, int recursive);


#endif //__MASTER_H__