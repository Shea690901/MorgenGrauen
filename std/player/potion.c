// MorgenGrauen MUDlib
//
// player/potion.c -- potion handling for player
//
// $Id: potion.c 9108 2015-01-21 22:15:29Z Zesstra $
//

#pragma strong_types,save_types

#include <input_to.h>

#define NEED_PROTOTYPES
#include <thing/properties.h>
#include <player/potion.h>
#include <attributes.h>
#include <living/life.h>
#include <player/base.h>
#undef NEED_PROTOTYPES

#include <properties.h>
#include <defines.h>
#include <wizlevels.h>

#define POTIONMASTER "/secure/potionmaster"

static mixed *list;

mixed *potionrooms;
mixed *known_potionrooms;

static mixed _query_potionrooms();
static mixed _query_known_potionrooms();

protected void create() 
{ 
  if (!potionrooms) potionrooms=POTIONMASTER->InitialList(); 
  Set(P_POTIONROOMS, NOSETMETHOD, F_SET_METHOD);         // no tampering by methods
  Set(P_POTIONROOMS, SECURED, F_MODE_AS);      // no tampering with list

  if (!known_potionrooms) known_potionrooms = ({});
  Set(P_KNOWN_POTIONROOMS, NOSETMETHOD, F_SET_METHOD);
  Set(P_KNOWN_POTIONROOMS, SECURED, F_MODE_AS);
}

static int ReportPotion(string s, object po, int num);
static int SelectWhich(object po, int num);
static int ask_question(object po);
static int get_answer(string erg, object po);
static int raise(string what, object po);
int AddKnownPotion(int nr);
int RemoveKnownPotion(int nr);

protected void updates_after_restore(int newflag)
{
  // P_VISITED_POTIONROOMS ist lang veraltet und unbenutzt, aber bis zum
  // 21.1.2015 sogar in neuen Spielern gespeichert worden.
  // Aehnlich fuer P_BONUS_POTIONS. Weg damit.
  Set(P_VISITED_POTIONROOMS, SAVE|PROTECTED, F_MODE_AD);
  Set(P_BONUS_POTIONS, SAVE|PROTECTED, F_MODE_AD);
}

varargs int FindPotion(string s)
{
  int flag, pnum;
  string str, fn;
  mixed rl;
  object env,po;

  po=previous_object();

  flag=1;

  if (QueryProp(P_TRANK_FINDEN) && IS_WIZARD(this_object())) {
    pnum=POTIONMASTER->HasPotion(previous_object());
    return ReportPotion(s,po,pnum);
  }
  
  if (QueryProp(P_KILLS)) return 0;

  if (!potionrooms || potionrooms==({}) ||
      !(POTIONMASTER->InList(previous_object(), potionrooms, 
			     known_potionrooms)))
    flag=0;

  if ((pnum=POTIONMASTER->HasPotion(previous_object()))<0)
    return 0;

  if (!flag) return 0;
  if (query_input_pending(this_object()) || query_editing(this_object())) {
    tell_object(ME,
	"Jetzt haettest Du fast einen Zaubertrank gefunden. Du solltest den\n"
	 "Editor/das More verlassen und es dann noch einmal versuchen!\n");
    return 1;
  }

// Hier der Ausbau der ZTs fuer Geister, wobei das natuerlich in der
// Geisterschlossquest immer noch gehen sollte.

  env=environment(this_object());
  fn=old_explode(object_name(env), "#")[0];
  if(QueryProp(P_GHOST) && 
     !(fn[0..24]=="/d/wald/bertram/gschloss/")) {
       tell_object(ME,"Als Geist einen Zaubertrank? Hier nicht!\n");
    return 1;
  }
  log_file("ARCH/POTIONS", dtime(time()) + "  " +capitalize(getuid()) + " in " + 
       object_name(previous_object()) + "\n");

  return ReportPotion(s, po, pnum);
}

static int ReportPotion(string s, object po, int pnum)
{
  if (stringp(s) && sizeof(s)) 
    tell_object(ME, s);
  else 
    tell_object(ME, "Du findest einen Zaubertrank, den Du sofort trinkst.\n");

  SelectWhich(po, pnum);
  
  return 1;
}

