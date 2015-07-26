// MorgenGrauen MUDlib
//
// player/skills.c -- Spielerskills
//
// $Id: skills.c 7514 2010-03-27 14:46:30Z Zesstra $

//
// 2003-01-20: Nun Zooks Baustelle
//
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

inherit "/std/living/skills";

#include <combat.h>
#include <new_skills.h>
#include <properties.h>
#include <break_string.h>

#define NEED_PROTOTYPES
#include <player/base.h>
#undef NEED_PROTOTYPES

// Dieses Mapping speichert die deaktivierten Skills der einzelnen Gilden
// Diese werden in den Gilden ueber P_GUILD_DEACTIVATED_SKILL gesetzt.
nosave mapping deactivated_skills = ([]);

// Flag fuer den Kompatibilitaetsmodus des Kampfs (Emulation von
// 2s-Alarmzeit). Wenn != 0 speichert das Flag gleichzeitig die Zeit des
// letzten Heartbeats. Auf diese Zeit wird der Startpunkt eines Spellfatigues
// ggf. zurueckdatiert. (max. eine Sekunde)
int spell_fatigue_compat_mode;

// Ein create() fuer das Mapping

protected void create()
{
  mapping act;

  ::create();

  // Wir holen die Gilden aus dem Gildenmaster 
  foreach(string guild:
      (string *)call_other(GUILDMASTER,"QueryProp",P_VALID_GUILDS))
  {
    if(catch(act=call_other("/gilden/"+guild,"QueryProp",
        P_GUILD_DEACTIVATE_SKILLS); publish ))
        log_file("WEAPON_SKILLS", sprintf ("%s: Gilde nicht ladbar: "
              +"TP: %O, TI: %O, PO: %O, Gilde: %s\n", dtime(time()),
              this_player(), this_interactive(), previous_object(), guild));
    else if (act) // wenn act, ins Mapping aufnehmen.
        deactivated_skills+=([guild:act]);
  }
  // keine echte Prop mehr, Save-Modus unnoetig.
  Set(P_NEXT_SPELL_TIME,SAVE,F_MODE_AD);

  Set(P_SKILLSVERSION, SAVE|SECURED, F_MODE_AS);
}

// Das Mapping kann man auch abfragen

public mapping GetDeactivatedSkills()
{
    return copy(deactivated_skills);
}

// Funktion, die sagt ob ein ANY-Skill deaktiviert ist.
public int is_deactivated_skill(string sname,string guild)
{
        if (deactivated_skills[guild])        
                return deactivated_skills[guild][sname];
        return 0;
}


// Funktion fuer die Waffenskills
// traegt die allg. Waffenskills ein. Wird ggf. von FixSkills() gerufen.
// (Das Eintragen bedeutet nicht, dass die aktiv sind! Aber bei Gildenwechsel
// werden sie nicht eingetragen).
private void set_weapon_skills() {

  if (QuerySkillAbility(FIGHT(WT_SWORD))<=0)                
      ModifySkill(FIGHT(WT_SWORD),([SI_SKILLABILITY:0]),150,"ANY");    
  if (QuerySkillAbility(FIGHT(WT_AXE))<=0)                
      ModifySkill(FIGHT(WT_AXE),([SI_SKILLABILITY:0]),150,"ANY");          
  if (QuerySkillAbility(FIGHT(WT_SPEAR))<=0)        
      ModifySkill(FIGHT(WT_SPEAR),([SI_SKILLABILITY:0]),150,"ANY");          
  if (QuerySkillAbility(FIGHT(WT_WHIP))<=0)
      ModifySkill(FIGHT(WT_WHIP),([SI_SKILLABILITY:0]),150,"ANY");
  if (QuerySkillAbility(FIGHT(WT_KNIFE))<=0)                
      ModifySkill(FIGHT(WT_KNIFE),([SI_SKILLABILITY:0]),150,"ANY");          
  if (QuerySkillAbility(FIGHT(WT_CLUB))<=0)                
      ModifySkill(FIGHT(WT_CLUB),([SI_SKILLABILITY:0]),150,"ANY");          
  if (QuerySkillAbility(FIGHT(WT_STAFF))<=0)
      ModifySkill(FIGHT(WT_STAFF),([SI_SKILLABILITY:0]),150,"ANY");
} 

// initialisiert die Skills fuer Spieler (momentan: allg. Waffenskills setzen
// und P_SKILLS_VERSION)
protected void InitSkills() {
  mapping ski;
  // schonmal initialisiert?
  if (mappingp(ski=Query(P_NEWSKILLS, F_VALUE)) && sizeof(ski))
    return;

  // allg. Waffenskills aktivieren
  set_weapon_skills();

  // Version setzen
  SetProp(P_SKILLSVERSION, CURRENT_SKILL_VERSION);
  Set(P_SKILLSVERSION, SAVE|SECURED, F_MODE_AS);
}

