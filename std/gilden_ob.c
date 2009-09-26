// MorgenGrauen MUDlib
//
// gilden_ob.c -- Basisfunktionen einer Gilde
//
// $Id: gilden_ob.c 6380 2007-07-20 22:32:38Z Zesstra $
#pragma strict_types
#pragma save_types
#pragma no_shadow
#pragma no_clone
#pragma range_check
#pragma pedantic

inherit "/std/restriction_checker";
#include <properties.h>
#include <attributes.h>
#include <new_skills.h>

void create() {
    // P_GUILD_SKILLS sollte nicht direkt von aussen manipuliert werden...
    SetProp(P_GUILD_SKILLS,([]));
    Set(P_GUILD_SKILLS,PROTECTED,F_MODE_AS);
}

mapping _query_guild_skills() {
    // eine Kopie zurueckliefern, um versehentliche Aenderungen des
    // Originalmappings in der Gilde zu vermeiden.
    return(deep_copy(Query(P_GUILD_SKILLS)));
}

string GuildName() {
  //Gilden muessen Blueprints sein, so. ;-)
  return object_name(this_object())[8..];
}

varargs int
IsGuildMember(object pl) {
  string plg;

  if (!pl && !(pl=this_player()))
    return 0;
  if (!(plg=(string)pl->QueryProp(P_GUILD)))
    return 0;
  if (GuildName()!=plg) {
    _notify_fail("Du gehoerst dieser Gilde nicht an!\n");
    return 0;
  }
  return 1;
}

int check_cond(mixed cond) {
  string res;

  if (intp(cond)) {
    _notify_fail("Dir fehlt noch die noetige Erfahrung. "+
                "Komm spaeter wieder.\n");
    return (this_player()->QueryProp(P_XP)>=cond);
  }
  if (mappingp(cond)) {
    res=check_restrictions(this_player(),cond);
    if (!stringp(res)) return 1;
    _notify_fail(res);
    return 0;
  }
  return 1;
}

int can_advance() {
  int lv;
  mapping lvs;

  if (!(lv=(int)this_player()->QueryProp(P_GUILD_LEVEL))) return 1;
  if (!(lvs=QueryProp(P_GUILD_LEVELS))) return 0;
  return check_cond(lvs[lv+1]); // Bedingung fuer naechsten Level testen.
}

void adjust_title(object pl) {
  int lv;
  mixed ti;


  if (!pl ||
      !(lv=(int)pl->QueryProp(P_GUILD_LEVEL)))
    return;
  switch((int)pl->QueryProp(P_GENDER)) {
  case MALE:
    ti=QueryProp(P_GUILD_MALE_TITLES);break;
  case FEMALE:
    ti=QueryProp(P_GUILD_FEMALE_TITLES);break;
  default:
    return;
  }
  if (mappingp(ti))
    ti=ti[lv];
  if (stringp(ti))
    pl->SetProp(P_GUILD_TITLE,ti);
}

void do_advance() {
  int lv;

  lv=this_player()->QueryProp(P_GUILD_LEVEL)+1;
  if (lv<1) lv=1;
  this_player()->SetProp(P_GUILD_LEVEL,lv);
  adjust_title(this_player());
}

int try_advance() {
  if (can_advance()) {
    if (IsGuildMember(this_player()))
      do_advance();
    return 1;
  }
  return 0;
}

int beitreten() {
  string res;
  int erg;

  if (res=check_restrictions(this_player(),QueryProp(P_GUILD_RESTRICTIONS))) {
    // Werden die Beitrittsbedingungen erfuellt?
    printf("Du kannst dieser Gilde nicht beitreten.\nGrund: %s",res);
    return -3;
  }
  if (erg=(int)GUILDMASTER->beitreten()) {
    if (erg<0)
      return erg;
    if (!(this_player()->QueryProp(P_GUILD_LEVEL)))
      try_advance(); // Level 1 wird sofort vergeben
    return 1;
  }
  return 0;
}

varargs int austreten(int loss) {
    return (int)GUILDMASTER->austreten(loss);
}

int bei_oder_aus_treten(string str) {
  if (!str) return 0;
  if (sizeof(regexp(({lower_case(str)}),
                    "\\<aus\\>.*gilde\\>.*\\<aus\\>")))
    return austreten();
  if (sizeof(regexp(({lower_case(str)}),
                     "(gilde\\>.*\\<bei\\>|\\<in\\>.*gilde\\>.*\\<ein\\>)")))
    return beitreten();
  return 0;
}

