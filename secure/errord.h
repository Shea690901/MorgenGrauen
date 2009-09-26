/*  MorgenGrauen MUDlib
    /sys/errord.h
    Headerfile fuer den Error-Daemonen
    Autor: Zesstra
    $Id: errord.h 6760 2008-03-14 19:16:04Z Zesstra $
    ggf. Changelog:
*/

#ifndef __ERRORD_H__
#define __ERRORD_H__

// Typen
#define T_RTERROR   1	 //Runtime Error / Laufzeitfehler
#define T_RTWARN    2    //Runtime Warning / Laufzeitwarnung
#define T_CTERROR   4    //Compiletime Error / Fehler beim Uebersetzen
#define T_CTWARN    8    //Compiletime Warning / Warnung beim Uebersetzen

// Status
#define STAT_DELETED  0x1
#define STAT_LOCKED   0x2
#define STAT_RESOLVED 0x4

// Achtung: viele Keys koennen nicht vorhanden oder Werte 0 sein!
#define F_TYPE        "F_TYPE"        //int: Typ-Werte, s.o.
#define F_HASHKEY     "F_HASHKEY"     //string: Hashkey des Fehlers
#define F_UID         "F_UID"         //string: "UID des Fehlers"
#define F_STATE       "F_STATE"       //int: Status-Werte, s.o.
#define F_MODSTAMP    "F_MODSTAMP"    //int
#define F_CREATESTAMP "F_CREATESTAMP" //int
#define F_READSTAMP   "F_READSTAMP"   //int 
#define F_PROG        "F_PROG"        //string == Bluename, falls kein replace_program()
#define F_OBJ         "F_OBJ"         //string
#define F_LOADNAME    "F_LOADNAME"    //string == Bluename, falls kein rename_object()
#define F_LINE        "F_LINE"        //int
#define F_MSG         "F_MSG"         //string
#define F_HB_OBJ      "F_HB_OBJ"      //string
#define F_CAUGHT      "F_CAUGHT"      //int
#define F_TITP        "F_TITP"        //string
#define F_STACK       "F_STACK"       //Array von mixed (mixed)
#define F_CLI         "F_CLI"         //string (Spielereingabe)
#define F_VERB        "F_VERB"        //string (Kommandoverb)
#define F_COUNT       "F_COUNT"       //int
#define F_TIENV       "F_TIENV"       //string, object_name() vom Env von TI/TP
#define F_LOCK        "F_LOCK"        //mixed (Array von 2 Elementen)
#define F_RESOLVER    "F_RESOLVER"    //string (wer hat gefixt?)
#define F_NOTES       "F_NOTES"       //Bemerkungen, Array von 3-elementigen Arrays

#ifdef __NEED_IMPLEMENTATION__

#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG(x)
#define DEBUG(x)  if (funcall(symbol_function('find_player),"zesstra"))\
        tell_object(funcall(symbol_function('find_player),"zesstra"),\
        "EDBG: "+x+"\n")

//Wer pflegt das Ding hier gerade?
#define MAINTAINER ({"zesstra"})

//Stanard-Expire
#define STDEXPIRE 2678400 //31 Tage

//Blacklist fuer Files, die nicht erfasst werden sollen (nur
//Uebersetzungsprobleme momentan)
#define BLACKLIST ({".tool.lpc.c",".xtool.h"})

//Zugriffsarten
#define M_READ   1
#define M_WRITE  2

// Changelog
#define CHANGELOG "/log/CHANGELOG"

// Log
//#define MAILLOG       "sent.log"

// Standardmailtext: TODO 
//#define STANDARDTEXT  HOME("mailtext.txt")

#define STANDARDMAILTEXT "Huhu lieber Mitmagier,\n\n" \
    "der unten angegebene Fehler in einem Objekt, fuer welches Du " \
    "(als programmierender Magier oder RM) zustaendig bist, wurde soeben " \
    "von %s als gefixt markiert. Bitte beachte ggf. die unten angebenen " \
    "Bemerkungen zum Fix.\n\n--- Fehler-Daten ---\n"

/* public Interface */

//Abfrage eines bestimmten Fehlers
varargs mapping  QueryError(string hash, int type, string uid);
//Alle Fehler eines Typs und einer UID in einem Mapping liefern.
mapping          QueryErrors(int type, string uid);
//Alle Errorkeys einer UID (in gegebenem Typ) liefern
varargs string*  QueryErrorIDs(int type,string uid);
//alle UIDs fuer einen Fehlertyp oder alle -Typen abfragen
varargs string*  QueryUIDsForType(int type);
//Wieviele unterschiedliche Fehler in diesem Typ und UID?
varargs int      QueryUniqueErrorNo(int type, string uid);
//Wieviele Fehler insgesamt in diesem Typs und UID?
varargs int      QueryErrorNo(int type,string uid);
//einen bestimmten Fehler loeschen
varargs int      ToggleDeleteError(string hashkey, string note, int type,
                                   string uid);

//letzten Fehler eines Typs abfragen
mapping          QueryLastError(int type);

// einen Fehler gegen automatisches Loeschen schuetzen, liefert int < 0 im
// Fehlerfall, im Erfolgsfall ein Array mit Lockinfos (wenn Lock schon
// gesetzt, wird der alte zurueckgeliefert und sonst nix gemacht!)
varargs mixed    LockError(string hashkey, string note, int type, 
                           string uid);
// Schutz gegen automatisches Loeschen entfernen, liefert < 0 im Fehlerfall
// und ein leeres Array, wenn der Lock aufgehoben wurde
varargs int      UnlockError(string hashkey, string note, int type, 
                             string uid);

// einen Fehler als gefixt markieren
varargs int      ResolveError(string hashkey, string note, int type, 
                              string uid);
// einen Fehler als nicht gefixt markieren
varargs int      ReOpenError(string hashkey, string note, int type, 
                             string uid);

// Notizen hinzufuegen
varargs int      AddNote(string hashkey, string note, int type, string uid);

// Fehler einer anderen UID zuweisen
varargs int ReassignError(string hashkey, string newuid, string note, 
                          int type, string uid);

public string     find_uid_for_hash(string hash, int type);
public string     find_global_hash(string hash);

public string format_stacktrace(mixed bt);
public string format_notes(mixed notes);
public string format_error(string hashvalue, mapping fehler);

/* int. Helpers */
private int       access_check(string uid,int mode);
private int       queryUniqueErrorNoForUID(int type, string uid);
private int       queryErrorNoForUID(int type,string uid);
void              save_me();
private mapping   filter_private(string hash, mapping error);
private varargs mixed set_lock(string hashkey, int lock, string note, int type, 
                                string uid);
private varargs int set_resolution(string hashkey, int res, string note,
                                   int type, string uid);

private varargs mapping getError(string hashkey, int type, string uid);
private int versende_mail(mapping fehler);

#endif // __NEED_IMPLEMENTATION__

#endif // __ERRORD_H__

