// MorgenGrauen MUDlib
//
// secure/master/destruct.inc -- module of the master object: stuff for destruct.
//
// $Id: master.c 7041 2008-10-13 18:18:27Z Zesstra $


// privilegierte Objekte, die das destruct() abbrechen duerfen (root objekte
// duerfen auch ohne, dass sie in dieser Liste erfasst sind):
private nosave string *deny_destruct_list = ({
    "/obj/shut", "/room/void", "/room/netztot", "/room/jail" });

// Helferfunktion fuer prepare_destruct()
private void recursive_remove(object ob, int immediate_destruct) {

  if (query_once_interactive(ob)) {
    // Spieler werden ins Void bewegt.
    int res;
    tell_object(ob, "Ploetzlich loest sich deine Welt in ihre " +
                  "Bestandteile auf. Zum Glueck wirst\nDu irgendwo " +
                  "hin geschleudert ...\n");
    // wenn Bewegung buggt oder nicht funktioniert und ob noch existiert,
    // rekursiv zerstoeren.
    object oldenv=environment(ob);
    if ( (catch(res=(int)ob->move("/room/void",M_TPORT|M_NOCHECK,0,"faellt");
           publish) || (ob && environment(ob) == oldenv) )
          && ob) {
            // Spieler speichern, dann erst Inventar entleeren, dann remove() und
        // und destruct() anwenden.
        catch(ob->save_me(1); publish);
        filter(all_inventory(ob), #'recursive_remove, immediate_destruct);
        if (!immediate_destruct) 
          catch(ob->remove(0);publish);
        if (ob) 
          destruct(ob);
    }
  }
  else {
    // kein Spieler. Rekursiv entfernen. Hierbei _zuerst_ rekursiv das
    // Inventar entfernen und dann ob selber, damit nicht erst das Inventar in
    // das Environment bewegt wird (soll ja eh zerstoert werden).
    filter(all_inventory(ob), #'recursive_remove, immediate_destruct);
    // ggf. zuerst remove versuchen
    if (!immediate_destruct)
      catch(ob->remove(1);publish);
    if (ob)
      destruct(ob);
  }
}

// Zerstoerung von ob vorbereiten
protected mixed prepare_destruct(object ob)
{
  object old_env,env,item;
  mixed res;

  // zuerst das notify_destruct() rufen und ggf. abbrechen, falls ob
  // privilegiert ist.
  catch(res = (mixed)ob->NotifyDestruct(previous_object()); publish);
  if (res &&
      (getuid(ob) == ROOTID ||
       (IS_ARCH(ob)) ||
       member(deny_destruct_list, object_name(ob)) >= 0)) {
    if (stringp(res) && strlen(res))
      return res;
    else
      return sprintf("%O verweigert die Zerstoerung mittels destruct(). "
          "Fehlende Rechte von %O?\n",ob, previous_object());
  }
  
//TODO: Mit Reboot entfernen.
#if __BOOT_TIME__ < 1234212788
  // Lichtsystem mit der aenderung versorgen. :-/
  foreach(env : all_environment(ob) || ({})) {
      // Ja. Man ruft die _set_xxx()-Funktionen eigentlich nicht direkt auf.
      // Aber das Lichtsystem ist schon *so* rechenintensiv und gerade der
      // P_LAST_CONTENT_CHANGE-Cache wird *so* oft benoetigt, dass es mir
      // da um jedes bisschen Rechenzeit geht.
      // Der Zweck heiligt ja bekanntlich die Mittel. ;-)
      //
      // Tiamak
      env->_set_last_content_change();
  }
#endif
  env = environment(ob);

  // Objekt hat kein Env: Alles zerstoeren, Spieler ins Void
  if (!env) {
    filter(all_inventory(ob), #'recursive_remove, 1);
  }
  else {
    // Ansonsten alles ins Environment 
    foreach(item : all_inventory(ob))
    {
      old_env=environment(item);
      // M_MOVE_ALL, falls item nen Unitobjekt ist. Sonst clonen die u.U. noch
      // wieder nen neues Objekt im alten Env.
      if(catch(item->move(env, M_NOCHECK|M_MOVE_ALL);publish))
        recursive_remove(item, 1);
      else if (item && environment(item) == old_env)
        recursive_remove(item, 1);
    }
  }

  return 0; // Erfolg
}

string NotifyDestruct(object caller) {
  // Nicht jeder Magier muss den Master entsorgen koennen.
  if ((caller != this_object() && 
        funcall(symbol_function('secure_level)) < ARCH_LVL)
      || funcall(symbol_function('process_call)) ) {
    return "Du darfst den Mudlib-Master nicht zerstoeren!\n";
  }
  return 0;
}

