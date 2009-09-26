/* Das "Potion-Handling" laeuft ueber eine AList. Die Raeume, die Potions ent-
   halten koennen, bekommen eine Zahl zugeordnet. Eine AList hat die Raumnamen
   als Schluessel, die Raumnummern als  Elemente.
   Es existieren 8 weitere AListen, die die "aktiven" Raeume enthalten.
   Einem Spieler wird beim ersten Einloggen eine Liste von 76 Raeumen (ueber
   ihre Nummerns spezifiziert) gegeben. Dabei werden ihm je RoomsPerList Raeume aus der
   ersten bis siebten Liste und 6 Raeume aus der achten Liste zugeordnet.
   Die Raeume sollten nach Schwierigkeit sortiert sein (die einfachsten in
   der ersten, die schwersten in der achten Liste). So ist gewaehrleistet, das
   der Spieler die ersten Advances relativ einfach, die letzten dafuer umso
   schwerer erreicht.
*/
#pragma strict_types
#pragma no_clone
#pragma no_shadow
#pragma no_inherit
#pragma verbose_errors
#pragma combine_strings
//#pragma pedantic
//#pragma range_check
#pragma warn_deprecated

#include "/secure/config.h"
#include "/secure/wizlevels.h"
#define RoomsPerList 10 

#define POTIONDUMP "/secure/ARCH/TRAENKE.dump"
#define DUMP(str) write_file(POTIONDUMP,str)

#define TIPS(x) "/secure/ARCH/ZT/"+x

mapping tipmap;

mixed *all_rooms;
mixed *active_rooms;
mixed *inactive_rooms;
int nextroom;

void save_info()
{
    save_object("/secure/ARCH/potions");
}

private static int secure()
{
  if (!process_call() && previous_object() && this_interactive() && ARCH_SECURITY)
    return 1;
  return 0;
}

void create()
{
    int i;

    seteuid(getuid(this_object()));
    if (!restore_object("/secure/ARCH/potions"))
    {
        all_rooms=({({}),({})});
        active_rooms=allocate(8);
        for (i=0;i<8;i++) active_rooms[i]=({});
        inactive_rooms=allocate(8);
        for (i=0;i<8;i++) inactive_rooms[i]=({});
    }
    all_rooms=order_alist(all_rooms);
    tipmap=([]);
}

int AddPotionroom(string roomname)
{
    if (!secure()) return 0;
    if (!stringp(roomname) || roomname=="") return -1;
    roomname=(string)"/secure/master"->_get_path(roomname,0);
    if (catch(load_object(roomname); publish)) return -2;
    //if (roomname[0..12]!="/room/potion/") return -4;
    if (assoc(roomname, all_rooms, -1)!=-1) return -3;
    all_rooms=insert_alist(roomname, nextroom, all_rooms);
    nextroom++;
    save_info();
    return nextroom;
}

int ChangeRoomPath(string oldpath, string newpath)
{
  int num;

  if (!secure()) return 0;
  if (!stringp(oldpath) || oldpath=="") return -1;
  if (!stringp(newpath) || newpath=="") return -2;
  oldpath=(string)"/secure/master"->_get_path(oldpath,0);
  newpath=(string)"/secure/master"->_get_path(newpath,0);
  if (assoc(oldpath,all_rooms,-1)==-1) return -3;
  if (assoc(newpath,all_rooms,-1)!=-1) return -4;
  if (catch(load_object(newpath);publish)) return -5;
  num=assoc(oldpath, all_rooms);
  all_rooms=remove_alist(oldpath,all_rooms);
  all_rooms=insert_alist(newpath, num, all_rooms);
  save_info();
  return num;
}

int SetListNr(string roomname, int listno)
{
    int nummer;
    int i;

    if (!secure()) return -1;
    nummer=assoc(roomname, all_rooms, -1);
    if (nummer==-1) return -2;
    if (listno<0 || listno>7) return -3;
    for (i=0;i<8;i++)
      if (member(active_rooms[i], nummer)!=-1)
	active_rooms[i]-=({nummer});
    for (i=0;i<8;i++)
      if (member(inactive_rooms[i], nummer)!=-1)
	inactive_rooms[i]-=({nummer});
    active_rooms[listno]+=({nummer});
    save_info();
    return listno;
}