varargs int
AddSkill(string sname, mapping ski) {
  mapping skills;

  if (!sname)
    return 0;

  // per Query() abfragen, hier ist keine Kopie noetig.
  if (!mappingp(skills=Query(P_GUILD_SKILLS))) {
    skills=([]);
    SetProp(P_GUILD_SKILLS,skills);
  }
  
  if (!mappingp(ski))
      ski=([]);
  else
      //Zur Sicherheit Kopie erstellen, wer weiss, was der Eintragende noch
      //mit seinem Mapping macht...
      ski=deep_copy(ski);

  if (!stringp(ski[SI_SKILLFUNC]))
    // Wenn keine Funktion angegeben ist, Funktionsname=Skillname
    ski[SI_SKILLFUNC]=sname;

  // Gilden-Offsets addieren
  ski=AddSkillMappings(QueryProp(P_GLOBAL_SKILLPROPS),ski);

  // Skill setzen.
  skills[sname]=ski;
  //SetProp() unnoetig, da oben per Query() das Originalmapping erhalten
  //wurde.
  //SetProp(P_GUILD_SKILLS,skills);
  return 1;
}


varargs int
AddSpell(string verb, mapping ski) {
  mapping skills;

  if (!verb)
      return 0;
  if (!mappingp(ski))
      ski=([]);

  if (!stringp(ski[SI_SPELLBOOK]) &&
      !stringp(ski[SI_SPELLBOOK]=QueryProp(P_GUILD_DEFAULT_SPELLBOOK)))
    // Wenn kein Spellbook angegeben ist muss ein
    // Default-Spellbook angegeben sein, sonst Fehler
    return 0;
  if (file_size(SPELLBOOK_DIR+ski[SI_SPELLBOOK]+".c")<0)
    return 0; // Spellbook sollte auch existieren...

  return AddSkill(lower_case(verb),ski);
}

mapping QuerySkill(string skill) {
  mapping ski;

  // Abfrage per Query(), da vielleicht nur ein Skill gewuenscht
  // wird und daher die komplette Kopie des Spellmappings in der Query-Methode
  // unnoetig ist.
  if (!skill
      || !(ski=Query(P_GUILD_SKILLS, F_VALUE))
      || !(ski=ski[skill])) // Gildenspezifische Skilleigenschaften
    return 0;
  // hier aber dann natuerlich eine Kopie erstellen. ;-)
  return(deep_copy(ski));
}

mapping QuerySpell(string spell) {
  mapping ski,ski2;
  string spellbook,sfunc;

  // QuerySkill() hier und QuerySpell() im Spellbook liefern Kopien der
  // Skillmappings zurueck, Kopieren hier daher unnoetig.
  if (!spell
      || !(ski=QuerySkill(spell))
      || !(spellbook=ski[SI_SPELLBOOK]))
    return 0;
  if (!(sfunc=ski[SI_SKILLFUNC]))
    sfunc=spell;
  spellbook=SPELLBOOK_DIR+spellbook;
  if (!(ski2=(mapping)(spellbook->QuerySpell(sfunc))))
    return 0;
  return AddSkillMappings(ski2,ski); // Reihenfolge wichtig!
  // Die Gilde kann Spelleigenschaften neu definieren!
}

varargs int
UseSpell(object caster, string spell, mapping sinfo) {
  mapping ski;
  string spellbook;

  if (!caster
      || !spell
      || !(ski=QuerySkill(spell)) // Existiert dieser Spell in dieser Gilde?
      || !(spellbook=ski[SI_SPELLBOOK])) // Spellbook muss bekannt sein
    return 0;
  if (sinfo)
    ski+=sinfo;
  spellbook=SPELLBOOK_DIR+spellbook;
  // printf("%O %O %O %O\n",spellbook,caster,spell,ski);
  return (int)spellbook->UseSpell(caster,spell,ski);
}

static int
InitialSkillAbility(mapping ski, object pl) {
  if (!ski || !pl) return 0;
  return (300*GetOffset(SI_SKILLLEARN,ski,pl)+
          (200*(int)pl->QueryAttribute(A_INT)*
           GetFactor(SI_SKILLLEARN,ski,pl))/100);
}