// Updated Skills aus Version 0 heraus.
private void FixSkillV0(string skillname, mixed sinfo) {
  // alte Skills auf mapping normieren
  if (intp(sinfo)) {
    sinfo = ([SI_SKILLABILITY: sinfo ]);
  }
  // wenn ein Spellbook vermerkt ist, ist es wohl ein Spell, sonst wird von
  // Skill ausgegangen. Bei Skills: hier Abbruch
  if (!sinfo[SI_SPELLBOOK])
      return;

  // Fuer Spells ggf. SI_SPELL erstellen
  if (!mappingp(sinfo[SI_SPELL]))
    sinfo[SI_SPELL] = ([]);
  // SP_NAME vermerken
  if (!sinfo[SP_NAME] && !sinfo[SI_SPELL][SP_NAME]) {
    sinfo[SP_NAME] = skillname;
    sinfo[SI_SPELL][SP_NAME] = skillname;
  }
  else if (sinfo[SP_NAME] && !sinfo[SI_SPELL][SP_NAME])
    sinfo[SI_SPELL][SP_NAME] = sinfo[SP_NAME];
  else if (!sinfo[SP_NAME] && sinfo[SI_SPELL][SP_NAME])
    sinfo[SP_NAME] = sinfo[SI_SPELL][SP_NAME];
}

// Updatet und repariert ggf. Skillmappings in Spielern
protected void FixSkills() {
/*
  // nur bei genug rechenzeit loslegen
  if (get_eval_cost() < 1300000) {
    call_out(#'FixSkills, 2);
    return;
  }
  // wenn gar keine Skills da (?): InitSkills() rufen.
  mapping allskills = Query(P_NEWSKILLS, F_VALUE);
  if (!mappingp(allskills) || !sizeof(allskills)) {
      InitSkills();
      return;
  }
  // bei Version 0
  if (QueryProp(P_SKILLSVERSION) == 0) {
    foreach(string gilde, mapping skills: allskills) {
      walk_mapping(skills, #'FixSkillV0);
    }
    // Waffenskills aktivieren
    set_weapon_skills();
    // Speicherflag fuer die Versionsprop muss noch gesetzt werden.
    Set(P_SKILLSVERSION, SAVE|SECURED, F_MODE_AS);
    // Versionszaehlen um eins auf 1 erhoehen.
    SetProp(P_SKILLSVERSION, 1);
  }

  // Falls noch nicht auf der aktuellen Version angekommen, neuer callout
  if (QueryProp(P_SKILLSVERSION) != CURRENT_SKILL_VERSION)
      call_out(#'FixSkills, 2);
      */
}


