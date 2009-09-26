// MorgenGrauen MUDlib
//
// money.c -- Unser Geld
//
// $Id: money.c 6962 2008-08-12 18:27:41Z Zesstra $


inherit "std/unit";

#pragma strong_types

#include <properties.h>
#include <language.h>
#include <defines.h>
#include <unit.h>

// Prototypes
static string DetKonterfei();
static string DetRand();

static int action_pruefe(string str);
static int action_schnipps(string str);
static int action_wurf(string str);

string current_long();

// Logg-Funktion, um Nicht-Standard-Geldbugs zu finden.
// Eingebaut 27.03.2000 - Tiamak
varargs int move( mixed dest, int method )
{
    object ob;

    if ( (!environment() && previous_object() &&
          !query_once_interactive(previous_object()) &&
          BLUE_NAME(previous_object()) != "/obj/money") &&
         (ob = find_object("/p/daemon/moneylog")) )
        ob->AddMoney( previous_object(), Query(U_REQ) );

    return ::move( dest, method );
}


void _set_autoloadobj( mixed args )
{
  if (pointerp(args))
    args=args[0];
  if (stringp(args))
    sscanf(args,"%d",args);
  if (!intp(args))
    args=0;
  SetProp(P_AMOUNT,args);
}

int _query_autoloadobj()
{
  return (int) Query(P_AMOUNT);
}