int ActivateRoom(string roomname)
{
    int nummer,listno;
    int i;

    if (!secure()) return -1;
    nummer=assoc(roomname, all_rooms, -1);
    if (nummer==-1) return -2;
    for (i=0;i<8;i++)
        if (member(active_rooms[i], nummer)!=-1) return -3;
    listno=-1;
    for (i=0;i<8;i++)
      if (member(inactive_rooms[i], nummer)!=-1) listno=i;
    if (listno==-1) return -4;
    inactive_rooms[listno]-=({nummer});
    active_rooms[listno]+=({nummer});
    save_info();
    return listno;
}

int DeactivateRoom(string roomname)
{
    int nummer,listno;
    int i;

    if (!secure()) return -1;
    nummer=assoc(roomname, all_rooms, -1);
    if (nummer==-1) return -2;
    for (i=0;i<8;i++)
        if (member(inactive_rooms[i], nummer)!=-1) return -3;
    listno=-1;
    for (i=0;i<8;i++)
      if (member(active_rooms[i], nummer)!=-1) listno=i;
    if (listno==-1) return -4;
    active_rooms[listno]-=({nummer});
    inactive_rooms[listno]+=({nummer});
    save_info();
    return listno;
}

mixed _create_list(int listno, int anz)
{
    mixed *list, tmp;
    int i, j;

    list=({});
    tmp=active_rooms[listno][0..<1];
    while (anz>0)
    {
        anz--;
        if (sizeof(tmp)==0)
        {
//        	write("Fehler ! Liste konnte nicht erzeugt werden ! PANIC !\n");
        	return list;
        }
        if (sizeof(tmp)==1)
        {
          list+=tmp;
          tmp=({});
        }
        else
        {
          j=random(sizeof(tmp));
          list+=({tmp[j]});
          tmp=tmp[0..j-1]+tmp[j+1..<1];
        }
    }
    return list;
}

mixed InitialList()
{
    mixed *list;

    list=({});

    list+=_create_list(0, RoomsPerList);
    list+=_create_list(1, RoomsPerList);
    list+=_create_list(2, RoomsPerList);
    list+=_create_list(3, RoomsPerList);
    list+=_create_list(4, RoomsPerList);
    list+=_create_list(5, RoomsPerList);
    list+=_create_list(6, RoomsPerList);
    list+=_create_list(7, RoomsPerList);

    return list;
}

mixed HasPotion(mixed room)
{
	return assoc(explode(object_name(room),"#")[0], all_rooms, -1);
}

int GetListByNumber(int nr)
{
  int i;
  for (i=0; i<8; i++)
    if (member(active_rooms[i], nr)!=-1)
      return i;
  return -1;
}

int GetInactListByNumber(int nr)
{
  int i;
  for (i=0; i<8; i++)
    if (member(inactive_rooms[i], nr)!=-1)
      return i;
  return -1;
}

mixed GetFilenameByNumber(int nr)
{
  int k;
  k=member(all_rooms[1], nr);
  if (k==-1) return -1;
  return all_rooms[0][k];
}

int InList(mixed room, mixed potionlist, mixed knownlist)
{
	room=assoc(explode(object_name(room),"#")[0], all_rooms, -1);
	if (room==-1) // Gibt's hier ueberhaupt einen ZT?
	  return 0;
	if (member(potionlist, room)==-1) // Hat der Spieler den ZT?
	  return 0;
	if (member(knownlist, room)==-1)  // Kennt der Spieler den ZT?
	  return 0;
	return 1;
}

void RemoveList(mixed room, mixed list, mixed klist)
{
  int tmp;

  room=assoc(explode(object_name(room),"#")[0], all_rooms, -1);

  if (room==-1)
    return;
// Nummern durch -1 ersetzen
  tmp = member(list, room);
  if (tmp!=-1)
    list[tmp] = -1;
  tmp = member(klist, room);
  if (tmp!=-1)
    klist[tmp] = -1;
}

