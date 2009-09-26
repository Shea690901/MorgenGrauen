/*****************************************************************************
** Der Paketschalter fuer die Paketpost
** (C) 1994-1996 Loco@Morgengrauen
** Kopien in andere Muds, auch veraendert, sind untersagt, ausser mit
** ausdruecklicher Genehmigung des Autors.
** Dies gilt fuer alle Objekte des Paketpostsystems.
******************************************************************************
** An diesem Schalter kann ausserdem ein Nachsendeauftrag gestellt oder
** geloescht werden. Die hierzu gehoerenden Codesequenzen unterliegen
** dem Copyright wie in /mail/mailer.c und duerfen fuer andere Muds
** extrahiert und in andere Objekte eingebaut werden.
** Sie sind zum leichteren Auffinden mit * FORWARD * gekennzeichnet.
******************************************************************************/

// Feigling 20.05.2004 - ENV-Bug, wenn der Spieler, waehrend des Briefmarken
//                       holen, weg geht 

#include "/mail/post.h"
#include <properties.h>
#include <language.h>
#include <moving.h>
#include <bank.h>

inherit "/std/thing";
inherit "/std/more";

static int postler;

int EarnedMoney = 0;
int desc_check();

static string my_long();

mapping stamps;

#define POSTLER  postler   /* isser da? */
#define M_NR 0
#define M_SIZE 1
#define M_PRICE 2
#define M_MOTIV 3
#define LEAVETIME 20
#define STAMPSAVEFILENAME (STAMPSAVEPATH+implode(explode(object_name(environment(this_object())),"/")[1..],"."))
#define BESTIMMUNGEN "/p/service/loco/doc/tarife"
#define EMPTY_PRICE 15
#define DEBUGVAR(x) tell_object(find_player("loco"),sprintf("Value of x is %O\n",x))

/* FORWARD */
#define IS_FORWARD(b) (b=="auftrag"||b=="nachsendeauftrag"||b=="forward")

void create() 
{
  if (!clonep(this_object())) return;
  (::create());
  seteuid(getuid());
  AddId("schalter");
  AddId("paketschalter");
  AddId("postschalter");
  SetProp(P_NAME,"Schalter");
  SetProp(P_GENDER,1);
  SetProp(P_NOGET,1);
  SetProp(P_SHORT,"Ein Paketschalter");
  AddCmd(({"kauf","kaufe"}),"BuyStamp");
  AddCmd(({"frag","frage"}),"AskClerk");
  AddCmd(({"zeige","zeig"}),"ShowStamps");
  AddCmd(({"wiege","wieg"}),"WeighParcel");
  AddCmd(({"sende","send","schicke","schick","versende","versend","verschicke","verschick"}),"SendParcel");
  AddCmd(({"lies","les","lese"}),"ReadTarifs");

  /* FORWARD */
  AddCmd(({"stelle","stell"}),"AddForward");
  AddCmd(({"loesche","loesch"}),"DeleteForward");

  stamps  = ([]);
  postler = 1;
  
  call_out("desc_check",0);
}

void init() 
{
  (::init());
//  DEBUGVAR(this_player());
  PARCELSERVER->DeliverAllParcels(this_player());
}

int move(mixed dest, int method, string dir, string textout, string textin) 
{
  int ret;
  
  ret = (::move(dest,method));
  
  if (environment(this_object()) &&
      sizeof(old_explode(object_name(environment(this_object())),"#")) == 1)
  {
    restore_object(STAMPSAVEFILENAME);
  }
  return ret;
}