static int SelectWhich(object po, int pnum)
{
  list=({"Intelligenz","Kraft","Geschicklichkeit","Ausdauer"});
  if (QueryRealAttribute(A_INT)>=20) list-=({"Intelligenz"});
  if (QueryRealAttribute(A_STR)>=20) list-=({"Kraft"});
  if (QueryRealAttribute(A_DEX)>=20) list-=({"Geschicklichkeit"});
  if (QueryRealAttribute(A_CON)>=20) list-=({"Ausdauer"});
  if (!sizeof(list)) {
    heal_self(10000000);
    tell_object(ME, "Der Trank hat Dich geheilt.\n");
    log_file("ARCH/POTIONS",
	     sprintf("  %s: Heiltrank (noch %d)\n",
		     capitalize(getuid()),sizeof(potionrooms)));

    potionrooms -= ({pnum});
    known_potionrooms -= ({pnum});
    save_me(1);
    return 1;
  }
  if (sizeof(list)==1) return raise(list[0],po);
  ask_question(po);
  return 1;
}

static int ask_question(object po)
{
  int i;

  tell_object(ME, "Deine Attribute sehen so aus:\n\n"
    +sprintf("Intelligenz     : %2d (%+d)\n",
         QueryRealAttribute(A_INT),QueryAttributeOffset(A_INT))
    +sprintf("Kraft           : %2d (%+d)\n",
         QueryRealAttribute(A_STR),QueryAttributeOffset(A_STR))
    +sprintf("Geschicklichkeit: %2d (%+d)\n",
         QueryRealAttribute(A_DEX),QueryAttributeOffset(A_DEX))
    +sprintf("Ausdauer        : %2d (%+d)\n",
         QueryRealAttribute(A_CON),QueryAttributeOffset(A_CON))
    );

  tell_object(ME,
      "\nWas moechtest Du erhoehen ? Du hast folgende Moeglichkeiten:\n");
  for (i=0;i<sizeof(list);i++)
    tell_object(ME, sprintf("%d) %s\n", i+1,list[i]));

  input_to("get_answer", INPUT_PROMPT,
      sprintf("\nBitte gib jetzt eine Zahl (1-%d) an: ", i), po);
  
  return 1;
}

static int get_answer(string erg, object po)
{
  int num;

  if (erg && stringp(erg))
  {
    if (sscanf(erg,"%d",num)<1) num=0;
    if (num>0 && num<=sizeof(list))
      return raise(list[num-1], po);
  }
  tell_object(ME, "Deine Wahl war ungueltig. Bitte versuchs nochmal !\n\n");
  return ask_question(po);
}

static int raise(string what, object po) {
  string attr;
  int yet;

  switch (what) {
  case "Geschicklichkeit":
    attr=A_DEX; break;
  case "Intelligenz":
    attr=A_INT; break;
  case "Kraft":
    attr=A_STR; break;
  case "Ausdauer":
    attr=A_CON; break;
  default: return 0;
  }

  yet=QueryRealAttribute(attr);
  SetRealAttribute(attr,yet+1);
  tell_object(ME,
      sprintf("Deine %s hat sich von %d auf %d erhoeht.\n", what, yet, yet+1));
  log_file("ARCH/POTIONS",
	   sprintf("  %s: %s %d->%d (noch %d)\n",
		   capitalize(getuid()),capitalize(attr),
		   yet,yet+1,sizeof(potionrooms)));

// RemoveList ersetzt den gefundenen Trank durch -1, das wird dann entfernt.
// Dies ist ein Hack um einen Reboot zu vermeiden.

  POTIONMASTER->RemoveList(po, potionrooms, known_potionrooms);
  potionrooms       -= ({ -1 });
  known_potionrooms -= ({ -1 });
  save_me(1);

  return 1;
}

static mixed _query_potionrooms()
{
  return copy(Set(P_POTIONROOMS, potionrooms));
}

static mixed _query_known_potionrooms()
{
  return copy(Set(P_KNOWN_POTIONROOMS, known_potionrooms));
}

int AddKnownPotion(int nr)
{
  if (!previous_object() || 
      object_name(previous_object()) != "/room/orakel")
    return -1; // Keine Berechtigung

  if (member(known_potionrooms, nr) != -1)
    return -2; // Nummer bereits eingetragen
  else
  {
    known_potionrooms += ({ nr });
    return 1;
  }
}

int RemoveKnownPotion(int nr)
{
  if (!previous_object() || 
      object_name(previous_object()) != "/room/orakel")
    return -1; // Keine Berechtigung

  if (member(known_potionrooms, nr) == -1)
    return -2; // Nummer nicht eingetragen
  else
  {
    known_potionrooms -= ({ nr });
    return 1;
  }
}