int
SkillListe(int what) {
  mapping skills;
  string *ind,*info,*info2,s,sp;
  int i,res;
  
  if (!mappingp(skills=QueryProp(P_GUILD_SKILLS))) return 0;
  if (!sizeof(ind=m_indices(skills))) return 0;
  info=({});info2=({});
  for (i=sizeof(ind)-1;i>=0;i--) {
    if (stringp(s=skills[sp=ind[i]][SI_SKILLINFO]))
      s=sprintf("%-20s (%s)",sp,s);
    else
      s=sp;
    if (capitalize(ind[i][0..0])!=ind[i][0..0])
      // Spells werden klein geschrieben, Skills gross
      info+=({s});
    else
      info2+=({s});
  }
  info=sort_array(info,#'>);info2=sort_array(info2,#'>);
  res=0;
  if ((what & 0x01) && (i=sizeof(info))) {
    write("Du kannst versuchen, folgende Zaubersprueche zu lernen:\n");
    for (--i;i>=0;i--)
      printf("   %s\n",capitalize(info[i]));
    res=1;
  }
  if ((what & 0x02) && (i=sizeof(info2))) {
    write("Du kannst versuchen, folgende Faehigkeiten zu erwerben:\n");
    for (--i;i>=0;i--)
      printf("   %s\n",capitalize(info2[i]));
    res=1;
  }
  return res;
}

varargs int
LearnSpell(string spell,object pl) {
  mapping ski,restr;
  string res;
  mixed learn_initfunc;
  int abil,diff;

  // Wenn kein pl gesetzt ist, nehmen wir this_player(), das ist der
  // Normalfall.
  if (!pl)
	  pl=this_player();
  if (!IsGuildMember(pl)) {
    _notify_fail("Du gehoerst dieser Gilde nicht an!\n");
    return 0;
  }
  _notify_fail("Was moechtest Du lernen?\n");
  if (!spell)
    return SkillListe(0x01);
  spell=lower_case(spell);
  if (!(ski=QuerySpell(spell)))
    return 0;
  if (pl->QuerySkill(spell)) {
    _notify_fail("Du kannst diesen Spruch doch schon!\n");
    return 0;
  }
  if ((restr=ski[SI_SKILLRESTR_LEARN])
      && (res=check_restrictions(pl,restr))) {
    printf("Du kannst diesen Spruch noch nicht lernen.\nGrund: %s",res);
    return 1;
  }
  abil=InitialSkillAbility(ski,pl);
  if (abil<1) abil=1;
  if (abil>7500) abil=7500;
  write("Du lernst einen neuen Zauberspruch.\n");
  if (!(diff=GetFValueO(SI_DIFFICULTY,ski,pl)))
    diff=GetFValueO(SI_SPELLCOST,ski,pl);
  pl->ModifySkill(spell,abil,diff);
  return 1;
}

int
LearnSkill(string skill) {
  mapping ski,restr;
  object pl;
  string res;
  mixed learn_initfunc;
  int abil,diff;

  if (!IsGuildMember(pl=this_player())) {
    _notify_fail("Du gehoerst dieser Gilde nicht an!\n");
    return 0;
  }
  _notify_fail("Was moechtest Du lernen?\n");
  if (!skill)
    return SkillListe(0x02);
  skill=capitalize(skill);
  if (!(ski=QuerySkill(skill)))
    return 0;
  if (pl->QuerySkill(skill)) {
    _notify_fail("Du hast diese Faehigkeit doch schon!\n");
    return 0;
  }
  if ((restr=ski[SI_SKILLRESTR_LEARN])
      && (res=check_restrictions(pl,restr))) {
    printf("Du kannst diese Faehigkeit noch nicht erwerben.\nGrund: %s",res);
    return 1;
  }
  abil=InitialSkillAbility(ski,pl);
  if (!abil) abil=1;
  if (abil>MAX_ABILITY) abil=MAX_ABILITY;
  if (abil<-MAX_ABILITY) abil=-MAX_ABILITY;
  write("Du erwirbst eine neue Faehigkeit.\n");
  diff=GetFValueO(SI_DIFFICULTY,ski,pl);
  pl->ModifySkill(skill,abil,diff);
  return 1;
}

int GuildRating(object pl)
{
  mapping ski;
  string *sk;
  int i, cnt, sum;
  closure qsa;
  
  if (!IsGuildMember(pl)||!query_once_interactive(pl))
    return 0;

  if (!(ski = QueryProp(P_GUILD_SKILLS)) ||
      !(qsa = symbol_function("QuerySkillAbility",pl)))
    return 0;

  sk = m_indices(ski);
  cnt = sizeof(ski);
  
  for (i=cnt-1, sum=0; i>=0; i--)
    sum += funcall(qsa, sk[i]);

  sum = sum/cnt;
  if (sum < 0)
    sum = 0;
  else if (sum > MAX_ABILITY)
    sum = MAX_ABILITY;
  
  return (int)pl->SetProp(P_GUILD_RATING, sum);
}

// Wird von /std/player/quest.c aufgerufen, wenn Quest geloest.
// (Jedes mal - nicht nur beim ersten mal.)
// Ist zum selbst-ueberschreiben gedacht, sollte aber definiert sein.
void NotifyGiveQuest(object pl, string key){  }