int desc_check()
{
  if(!QueryProp(P_SHORT))
  {
    SetProp(P_SHORT,"Ein Paketschalter");
  }
  if(!QueryProp(P_LONG))
  {
    //Set(), nicht SetProp(). Zesstra. ;-)
    //SetProp(P_LONG,#'my_long,F_QUERY_METHOD);
    Set(P_LONG,#'my_long,F_QUERY_METHOD);
  }
  return 1;
}

static string my_long()
{
  return(break_string(
   "Dies ist ein Paketschalter, an dem man Pakete an andere Spieler aufgeben "
   "und seine eigenen abholen kann. Es werden auch Briefmarken und leere "
   "Pakete verkauft. Die Tarif- und Befoerderungsbestimmungen fuer "
   "Paketsendungen haengen aus, Du kannst sie lesen. Wenn Du Dich fuer "
   "Briefmarken interessierst, zeigt Dir der Postbeamte gerne die Liste der "
   "vorraetigen Postwertzeichen, frag ihn doch mal nach Marken. Mit 'wiege "
   "paket' wird der Beamte Dir sagen, was das Paket kostet, und mit 'versende "
   "paket' bzw. 'versende kurierpaket' kannst Du ein Paket verschicken. "
   "Natuerlich muss das Paket ausreichend frankiert und adressiert sein. "
   "Ausserdem kannst Du hier Nachsendeauftraege stellen und loeschen, frag "
   "dafuer den Beamten nach Auftrag. "
  +(POSTLER ? "Der Postbeamte sitzt hinter der Glasscheibe und wartet.":
	      "Der Postbeamte scheint gerade nicht da zu sein. Er kommt aber "
	     +"wahrscheinlich gleich wieder."),78));
}

/* 
 * FORWARD 
 */

int AddForward(string str) 
{
  if (!this_player() || !this_interactive()) return 0;
  if (!IS_FORWARD(str)) return

  notify_fail("Willst Du einen Nachsendeauftrag stellen oder was anderes?\n"),0;
  write(FWSERV->AddForward(geteuid(this_player()))+"\n");

  return 1;
}

/* 
 * FORWARD 
 */

int DeleteForward(string str) 
{
  if (!this_player() || !this_interactive()) return 0;
  if (!IS_FORWARD(str)) return

  notify_fail("Willst Du einen Nachsendeauftrag loeschen oder was anderes?\n"),0;
  write(FWSERV->DeleteForward(geteuid(this_player()))+"\n");

  return 1;
}

int SendParcel(string str) 
{
  string *args;
  int i,weight,price;
  object parcel,to;
  
  if (!str || (i=sizeof(args=explode(str," ")))>2 ||
      (i==1 && !(parcel=present("postpaket",this_player()))) ||
      (i==2 && !(parcel=present("postpaket "+args[1]))) ||
      (args[0]!="paket" && args[0]!="postpaket" && args[0]!="expresspaket" &&
       args[0]!="schnellpaket" && args[0]!="kurierpaket"))
    return notify_fail("Was willst Du denn verschicken? Ein Paket oder ein Kurierpaket vermutlich?\n"),0;
  if (!parcel->QueryAdress()) return write("Es fehlt noch die Adresse.\n"),1;
  if (parcel->QueryStampsUsed()) return write("Die Marken sind schon gestempelt. Du solltest neue draufkleben."),0;
  weight=parcel->QueryProp(P_WEIGHT);
  
  /* Postlagernde Pakete */
  if (args[0]=="paket"||args[0]=="postpaket") {
    price=PARCELSERVER->QueryNormalPrice(weight);
    if (!PARCELSERVER->QueryNormalAllowed(parcel)) return
      write("Leider konnte diese Sendung nicht angenommen werden.\n"),0;
    if (price>(i=parcel->QueryStampValue())) return
      write("Das Paket ist unterfrankiert. Es fehlen noch "+(price-i)+" Muenzen.\n"),1;
    parcel->StampThem(environment(this_object())->QueryProp(P_INT_SHORT));
    parcel->SetSender(geteuid(this_player()));
    tell_room(environment(this_player()),this_player()->name()+" gibt ein Paket auf.\n",({this_player()}));
    tell_room(environment(this_player()),"Der Postbeamte nimmt das Paket und stellt es ins Lager.\n");
    if (to=find_player(parcel->QueryAdress()))
      tell_object(to,"Eine Brieftaube flattert vorbei und teilt Dir mit, dass auf dem Postamt\nein Paket fuer Dich angekommen ist.\n");
    PARCELSERVER->StoreParcel(parcel);
    return 1;
  }
  
  else {
    if (!price=PARCELSERVER->QueryExpressPrice(weight)) return
      write("Der Postbeamte schuettelt den Kopf: Das Paket ist leider zu schwer.\n"),1;
    if (!(2&PARCELSERVER->QueryAllowed(parcel))) return
      write("Leider konnte diese Sendung nicht angenommen werden.\n"),0;
    if (price>(i=parcel->QueryStampValue())) return
      write("Das Paket ist unterfrankiert. Es fehlen noch "+(price-i)+" Muenzen.\n"),1;
     if (! (to=find_player(parcel->QueryAdress())) || !interactive(to) || !query_ip_number(to)) return
      write("Der Postbeamte macht Dich darauf aufmerksam, dass "+capitalize(parcel->QueryAdress())+"\n	 im Moment nicht anwesend ist. Das Paket wird nicht angenommen.\n"),1;
    if (!PARCELSERVER->QueryRoomAllowed(environment(to))) return
      write("Der Postbeamte bedauert: "+to->name()+" befindet sich leider im Moment\n\
   in einem Raum, den unsere Kuriere nicht erreichen koennen.\n"),1;
    parcel->SetSender(geteuid(this_player()));
    parcel->StampThem(environment(this_object())->QueryProp(P_INT_SHORT));
    parcel->move(environment(to));
    tell_room(environment(this_player()),this_player()->name()+" gibt ein Kurierpaket auf.\n");
    tell_room(environment(this_player()),"Der Postbeamte nimmt das Paket, ruft einen Kurier herbei, und der rast davon.\n");
    tell_object(to,"\
Ein Kurier ruft Dir zu: Ich habe ein eiliges Paket fuer Dich!\n\
Der Kurier rast in den Raum, stellt das Paket direkt vor Deine Fuesse,\n\
murmelt noch irgendetwas wie \"Gruss von "+(this_player()->name())+"\" und verschwindet\n\
wieder.\n");
    tell_room(environment(to),"Ein Kurier rast herein und stellt ein Paket fuer "+capitalize(parcel->QueryAdress())+" in den Raum.\n",({to}));
    return 1;
    
  }
}

int ReadTarifs(string str) 
{
  if (str != "bestimmungen" && str != "tarife" && str != "tarifbestimmungen")
  {
    return 0;
  }
  More(read_file(BESTIMMUNGEN));
  return 1;
}

int QueryClerk() { return POSTLER; }

mixed GetStampList() 
{
  mixed versand;

  versand = STAMPSERV->QueryStampList();

  return versand+(m_indices(stamps)-versand);
}


int WeighParcel(string str) {
  object ob;
  int i,w,p;
  if (!str || !ob=present(str,this_player())) 
    return notify_fail("So was find ich hier nicht. Was willst Du wiegen lassen?\n"),0;
  if (!ob->id("postpaket")) return notify_fail("Der Postbeamte schuettelt den Kopf: Das ist doch kein Postpaket!\n"),0;
  write("Der Schalterbeamte stellt das Paket auf die Waage.\n\
Er blaettert in einer Liste, und teilt Dir dann mit:\n");
  if ((i=PARCELSERVER->QueryAllowed(ob)) & 1)
    write("Der Preis als postlagerndes Paket betraegt "+
	  PARCELSERVER->QueryNormalPrice(w=ob->QueryProp(P_WEIGHT))+
	  " Muenzen.\n");
  if (i & 2) 
    write("Der Preis als Kurierpaket betraegt "+
	  PARCELSERVER->QueryExpressPrice(w)+
	  " Muenzen.\n");
  return 1;
}


int AskClerk(string str) {
  string a,b;
  if (!str||sscanf(str,"%s nach %s",a,b)!=2) 
    return notify_fail("Wen willst Du denn wonach fragen?\n"),0;
  if (a!="postler" && a[0..8]!="postbeamt" && a[0..4]!="beamt") 
    return notify_fail("Ist so jemand da?\n"),0;
  if (!POSTLER) 
    return notify_fail("Der ist gerade nicht da.\n"),0;
  if (b[0..4]=="marke" || b[0..9]=="briefmarke")
   return write("\
Der Postbeamte scheint sich zu freuen, dass sich jemand fuer seine Marken\n\
interessiert und erzaehlt Dir, dass er selber auch begeisterter Sammler ist.\n\
Wenn Du wissen willst, was an Briefmarken vorraetig ist, sollst Du Dir\n\
einfach mit 'zeige marken' die Liste zeigen lassen, und mit 'kaufe marke\n\
<wert>' kannst Du eine Marke kaufen. Er versichert Dir noch, wenn der\n\
Bogen der Marke, die Du willst, leer ist, wuerde er selbstverstaendlich\n\
einen neuen Bogen aus dem Lager holen.\n"),1;
  if (b[0..4]=="paket" || b[0..8]=="postpaket")
   return write("\
Der Postbeamte erklaert Dir geduldig: Wenn Du ein Paket verschicken willst,\n\
musst Du erst eines kaufen, wenn Du keins hast. Das kostet "+EMPTY_PRICE+" Muenzen.\n\
Dann musst Du das Paket adressieren (adressiere paket an ...) und fuellen\n\
(stecke <gegenstand> in paket). Mit 'wiege paket' kannst Du fragen, was es\n\
kostet, Briefmarken bekommst Du bei mir, die Du mit 'klebe marke auf paket'\n\
aufkleben kannst. Wenn alles fertig ist, schick das Paket mit 'sende paket'\n\
oder 'sende kurierpaket' ab, je nachdem, welche Versendungsform Du willst.\n\
Man kann die Briefmarken auch wieder abloesen und sammeln.\n"),1;

/* FORWARD */
  if (IS_FORWARD(b))
    return write("\
Der Postbeamte faengt an, zu erlaeutern: Du kannst hier einen Nachsende-\n\
auftrag stellen. Damit hat es folgendes auf sich:\n\
\n\
Deine Post wird dann in Zukunft nicht mehr an Dich direkt ausgeliefert,\n\
sondern an diejenige Adresse, die Du als email-Adresse beim ersten Einloggen\n\
oder mit dem Befehl 'email' angegeben hast. Bitte ueberpruef diese Adresse\n\
erstmal! Wenn sie falsch ist, kann es sein, dass Deine Post verloren geht,\n\
und das willst Du doch bestimmt nicht!\n\
\n\
Die email-Adresse kann auch der Name eines anderen Spielers sein, wenn Du\n\
z.B. als Zweiti Post an Deinen Ersti (oder umgekehrt) weiterleiten willst.\n\
Und noch ein Hinweis: Wir senden nur einmal weiter, die angegebene Adresse\n\
gilt, auch wenn fuer diese wieder ein Nachsendeauftrag existiert!\n\
\n\
Willst Du einen Nachsendeauftrag stellen, so tippe 'stelle auftrag'.\n\
Willst Du Deinen Nachsendeauftrag loeschen, so tippe 'loesche auftrag'.\n\
"),1;

  return notify_fail("Der Postler interessiert sich nur fuer Pakete, Briefmarken und\nNachsendeauftraege.\n"),0;
}

int CloneStamp(string m)
{
  object ob;
  //Aenderung Feigling - Abfrage ob der Spieler da ist vorverlegt
  if (present(this_player(),environment(this_object())))
    {  
      stamps[m]--;
 
      if (!stamps[m])
	{
	  stamps = efun::m_delete(stamps,m);
	}
 
      ob = clone_object(STAMP);
      ob->SetIndex(m);
  
      write("Der Postbeamte gibt Dir die gewuenschte Briefmarke.\n");
      ob->move(this_player(),M_SILENT|M_NOCHECK);
    }
  save_object(STAMPSAVEFILENAME);  
  return 1;
}

void ReturnPostler(string m)
{
  postler = 1;

  tell_room(environment(this_object()),break_string(
   "Der Postbeamte kommt mit einem Bogen Briefmarken in der Hand zurueck "
   "und setzt sich auf seinen Stuhl hinter dem Schalter.",78));
 
  stamps[m] = STAMPSERV->OrderSheet(m);

  CloneStamp(m);
}

int FetchSheet(string m)
{
  write(break_string("Der Postbeamte stellt fest, dass diese Marke gerade "
   "nicht da ist, steht auf und verschwindet in den Hinterraum, um einen "
   "Bogen Briefmarken nachzuholen.",78));
  tell_room(environment(this_object()),"Der Postbeamte steht auf und "
   +"verschwindet in den Hinterraum.\n",({ this_player() }));

  postler = 0;
 
  if(find_call_out("ReturnPostler") == -1)
  {
    call_out("ReturnPostler",LEAVETIME,m);
  }
 
  return 1;
}

int BuyStamp(string str)
{
  string m, s;
  mixed list;
  object ob;
 
  _notify_fail("Was willst Du kaufen?\n");
  
  if (!str) return 0;
  
  _notify_fail("Es ist niemand da, der Dir was verkaufen koennte.\n");

  if (!POSTLER) return 0;
  
  if (str == "paket" || str == "postpaket")
  {
    if (this_player()->QueryMoney() < EMPTY_PRICE)
    {
      write("Der Postbeamte bedauert: Tut mir leid, Du hast nicht genug "
       +"Geld.\n");
      return 1;
    }
 
    this_player()->AddMoney(-EMPTY_PRICE);
    EarnedMoney += EMPTY_PRICE;

    write("Du bezahlst, und der Postbeamte reicht Dir ein leeres Paket.\n");
    tell_room(environment(this_object()),this_player()->Name()+" kauft ein "
     +"Leerpaket.\n",({ this_player() }));
   
    ob = clone_object(PARCEL);
  
    if (ob->move(this_player(),M_GET) <= 0)
    {
      write("Da es so schwer ist, laesst Du es fallen.\n");
      tell_room(environment(this_object()),this_player()->Name()+" laesst "
       +"das Paket fallen.\n",({ this_player() }));
  
      ob->move(environment(this_object()),M_NOCHECK);
    }

    return 1;
  }

  else if (str == "marke" || str == "briefmarke")
  {
    _notify_fail("Syntax: kaufe briefmarke <wert>\n");
    return 0;
  }
 
  else if (sscanf(str,"%s %s",s,m) == 2)
  {
    if (s == "marke" || s == "briefmarke")
    {
      list = GetStampList();
 
      if (member(list,m) == -1)
      {
	_notify_fail("Der Postbeamte bedauert, diese Briefmarke nicht im "
	 +"Angebot zu haben.\n");
	return 0;
      }
  
      else if (this_player()->QueryMoney() < STAMPSERV->QueryStampPrice(m))
      {
	write("Der Postbeamte bedauert: Tut mir leid, Du hast nicht genug "
	 +"Geld.\n");
	return 1;
      }

      write("Du bezahlst, und der Postbeamte sucht die Briefmarke heraus.\n");
      tell_room(environment(this_object()),this_player()->Name()+" kauft "
       +"eine Briefmarke.\n",({ this_player() }));
 
      this_player()->AddMoney(-(STAMPSERV->QueryStampPrice(m)));
      EarnedMoney += (STAMPSERV->QueryStampPrice(m));
    
      if (stamps[m])
      {
	CloneStamp(m);
      }
      else
      {
	FetchSheet(m);
      }
 
      return 1;
    }
  }
  return(0);
}

void reset()
{
  if (!environment()) return;
  ::reset();
  if (EarnedMoney > 0)
  {
    ZENTRALBANK->PayIn(EarnedMoney);
    EarnedMoney = 0;
  }
}

int ShowStamps(string str) 
{
  int i;
  mixed marken;

  if (str != "marken" && str != "briefmarken")
  {
    return notify_fail("Was denn zeigen? Hier gibt es nur Briefmarken zu "
                       +"kaufen.\n"),0;
  }
  if (!POSTLER)
  {
    return notify_fail("Es ist niemand da, der Dir die Marken zeigen "
		      +"koennte.\n"),0;
  }

  write(break_string("Der Postbeamte schaut Dich freundlich an und zeigt Dir "
   "die Liste der vorraetigen Postwertzeichen:",78));
  write("Index	   Wert Motiv\n");

  marken = GetStampList();

  for (i = 0;i < sizeof(marken);++i)
  {
    write(sprintf("%-10s%4d %s\n",marken[i],
				  STAMPSERV->QueryStampPrice(marken[i]),
		       capitalize(STAMPSERV->QueryStampShort(marken[i]))));
  }
  
  write("\nDu kannst eine Marke kaufen mit 'kaufe marke <index>'\n");
  return 1;
}
