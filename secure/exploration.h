// MorgenGrauen MUDlib
//
// exploration.h -- Definitionen fuer den explorationmaster
//
// $Id: exploration.h 6151 2007-02-03 12:02:14Z Muadib $

#ifndef __EXPLORATION_H__
#define __EXPLORATION_H__

/* Dateinamen */
#define EPMASTER    "/secure/explorationmaster"
#define EPSAVEFILE  "/secure/ARCH/exploration"
#define DUMPFILE    "/secure/ARCH/EPROOMS.dump"
#define MAKE_EPSTAT "/secure/ARCH/epstat"
#define EPSTAT_INFO "/secure/ARCH/epstat.o"

/* Ziele von ChangeEPObject() */
#define CHANGE_OB   0 // das Objekt selbst
#define CHANGE_KEY  1 // Schluessel
#define CHANGE_TYPE 2 // der Typ
#define CHANGE_BONUS 3 // ist es ein Bonus-EP?

/* Positionen im Mapping */
#define MPOS_KEY  0   // Die Schluessel
#define MPOS_NUM  1   // Nummer des EP
#define MPOS_TYPE 2   // Typ (s.u.)

/* Typen von EP-Spendern */
#define EP_DETAIL 0   // Details, SpecialDetails
#define EP_EXIT   1   // Exits und SpecialExits
#define EP_CMD	  2   // AddCmd()-Kommandos
#define EP_INFO   3   // Infos von NPCs
#define EP_MISC   4   // eigene GiveExplorationPoint()-Aufrufe
#define EP_RDET   5   // ReadDetails
#define EP_PUB    6   // Speisen und Getraenke in Kneipen
#define EP_SMELL  7   // Gerueche
#define EP_SOUND  8   // Geraeusche
#define EP_MAX    8   // max. Anzahl von Typen

#define EP_TYPES ({ "det ", "exit", "cmd ", "info", "misc", "rdet", "pub ", "sme ", "sond" })

/* Fehler bei Funktionsaufrufen */
#define EPERR_NOT_ARCH	  -1  // this_interactive() war kein Erzmagier
#define EPERR_INVALID_OB  -2  // ungueltiges Objekt (zB. VC-Raum)
#define EPERR_NO_ENTRY	  -3  // Eintrag nicht gefunden
#define EPERR_INVALID_ARG -4  // Ungueltiges Argument

/* und hier was fuer die Statistik... */
#define MIN_EP  10

/* FP-Scripte... */
#define LF_LOG "ARCH/FPS_TOO_FAST"
#define LF_TIME 900
#define LF_WARN 3

/* FP-Logfile */
#define FP_LOG "ARCH/FPS_FOUND"

// ein paar defines fuer ektips
#define FPTIPS_MAX_RETRY		10
// one tip per level in list, above top entry one tip every level
#define FPTIPS_LEVEL_LIMITS 	({61,66,71,75,79,83,86,89,92,94,96,98})

#endif


#ifdef NEED_PROTOTYPES

#ifndef __EXPLORATION_H_PROTO__
#define __EXPLORATION_H_PROTO__

/* prototypes */
static void GiveEP(int type, string key); // in /std/thing/description.c

#endif // __EXPLORATION_H_PROTO__

#endif	// NEED_PROTOYPES

