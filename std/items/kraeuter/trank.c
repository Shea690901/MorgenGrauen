#pragma strong_types,rtt_checks

inherit "/std/thing";

#include <properties.h>
#include <defines.h>
#include <items/kraeuter/kraeuter.h>
#include <items/kraeuter/trankattribute.h>
#include <hook.h>
#include <class.h>
#include <new_skills.h>

#ifndef BS
#  define BS(x)             break_string(x, 78)
#endif

// for debug
#define private public

// Ablaufzeit des Tranks, ab dann keine Wirkung mehr
private nosave int expiry;
// Ablaufzeit der wirkungen des Trankes (0, wenn Trank nicht getrunken)
private nosave int duration;
// Trankattribute, vom Krautmaster schon skaliert, gekappt, beschraenkt auf
// die jeweils max. positiven Effekte
private nosave mapping data;
// Klassen, die bei Schutz oder verstaerktem Schaden betroffen sind. Werte
// ist der Schutz- oder Attackebonus/-malus.
private nosave mapping att_classes;
private nosave mapping prot_classes;
// max. Summe von Giftstufen (P_POISON und P_LEVEL/10 bei CL_POISON).
private nosave int prot_poison;
// max. geheilte Summe von Krankheitslevel (P_LEVEL in CL_DISEASE).
private nosave int prot_disease;

#define allowed(x) (object_name(x)==PLANTMASTER)

mixed _query_data() {return data;}
int _set_data(mixed d) {data=d;return data!=0;}

protected void create()
{
   if (object_name(this_object()) == __FILE__[0..<3])
   {
     set_next_reset(-1);
     return;
   }
   ::create();
   SetProp(P_GENDER, FEMALE);
   SetProp(P_NAME, "Glasflasche");
   SetProp(P_NAME_ADJ, ({"klein"}));
   SetProp(P_VALUE, 10);
   AddId(({"glasflasche", "flasche"}));
   AddAdjective(({"klein", "kleine"}));
   AddCmd(({"trink", "trinke"}), "cmd_trinken");
}

protected void create_super()
{
  set_next_reset(-1);
}

static string _query_short()
{
   if (!clonep(ME)) return "Eine kleine Glasflasche";
   return Name(WEN, 0)+(data==0 ? "" : " (gefuellt)");
}

static string _query_long()
{
  if (data==0)
     return break_string(
         "Eine kleine leere Glasflasche die mit einem Korken verschlossen "
         "ist.\n"
         "Flaschen wie diese werden ueblicherweise verwendet, um darin "
         "magische Traenke abzufuellen.",78,BS_LEAVE_MY_LFS);

  return break_string(
      "Eine kleine Glasflasche die mit einer trueben Fluessigkeit "
      "gefuellt ist.\n"
      "Sie ist mit einem Korken verschlossen um den Inhalt der "
      "Flasche zu schuetzen.",78,BS_LEAVE_MY_LFS);
}

private string num2desc(int bumms)
{
  switch(abs(bumms))
  {
    case 0..499:
      return "ein wenig";
    case 500..999:
      return "so einiges";
    case 1000..1499:
      return "erheblich";
    case 1500..2000:
      return "unglaublich";
  }
  return "ungenehmigt viel"; // kommt hoffentlich nicht vor.
}

// Wird gerufen, wenn die Wirkung des Trankes ablaufen soll.
private void terminate_effects()
{
  tell_object(environment(),
      "Die letzten Wirkungen des Kraeutertrankes klingen ab.\n");
  remove(1);
}

