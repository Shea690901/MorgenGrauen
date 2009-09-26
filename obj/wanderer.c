inherit "std/npc";

#define DELAY 60
#define MAXWAY 30

#include <properties.h>
#include <language.h>
#include <moving.h>
#include "/secure/questmaster.h"

create()
{
  if ( clonep(this_object()) ){
	  destruct(this_object());
	  return;
  }
  
  ::create();
  SetProp(P_NAME,"Wanderer");
  set_living_name("orakel");
  SetProp(P_GENDER, MALE);
  SetProp(P_SHORT,"Ein alter Wanderer");
  SetProp(P_LONG,
  "Ein uralter Wanderer mit einem langen Hirtenmantel und einem Hirtenstab.\n");
  AddId(({"wanderer","hirte","orakel"}));
	create_default_npc( 19 );
  SetProp(P_MAX_HP, 5000);
  SetProp(P_HP, 5000);
  SetProp(P_GENDER,MALE);
  SetProp(P_ALIGN, 1000);
  SetProp(P_XP, 1000);
  SetProp(P_HANDS, ({" mit seinem Hirtenstab", 50}) );
  SetProp(P_NO_ATTACK, 1);
  SetProp(P_REJECT, ({ REJECT_GIVE, Name(WER) + " sagt: Danke, aber das "
			  "brauche ich nicht.\n" }) );
  call_out("lauf", DELAY);
  // Sollte eigentlich ueber das `info'-System laufen ;(
  AddCmd( ({"frag","frage"}), "frag" );
}

frag(str)
{
  int i,j,geredet;
  mixed *quests;
  string tmp;
  
  if (!str) return;
  str=lower_case(str);
  str=old_explode(str," ");
  if (!sizeof(str)) return;
  if (!id(str[0])) return;
  if (sizeof(str)<3 || (str[1]!="nach" && str[1]!="ueber"))
  {
    tell_object(this_player(),"Der Wanderer sagt: Ich versteh dich nicht.\n");
    return 1;
  }
  geredet=0;
  str=implode(str[2..]," ");
  if (str=="aufgabe" || str=="aufgaben" || str=="abenteuer")
  {
    tell_object(this_player(),"Der Wanderer sagt: Folgende Aufgaben gibt es hier:\n");
    quests=QM->QueryQuests();
    for (i=0;i<sizeof(quests[0]);i++)
    {
      tell_object(this_player(),quests[0][i]);
      if (this_interactive()->QueryQuest(quests[0][i]))
        tell_object(this_player()," [hast Du schon geloest]");
      tell_object(this_player(),"\n");
    }
    return 1;
  }
  quests=QM->QueryQuests();
  for (i=0;i<sizeof(quests[0]);i++)
  {
    if (member_array(str,old_explode(lower_case(quests[0][i])," "))>=0
	|| strstr(lower_case(quests[0][i]), str)>=0)
    {
      tmp=quests[0][i]+"? Dazu kann ich Dir folgendes sagen: ";
      tmp+=quests[1][i][3];
      tmp=old_explode(break_string(tmp,60),"\n");
      geredet=1;
      for (j=0;j<sizeof(tmp);j++)
	tell_object(this_player(),"Der Wanderer sagt: "+tmp[j]+"\n");
    }
  }
  if (!geredet) 
    tell_object(this_player(),"Der Wanderer sagt: "+capitalize(str+" ")+"? Interessiert mich nicht !\n"); 
  return 1;
}

lauf()
{
  int i;
  mapping liste;
  string *dirs;
  string *dirs2;
  int ok;
  
  liste = environment()->QueryProp(P_EXITS);
  dirs = m_indices(liste);
  dirs2 = ({ });
  for (i=0;i<sizeof(liste);i++)
    if (!catch(ok=liste[dirs[i]]->QueryProp(P_ORAKEL)) && ok) 
      dirs2 += ({ dirs[i] });
  
  if (sizeof(dirs2))
  {
    i=random(sizeof(dirs2));
    command_me(dirs2[i]);
  }
  call_out("lauf",DELAY);
}
