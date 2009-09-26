// MorgenGrauen MUDlib
//
// gilde.c -- Standardgilde
//
// $Id: gilde.c 7290 2009-09-16 22:11:31Z Zesstra $
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

inherit "std/room";

#include <properties.h>
#include <defines.h>
#include <rooms.h>
#include <wizlevels.h>
#include <language.h>
#include <new_skills.h>
#include <exploration.h>
#include <ansi.h>
#include "/secure/questmaster.h"
#include "/secure/lepmaster.h"
#include <events.h>

mixed get_next_exp(int lev);
varargs string get_new_title(int lev, object pl);

#define kosten_0 \
  ([0:                "Bis Du Dich das naechste Mal hier blicken laesst, solltest Du eine Menge Stufenpunkte sammeln. Sonst wird das nix!",\
    "zauberer":       "Jetzt fang hier ja nicht an zu jammern. Du hast doch grade erst erhoeht. Fuer die naechste Stufe musst Du erstmal Leistung zeigen!",\
    "kaempfer":       "Hat grade erst erhoeht und faengt schon an zu jammern wie ein altersschwacher Zauberer. Nimm gefaelligst Haltung an!",\
    "klerus":         "Wer hat Dir denn das Gehirn gelaeutert? Zeig erstmal was von Deinem Koennen, bevor Du schon wieder erhoehen willst!",\
    "chaos":          "So chaotisch kannst Du doch nicht sein zu glauben, dass Du schon wieder erhoehen kannst!",\
    "karate":         "Ruh Deine Knochen erstmal ein bisschen aus. Bis zur naechsten Stufe werden sie noch genug leiden muessen!",\
    "katzenkrieger":  "Da hilft kein Maunzen und kein Schnurren. Du hast grade erst erhoeht und dabei bleibt das!",\
    "tanjian":        "Jetzt schrei nicht gleich nach Siamil. Du musst noch eine Menge leisten bis zur naechsten Stufe!",\
    "bierschuettler": "Du hast das Bier auf Deinen letzten Erfolg noch nicht einmal ausgetrunken. Werd jetzt ja nicht groessenwahnsinnig!",\
    "werwoelfe" :     "Und wenn Du noch so grimmig guckst und den Mond anheulst. Du hast gerade erst erhoeht. Nun musst Du erstmal wieder etwas tun!",\
  ])
#define kosten_20 \
  ([0:                "Da liegt aber noch ein weiter Weg vor Dir. Aber Du schaffst es ganz sicher!",\
    "zauberer":       "Du kannst zwar zaubern, aber nicht hexen. Fuer die naechste Stufe musst Du schon noch einiges tun!",\
    "kaempfer":       "Oh weh, da hast Du aber noch ne Menge vor Dir. Beiss die Zaehne zusammen und los gehts!",\
    "klerus":         "Du wirst bestimmt noch einige Heiltraenke brauchen bis zur naechsten Stufe!",\
    "chaos":          "Hast Du versucht Dein Chaos zu beseitigen, oder was hast Du gemacht? Sehr weit gekommen biste jedenfalls noch nicht. Musst Dich schon ein bisschen anstrengen!",\
    "karate":         "Ganz schoen anstrengend immer nur mit Hand und Fuss zu metzeln, oder? Aber es hat Dich schon etwas naeher an die naechste Stufe rangebracht!",\
    "katzenkrieger":  "Bisschen weniger Fellpflege betreiben, und ein bisschen mehr Stufenpunkte sammeln. Du hast naemlich noch ein gutes Stueck vor Dir!",\
    "tanjian":        "Du hast noch ne Menge zu tun bis zur naechsten Stufe. Sterb nicht so oft, aber sowas tun Tanjian ja eh nicht!",\
    "bierschuettler": "Bier trinken alleine wird Dich nicht weiter bringen, obwohl Du schon ein Stueck weit gekommen bist!",\
    "werwoelfe" :     "Staerke Deine Willenskraft. Du hast noch einiges vor Dir!",\
  ])