int DumpList()
{
  int i,j,k,s;
	int *LIST;

  rm(POTIONDUMP);
	LIST=all_rooms[1];
  for (i=0;i<8;i++)
  {
		LIST-=active_rooms[i];
    s = sizeof(active_rooms[i]);
    DUMP(sprintf("################## Liste %d ################## (%d)\n\n",
		 i,s));
    for (j=0;j<s;j++)
    {
      DUMP(sprintf("%3d. ",active_rooms[i][j]));
      k=member(all_rooms[1], active_rooms[i][j]);
      if (k==-1)
        DUMP("KEIN ZUGEORDNETER RAUM !!\n");
      else
        DUMP(all_rooms[0][k]+"\n");
    }
    DUMP("\n\n");  
  }
  DUMP(sprintf("################## Inaktiv ##################\n\n"));
  for (i=0;i<8;i++)
  {
    LIST-=inactive_rooms[i];
    s = sizeof(inactive_rooms[i]);

    for (j=0;j<s;j++)
    {
      DUMP(sprintf("%3d. (Liste %d) ",inactive_rooms[i][j],i));
      k=member(all_rooms[1], inactive_rooms[i][j]);
      if (k==-1)
        DUMP("KEIN ZUGEORDNETER RAUM !!\n");
      else
        DUMP(all_rooms[0][k]+"\n");
    }
  }
  DUMP("\n");  
  s=sizeof(LIST);
  for (j=0;j<s;j++)
    {
      DUMP(sprintf("%3d. ",LIST[j]));
      k=member(all_rooms[1], LIST[j]);
      if (k==-1)
	DUMP("KEIN ZUGEORDNETER RAUM !!\n");
      else
	DUMP(all_rooms[0][k]+"\n");
    }
  DUMP("\n\n");
  return 1;
}

//ORAKEL-Routinen

/* Aufbau eines Tips:

   Tralala, lalala
   Dideldadeldum
   Huppsdiwupps
   XXXXX
   Noch ein zweiter Tip
   Dingeldong
   %%%%%

   Die Prozentzeichen sind das Endezeichen, ab hier koennen eventuelle
   Kommentare stehen. Die 5 X sind das Trennzeichen zwischen zwei Tips
   zum selben ZT.
*/

// Der Tip mit der Nummer nr wird bei Bedarf von der Platte gelesen und
// gespeichert. Das Ergebnis ist im Normalfall eine Liste von Strings,
// im Fehlerfall ist es ein negativer integer-Wert.
mixed TipLesen(int nr)
{
   string  tmpstr;
   int	   start;
   string  fn;
   string  tip;
   string* tips;
   string  ret;

   if (previous_object() != find_object("/room/orakel"))
	   return 0;

//   write_file("/log/ARCH/zt.log",
//		 sprintf("%s: ZT-Tip gelesen: %d von TP: %O, PO: %O, TI: %O\n",
//			 dtime(time()), nr, this_player(),
//			 previous_object(), this_interactive()));

   if (ret=tipmap[nr])	 // Schon bekannt
      return ret;

   fn = TIPS(nr+".zt");
   if (file_size(fn) == -1)
      return -1;   // Datei existiert nicht, Fehler

   tmpstr = "";
   start = 1;
   tips = ({ });
   tip = "";

  while (tmpstr[0..4]!="%%%%%")
   {
      tmpstr = read_file(fn, start++, 1);
      if (tmpstr[0..4] == "XXXXX" || tmpstr[0..4] == "%%%%%")
      {
	 tips += ({ tip });
	 tip = "";
      }
      else
	 tip += tmpstr;
   }
   if (sizeof(tips))
   {
      tipmap[nr] = tips;
      return tips;
   }
   else // Datei ist leer oder es ist sonstwas schlimmes passiert, Fehler
      return -2;
}

