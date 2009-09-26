// Ich fand es ein wenig langweilig in der Gilde, also habe ich ein wenig daran
// gebastelt.
//  Hate.
// 2002-12-06 Mandragons Projektbrett eingebaut, Humni.
// 2003-07-11 Bugfix
// 2005-08-19 Miril Arks Bienen eingebaut

inherit  "/std/gilden_room";

#include <properties.h>
#include <rooms.h>
#include <wizlevels.h>
#include <language.h>
#include <defines.h>
#include <moving.h>
#include <new_skills.h>

#define P_ANFAENGER_BIENENORT_ "anfaenger_bienenort_"
#define P_ANFAENGER_BIENENZIEL_ "anfaenger_bienenziel_"

string Funktion(); // Funktion der Gilde
string Schmutz();  // Wohl bald ein Zaubertrank

int _query_light() {return 999999999;}

void create() {
  ::create();

  SetProp(P_GUILD_DEFAULT_SPELLBOOK,"ab_spells");
  SetProp(P_LIGHT,999999999);
  AddSpell("licht");
  AddSpell("kampfschrei");
  AddSpell("ausweichen");
  AddSpell("schaetz");
  AddSpell("pfeil");
  AddSpell("identifiziere");
  AddSpell("feuerball");
  AddSpell("schnell");
  AddSpell("saeurestrahl");

  SetProp(P_INT_SHORT,"Die beruehmte Abenteurergilde");
  SetProp(P_INT_LONG,
 "Der grosse Raum mit seiner niedrigen Decke und den grob geschnittenen\n"
+"Querbalken hat Platz fuer sehr viele Personen. Knarrende Dielen erzaehlen\n"
+"ueber Heldentaten laengst vergessener Abenteurer, beruehmter als mancher\n"
+"Weise unserer Zeit.\n"
+"Abenteurer, aber auch andere Bewohner dieser Welt kommen hierher, um\n"
+"sich zu informieren, ihre Erfahrungen auszutauschen oder sich in den\n"
+"verschiedensten Wissenschaften zu verbessern.\n"
+"Durch die immer offene Tuer scheint die aufgehende Sonne herein und fuellt\n"
+"den Raum mit ihrem goldenen Licht. Sonst gibt es hier nur noch den Ausgang\n"
+"im Norden, welcher auf die Hafenstrasse fuehrt und Bilder an den Waenden,\n"
+"mit Zeichnungen und Gemaelden von MorgenGrauen. Eins der Bilder erlaeutert\n"
+"die Funktion der Gilde und eine grosse Uhr an der Wand zeigt, was die\n"
+"Stunde schlaegt. An einer anderen Wand haengt eine steinerne Tafel und in\n"
+"einer Ecke steht ein kleines Projektbrett.\n"
+"Nach oben fuehrt eine Treppe in das Buero des Foerdervereins Projekt\n"
+"MorgenGrauen e.V., das Du unbedingt besuchen solltest.\n");

  AddDetail(({"raum", "gilde", "abenteurergilde", "haus", "gildenhaus",
              "gildehaus"}),
  "Du stehst mitten in der Abenteurergilde. Hier trifft man sich und kann\n"
 +"auch seine persoenlichen Werte verbessern.\n");
  AddDetail(({"tuer", "ausgang"}),
  "Der immer offenstehende Ausgang fuehrt hinaus auf die Hafenstrasse.\n"
 +"Goldenes Licht einer ewig aufgehenden Sonne scheint herein.\n");
  AddSpecialDetail(({"bild", "bilder", "zeichnungen", "gemaelde", "karten",
              "landschaftsbilder"}),"det_bild");
  AddDetail(({"funktion", "schild"}), #'Funktion);
  AddDetail(({"wand", "waende"}),
  "Aus fein saeuberlich geschlagenen Steinen sind die Waende der Gilde\n"+
 "zusammengesetzt. Viele interessante Bilder und eine Liste haengen daran.\n");
  AddDetail(({"stein", "steine"}), "Es sind ganz einfache Sandsteine.\n");
  AddDetail(({"fenster"}),
  "Eine wundervolle Welt eroeffnet sich Dir, genannt Realitaet. Aber gibt es\n"
 +"diese wirklich? Ist sie nicht voller Gefahren und Stress?\n");
  AddDetail(({"seite", "suedseite"}),
  "Am Suedende des Raumes ist ein eigenartiges Fenster auszumachen.\n");
  AddDetail(({"balken", "querbalken", "holzbalken", "deckenbalken"}),
  "Diese Balken halten das Obergeschoss des Gildenhauses, wohin nur Magier\n"
 +"kommen und sind an manchen Stellen ganz schoen durchgebogen. Das grob ge-\n"
 +"schnittene Holz ist teilweise von Schwertstreichen zernarbt.\n");
  AddDetail(({"hafen", "hafenstrasse", "strasse"}),
  "Zur Tuer hinaus gelangt man ueber die Hafenstrasse zum Hafen von Port Vain"
 +"\n");
  AddDetail(({"boden", "fussboden", "dielen"}),
  "Die knarrenden Dielen der Gilde sind schon alt und abgeschabt. In den\n"
 +"Ritzen zwischen ihnen ist viel Dreck von den Stiefeln der Abenteurer.\n");
  AddDetail(({"decke"}),
  "Die Decke des Raumes wird von Holzbalken gehalten.\n");
  AddDetail(({"licht", "sonnenlicht", "sonne"}),
  "Warmes Sonnenlicht scheint durch den Ausgang zur Strasse herein. Jedesmal\n"
 +"wenn jemand hereinkommt wird er vom Licht umspuelt und erscheint Gott-\n"
 +"gleich mit dieser Aura.\n");
  AddDetail(({"ebene"}),
  "Du betrachtest das Bild der Ebene und denkst, welche Schaetze in dieser\n"
 +"flachen, gruenen Landschaft verborgen sein muessen.\n");
  AddDetail(({"gebirge", "huegel", "berge", "berg"}),
  "Das Bild der Huegel und Berge betrachtend, schleicht sich ein wenig Sehn-\n"
 +"sucht nach dieser herrlichen, aber gefaehrlichen Landschaft ein.\n");
  AddDetail(({"perspektiven", "perspektive", "darstellung", "landschaft"}),
  "Manche der Landschaftsbilder sind aus der Vogelperspektive oder von\n"
 +"hohen Bergen aus oder von Zwergen gemalt worden.\n");
  AddDetail(({"see", "meer", "wellen"}),
  "Die Wellen auf dem Meer glitzern wie tausend Edelsteine.\n");
  AddDetail(({"spritzer", "lichtspritzer"}),
  "Wie tausend kleine Edelsteine verteilen sich diese winzigen, gluehenden\n"
 +"Lichtspritzer ueber das wundervolle Gemaelde\n");
  AddDetail(({"geschoss", "obergeschoss"}),
  "Ins obere Geschoss der Gilde kommen nur Magier, mittels eines besonderen\n"
 +"Zauberspruchs, der die Decke durchlaessig fuer sie macht.\n");
  AddDetail(({"ritzen", "fugen"}),
  "In den Fugen zwischen den Dielen befindet sich eine Menge Schmutz und\n"
 +"Dreck von den Besuchern der Gilde.\n");
  AddDetail(({"fugen", "dreck", "schmutz"}), #'Schmutz);
  AddDetail(({"holz"}), "Die Dielen und die Deckenbalken sind aus Holz.\n");
  AddDetail(({"kartograph", "kartographen"}),
  "Dieselben laufen hier zwar im Moment nicht herum, aber ihre Karten sehen\n"
 +"ziemlich verwirrend aus mit den ganzen Linien, welche diese Welt "
  "darstellen.\n");
  AddDetail(({"stellen", "stelle"}),
  "Nichts sieht aus wie das andere, und die Ecken sind da keine Ausnahmen.\n");
  AddDetail(({"ecken","ecke"}),
  "In einer der Ecken, gleich neben der Treppe, steht ein kleines braunes\n"
 +"Holzbrett, das sofort Deine Aufmerksamkeit erregt. Wozu es wohl dient?\n");
  AddDetail("aufmerksamkeit",
  "Du beschliesst, Dir das Brett sofort naeher zu betrachten, um sie zu\n"
      +"befriedigen.\n");
  AddDetail(({"schwarzes brett","holzbrett"}),
  "Ein unscheinbares kleines Brett steht in der Ecke des Raumes. Wenn man\n"
 +"bedenkt, wie viele kleine Zettel daran befestigt wurden, kann es sich\n"
 +"nur um ein schwarzes Brett handeln. Welche Informationen hier wohl\n"
 +"ausgetauscht werden? Du koenntest natuerlich anfangen, die Zettel zu\n"
 +"lesen, um das herauszufinden ...\n");
  AddDetail("zettel",
  "Viele verschiedene Zettel stecken an dem Brett. Rote, gelbe, blaue,\n"
 +"jeder scheint eine andere Farbe zu haben. Auf jedem von ihnen scheint\n"
 +"etwas geschrieben zu sein.\n");
  AddDetail(({"farbe","farben"}),
  "Es sind so viele, Du wuerdest bestimmt jahrelang zaehlen.\n");
  AddDetail("informationen",
  "Wenn Du die Zettel liest, findest Du bestimmt welche.");
  AddDetail(({"wissenschaften", "eigenschaften"}), #'Funktion); //');
  AddDetail(({"narben", "schwertstreich"}),
  "Die Narben in den Holzbalken sind schon recht alt. Es ist lange her, dass\n"
   +"hier richtig gekaempft wurde.\n");
  AddDetail(({"aura"}),
 "Eine Aura von Licht umgibt jeden, der von der Strasse ins Haus kommt.\n");
  AddDetail(({"bewohner", "abenteurer"}),
 "Hier, im Gildehaus, ist jeder gern willkommen.\n");
  AddDetail(({"weise", "weiser", "recken"}),
  "Die Weisen dieses Landes sind beruehmt, kommen aber nicht an die Be-\n"
  +"ruehmtheit ihrer Vorfahren, die grossen Recken der ersten Jahre, heran\n");
  AddDetail(({"realitaet"}), "Was ist schon real?\n");
  AddDetail(({"tafel","steintafel"}),
(["mensch":"\
Auf dieser Tafel sind die Namen der heldenhaftesten und tapfersten Menschen\n\
festgehalten, die dieser Ort hervorgebracht hat. Moege ihr Ruhm Dir zum Vor-\n\
bild gereichen. Du kannst die Tafel lesen.\n",
  0:"\
Hier sind die Namen der heldenhaftesten und tapfersten Menschen festgehalten,\n\
die Port Vain hervorgebracht hat. Du kannst die Tafel lesen.\n"]));

AddDetail(({"buero","foerderverein"}),
"Wichtige Informationen warten auf Dich im Buero des Foerdervereins!\n");
AddDetail("informationen","Du musst schon ins Buero gehen, wenn Du die Informationen erhalten willst.\n");
AddDetail("treppe","Die Treppe fuehrt nach Oben in das Buero des Foerdervereins Projekt MorgenGrauen e.V.\n");
 AddDetail(({"liste", "topliste"}), "Du kannst die Liste lesen.\n");
 AddReadDetail(({"liste", "topliste"}), "@@topliste@@");

 AddExit("norden", "/d/ebene/PortVain/konstrukt");

  AddCmd(({"treff"}), "GotoMagierTreff");
  AddCmd(({"lies","lese","les"}),"lesen");


  AddCmd("gehe|geh&\ngehtnicht",0,"Um Dich hier fortzubewegen, "
      "solltest Du direkt das Kommando der Richtung eingeben,"
      " wie ,,norden'' oder ,,suedwesten''. Du kannst das auch "
      "abkuerzen, durch ,,n'' oder ,,sw''.");



  AddItem("/obj/zeitungsautomat", REFRESH_REMOVE);

  AddSpecialDetail("karte","karte");
  AddSpecialDetail("brett","brett_fun");

  AddItem("/p/service/mandragon/projektbrett/board", REFRESH_REMOVE);

}

string det_bild()
{
  return
   "Die Zeichnung und Gemaelde stellen MorgenGrauen aus den verschiedensten\n"
   "Perspektiven dar. Einige davon zeigen abstrakte Darstellungen, wie sie\n"
   "gerne von Kartographen gemacht werden, und andere zeigen wunderschoene\n"
   "Darstellungen von Landschaften, wie dem Gebirge oder der Ebene. Besonders\n"
   "schoen ist ein Bild der See. Tausende von Lichtspritzern verteilen sich\n"
   "ueber die Wellen.\n"
   "Ein kleines Schild zeigt folgendes:\n"+Funktion();
}

string brett_fun()
{
  if (present("party\nbrett",this_object()))
    return "Welches Brett meinst Du denn? Das Partybrett oder das Holzbrett?\n";
  return GetDetail("holzbrett");
}

int beitreten()
{
  if (::beitreten()<1)
    return 1; // Meldung vom Gildenmaster oder Gildenobjekt
  write("Du bist jetzt ein Abenteurer.\n");
  return 1;
}

varargs int austreten(int loss)
{
  if (::austreten()<1)
    return 1; // Meldung kommt vom Gildenmaster
  write("Du bist nun kein Abenteurer mehr.\n");
  return 1;
}

string Funktion()
{   string re;
  re = ("Moegliche Befehle sind:\n"
        +"                liste\n"
        +"                erhoehe (stufe)\n"
        +"                kosten\n"
        +"                lerne\n");
  if (this_player()->QueryProp(P_GUILD)!="abenteurer")
  re += ("                trete in gilde ein\n"+
         "                trete aus gilde aus\n");
  return re;
}

string Schmutz()
{
  // hier sollte sich doch glatt ein Zaubertrank lohnen!!
  return "In den Fugen befindet sich viel Schmutz.\n";
}

int GotoMagierTreff()
{
  if(IS_LEARNER(this_player()))
  {
    write("Ein Zauberspruch zieht vor Deinem geistigen Auge vorbei und Du\n"
    +"sprichst ihn nach.\n");
    say(PL->name()+" murmelt einen geheimen Zauberspruch und schwebt langsam\n"
  +"zur Decke hinauf und verschwindet durch die Wand.\n");
    write("Du schwebst langsam zur Decke hinauf und als ob diese nicht da\n"
    +"waere mitten hindurch in den Magiertreff.\n");
    return (PL->move("/secure/merlin", M_TPORT | M_SILENT ) >= 0);
  }
  write("Du springst zur Decke hinauf und nix passiert.\n");
  return 1;
}

int lesen(string str)
{
  if (str=="tafel"||str=="steintafel"||str=="steinerne tafel")
  {
    this_player()->More("/etc/humanlist.read",1);
    return 1;
  }
  return ::lies(str);
}

void adjust_title(object pl) {
  int lv;

  // bei P_LEVEL < 0 ist der Spieler noch "nicht fertig".
  // Der Titel wird anschliessend noch korrekt gesetzt.
  if (!pl || !IsGuildMember(pl) || (lv=pl->QueryProp(P_LEVEL)) < 0)
    return;

  pl->SetProp(P_GUILD_LEVEL,lv);
  if (!lv) lv=1;
  pl->SetProp(P_GUILD_TITLE,get_new_title(lv-1,pl));
}

string karte() {
  if (this_player() && present("karte",this_player())) return 0;
  return GetDetail("karten");
}

void init() {
  int res;
  object ob;

  ::init();
  if ((res=set_light(0))<=0)
    set_light(1-res);

  // Von Ark, an alle Rassenstartpunkte
  if (ob=present("\nentdecker_obj",this_player()))
	  ob->Check_And_Update(this_player(),"Menschen");
}

string topliste()
{
   this_player()->More("/etc/abenteurerlist.read", 1);
   return "";
}