#define kosten_40 \
  ([0:                "Also die Haelfte hast Du schon ungefaehr geschafft. Aber ruh Dich jetzt ja nicht darauf aus. Sieh mal zu, dass Du die zweite Haelfte zur naechsten Stufe auch noch schaffst. Auf einem Bein kann man schliesslich nicht lange stehn!",\
    "zauberer":       "Teile Llystrathe mit Hilfe. Och schade scheint nicht zu funktionieren, dann musst Du eben selber noch ne Menge Stufenpunkte sammeln!",\
    "kaempfer":       "Kaempfer schnell an die Waffen! Die Haelfte hast Du ungefaehr geschafft, aber es gibt noch viel zu tun!",\
    "klerus":         "Beile Dich! Das Boese sitzt Dir im Nacken und Du hast noch ungefaehr die Haelfte vor Dir!",\
    "chaos":          "Schnapp Dir Deinen Daemon und teil Dir die zweite Haelfte der Arbeit mit ihm!",\
    "karate":         "Du scheinst eingesehen zu haben, dass hirnloses Draufhauen nix nutzt. Die Haelfte haste ungefaehr schon geschafft!",\
    "katzenkrieger":  "So ungefaehr bis zur Haelfte hast Du Dich schon an die neue Stufe herangeschlichen!",\
    "tanjian":        "Man kann den Wert zwischen den beiden Stufen durchaus als ausgewogen bezeichnen. Trotzdem hast Du noch ein Stueck des Weges vor Dir!",\
    "bierschuettler": "War das Fass nu halb voll oder halb leer? Jedenfalls biste ungefaehr bei der Haelfte angekommen!",\
    "werwoelfe" :     "Noch einige Gegner werden sich vor Dir fuerchten muessen, obwohl Du schon viel geschafft hast!",\
  ])
#define kosten_60 \
  ([0:                "Also ein bisschen was musst Du noch tun, aber es sieht schon ziemlich gut aus!",\
    "zauberer":       "Ein bisschen Wille und Magie und Du schaffst es wieder ein Stueck naeher an die neue Stufe heranzukommen!",\
    "kaempfer":       "Staerke Deinen Kampfwillen. Die groessten Anstrengungen hast Du schon hinter Dir!",\
    "klerus":         "So nah am Ziel wirst Du doch nicht aufgeben wollen. Hol Dir noch ein bisschen goettliche Unterstuetzung und weiter gehts!",\
    "chaos":          "Chaotisch sein alleine hilft nicht. Ein bisschen was musst Du schon noch tun!",\
    "karate":         "Die Handkanten geschaerft, den Gi fest zugebunden. Bald hast Du es geschafft!",\
    "katzenkrieger":  "Auf Samtpfoten schleichst Du mit grossen Schritten ans Ziel heran. Aber ein bisschen fehlt noch!",\
    "tanjian":        "Eine Quest hier, ein paar Monster da. Aber immer schoen ausgeglichen bleiben, dann klappts auch weiterhin!",\
    "bierschuettler": "Noch ein paar kraeftige Schlucke und Du hast es fast geschafft!",\
    "werwoelfe" :     "Verlass Dein Rudel und zieh hinaus ins Abenteuer. Dann wirst Du bald siegreich heimkehren!",\
  ])
