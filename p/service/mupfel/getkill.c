// /p/service/mupfel/getkill.c
// Ermittelt, wenn in 'die' aufgerufen, den Killer, auch wenn
// mit Bumerang, Kampfdaemon o.ae. Hilfsmitteln getoetet wurde
// 22-07-1996  - Mupfel@MorgenGrauen -
// Jetzt wird auch ein Beistand (Schutzengel, Phoenix, etc) erkannt
// 06-07-1997  - Mupfel@MorgenGrauen -
// Args der Pflanzenkrieger hat ja auch die id "Beistand" ;(
// 09-01-2001  - Mupfel@MorgenGrauen -
// Immer neue Retrowaffen... jetzt wird auch Tsunamis Toga erkannt
// 04-01-2005  - Mupfel@MorgenGrauen -
// 21.11.2006 - Zesstra
// Der Bumerang ist umgezogen. Pfad aktualisiert

#include <thing/properties.h>
#include <thing/language.h>

//#define DD(x) tell_object(find_player("debugger")||this_object(), x)
#define DD(x)

#define ME this_object()
#define PL this_player()
#define PO previous_object()
#define CAP(x) capitalize(x)

string get_killer()
{
  string killer;

  DD(sprintf("TP    = %O\n", this_player()));
  DD(sprintf("TI    = %O\n", this_interactive()));
  DD(sprintf("PO(1) = %O\n", previous_object()));
  DD(sprintf("PO(2) = %O\n", previous_object(2)));
  DD(sprintf("PO(3) = %O\n", previous_object(3)));
  DD(sprintf("PO(4) = %O\n", previous_object(4)));
  DD(sprintf("Stacktiefe = %i\n", caller_stack_depth()));

  if (PL)
  {
    if (PL->id("\nchaos_gilden_daemon"))
      return CAP(PL->QueryProp("chaos_daem_beschwoerer"))+" (mit Kampfdaemon)";
    if (PL->id("Beistand") && !PL->id("P-KRIEGER"))
      return CAP((PL->QueryProp("klerus_owner"))->name())+" (mit Beistand)";
    if (PL == ME)   // Goblinring und andere Retrowaffen
    {
      if (!PO)
        return PL->name(RAW);
      if (old_explode(object_name(PO), "#")[0]=="/d/ebene/obj/gring")
        return (PO->QueryProp("worn"))->name(RAW)+" (mit Goblinring)";
      if (old_explode(object_name(PO), "#")[0]=="/d/unterwelt/wurzel/dorf/shabz")
        return (PO->QueryProp("worn"))->name(RAW)+" (mit Abzeichen)";
      if (old_explode(object_name(PO), "#")[0]=="/d/wueste/tsunami/schule/rue/hex_toga2")
        return (PO->QueryProp("worn"))->name(RAW)+" (mit Toga)";
      return PL->name(RAW);
    }
    return PL->name(RAW);
  }
  else if (PO)
  {
    if (old_explode(object_name(PO), "#")[0]=="/p/seher/laden/obj/bumerang.c")
      return (PO->query_werfer())->name(RAW)+" (mit Bumerang)";
    else
      return PO->name(RAW);
  }
  else
    return "<unbekannt>";
}

object get_killer_object()
{
  DD(sprintf("TP    = %O\n", this_player()));
  DD(sprintf("TI    = %O\n", this_interactive()));
  DD(sprintf("PO(1) = %O\n", previous_object()));
  DD(sprintf("PO(2) = %O\n", previous_object(2)));
  DD(sprintf("PO(3) = %O\n", previous_object(3)));
  DD(sprintf("PO(4) = %O\n", previous_object(4)));
  DD(sprintf("Stacktiefe = %i\n", caller_stack_depth()));

  if (PL)
  {
    if (PL->id("\nchaos_gilden_daemon"))
      return find_player(PL->QueryProp("chaos_daem_beschwoerer"));
    if (PL->id("Beistand") && !PL->id("P-KRIEGER"))
      return PL->QueryProp("klerus_owner");
    if (PL == ME)   // Goblinring und andere Retrowaffen
    {
      if (!PO)
        return PL;
      if (old_explode(object_name(PO), "#")[0]=="/d/ebene/obj/gring")
        return PO->QueryProp("worn");
      if (old_explode(object_name(PO), "#")[0]=="/d/unterwelt/wurzel/dorf/shabz")
        return PO->QueryProp("worn");
      if (old_explode(object_name(PO), "#")[0]=="/d/wueste/tsunami/schule/rue/hex_toga2")
        return PO->QueryProp("worn");
      return PL;
    }
    return PL;
  }
  else if (PO)
  {
    if (old_explode(object_name(PO), "#")[0]=="/p/seher/laden/obj/bumerang.c")
      return PO->query_werfer();
    else
      return PO;
  }
  else
    return 0;
}
