inherit "/std/container";

#define GELDBOERSE "\ngeldboerse2"
#define U_REQ      "u_req"

#include <properties.h>
#include <language.h>
#include <defines.h>
#include <moving.h>

void create()
{
  ::create();
  SetProp(P_NAME, "Geldboerse");
  SetProp(P_SHORT, "Eine Geldboerse");
  SetProp(P_LONG,  "Eine schoene aus Leder gefertigte Geldboerse.\n");
  SetProp(P_MATERIAL, ([MAT_LEATHER:100]));
  SetProp(P_GENDER, FEMALE);
  SetProp(P_VALUE,  80);
  SetProp(P_WEIGHT, 300);
  SetProp(P_WEIGHT_PERCENT, 50);
  SetProp(P_MAX_WEIGHT, 250000); // 1 mio. Muenzen.
  AddId(({"geldboerse", "boerse", "\ngeldboerse"}));
  AddCmd(({"steck", "stecke", "leg", "lege"}), "put_thing");
}

static string _query_keep_on_sell()
{
  if (sizeof(all_inventory()) && living(environment()))
     return getuid(environment());
  return Query(P_KEEP_ON_SELL);
}

static int _query_amount()
{
  object ob;
  if (ob=present("geld", this_object()))
    return ob->QueryProp(P_AMOUNT);
}

void AddAmount(int am)
{
  object ob;
  if (ob=present("geld", this_object())) ob->AddAmount(am);
}

string short()
{
  int i;
  switch (i=QueryProp(P_AMOUNT)) {
    case 0:  return "Eine leere Geldboerse.\n";
    case 1:  return "Eine Geldboerse mit einer Muenze.\n";
    default: return "Eine Geldboerse mit "+i+" Muenzen.\n";
  }
}

varargs int PreventInsert(object ob)
{
  object ob2;
  if (ob && object_name(ob)[0..9]=="/obj/money")
    if (!sizeof(all_inventory()) && living(environment()) &&
        (ob2=present(GELDBOERSE, environment()))) {
      write("Du kannst immer nur eine Geldboerse gleichzeitig benutzen.\n");
      return 1;
    }
    else return (int)::PreventInsert(ob);
  else return 1;
}

static int put_thing(string str)
// sehr unschoen, aber es verhindert eine doppelte Fehlermeldung im put_and_get
// wie es sie geben wuerde, wenn dies nur ueber PreventInsert geregelt waere.
{
  string s1,s2;

  if (!str || sscanf(str, "%s in %s", s1, s2)!=2) return 0;
  if (!id(s2)) return 0;
  if (!sizeof(all_inventory()) && living(environment()) &&
       present(GELDBOERSE, environment())) {
    write("Du kannst immer nur eine Geldboerse gleichzeitig benutzen.\n");
    return 1;
  }
}

varargs int id(string str, int lvl)
{
  if (str==GELDBOERSE && sizeof(all_inventory())) return 1;
  return (int)::id(str, lvl);
}

int IsEqual(object ob)
{
  int amount;
  object ob2;

  if (object_name(ob)[0..9]!="/obj/money" || environment(ob)==ME) return 0;
  if (!sizeof(all_inventory()) && (ob2=present(GELDBOERSE, environment())))
    return 0;
  if ( 0 > amount=ob->Query(U_REQ)) {
    int amount2;
    amount2=QueryProp(P_AMOUNT);
    if (-amount>amount2) {
      ob->AddAmount(amount2);
      AddAmount(-amount2);
    }
    else {
      ob->AddAmount(-amount);
      AddAmount(amount);
    }
    ob2=ME;
    SetProp(P_LAST_CONTENT_CHANGE,time());
    while (ob2=environment(ob2))
       ob2->SetProp(P_LAST_CONTENT_CHANGE,time());
  }
  /* Falls gewuenscht, kann man das jederzeit so einbauen...
  else if (amount && query_verb() && query_verb()[0..6]=="verkauf") {
    ob->AddAmount(-amount);
    AddAmount(amount);
    ob2=this_object();
    Set(P_LAST_CONTENT_CHANGE,time());
    while (ob2=environment(ob2))
       ob2->Set(P_LAST_CONTENT_CHANGE,time());
  }
  */
}

varargs int move(mixed dest, int method)
{
  int i;
  object ob;

  if (stringp(dest)) {
    call_other(dest, "???");
    dest=find_object(dest);
  }
  if (objectp(dest)) ob=present(GELDBOERSE, dest);
  i=(int)::move(dest, method);
  if (i!=1) return i;
  if (!living(environment())) return 1;
  if (objectp(ob)) filter_objects(all_inventory(), "move", ob, M_MOVE_ALL);
  return 1;
}