#define kosten_80 \
  ([0:                "Huch na sowas, Du hast die naechste Stufe ja schon fast erreicht. Na also! Der Rest ist ja wirklich nur noch Kleinkram!",\
    "zauberer":       "Die paar fehlenden Puenktchen haste Dir doch rasch zusammengezaubert!",\
    "kaempfer":       "Das fehlende bisschen machst Du als gnadenlos guter Kaempfer doch mit Links. Jetzt halt Dich aber mal ran. Oder willst Du Dich auf dem letzten Stueck noch von so einem luschigen Zauberer ueberrunden lassen?",\
    "klerus":         "Bei Saphina, Kandri und Lembold, Du bist fast bei der naechsten Stufe angekommen!",\
    "chaos":          "Boese waehrt am laengsten. Fast hast Du es geschafft!",\
    "karate":         "So viele blaue Flecken wie Du an Armen und Beinen hast, kann die naechste Stufe nicht mehr weit sein!",\
    "katzenkrieger":  "Pass bloss auf, dass Dich auf den letzten Zentimetern nicht noch ein Hund erwischt!",\
    "tanjian":        "Siamil wird stolz auf Dich sein. Du hast Dich gegen Gut und Boese durchgesetzt und das Ziel fast erreicht!",\
    "bierschuettler": "Lange musst Du nicht mehr warten. Kannst ruhig schon mal ein frisches Bier in Auftrag geben!",\
    "werwoelfe" :     "Noch ein letztes Mal die fuerchterlichen Krallen schaerfen, dann hast Du Deinen neuen Level erreicht!",\
  ])

void create()
{
  if (object_name(this_object()) == __FILE__[0..<3]) {
      set_next_reset(-1);
      // hier mal kein return;
  }
  room::create();
  SetProp(P_INDOORS,1);
  SetProp(P_INT_LONG,
      "Du befindest Dich in der beruehmten Abenteurer Gilde.\n"
      +"Hier kannst Du Deine Erfahrungsstufe erhoehen.\n"
      +"An der Wand siehst Du eine grosse Uhr.\n"
      +"Moegliche Befehle sind:\n"
      +"                   liste\n"
      +"                   erhoehe (stufe)\n"
      +"                   kosten (kurz)\n");
  SetProp(P_INT_SHORT,"Abenteuer-Gilde");
  AddCmd("kosten","kosten");
  AddCmd("liste","liste");
  AddSpecialDetail("uhr","zeige_reboot");
  AddCmd("erhoehe","advance");
  SetProp( P_LIGHT, 1 );
}

void init()
{
  room::init();
}

string zeige_reboot()
{
  string str;
  int t,t2;

  t=time()-last_reboot_time();
  t2=t;
  str="Seit dem letzten Shutdown sind "+t+" Sekunden vergangen.\n";
  if (t<60) return str;
  str+="Das sind ";
  if (t>=86400)
  {
    str+=t/86400+" Tag";
    if (t>=86400*2)
      str+="e";
    str+=", ";
    t2=t2%86400;
  }
  if (t>=3600)
  {
    str+=t2/3600+" Stunde";
    if (t2/3600!=1)
      str+="n";
    str+=", ";
    t2=t2%3600;
  }
  if (t>=60)
  {
    str+=t2/60+" Minute";
    if (t2/60!=1)
      str+="n";
    str+=" und ";
    t2=t2%60;
  }
  str+=t2+" Sekunde";
  if (t2!=1)
    str+="n";
  str+=".\n";
  return str;
}

#define male_title_str ({ \
      "der hoffnungsvolle Anfaenger",\
      "der Landstreicher",           \
      "der Streuner",                \
      "der Wandergeselle",           \
      "der Waldlaeufer",             \
      "der Faehrtensucher",          \
      "der Wegekundige",             \
      "der Jaeger",                  \
      "der Kundschafter",            \
      "der Reisende",                \
      "der Abenteurer",              \
      "der Weltenbummler",           \
      "der Draufgaenger",            \
      "der Schatzsucher",            \
      "der Feuerbaendiger",          \
      "der Entdecker",               \
      "der Eroberer",                \
      "der Held",                    \
      "der angehende Seher",         \
      "der Seher" })