void create()
{
  mixed plist, i;

  ::create();
  SetProp(P_NAME,({"Muenze","Muenzen"}));
  SetProp(P_LONG,"@@current_long@@");
  SetProp(P_SHORT,"Geld");
  SetProp(P_INFO,"Du hast bei diesem Geld ein sicheres Gefuehl.\n");
  SetProp(P_MATERIAL,([MAT_GOLD:100]));
  AddSingularId("muenze");
  AddPluralId("muenzen");
  AddSingularId("goldstueck");
  AddPluralId("goldstuecke");
  AddId(({"geld", "\ngeld"}));
  SetProp(P_GENDER,FEMALE);
  SetCoinsPerUnits(1,1);
  SetGramsPerUnits(1,4);

  if (!clonep(this_object())) {
    set_next_reset(-1);
    return;
  }
  AddDetail(({"rand","muenzrand"}),#'DetRand);
  AddDetail(({"oberflaeche","flaeche"}),break_string(
   "Diese Muenze ist schon durch viele Haende gegangen. Die Kratzer "
   "zeugen davon, dass sie nicht selten allein getragen wurde und die "
   "Bissspuren von mangelndem Vertrauen.",78));
  AddDetail("kratzer",break_string(
   "Das Konterfei ist schon ganz schoen zerkratzt.",78));
  AddDetail(({"bissspur","bissspuren"}),break_string(
   "Da hat doch tatsaechlich jemand so wenig Vertrauen in das Gold "
   "gezeigt, dass er hineingebissen hat, um es zu pruefen.",78));
  AddDetail(({"bild","konterfei","praegung"}),#'DetKonterfei);
  AddDetail("gold","Es glaenzt Dich verheissungsvoll an.\n");

  AddCmd(({"pruef","pruefe","beiss","beisse","test","teste"}),
         #'action_pruefe);
  AddCmd(({"schnipp","schnippe","schnipps","schnippse"}),
         #'action_schnipps);

  AddCmd( ({ "wirf", "werf", "werfe" }), #'action_wurf);
}

int IsEqual(object ob)
{
   if ( BLUE_NAME(ob) == "/obj/boerse" || BLUE_NAME(ob) == "/obj/seercard" )
       return 1;

   return ::IsEqual(ob);
}

string current_long() {
 int r;
 r=Query(U_REQ);
 return(break_string(
  ((r==1)?"":"Du betrachtest eine der Muenzen genauer:\n")+
  "Die Oberflaeche der Goldmuenze ist vom taeglichen Gebrauch ziemlich "
  "mitgenommen. Kratzer und sogar Bissspuren zieren die Praegemuenze "
  "zusaetzlich zum Konterfei des "+
  ({"Jof","Rumata","Zook"})[(Query(P_AMOUNT)+r)%3]+
  ". Der Rand "+
  ((!(r%2))?"scheint etwas beschaedigt zu sein.":
            "ist wenigstens unbeschaedigt."),
  78,0,BS_LEAVE_MY_LFS));
}

static int action_schnipps(string str) {
 if(!id(str)) return 0;
 write(break_string(
  ((Query(U_REQ)==1)?"Du nimmst die Muenze":
                     "Du nimmst eine der Muenzen")+
  " und schnippst sie in die Luft. Sie klingt hell auf. "+
  ((this_player() && this_player()->QueryAttribute(A_DEX)>10)?
   "Geschickt faengst Du sie wieder auf.":
   "Sie plumpst Dir wieder in die Hand und beinahe laesst Du "
   "sie fallen."),78));
 if(this_player() && environment(this_player()))
  tell_room(environment(this_player()),
            break_string(this_player()->Name()+" schnippt mit hellem "
                         "Klingen ein Muenze in die Luft und faengt sie "
                         "wieder.",78),
            ({this_player()}));
 return 1;
}

static int action_pruefe(string str) {
 if(!id(str)) return 0;
 write(break_string(
  ((Query(U_REQ)==1)?"Du nimmst die Muenze":
                     "Du nimmst eine der Muenzen")+
  " und beisst vorsichtig hinein. Dein Zahn versinkt tief im reinen Gold, "
  "und Du bist Dir nun sicher, dass diese Muenze auch wirklich echt ist.",
  78));
 if(this_player() && environment(this_player()))
  tell_room(environment(this_player()),
            break_string(this_player()->Name()+" beisst in eine Muenze "
                         "und schaut irgendwie befriedigt drein.",78),
            ({this_player()}));
 return 1;
}


static string DetKonterfei() {
 string m;
 switch((Query(P_AMOUNT)+Query(U_REQ))%3) {
  default:
  case 0:
        m="Jofs rundes Gesicht grinst Dich vertrauenserweckend und "
          "freundlich an.";break;
  case 1:
        m="Rumata scheint Dir zuzublinzeln.";break;
  case 2:
        m="Zooks Gesicht schaut ziemlich frisch gepraegt drein. Wenn "
          "eine der Muenzen beim Schnippen klingt, dann bestimmt die.";break;
 }
 return(break_string(m,78));
}


static string DetRand() {
 return(break_string(
  ((!(Query(U_REQ)%2))?
            "Der Rand der Muenze ist zerklueftet und abgeschabt. Ob "
            "da jemand versucht hat, das Gold der Muenze abzuschaben, "
            "um sich etwas nebenher zu verdienen?":
            "Der Rand ist vollkommen in Ordnung, ein wirklich frische "
            "Praegung."),78));
}


static int action_wurf(string str)
{
  int num, part;

  if (!id(str))
    return 0;

  num = Query(U_REQ);
  write (break_string(
	 ((num==1)?"Du nimmst die Muenze":
	  sprintf("Du nimmst %d Muenzen", num))+
	   ", wirfst sie hoch, laesst sie zu Boden prasseln und "
	   "betrachtest das Ergebnis:", 78));
  if (this_player() && environment(this_player()))
    tell_room(environment(this_player()),
	      break_string(this_player()->Name()+" wirft "+
         	   ((num==1)?"eine Muenze":
			   sprintf("%d Muenzen", num))+
		   " hoch, laesst sie zu Boden prasseln und betrachtet "
		   +"das Ergebnis:", 78), ({this_player()}));

  if (num == 1) {
    if (random(1000000) == 123456) {
      write("   Die Muenze bleibt auf dem Rand stehen.\n");
      if (this_player() && environment(this_player()))
	tell_room(environment(this_player()),
		  "   Die Muenze bleibt auf dem Rand stehen.\n",
		  ({this_player()}));
    }
    else if (random(100) < 50) {
      write("   Kopf.\n");
      if (this_player() && environment(this_player()))
	tell_room(environment(this_player()),
		  "   Kopf.\n",
		  ({this_player()}));
    }
    else {
      write("   Zahl.\n");
      if (this_player() && environment(this_player()))
	tell_room(environment(this_player()),
		  "   Zahl.\n",
		  ({this_player()}));
    }
  }
  else {
    part = 5*num + random(num+1) - random(num+1) // etwas Gauss-verteilen
		 + random(num+1) - random(num+1)
		 + random(num+1) - random(num+1)
		 + random(num+1) - random(num+1)
		 + random(num+1) - random(num+1);
    part = (part+5)/10;

    if (part) {
      printf("   %dx Kopf", part);
      if (part != num)
	printf(" und %dx Zahl", num-part);
      if(this_player() && environment(this_player()))
	tell_room(environment(this_player()),
		  sprintf("   %dx Kopf", part)+
		  ((part!=num)?sprintf(" und %dx Zahl", num-part):"")+
		  ".\n",
		  ({this_player()}));
    }
    else {
      printf("   %dx Zahl", num);
      if (this_player() && environment(this_player()))
	tell_room(environment(this_player()),
		  sprintf("   %dx Zahl.\n", num),
		  ({this_player()}));
    }
    write (".\n");
  }

  write(break_string(
	"Schnell sammelst Du "+
	((num==1)?"Deine Muenze":"Deine Muenzen")+" wieder ein.", 78));
  if (this_player() && environment(this_player()))
    tell_room(environment(this_player()),
	      break_string(this_player()->Name()+ " sammelt schnell "+
	      ((num==1)?(this_player()->QueryPossPronoun(FEMALE,WEN, SINGULAR)
			 +" Muenze wieder ein."):
	       (this_player()->QueryPossPronoun(FEMALE,WEN, PLURAL)+
		" Muenzen wieder ein."))), ({this_player()}));
  return 1;
}


varargs string GetDetail(mixed key,mixed race,int sense) {
 string m;
 if(stringp(m=::GetDetail(&key,&race,&sense)) && strlen(m)) {
  int r;
  r=Query(U_REQ);
  if(r!=1) m=sprintf("Du betrachtest eine der %d Muenzen genauer:\n",r)+m;
 }
 return m;
}