// Von den Hooks H_HOOK_ATTACK_MOD und H_HOOK_DEFEND gerufen, erhoeht oder
// verringert den Schaden gegen Lebenwesen bestimmter Klassen (Keys in
// <classes>). Der Malus/Bonus steht als Wert zum jeweiligen Key in dem
// Mapping.
mixed hcallback(object hookSource, int hookid, mixed hookData)
{
    if (hookSource != environment())
        return ({H_NO_MOD,hookData});
    switch(hookid)
    {
      case H_HOOK_ATTACK_MOD:
        foreach(string class, int modval : att_classes)
        {
          if (hookData[SI_ENEMY]->is_class_member(class))
          {
            // Yeah. Treffer. Schaden erhoehen oder verringern... ;)
            hookData[SI_SKILLDAMAGE] += modval;
            // Ende. keine weiteren Klassen pruefen.
            return ({H_ALTERED, hookData});
          }
        }
        break;
      case H_HOOK_DEFEND:
        // hookData: ({dam,dam_type,spell,enemy})
        foreach(string class, int modval : prot_classes)
        {
          if (hookData[3]->is_class_member(class))
          {
            // Yeah. Treffer. Schaden erhoehen oder verringern... ;)
            hookData[0] += modval;
            // Ende. keine weiteren Klassen pruefen.
            return ({H_ALTERED, hookData}); 
          }
        }
        break;
      case H_HOOK_INSERT:
        // Wenn die Giftschutzkapazitaet noch ausreicht, wird das reinkommende
        // Objekt zerstoert (und die Kapazitaet reduziert).
        // hookData: neues object
        if (prot_poison > 0
            && hookData->is_class_member(CL_POISON))
        {
          int p=hookData->QueryProp(P_LEVEL) / 10;
          if (p < prot_poison)
          {
            hookData->remove(1);
            prot_poison-=p;
            return ({H_CANCELLED, hookData});
          }
        }
        if (prot_disease > 0
            && hookData->is_class_member(CL_DISEASE))
        {
          int lvl = hookData->QueryProp(P_LEVEL);
          if (lvl < prot_disease)
          {
            hookData->remove(1);
            prot_disease-=p;
            return ({H_CANCELLED, hookData});
          }
        }
        break;
      case H_HOOK_POISON:
        // hookData: poisonval
        // Alle Giftlevel werden reduziert auf 0 und von prot_poison
        // abgezogen. Wenn das 0 ist, endet der Giftschutz.
        if (prot_poison>0)
        {
          if (hookData < prot_poison)
          {
            prot_poison-=hookData;
            hookData = 0;
          }
          else
          {
            hookData -= prot_poison;
            prot_poison=0;
          }
          return ({H_ALTERED, hookData});
        }
        break;
    }
    return ({H_NO_MOD, hookData});
}

private int act_attr_tragen(int effekt)
{
  SetProp(P_WEIGHT, QueryProp(P_WEIGHT) - effekt*4);
  tell_object(environment(),
      BS("Du fuehlst Dich ploetzlich "+num2desc(effekt)
        + (effekt>0 ? "entlastet" : "belastet") + "."));
  return 1;
}

// Berechnet eine max. Verzoegerung der Wirkung abhaengig von der Wirkung und
// einer Wirkschwelle. Der rel. Abstand von der Wirkschwelle (relativ zum max.
// moeglichen Abstand) wird hierbei genutzt. Ausserdem ist die max.
// Verzoegerung natuerlich die Wirkungsdauer des Trankes.
// <duration> muss im Trank schon gesetzt sein.
private int calc_max_delay(int effekt, int wirkschwelle)
{
  int abstand = abs(effekt - wirkschwelle);
  if (!duration) duration = time()+600;
  printf("calc_max_delay: %d\n",((duration-time()) * abstand) /
      (2000-abs(wirkschwelle)));
  return ((duration-time()) * abstand) / (2000-abs(wirkschwelle));
}