#define fem_title_str ({ \
      "die hoffnungsvolle Anfaengerin", \
      "die Landstreicherin",            \
      "die Streunerin",                 \
      "die Wandergesellin",             \
      "die Waldlaeuferin",              \
      "die Faehrtensucherin",           \
      "die Wegekundige",                \
      "die Jaegerin",                   \
      "die Kundschafterin",             \
      "die Reisende",                   \
      "die Abenteurerin",               \
      "die Weltenbummlerin",            \
      "die Draufgaengerin",             \
      "die Schatzsucherin",             \
      "die Feuerbaendigerin",           \
      "die Entdeckerin",                \
      "die Erobererin",                 \
      "die Heldin",                     \
      "die angehende Seherin",          \
      "die Seherin" })

varargs string get_new_title(int lev, object pl)
{
  if (!pl) pl=PL;
  if (lev<0) lev=0;
  if (lev >= 19)
    lev = IS_SEER(PL) ? 19 : 18;

  if (lev <= 19)
    if (pl->QueryProp(P_GENDER) == MALE)
      return male_title_str[lev];
    else
      return fem_title_str[lev];
  else
    return (string)pl->Query(P_TITLE); // Keine Endlos-Rekursion
}

mixed get_next_exp(int lev)
{
  if (lev<1) return -1;
  if (lev>31) return 100000000+(lev-31)*25000000;
  return
    ({
      676, 1014, 1522, 2283, 3425,
      5138, 7707, 11561, 17341, 26012,
      39018, 58527, 77791, 97791, 131687,
      197530, 296296, 444444, 666666, 1000000,
      1500000, 2250000, 3375000, 5062500, 7600000,
      11000000, 17000000, 25000000, 40000000, 60000000,
      80000000, 100000000, 125000000, 150000000, 175000000,
      200000000
      })[lev];
}

int next_ap(int lev, object pl)
{
  int need;

  need=pl->QueryProp(P_NEEDED_QP);
  lev--;
  if(lev>=20) return 0;
  if(lev==19) return need;
  return (int)((exp(log(1.262)*lev)+lev-2)*need/100);

  return 0;
}

int seer_cond(int silent)
{
  int cond;

  cond=LEPMASTER->QueryReadyForWiz(this_player());
  
  if (!silent)
    write(break_string(LEPMASTER->QueryReadyForWizText(this_player()),
          78, 0, 1));

  return cond;
}

varargs int kosten(string str)
{
  int diff, lvl;
  string tmp;

  diff = 100 * ((lvl=PL->QueryProp(P_LEVEL)+1)) - PL->QueryProp(P_LEP);

  switch ( diff ){
  case 101..1000000000:
      // Bei Spielern < Lvl 11 gibt es groessere Stufenpunkt-Verluste beim
      // Erhoehen, da die Erfahrungspunkte mit steigendem Level weniger
      // wert sind
      if ( lvl < 11 )
          tmp = "Na komm, Du hast doch gerade erst die letzte Stufe geschafft.";
      else
          tmp = "Wie bist Du ueberhaupt an Deinen Level gekommen?\nSei froh, "
          "dass Du nicht wieder abgestuft wirst.";
      break;

  case 81..100:
      tmp=kosten_0[PL->QueryProp(P_GUILD)] || kosten_0[0];
      break;

  case 61..80:
      tmp=kosten_20[PL->QueryProp(P_GUILD)] || kosten_20[0];
      break;
      
  case 41..60:
      tmp=kosten_40[PL->QueryProp(P_GUILD)] || kosten_40[0];
      break;
      
  case 21..40:
      tmp=kosten_60[PL->QueryProp(P_GUILD)] || kosten_60[0];
      break;
      
  case 1..20:
      tmp=kosten_80[PL->QueryProp(P_GUILD)] || kosten_80[0];
      break;
      
  default:
      if ( lvl < 5 )
          tmp = "Probier mal den Befehl 'erhoehe'.";
      else
          tmp = "Den Befehl 'erhoehe' kennst Du aber, ja?";
  }

  write( break_string( tmp, 78, 0, BS_LEAVE_MY_LFS ) );
  
  if (!IS_SEER(this_player()) && str != "kurz") {
    seer_cond(0);
    write (break_string("\nMit 'kosten kurz' kannst Du die Angabe der "
			"Seher-Anforderungen unterdruecken.", 78,0,1));
  }

  return 1;
}

