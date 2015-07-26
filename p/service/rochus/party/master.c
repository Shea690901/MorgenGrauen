// Boardmaster, von Jof

#define __PARTY_NEED_ROOMS__
#include "party.h"
#include <wizlevels.h>
#include <daemon.h>
#include <moving.h>

mapping data;

create()
{
  seteuid(getuid());
  if (!restore_object(SAVEFILE))
    data=([]);
}

ok()
{
  return ((geteuid()==geteuid(previous_object()))
	  || IS_ELDER(this_interactive()));
}

QueryData()
{
  if (ok()) return data;
}

QueryUser(name)
{
  if(ok()) return data[name];
}

DeleteUser(name)
{
  if (!ok()) return;
  if (!data[name]) return;
  m_delete(data,name);
  save_object(SAVEFILE);
  return 1;
}

SetUser(name,what)
{
  if (!ok()) return;
  if (!mappingp(what)) return;
  data[name]=what;
  save_object(SAVEFILE);
}

nomask void StartQuery()
{
  int i;
  object board, env;
  string envnam;

  if (!this_interactive() || !IS_ELDER(this_interactive())) {
    write("Das darf nur ein Erzmagier!\n");
    return;
  }

  for (i=sizeof(BOARD_ROOMS)-1; i>=0; i--) {
    envnam = BOARD_ROOMS[i];
    catch(call_other(envnam, "???"));
    if (env = find_object(envnam)) {
      if (!present("PartyBrett", env)) {
	board = clone_object("/p/service/rochus/party/brett");
	board->move(env, M_NOCHECK);
	OBJECTD->AddObject(board, envnam);
	printf("Partybrett in '%s.c' aufgestellt.\n", envnam);
      }
      else
	printf("In '%s.c' befindet sich schon ein Partybrett!\n", envnam);
    }
    else
      printf("Kann kein Partybrett in '%s.c' aufstellen!\nDer Raum laesst sich "
	     "nicht laden!\n", envnam);
  }
}

nomask void EndQuery()
{
  int i;
  object board, env;
  string envnam;

  if (!this_interactive() || !IS_ELDER(this_interactive())) {
    write("Das darf nur ein Erzmagier!\n");
    return;
  }

  for (i=sizeof(BOARD_ROOMS)-1; i>=0; i--) {
    envnam = BOARD_ROOMS[i];
    catch(call_other(envnam, "???"));
    if (env = find_object(envnam)) {
      if (board=present("PartyBrett", env)) {
	OBJECTD->RemoveObject(board, envnam);
        env->RemoveItem("/p/service/rochus/party/brett");
	printf("Partybrett in '%s.c' abgebaut.\n", envnam);
      }
      else
	printf("In '%s.c befindet sich kein Partybrett!?\n", envnam);
    }
    else
      printf("Kann kein Partybrett in '%s.c' abbauen!\nDer Raum laesst sich "
	     "nicht laden!\n", envnam);
  }
}