// Standardisierte Nahkampf-Funktion fuer alle Nahkampf-Waffenarten
protected mapping ShortRangeSkill(object me, string sname, mapping sinfo) 
{ 
  int val, w;
  object enemy;
  
  if (!mappingp(sinfo) || !objectp(sinfo[P_WEAPON]))
    return 0;

  w = ([WT_KNIFE : 8,
        WT_SWORD : 5,
        WT_AXE   : 4,
        WT_SPEAR : 6,
        WT_CLUB  : 1,
        WT_WHIP  : 9,
        WT_STAFF : 7])[sinfo[P_WEAPON]->QueryProp(P_WEAPON_TYPE)];
      

  val = sinfo[SI_SKILLABILITY]*(sinfo[P_WEAPON]->QueryProp(P_WC)*
                                (w*QueryAttribute(A_DEX)+
                                 (10-w)*QueryAttribute(A_STR))/800)
        /MAX_ABILITY;

  if (val > 75) {
    log_file("WEAPON_SKILLS", sprintf("%s: Zu hoher Schaden von: "
    +"TO: %O, TI: %O, PO: %O, val: %d, A_DEX: %d, A_STR: %d, "
                                   +"P_WEAPON: %O, P_WC: %d\n", dtime(time()),
                                   this_object(), this_interactive(), 
                                   previous_object(), val, 
                                   QueryAttribute(A_DEX),
                                   QueryAttribute(A_STR), sinfo[P_WEAPON],
                                   sinfo[P_WEAPON]->QueryProp(P_WC)));
    val = 75;
  }

  /*
    Der zusätzliche Schaden der allgemeinen Waffenskills berechnet
    sich wie folgt: 

    sinfo[SI_SKILLABILITY)* (P_WC * ( X ) / 800) / MAX_ABILITY

    Dabei beruecksichtigt X je nach Waffentyp in unterschiedlicher
    Gewichtung die Werte fuer Geschicklichkeit und Staerke. 

     X == 

       Messer   : 8*A_DEX + 2*A_STR
       Schwert  : 5*A_DEX + 5*A_STR
       Axt      : 4*A_DEX + 6*A_STR 
       Speer    : 6*A_DEX + 4*A_STR
       Keule    : 1*A_DEX + 9*A_STR
       Peitsche : 9*A_DEX + 1*A_STR
  */

  sinfo[SI_SKILLDAMAGE]+=val;


  /* Lernen: Wird immer schwieriger, nur bei jedem 20. Schlag im Schnitt,
   * und nur dann, wenn der Gegner auch XP gibt. */
  if (random(MAX_ABILITY+1)>sinfo[SI_SKILLABILITY] && !random(20))
  {
         enemy=sinfo[SI_ENEMY];
         if (objectp(enemy) && (enemy->QueryProp(P_XP)>0))
         {
           object ausbilder;
        //         log_file("humni/log_wurm","Haut: %s und zwar %s, mit xp %d\n",geteuid(this_object()),to_string(enemy),enemy->QueryProp(P_XP));
            LearnSkill(sname, random(5), 150);
        // Gibt es einen Ausbilder?
        if (QueryProp(P_WEAPON_TEACHER) && 
                        ausbilder=find_player(QueryProp(P_WEAPON_TEACHER)))
          {
            // Ist der Ausbilder anwesend?
            if (present(ausbilder,environment()))
              {
              // Ausbilder und Azubi muessen dieselbe Waffe haben.
              //string wt_aus,wt_azu;
              object waf_aus,waf_azu;

              waf_azu=QueryProp(P_WEAPON);
              waf_aus=call_other(ausbilder,"QueryProp",P_WEAPON);

              //wt_azu=call_other(waf_azu,"QueryProp",P_WEAPON_TYPE);
              //wt_aus=call_other(waf_aus,"QueryProp",P_WEAPON_TYPE);
              //if (wt_azu==wt_aus)
              if (objectp(waf_aus) && objectp(waf_azu) &&
                  (string)waf_aus->QueryProp(P_WEAPON_TYPE)
                     == (string)waf_azu->QueryProp(P_WEAPON_TYPE)) 
                {
                // Bonus von bis zu 5 Punkten
                //log_file("humni/log_azubi",
                  // sprintf("Azubi %O und Ausbilder %O : Waffentypen %s und %s, gelernt\n",this_object(),
                    //   ausbilder, wt_azu, wt_aus));
                LearnSkill(sname,random(6),150);
                }
          }
        }
    }
  }

  /* 
     Die Schwierigkeit liegt bei 150, so dass 
     ein Lvl. 1 Spieler maximal 15% Skill 
     usw...
     lernen kann. (Genaue Tabelle in /std/living/skills bei LimitAbility)
  */         

  return sinfo;
}


// Standardisierte Fernkampf-Funktion fuer alle Fernkampf-Waffenarten

// *** noch deaktiviert ***

protected mapping LongRangeSkill(object me, string sname, mapping sinfo, int dam) 
{ int abil,val;

  if (!mappingp(sinfo) || !dam || !objectp(sinfo[P_WEAPON]) ||
      (sinfo[P_WEAPON]->QueryProp(P_SHOOTING_WC))<5)
    return 0;

  abil=sinfo[SI_SKILLABILITY]+OFFSET(sinfo[SI_SKILLABILITY]);
  val=dam*abil/MAX_ABILITY;
  val=val/2+random(val/2+1);
  val=(val*QuerySkillAttribute(SA_DAMAGE))/100;
  sinfo[SI_SKILLDAMAGE]+=val;

  if (random(MAX_ABILITY+1)>sinfo[SI_SKILLABILITY] && random(50)==42)
    LearnSkill(sname, 1, 150);

  return sinfo;
}



// Die einzelnen Waffenskills rufen dann nur die Standard-Funktion auf.

protected mapping StdSkill_Fight_axe(object me, string sname, mapping sinfo)
{
  return ShortRangeSkill(me, sname, sinfo);
}

protected mapping StdSkill_Fight_club(object me, string sname, mapping sinfo)
{
  return ShortRangeSkill(me, sname, sinfo);
}

protected mapping StdSkill_Fight_knife(object me, string sname, mapping sinfo)
{
  return ShortRangeSkill(me, sname, sinfo);
}

protected mapping StdSkill_Fight_spear(object me, string sname, mapping sinfo)
{
  return ShortRangeSkill(me, sname, sinfo);
}

protected mapping StdSkill_Fight_sword(object me, string sname, mapping sinfo) 
{
  return ShortRangeSkill(me, sname, sinfo);
}

protected mapping StdSkill_Fight_whip(object me, string sname, mapping sinfo)
{
  return ShortRangeSkill(me, sname, sinfo);
}