int advance(string arg)
{
  string name_of_player;
  string uid;
  int level;
  string s, title;
  mapping ts;
  int gap;
  object pl;

  if (arg && 
      arg != "stufe" && arg != "spielerstufe" && arg != "spieler"
      && arg != "spielerlevel")
    return 0;
  if (PL->QueryProp(P_KILLS)>0) {
    notify_fail("Du hast einen Mitspieler umgebracht!\n"+
		"In diesem Fall kannst Du Deine Stufe nicht erhoehen.\n"+
		"Bitte geh zur Polizei und bring das in Ordnung.\n");
    say(capitalize(PL->name(WER))+" hat soeben auf schmerzliche Weise "+
	"erfahren muessen,\ndass es wirklich nicht foerderlich ist, "+
	"Mitspieler umzubringen.\n",PL);
    return 0;
  }

  uid = getuid(PL);
  name_of_player = PL->name(WER);
  title = PL->QueryProp( P_TITLE );
  level = PL->QueryProp( P_LEVEL );
  if (level == -1) level = 0;

  if (100*(++level) > PL->QueryProp(P_LEP)){
      return kosten("");
  }

  title = get_new_title(level-1, pl);

  say( name_of_player + " hat jetzt Stufe " + level + " erreicht.\n");
  pl = find_player(uid);
  pl->SetProp( P_LEVEL, level );

  if (!mappingp(ts=pl->Query(P_GUILD_TITLE)))
      ts=([]);
  ts["abenteurer"]=title;
  pl->Set(P_GUILD_TITLE,ts);
  if (!IS_SEER(pl))
    pl->SetProp(P_TITLE,0);

  title=pl->QueryProp(P_TITLE);
  // Aufstiegs-Event ausloesen
  EVENTD->TriggerEvent(EVT_LIB_ADVANCE, ([
	E_OBJECT: pl, E_PLNAME: getuid(pl),
	E_ENVIRONMENT: environment(pl), 
	E_GUILDNAME: pl->QueryProp(P_GUILD),
	P_LEVEL: pl->QueryProp(P_LEVEL),
	P_TITLE: pl->QueryProp(P_TITLE) ]) );

  if (level < 7) {
    write("Du bist jetzt " + name_of_player + " " + title +
      " (Stufe " + level + ").\n");
    return 1;
  }
  if (level < 14) {
    write("Gut gemacht, " + name_of_player + " " + title +
      " (Stufe " + level + ").\n");
    return 1;
  }
  write("Willkommen auf Deiner neuen Stufe.\n" +
    "Du bist jetzt " + name_of_player+" "+ title +
    " (Stufe " + level + ").\n");

  return 1;
}

static int compare( mixed *i, mixed *j )
{
  if( i[2] == j[2] )
    return i[1] > j[1];
  else
    return i[2] > j[2];
}

static string asText( int max, int dontneed )
{
  if( !dontneed ) return "muessen alle";
  switch( max-dontneed )
  {
    case 0: return "muss keine";
    case 1: return "muss eine";
    default:
      return sprintf( "muessen %d", max-dontneed );
  }
}

varargs int liste(mixed pl)
{
  string str; 
  if (!this_player()) return 0;

  if(!objectp(pl))
    if(stringp(pl))
      pl=find_player(pl)||find_netdead(pl);
  if(!objectp(pl))
    pl=PL;
  if (pl != this_player()) {
    write ("Du kannst Dir nur Deine eigenen Abenteuer ansehen.\n");
    return 1;
  }

  str = "";
  str += QM->liste(pl);
  
  if (!IS_SEER(pl))
    str+=LEPMASTER->QueryReadyForWizText(pl);

  this_player()->More( str, 0 );
  return 1;
}
