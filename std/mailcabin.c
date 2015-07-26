#pragma strict_types,save_types

inherit "/p/service/loco/std/mailcabin";

#include <properties.h>
#define TP this_player()
#define ME this_object()
#ifndef P_SWAP_CHANNELS
#define P_SWAP_CHANNELS "swap_channels"
#endif

private nosave mapping store_player_props;

void create() {
  if (!clonep() || object_name(this_object()) == __FILE__[0..<3]) {
      set_next_reset(-1);
      return;
  }
  ::create();
  store_player_props=([]);
}

int rein(string str) {
  mixed x,away;
  int res;
  
  if (environment(TP)==ME)
    return 0;
  res=::rein(str);
  if (environment(TP)!=ME || !query_once_interactive(TP))
    return res;
  if (store_player_props[getuid(TP)])
    return res;
  if (!pointerp(x=(mixed)TP->QueryProp(P_IGNORE)))
    x=({});
  away=(string)TP->QueryProp(P_AWAY);
  store_player_props[getuid(TP)]=({x[0..],
    away,
    TP->QueryProp(P_SWAP_CHANNELS),
    TP->QueryProp(P_BUFFER)});
  if (member(x,"ruf")<0) x+=({"ruf"});
  if (member(x,"rufe")<0) x+=({"rufe"});
  TP->SetProp(P_IGNORE,x);
  if (!away)
    TP->SetProp(P_AWAY,"Liest oder schreibt Mail.");
  // TP->ChannelAdmin("aus");
  TP->_flush_cache("ein");
  return res;
}

int _normalfunction() {
  mixed x,stor;
  int res;
  
  if (environment(TP)!=ME)
    return 0;
  res=::_normalfunction();
  if (environment(TP)==ME || !query_once_interactive(TP))
    return res;
  if (!pointerp(stor=store_player_props[getuid(TP)]))
    stor=({({}),0,0,0});
  TP->SetProp(P_IGNORE,stor[0]);
  TP->SetProp(P_AWAY,stor[1]);
  if (!stor[2] && 0)
    TP->ChannelAdmin("ein");
  if (!stor[3])
    TP->_flush_cache("aus");
  efun::m_delete(store_player_props,getuid(TP));
  return res;
}