protected mapping StdSkill_Fight_staff(object me, string sname, mapping sinfo)
{
  return ShortRangeSkill(me, sname, sinfo);
}




// Die Fernwaffenskills sind Munitionsabhaengig

// *** noch deaktiviert ***

protected mapping StdSkill_Shoot_arrow(object me, string sname, mapping sinfo)
{
  return LongRangeSkill(me, sname, sinfo, 40);
}

protected mapping StdSkill_Shoot_bolt(object me, string sname, mapping sinfo)
{ 
  return LongRangeSkill(me, sname, sinfo, 40);
}

protected mapping StdSkill_Shoot_dart(object me, string sname, mapping sinfo)
{
  return LongRangeSkill(me, sname, sinfo, 20);
}

protected mapping StdSkill_Shoot_stone(object me, string sname, mapping sinfo)
{
  return LongRangeSkill(me, sname, sinfo, 40);
}

protected string *_query_localcmds() {
    return ({ ({"spruchermuedung","enable_spell_fatigue_compat",0,0}) });
}



// *** Kompatibilitaetsmodus fuer Spellfatigues (Emulation 2s-Alarmzeit) ***

/** Speichert eine Spellfatigue von <duration> Sekunden fuer <key>.
 * Ist hier nur fuer den Spellfatigue-Compat-mode.
 * <key> darf 0 sein und bezeichnet das globale Spellfatigue.
 * Rueckgabewert: Ablaufzeit der gesetzten Sperre
                  -1, wenn noch eine nicht-abgelaufene Sperre auf dem <key> lag.
                  0, wenn duration 0 ist.
 */
public varargs int SetSpellFatigue(int duration, string key) {

  // 0 sollte nie eine Sperre bewirken, auch nicht im compat mode.
  if (!duration) return 0;

  if (spell_fatigue_compat_mode) {
    // Spell-Fatigues auf HBs synchronisieren (2s-Alarmzeit-Emulation).
    // Aufrunden auf ganzzahlige Vielfache von __HEART_BEAT_INTERVAL__
    if (duration % __HEART_BEAT_INTERVAL__)
        ++duration;

    // Startpunkt des Delay soll Beginn der aktuellen Kampfrunde (HB-Zyklus)
    // sein, ggf. um max. eine Sekunde zurueckdatieren.
    // (spell_fatigue_compat_mode hat die Zeit des letzten HB-Aufrufs)
    // Falls durch irgendein Problem (z.B. sehr hohe Last), der letzte HB
    // laenger als 1s zurueckliegt, funktioniert das natuerlich nicht, aber
    // bei fatigue+=spell_fatigue_compat_mode kann der Spieler zuviel Zeit
    // einsparen.
    if (time() > spell_fatigue_compat_mode)
        --duration;  //1s zurueckdatieren
  }

  return ::SetSpellFatigue(duration, key);
}

/** Befehlsfunktion fuer Spieler um den Spellfatigue-Kompatibilitaetsmodus
 * umzuschalten.
 */
public int enable_spell_fatigue_compat(string cmd) {
  if (QueryProp(P_LAST_COMBAT_TIME) + 600 > time()) {
    write(break_string(
      "Im Kampf oder kurz nach einem Kampf kannst Du nicht zwischen "
      "alter und neuer Spruchermuedung umschalten.\n"
      "Momentan benutzt Du die "
      + (spell_fatigue_compat_mode ? "alte (ungenauere)" : "neue (normale)")
      + " Spruchermuedung.",78,0,BS_LEAVE_MY_LFS));
    return 1;
  }

  if (cmd=="alt") {
    spell_fatigue_compat_mode=time();
    write(break_string(
      "Alte Spruchermuedung wurde eingeschaltet. Alle Ermuedungspausen "
      "zwischen Spruechen werden auf Vielfache von 2s aufgerundet und "
      "beginnen in der Regel am Anfang Deiner Kampfrunde."));
    return 1;
  }
  else if (cmd=="neu" || cmd=="normal") {
    spell_fatigue_compat_mode=0;
    write(break_string(
      "Normale Spruchermuedung wurde eingeschaltet. Alle Ermuedungspausen "
      "zwischen Spruechen werden sekundengenau berechnet."));
    return 1;
  }

  notify_fail(break_string(
      "Moechtest Du die alte oder die neue Spruchermuedung?\n"
      "Momentan benutzt Du die "
      + (spell_fatigue_compat_mode ? "alte (ungenauere)" : "neue (normale)")
      + " Spruchermuedung.",78,0,BS_LEAVE_MY_LFS));
  return 0;
}

/** Speichert die Zeit des letztes Heartbeats.
 */
protected void heart_beat() {
  if (spell_fatigue_compat_mode)
    spell_fatigue_compat_mode = time();
}


