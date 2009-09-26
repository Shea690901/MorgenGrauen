// MorgenGrauen MUDlib
//
// living/moneyhandler.c -- money handler for livings
//
// $Id: moneyhandler.c 6738 2008-02-19 18:46:14Z Humni $
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

#define NEED_PROTOTYPES
#include <thing/properties.h>
#undef NEED_PROTOTYPES

#include <properties.h>
#include <moving.h>
#include <wizlevels.h>
#include <money.h>

public int AddMoney( int amount )
{
  object ob;
  int ret;

  if ( !amount )
    return 1;
    
  if ( objectp(ob = find_object("/p/daemon/moneylog"))
       && amount > 0
       && query_once_interactive(this_object())
       && !IS_WIZARD(this_object())
       && !Query(P_TESTPLAYER) )
    ob->AddMoney( previous_object(), amount );

  ob = clone_object( "/obj/money" );
  ob->SetProp( P_AMOUNT, amount );

  ret=ob->move( this_object(), M_PUT|M_MOVE_ALL );

  if (ret!=MOVE_OK) 
    ob->remove();
 
  return ret;
}

public int QueryMoney()
{
  object money;
  int geld;

  if ( money = present_clone(GELD, this_object()) )
    geld = money->QueryProp(P_AMOUNT);

  if ( money = present_clone(BOERSE, this_object()) )
    geld += money->QueryProp(P_AMOUNT);

  if ( money = present_clone(SEHERKARTE, this_object()) )
    geld += money->QueryProp(P_AMOUNT);

  return geld;
}
