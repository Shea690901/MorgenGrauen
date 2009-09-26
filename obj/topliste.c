inherit "std/thing";

#include <properties.h>
#include <language.h>

string _query_long();

void create()
{
  if (!clonep(this_object())) return;
  ::create();
  SetProp(P_SHORT,"Die Top-Liste, Top-Spielerliste, Top-Seherliste und "
      "Top-Hardcoreliste");
  SetProp(P_WEIGHT,0);
  SetProp(P_GENDER,FEMALE);
  SetProp(P_WEIGHT,20);
  AddId(({"liste","topliste","top-liste",
          "topspielerliste","top-spielerliste","topspieler","spielerliste",
          "topseherliste","top-seherliste","topseher","seherliste",
          "tophardcoreliste","top-hardcoreliste","tophardcore","hardcoreliste"}));
  SetProp(P_NAME,"Liste");
  SetProp(P_NOGET,"Die ist an der Wand festgenagelt.\n");
  AddCmd(({"lies","lese"}),"lies");
}

static string whatdenn;

varargs int id(string str,int lvl)
{
  if (!::id(str,lvl))
    return 0;
  whatdenn="/etc/toplist.read";
  if (strstr(str,"seher")>=0)
    whatdenn="/etc/seherlist.read";
  else if (strstr(str,"spieler")>=0)
    whatdenn="/etc/spielerlist.read";
  else if (strstr(str,"hardcore")>=0)
    whatdenn="/etc/hardcorelist.read";
  return 1;
}

int lies(string str)
{
  if (!id(str))
    return 0;
  _query_long();
  return 1;
}

string _query_long()
{
  if (this_player()) this_player()->More(whatdenn,1);
  whatdenn="";
  return "";
}

varargs string long(int mode)
{
  if (this_player()) this_player()->More(whatdenn,1);
  whatdenn="";
  return "";
}
