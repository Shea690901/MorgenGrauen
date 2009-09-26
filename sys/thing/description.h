// MorgenGrauen MUDlib
//
// thing/description.h -- header
//
// $Id: description.h 6308 2007-05-20 11:58:33Z Zesstra $
// MorgenGrauen MUDlib
//
// thing/description.h -- header
//
// $Id: description.h 6308 2007-05-20 11:58:33Z Zesstra $
 
#ifndef __THING_DESCRIPTION_H__
#define __THING_DESCRIPTION_H__

// properties
#define P_NAME               "name"       // name of the object
#define P_NAME_ADJ           "name_adj"   // adjective of the name
#define P_SHORT              "short"      // short desciption of the object
#define P_LONG               "long"       // long description of the object
#define P_IDS                "ids"        // identifier of the object
#define P_ADJECTIVES         "adjectives" // adjectives of the object
#define P_SHOW_INV           "show_inv"   // Show in inventory of player
#define P_CLASS              "class"      // class, like undead or explosive

#define P_KILL_MSG           "kill_msg"
#define P_KILL_NAME          "kill_name"

#define P_HEAL               "heal"
#define P_COMBATCMDS         "combatcmds"

#define P_VALUE              "value"
#define P_NOBUY              "nobuy"
#define P_NOSELL             "nosell"
#define P_MAGIC              "magic"
#define P_NOMAGIC            "nomagic"

#define P_LOG_FILE           "log_file"   // bugs in anderes repfile umleiten

#define P_LIGHT              "light"
#define P_TOTAL_LIGHT        "total_light"

#define P_CURSED             "cursed"
#define P_KEEP_ON_SELL       "keep_on_sell"

#define P_CLONER             "cloner"
#define P_CLONE_TIME         "clone_time"

/* read_msg ist auch fuer nicht fernwest-kundige lesbar */
#define P_READ_MSG           "read_msg"
#define P_FW_ALWAYS_READABLE "fw_always_readable"
#define P_DETAILS            "details"
#define P_SPECIAL_DETAILS    "special_details"
#define P_READ_DETAILS       "read_details"
#define P_SMELLS             "smell_details"
#define P_SOUNDS             "sound_details"
#ifndef P_INVIS
#define P_INVIS              "invis"
#endif

#define SENSE_DEFAULT "\ndefault"
#define SENSE_VIEW  0
#define SENSE_SMELL 1
#define SENSE_SOUND 2

#endif // __THING_DESCRIPTION_H__

#ifdef NEED_PROTOTYPES

#ifndef __THING_DESCRIPTION_H_PROTO__
#define __THING_DESCRIPTION_H_PROTO__

// prototypes
// check if the objects can by identified by str
varargs int id(string str, int lvl);

// check IDs vs. string*
int match_ids(string *list);

// add an id to the current object
void AddId(mixed str);

// remove an id from the object
void RemoveId(mixed str);

// add an adjective to describe the object
void AddAdjective(mixed str);

// remove an adjective from the object
void RemoveAdjective(mixed str);

// get the name with casus and demonstrative
varargs string name(int casus, int demon);
varargs string Name(int casus, int demon);

// check if object is member of a class
int is_class_member(string str);

// add object to a class
void AddToClass(string str);

// add a detail to object
void AddDetail( mixed *keys, mixed descr );
void AddReadDetail( mixed keys, mixed descr );
void AddSmells( mixed keys, mixed descr );
void AddSounds( mixed keys, mixed descr );

// remove a detail from object
varargs void RemoveDetail( mixed *keys );

// add a special detail to object
void AddSpecialDetail( mixed keys, string functionname );

// remove a special detail to object
void RemoveSpecialDetail( mixed keys );

static void GiveEP(int type, string key);

#endif // __THING_DESCRIPTION_H_PROTO__

#endif	// NEED_PROTOYPES
