// MorgenGrauen MUDlib
//
// skills.h -- Defines and prototypes for /std/living/skills.c
//
// $Id: skills.h 6776 2008-03-15 18:12:42Z Zesstra $

#ifndef _LIVING_SKILLS_H_
#define _LIVING_SKILLS_H_

#ifdef NEED_PROTOTYPES

public    varargs int     UseSpell(string str, string spell);

public    varargs mapping QuerySkill(string sname, string gilde);
public    varargs int     QuerySkillAbility(mixed sname, mixed argl);

protected varargs mixed   LimitAbility(mixed sinfo, int diff);
public    varargs mixed   UseSkill(string skill, mapping args);

public    varargs void    LearnSkill(string sname, int add, int diff);
public    varargs void    ModifySkill(string sname, mixed val, int diff, 
                                      string gilde);

// _set- und _query-Funktionen

static  int     _query_guild_level();
static  int     _set_guild_level(int num);

static  varargs mapping _query_newskills(string gilde);
static  varargs mapping _set_newskills(mapping value, string gilde);

static  string  _query_guild();
static  string  _query_guild_title();
static  string  _query_title();
static  string  _query_visible_guild();
static  string  _set_guild(string gildenname);
static  string  _set_guild_title(string t);

#endif

#endif