private int act_attr_flee_tport(int effekt)
{
  // effekt > 0 teleportiert sofort zu halbwegs sicheren Orten
  // -750 < effekt < 0 teleportiert auch sofort zu nicht so dollen Orten
  // effekt < -750 teleportiert spaeter zu bloeden Orten
  if (effekt < -750)
  {
    // Verzoegerung ausrechnen und dies hier per call_out nochmal mit einem
    // effekt rufen, der eine instantane Reaktion ausloest.
    call_out(#'act_attr_flee_tport,
        random(calc_max_delay(effekt, -750))+1, -750);
    tell_object(environment(),
        "Deine Umgebung fuehlt sich nach und nach unwirklicher an.\n");
    return 1;
  }

  // Effekte instantan ausloesen.

  // wenn hier kein P_NO_TPORT erlaubt ist, mal gucken, ob wir spaeter noch
  // zeit haben.
  if (environment(environment())->QueryProp(P_NO_TPORT)==NO_TPORT_OUT)
  {
    tell_object(environment(),
        BS("Irgendetwas haelt Dich an diesem Ort fest."));
    int delay = duration - time();
    // wenn noch genug Restzeit, nochmal versuchen, sonst nix machen.
    if (delay>10)
    {
      call_out(#'act_attr_flee_tport, random(delay), effekt);

    }
    return 0;
  }

  if ( effekt > 0 )
  {
    string* dests;
    switch(effekt)
    {
      case 0..249:
        dests = ({"/d/inseln/zesstra/vulkanweg/room/r1",
                  "",
                 });
        break;
      case 250..499:
        dests = ({"/d/ebene/miril/schloss/heide11",
                  "/d/polar/alle/room/weg4_15",
                 });
        break;
      case 500..749:
        dests = ({"/d/ebene/bertram/ebene/wasser8",
                 });
        break;
      case 750..999:
        dests = ({"/d/gebirge/silvana/cronoertal/room/tf4",
                 });
        break;
      case 1000..1249:
        dests = ({"/d/fernwest/shinobi/konfu_quest/room/insel4",
                 });
        break;
      case 1250..1499:
        dests = ({"/d/gebirge/silvana/cronoertal/room/tf4",
                 });
        break;
      case 1500..1749:
        dests = ({"/gilden/bierschuettler",
                  "/gilden/kaempfer",
                  "/d/wald/gundur/hobbitdorf/schrein",
                  "/d/vland/morgoth/room/city/rathalle",
                 });
        break;
      default:
        dests = ({"/gilden/abenteurer",
                  "/gilden/katzenkrieger",
                  "/d/ebene/room/PortVain/po_haf2",
                  "/d/gebirge/room/zkapelle",
                  "/d/gebirge/room/he3x3",
                  "/d/ebene/room/huette",
                 });
        break;
    }
  }
  else if ( effekt < 0 )
  {
    switch(effekt)
    {
      case -499..0:
        dests = ({"/d/polar/bendeigid/rooms/neskaya/neskay12",
                  "/players/ketos/gl/room/gl1x1",
                  "/d/inseln/zesstra/vulkanweg/room/r10",
                 });
        break;
      case -999..-500:
        dests = ({"/d/ebene/room/Hoehle/hg6",
                 });
        break;
      case -1499..-1000:
        dests = ({"/d/polar/bendeigid/rooms/pass/pass_e1",
                  "/d/ebene/room/gebuesch",
                 });
        break;
      case -1749..-1500:
        dests = ({"/d/dschungel/wurzel/t2",
                 });
        break;
      default:
        dests = ({"/d/ebene/rochus/room/sp10",
                  "/d/ebene/rochus/quest_3player/room/schacht10",
                 });
        break;
    }
  }
  tell_object(environment(),
      BS("Eine Kraft zerrt an Dir, die Welt verschwimmt..."));
  if (environment()->move(dests[random(sizeof(dests))],M_TPORT) == MOVE_OK)
    tell_object(environment(),
        "Einen Moment spaeter bist Du ganz woanders.\n");
  else
    tell_object(environment(),
        "Aber sonst passiert nichts.\n");
  return 1;
}

private int act_attr_change_dimension(int effekt)
{
  // nur effekt > 750 aendern die Dimension instantan. ;-)
  if (effekt > 0 && effekt < 1000)
  {
    // Verzoegerung ausrechnen und dies hier per call_out nochmal mit einem
    // effekt rufen, der eine instantane Reaktion ausloest.
    call_out(#'act_attr_change_dimension,
        random(calc_max_delay(effekt,1000))+1, 1000);
    tell_object(environment(),BS("Um Dich herum wird alles "
        "langsam grauer.\n"));
    return 1;
  }
  // nur -500 < effekt < 0 froscht instantan ;-)
  if (effekt < -600)
  {
    // Verzoegerung ausrechnen und dies hier per call_out nochmal mit einem
    // effekt rufen, der eine instantane Reaktion ausloest.
    call_out(#'act_attr_change_dimension,
        random(calc_max_delay(effekt, -600))+1, -600);
    tell_object(environment(),BS("Um Dich herum wird alles "
        "langsam grauer.\n"));
    return 1;
  }
  // Effekte instantan ausloesen.
  // wenn hier kein Para-Trans erlaubt ist, mal gucken, ob wir spaeter noch
  // zeit haben.
  if (environment(environment())->QueryProp(P_NO_PARA_TRANS))
  {
    int delay = duration - time();
    // wenn noch genug Restzeit, nochmal versuchen, sonst nix machen.
    if (delay>10)
    {
      call_out(#'act_attr_change_dimension, random(delay), effekt);
      tell_object(environment(), BS("Die Welt um Dich wird ein "
            "wenig grauer."));
    }
    return 0;
  }
  if ( effekt > 0 )
  {
    if ( environment()->QueryProp(P_PARA) > 0 )
    {
      environment()->SetProp(P_PARA, 0);
      tell_object(environment(), BS("Fuer einen kurzen Moment siehst Du nur "
          "ein graues Wabern um Dich herum, bevor die Welt wieder "
          "normal aussieht.\n"));
    }
    else
    {
      tell_object(environment(), BS("Fuer einen kurzen Moment sieht alles um "
            "Dich herum grau aus."));
    }
  }
  else if ( effekt < 0 )
  {
    if ( !environment()->QueryProp(P_PARA) )
    {
      environment()->SetProp(P_PARA, 1);
       tell_object(environment(), BS("Fuer einen kurzen Moment siehst Du nur "
          "ein graues Wabern um Dich herum, bevor die Welt wieder "
          "normal aussieht. Aber es bleibt ein ungutes Gefuehl.\n"));
    }
    else {
      tell_object(environment(), BS("Fuer einen kurzen Moment sieht alles um "
            "Dich herum grau aus."));
    }
  }
  return 1;
}

private int act_attr_defrog(int effekt)
{
  // nur effekt > 1000 entfroscht instantan. ;-)
  if (effekt > 0 && effekt < 1000)
  {
    // Verzoegerung ausrechnen und dies hier per call_out nochmal mit einem
    // effekt rufen, der eine instantane Reaktion ausloest.
    call_out(#'act_attr_defrog, random(calc_max_delay(effekt,1000))+1, 1000);
    tell_object(environment(),BS(
        "Du hoerst ploetzlich lautes Gequake, was langsam immer leiser "
        "wird.\n"));
    return 1;
  }
  // nur -500 < effekt < 0 froscht instantan ;-)
  if (effekt < -500)
  {
    // Verzoegerung ausrechnen und dies hier per call_out nochmal mit einem
    // effekt rufen, der eine instantane Reaktion ausloest.
    call_out(#'act_attr_defrog, random(calc_max_delay(effekt, -500))+1, -501);
    tell_object(environment(),BS(
        "Du hoerst ploetzlich ganz leisess Gequake, was langsam immer lauter "
        "wird.\n"));
    return 1;
  }

  // Effekte instantan ausloesen.
  if ( effekt > 0 )
  {
    if ( environment()->QueryProp(P_FROG) )
    {
      environment()->SetProp(P_FROG,0);
      tell_object(PL, "Du fuehlst Dich deutlich weniger gruen.\n");
    }
    else
    {
      tell_object(environment(), break_string("Die Welt um Dich herum verliert "
        "ploetzlich alle gruenen Farbtoene, die bald darauf allmaehlich "
        "zurueckkehren.",78));
    }
  }
  else if ( effekt < 0 ) {
    if ( !environment()->QueryProp(P_FROG) ) {
      environment()->SetProp(P_FROG, 1);
      tell_object(environment(), "Quak!\n");
    }
    else {
      tell_object(environment(), break_string("Deine Sicht verschwimmt, alles wird "
        "intensiv gruen. Merkwuerdig. Zum Glueck ist das schnell wieder "
        "vorbei.",78));
    }
  }
  return 1;
}

private int act_attr_heal_lp(int effekt)
{
  if (effekt > 0)
  {
    tell_object(environment(),
        BS("Du fuehlst Dich schlagartig "+num2desc(effekt)
          + "besser."));
    environment()->restore_hit_points(effekt/10);
  }
  else
  {
    tell_object(environment(),
        BS("Du fuehlst Dich schlagartig "+num2desc(effekt)
          + "schlechter."));
    environment()->reduce_hit_points(effekt/10);
  }
  return 1;
}

private int act_attr_heal_sp(int effekt)
{
  if (effekt > 0)
  {
    tell_object(environment(),
        BS("Du fuehlst Dich schlagartig "+num2desc(effekt)
          + "konzentrierter."));
    environment()->restore_spell_points(effekt/10);
  }
  else
  {
    tell_object(environment(),
        BS("Du fuehlst Dich schlagartig "+num2desc(effekt)
          + "unkonzentrierer."));
    environment()->reduce_spell_points(effekt/10);
  }
  return 1;
}


private int modify_sa(string sa, int effekt)
{
  string msg;
  switch (sa)
  {
    case SA_SPEED:
      msg = "Du wirst gerade " + num2desc(effekt)
            +(effekt>0 ? " schneller." : " langsamer.");
      effekt = (effekt * 30) / 2000;
      break;
    case SA_DURATION:
      msg = "Du wirst gerade " + num2desc(effekt)
            +(effekt>0 ? " weniger ausdauernd." 
                       : " ausdauernder.");
      effekt = (effekt * 25) / 2000;
      break;
    case SA_SPELL_PENETRATION:
      msg = "Deine geistige Durchsetzungskraft hat sich gerade "
            + num2desc(effekt)
            +(effekt>0 ? " verbessert." : " verschlechtert.");
      effekt = (effekt * 30) / 2000;
      break;
  }
  if (ModifySkillAttribute(sa, effekt, duration-time()) == SA_MOD_OK)
  {
    tell_object(environment(),BS(msg));
    return 1;
  }
  return 0;
}

private int act_attr_change_sa_speed(int effekt)
{
  return modify_sa(SA_SPEED, effekt);
}
private int act_attr_change_sa_duration(int effekt)
{
  return modify_sa(SA_DURATION, effekt);
}
private int act_attr_change_sa_spell_penetration(int effekt)
{
  return modify_sa(SA_SPELL_PENETRATION, effekt);
}


// Spieler MUSS das environment() sein!
void effekt()
{
  // Als erstes die Wirkungsdauer verwursten, einige Effekte brauchen
  // <duration>.
  // Wann laufen die Effekte denn spaetenstens ab?
  duration = time() + data[T_EFFECT_DURATION];
  call_out(#'terminate_effects, data[T_EFFECT_DURATION]);

  // nur echte wirkungen beruecksichtigen, keine "Metadaten"
  mapping effects = data & T_KRAUT_EFFECTS;
  // fuer Spieler wird der Paratrans nicht stattfinden.
  if (!IS_SEER(environment()))
    m_delete(data, T_CHANGE_DIMENSION);

  // auftrennen in positive und negative Effekte. Keys mit Wert 0 werden
  // ignoriert.
  mapping peff = filter(effects, function int (string k, int val)
                        {return val > 0;});
  mapping neff = filter(effects, function int (string k, int val)
                        {return val < 0;});
  // erst die positiven, dann die negativen Wirkungen aktivieren
  // fuer jede Wirkung wird eine lfun act_<trankattribut>() gerufen, z.B.
  // act_attr_tragen() (-> act_T_CARRY() )
  mapping notactivated =
          filter(peff, function int (string k, int val)
              {return !funcall(symbol_function("act_"+k,this_object()), val);})
         +
          filter(neff, function int (string k, int val)
              {return !funcall(symbol_function("act_"+k,this_object()), val);});
  // Meldungen ausgeben ueber nicht aktivierte Wirkungen?
  // TODO

  // Waehrend der Wirkung ist dieses Objekt schonmal unsichtbar.
  SetProp(P_INVIS, 1);
  SetProp(P_WEIGHT, 0);
  // neue, leere Flaschen ins Inventar des Spielers bewegen.
  clone_object(TRANKITEM)->move(environment(),M_NOCHECK|M_SILENT);
}

int cmd_trinken(string str)
{
   notify_fail("WAS moechtest Du trinken?\n");
   if (!id(str))
     return 0;
   if (environment(this_object()) != PL)
   {
     notify_fail("Du hast " + name(WEN) + " ja gar nicht dabei.\n");
     return 0;
   }
   else if (data==0)
   {
     write("Die Flasche ist bereits leer.\n");
   }
   else
   {
     write(BS("Du oeffnest die Flasche und trinkst "+name(WEN, 1)+"."));
     say(BS(PL->Name(WER, 0)+" oeffnet eine Flasche und trinkt "+name(WEN, 1)
                            +" in einem Schluck aus."));
     effekt();
     data=0;
   }
   return 1;
}

// AN/TODO: Super ist hier auch, dass unterschiedliche Rueckgabewerte
// existieren, aber die Funktion Fill() im Kessel das ueberhaupt nicht
// auswertet.
public nomask int Fill(object *plants)
{
  if (!pointerp(plants)) return -1;
  if (data) return -2;  // schon voll
  data = PLANTMASTER->make_potion(plants);
  if (pointerp(data))
  {
    // Pflanzen auch zerstoeren, wenns geklappt hat.
    plants->remove(1);
    expiry=time()+data[T_EXPIRE];
    return 1;
  }
  return -2;
}

/*
void effekt(object player)
{
  // Wirkungsdauer der Effekte
  int eff_duration = 300; // -> T_EFFECT_DURATION

  // Heilung 
  if (data[T_HEAL_LP]>150)
    {
      int lps;
      lps=data[T_HEAL_LP]/50;
      string wie="ein wenig";
      if (lps>10)
        wie="um einiges";
      if (lps>20)
        wie="deutlich";
      if (lps>30)
        wie="erheblich";
      if (lps>50)
        wie="unglaublich";
      tell_object(player,"Du fuehlst Dich "+wie+" besser!\n");
      PL->heal_self(lps);
    }
  if (data[T_HEAL_SP]>150)
    {
      int lps;
      lps=data[T_HEAL_SP]/50;
      string wie="ein wenig";
      if (lps>10)
        wie="um einiges";
      if (lps>20)
        wie="deutlich";
      if (lps>30)
        wie="erheblich";
      if (lps>50)
        wie="unglaublich";
      tell_object(player,"Du fuehlst Dich "+wie+" konzentrierter!\n");
      PL->restore_spell_points(lps);
    }
  if ( data[T_FROG] > 0 ) {
    if ( PL->QueryProp(P_FROG) ) {
      PL->SetProp(P_FROG,0);
      tell_object(PL, "Du fuehlst Dich deutlich weniger gruen.\n");
    }
    else {
      tell_object(PL, break_string("Die Welt um  Dich herum verliert "
        "ploetzlich alle gruenen Farbtoene, die bald darauf allmaehlich "
        "zurueckkehren."));
    }
  }
  if (data[T_HEAL_LP]<0)
    {
      int lps;
      lps=data[T_HEAL_LP]/50;
      string wie="ein wenig";
      if (lps>10)
        wie="um einiges";
      if (lps>20)
        wie="deutlich";
      if (lps>30)
        wie="erheblich";
      if (lps>50)
        wie="unglaublich";
      tell_object(player,"Du fuehlst Dich "+wie+" ungesuender!\n");
      PL->do_damage(lps);
    }
  if (data[T_HEAL_SP]<0)
    {
      int lps;
      lps=data[T_HEAL_SP]/50;
      string wie="ein wenig";
      if (lps>10)
        wie="um einiges";
      if (lps>20)
        wie="deutlich";
      if (lps>30)
        wie="erheblich";
      if (lps>50)
        wie="unglaublich";
      tell_object(player,"Du fuehlst Dich "+wie+" unkonzentrierter!\n");
      PL->restore_spell_points(-lps);
    }
  if ( data[T_FROG] < 0 ) {
    if ( !PL->QueryProp(P_FROG) ) {
      PL->SetProp(P_FROG, 1);
      tell_object(PL, "Quak!\n");
    }
    else {
      tell_object(PL, break_string("Deine Sicht verschwimmt, alles wird "
        "intensiv gruen. Merkwuerdig. Zum Glueck ist das schnell wieder "
        "vorbei."));
    }
  }
  if (member(data, T_DAMAGE_ANIMALS)
      || member(data, T_DAMAGE_MAGIC)
      || member(data, T_DAMAGE_UNDEAD))
  {
    if (environment()->HRegisterToHook(H_HOOK_ATTACK_MOD,
                           #'hcallback,H_HOOK_OTHERPRIO(0),
                           H_DATA_MODIFICATOR, eff_duration) <= 0)
    {
      environment()->do_damage(10000);
    }
  }
  if (member(data, T_PROTECTION_ANIMALS)
      || member(data, T_PROTECTION_MAGIC)
      || member(data, T_PROTECTION_UNDEAD))
  {
    if (environment()->HRegisterToHook(H_HOOK_DEFEND,
                           #'hcallback,H_HOOK_OTHERPRIO(0),
                           H_DATA_MODIFICATOR, eff_duration) <= 0)
    {
      environment()->do_damage(10000);
    }
  }

  // Als letztes die Wirkungsdauer verwursten.
  // Wann laufen die Effekte denn spaetenstens ab? Haengt ab von T_SPELL_TIME.
  call_out(#'terminate_effects, eff_duration); // -> T_SPELL_TIME
  // Waehrend der Wirkung ist dieses Objekt schonmal unsichtbar.
  SetProp(P_INVIS, 1);
  SetProp(P_WEIGHT, 0);
  // neue, leere Flaschen ins INventar des Spielers bewegen.
  clone_object(TRANKITEM)->move(environment(),M_NOCHECK|M_SILENT);
}
*/

