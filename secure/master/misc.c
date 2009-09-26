// MorgenGrauen MUDlib
//
// master/misc.c -- Diverses: (T)Banish, Projektverwaltung, Levelaufstieg, ...
//
// $Id: misc.c 7288 2009-09-16 22:09:57Z Zesstra $

#pragma strict_types

#include "/secure/master.h"
#include "/mail/post.h"

static mixed *banished;
static mapping tbanished, sbanished;
static string *deputies;

static string *explode_files(string file) {
  string data;
  mixed *exploded;
  int i;

  data=read_file(file);
  if (!data || !stringp(data) || data == "") return ({});
  exploded = efun::explode(data,"\n");
  for (i=sizeof(exploded);i--;)
    if (!stringp(exploded[i]) || exploded[i]=="" || exploded[i][0]=='#')
      exploded[i]=0;
  exploded-=({0});
  printf("%-30s: %3d Objekt%s\n",file,i=sizeof(exploded),(i==1?"":"e"));
  return exploded;
}

void UpdateTBanish();

mixed QueryBanished(string str){
  int i;

  if (!str) return 0;
  if (!pointerp(banished)) return 0;
  for (i=sizeof(banished)-1;i>=0;i--)
    if (sizeof(regexp(({str}),"^"+banished[i][0]+"$")))
    {
      if (!banished[i][1] || banished[i][1]=="")
        return "Dieser Name ist gesperrt.";
      else
        return banished[i][1];
    }
  return 0;
}

mixed QueryTBanished(string str) {
  int i;

  if (!str || !mappingp(tbanished) || !(i=tbanished[str]))
    return 0;

  if (i == -1 || i > time())
    return sprintf("Es gibt schon einen Spieler diesen Namens.\n"
        +"Allerdings kann er/sie erst am %s wieder ins Mud kommen.\n",
          (i == -1 ? "St. Nimmerleinstag" :
           funcall(symbol_function('dtime/*'*/),i)[0..16]));

  // Ansonsten: die Zeit ist abgelaufen, Spieler darf wieder...
  efun::m_delete(tbanished, str);
  UpdateTBanish();
  return 0;
}

void ReloadBanishFile(){
    int i, t;
    string s1, s2, *s;

    banished = efun::explode( read_file("/secure/BANISH") || "", "\n" );
    banished = banished - ({""});
    for ( i = sizeof(banished); i--; ){
        s = efun::explode( banished[i], " " );
        s1 = s[0];
        s2 = implode( s[1..], " " );
        banished[i] = ({ s1, s2 });
    }

    if ( !mappingp(tbanished) ){
        tbanished = ([]);

        s = efun::explode( read_file("/secure/TBANISH") || "", "\n" );
        s -= ({""});

        for ( i = sizeof(s); i--; ){
            sscanf( s[i], "%s:%d", s1, t );
            tbanished += ([ s1 : t ]);
        }
    }

    if ( !mappingp(sbanished) ){
        sbanished = ([]);

        s = efun::explode( read_file("/secure/SITEBANISH") || "", "\n" );
        s -= ({""});

        for ( i = sizeof(s); i--; ){
            sscanf( s[i], "%s:%d:%s", s1, t, s2 );
            sbanished += ([ s1 : t; s2 ]);
        }
    }
}

int IsDeputy(mixed name)
{
    if ( IS_ARCH(name) )
        return 1;

    if ( objectp(name) )
        name = getuid(name);

    if ( member( deputies || ({}), name ) >= 0 )
        return 1;

    return 0;
}


varargs void BanishName( string name, string reason, int force )
{
  string *names;
  int i;

  if ( PO != TO && funcall(symbol_function('secure_level)) < LORD_LVL
           && !IsDeputy( funcall(symbol_function('secure_euid)) ) )
      return;

  if ( !stringp(name) || !strlen(name) )
      return;

  name = lower_case(name);

  if ( !reason || !stringp(reason) )
      reason = "";

  if ( QueryBanished(name) && lower_case(reason) != "loeschen" ){
      write("Der Name ist schon gebannt.\n");
      return;
  }

  if ( !force && file_size(SAVEPATH+name[0..0]+"/"+name+".o") > 0 ){
      write("Es existiert bereits ein Spieler dieses Namens.\n");
      return;
  }

/*  if (!("/secure/login"->valid_name(name))) return;*/
  if ( lower_case(reason) != "loeschen" ){
      names = ({ name + " " + reason });

      for ( i = sizeof(banished); i--; )
          names += ({ banished[i][0] + " " + banished[i][1] });
  }
  else{
      names = ({});

      for ( i = sizeof(banished); i--; )
          if ( banished[i][0] != name )
              names += ({ banished[i][0] + " " + banished[i][1] });
  }

  names = sort_array( names, #'> );
  rm("/secure/BANISH");
  write_file( "/secure/BANISH", implode(names, "\n") );
  write( "Okay, '"+capitalize(name)+"' ist jetzt "+
      (lower_case(reason) == "loeschen" ? "nicht mehr " : "")+"gebanisht.\n" );
  ReloadBanishFile();
}

void UpdateTBanish()
{
  int i;
  string *names;

  for (i=sizeof(names = sort_array(m_indices(tbanished), #'</*'*/))-1;i>=0;i--)
    names[i] = sprintf("%s:%d", names[i], tbanished[names[i]]);

  rm("/secure/TBANISH");
  write_file("/secure/TBANISH", implode(names, "\n"));
}

void UpdateSBanish()
{
    int i;
    string *names;

    for ( i = sizeof(names = sort_array(m_indices(sbanished), #'</*'*/)); i--; )
        names[i] = sprintf( "%s:%d:%s", names[i], sbanished[names[i],0],
                            sbanished[names[i],1] );

    rm( "/secure/SITEBANISH" );
    write_file( "/secure/SITEBANISH", implode( names, "\n" ) );
}

int TBanishName(string name, int days)
{
  int t;

  if ( (getuid(TI) != name) &&
       !IsDeputy( funcall(symbol_function('secure_euid/*'*/)) ) )
    return 0;

  if (days && QueryTBanished(name)){
    write("Der Name ist schon gebannt.\n");
    return 0;
  }

  if (file_size(SAVEPATH+name[0..0]+"/"+name+".o")<=0){
    write("Es existiert kein Spieler dieses Namens!\n");
    return 0;
  }

  if (!days && member(tbanished, name))
    efun::m_delete(tbanished, name);
  else {
    if (!mappingp(tbanished))
      tbanished = ([]);
    if (days <= -1)
      t = -1;
    else
      t = (time()/86400)*86400 + days*86400;
    tbanished += ([ name : t ]);
  }

  UpdateTBanish();
  return 1;
}


mixed QuerySBanished( string ip )
{
    int i;
    string *adr;

    if ( !ip || !stringp(ip) || !mappingp(sbanished) || !sizeof(sbanished) )
        return 0;

    adr = m_indices(sbanished);

    for ( i = sizeof(adr); i--; )
        if ( sbanished[adr[i]] > 0 && sbanished[adr[i]] < time() )
            efun::m_delete( sbanished, adr[i] );

    UpdateSBanish();

    if ( !sizeof(sbanished) )
        return 0;

    if ( sizeof(regexp( ({ ip + "." }),
                        "^(" + implode( m_indices(sbanished), "|" ) + ")" )) )
        return "\nSorry, von Deiner Adresse kamen ein paar Idioten, die "
            "ausschliesslich\nAerger machen wollten. Deshalb haben wir "
            "die Moeglichkeit,\neinfach neue Charaktere "
            "anzulegen, kurzzeitig fuer diese Adresse gesperrt.\n\n"
            "Falls Du bei uns spielen moechtest, schicke bitte eine Email "
            "an\n\n                         mud@mg.mud.de\n\n"
            "mit der Bitte, einen Charakter fuer Dich anzulegen.\n";

    return 0;
}


static int _sbanished_by( string key, string name )
{
    return sbanished[key, 1] == name;
}


mixed SiteBanish( string ip, int days )
{
    string *s, tmp, euid;
    int t, level;

    euid = funcall(symbol_function('secure_euid/*'*/));
    level = funcall(symbol_function('secure_level/*'*/));

    // Unter L26 gibt's gar nix.
    if ( level <= DOMAINMEMBER_LVL )
        return -1;

    // Die Liste der gesperrten IPs anschauen darf jeder ab L26.
    if ( !ip && !days )
        return copy(sbanished);


    if ( !stringp(ip) || !intp(days) )
        return 0;

    if ( days && QuerySBanished(ip) ){
        write( "Diese Adresse ist schon gebannt.\n" );
        return 0;
    }

    if ( !days ){
        tmp = regreplace( ip, "\\.", "\\\\.", 1 ) + "\\.";

        if( member(sbanished, tmp) ){
            // Fremde Sitebanishs duerfen nur Deputies loeschen.
            if ( sbanished[tmp, 1] != euid && !IsDeputy(euid) )
                return -1;

            funcall(symbol_function('log_file/*'*/), "ARCH/SBANISH",
                    sprintf( "%s: %s hebt die Sperrung der Adresse %s von %s "
                             + "auf.\n", ctime(time()), capitalize(euid), ip,
                             capitalize(sbanished[tmp, 1]) ) );

            efun::m_delete( sbanished, tmp );
        }
        else
            return 0;
    }
    else {
        // Alles, was nicht Deputy ist, darf nur fuer einen Tag sbanishen.
        if ( days != 1 && !IsDeputy(euid) )
            return -1;

        // Nur Deputies duerfen mehr als 10 Sperrungen vornehmen.
        if ( sizeof(filter_indices(sbanished, #'_sbanished_by/*'*/, euid)) >= 10
             && !IsDeputy(euid) )
            return -2;

        s = efun::explode( ip, "." ) - ({""});

        if ( sizeof(s) < 2 || sizeof(s) > 4 ){
            write( "Ungueltiges Adress-Format!\n" );
            return 0;
        }

        // L26 duerfen exakt eine IP sperren, RMs ganze Class C-Subnetze
        // und Deputies auch Class B-Subnetze.
        if ( sizeof(s) < 4 && level < LORD_LVL
             || sizeof(s) < 3 && !IsDeputy(euid) )
            return -1;

        for ( t = sizeof(s); t--; )
            if ( (int) s[t] > 255 || (int) s[t] < 0 ){
                write( "Ungueltiges Adress-Format!\n" );
                return 0;
            }

        ip = implode( s, "\\." ) + "\\.";

        if ( !mappingp(sbanished) )
            sbanished = ([]);

        if ( days < 0 )
            t = -1;
        else
            t = (time() / 86400) * 86400 + days * 86400;

        sbanished += ([ ip : t; euid ]);
        funcall(symbol_function('log_file/*'*/), "ARCH/SBANISH",
                sprintf( "%s: %s sperrt die Adresse %s %s.\n",
                         ctime(time()), capitalize(euid),
                         regreplace( ip, "\\\\.", ".", 1 )[0..<2],
                         days > 0 ? (days > 1 ? "fuer " + days + " Tage"
                                     : "fuer einen Tag")
                         : "bis zum St. Nimmerleinstag" ) );
    }

    UpdateSBanish();
    return 1;
}

static void CheckDeputyRights()
{
    object ob;
    mixed *ginfo;

    // Lese- und Schreibberechtigungen fuer die Rubrik 'polizei' setzen
    call_other( "/secure/news", "???" );
    ob = find_object("secure/news");
    ginfo = ((mixed)ob->GetGroup("polizei"))[5..6];
    ob->RemoveAllowed( "polizei", 0, ginfo[0], ginfo[1] );
    ob->AddAllowed( "polizei", 0, deputies, deputies );
    LoadDeputyFileList();
}

int ReloadDeputyFile()
{
    deputies = efun::explode( read_file("/secure/DEPUTIES") || "", "\n" );
    deputies -= ({""});
    deputies = map( deputies, #'lower_case/*'*/ );
    call_out( "CheckDeputyRights", 2 );
    return(1);
}


static int create_home(string owner, int level)
{
  string def_castle;
  string dest, castle, wizard;
  object player;
  string *domains;
  int i;

  player = funcall(symbol_function('find_player),owner);
  if (!player)
    return -5;
  domains=get_domain_homes(owner);
  if (!sizeof(domains) && level >= DOMAINMEMBER_LVL)
  {
    tell_object(player,"Du gehoerst noch keiner Region an !\n");
    return -6;
  }
  tell_object(player,"Du bist Mitarbeiter der folgenden Regionen:\n");
  for (i=0;i<sizeof(domains);i++)
  {
    if (i) tell_object(player,", ");
    tell_object(player,domains[i]);
  }
  tell_object(player,".\n");
  update_wiz_level(owner, level);
  wizard = "/players/" + owner;
  castle = "/players/" + owner + "/workroom.c";
  if (file_size(wizard) == -1) {
    tell_object(player, "Verzeichnis " + wizard + " angelegt\n");
    mkdir(wizard);
  }
  dest = object_name(environment(player));
  def_castle = read_file("/std/def_workroom.c");
  if (file_size(castle) > 0) {
    tell_object(player, "Du HATTEST ja schon ein Arbeitszimmer !\n");
  } else {
    if (write_file(castle, def_castle))
    {
      tell_object(player, "Arbeitszimmer " + castle + " erzeugt.\n");
      // Arbeitszimmer als Home setzen
      player->SetProp(P_START_HOME,castle[0..<3]);
    }
    else
      tell_object(player, "Arbeitszimmer konnte nicht erzeugt werden !\n");
  }
  return 1;
}

// Sendet dem befoerderten Magier eine Hilfemail zu.
protected void SendWizardHelpMail(string name, int level) {
  
  object pl=funcall(symbol_function('find_player),name);
  if (!objectp(pl)) return;

  string file=sprintf("%sinfo_ml%d", WIZ_HELP_MAIL_DIR, level);
  // wenn kein Hilfetext fuer den Level da ist: raus
  if (file_size(file) <= 0)
    return;

  string subject = read_file(file,1,1);
  string text = funcall(symbol_function('replace_personal),
                         read_file(file,2), ({pl}));
  
  mixed mail = ({"Merlin", "<Master>", name, 0, 0, subject,
                 funcall(symbol_function('dtime),time()), 
                 MUDNAME+time(), text });
  MAILDEMON->DeliverMail(mail, 0);
}

int allowed_advance_wizlevel(mixed ob)
{
  string what;

  if (objectp(ob) && geteuid(ob)==ROOTID) return 1;

  if (!stringp(ob))
    what=efun::explode(object_name(ob),"#")[0];
  else
    what=ob;

  if (what=="/secure/merlin") return 1;

  return 0;
}

int advance_wizlevel(string name, int level)
{
  int answer;
  mixed *user;

  if (!allowed_advance_wizlevel(PO))
    return -1;

  if (level>80) return -2;

  if (!find_userinfo(name)) return -3;

  user=get_full_userinfo(name);

  if (user[USER_LEVEL+1]>level) return -4;

  if (user[USER_LEVEL+1]==level) return 1;

  if (level>=10 && level<20)
  {
    update_wiz_level(name, level);
    SendWizardHelpMail(name, level);
    return 1;
  }
  if (level>=20 && user[USER_LEVEL+1]<21)
  {
    answer = create_home(name, level);
    if ( answer > 0 ) {
      answer = update_wiz_level(name, level);
      SendWizardHelpMail(name, level);
    }
    return answer;
  }

  update_wiz_level(name, level);
  SendWizardHelpMail(name, level);

  return 1;
}

void restart_heart_beat(object heart_beat)
{
  if (heart_beat) heart_beat->_restart_beat();
}

int renew_player_object(mixed who)
{
  object newob;
  object *obs, *obs2;
  mixed err;
  string ob_name;
  object *armours, weapon;
  object tp;
  int i,active,j;

  if (stringp(who))
  {
    who=funcall(symbol_function('find_player),who);//'))
    if (!who)
    {
      who=funcall(symbol_function('find_netdead),who);//'))
      if (!who)
        return -1;
    }
  }
  if (!objectp(who))
    return -2;
  if (!query_once_interactive(who))
    return -3;
  active=interactive(who);
  printf("OK, renewing %O\n",who);
  seteuid(geteuid(who));
  err=catch(newob=clone_object(query_player_object(getuid(who))); publish);
  seteuid(getuid(TO));
  if (err)
  {
    printf("%O\n",err);
    return -4;
  }
  if (!newob)
    return -5;
  /* Ok, the object is here now ... lets go for it ... */
  who->save_me(0);
  /* SSL ip weiterreichen */
  if( query_ip_number(who) != efun::query_ip_number(who) ) {
    newob->set_realip( query_ip_number(who) );
  }
  disable_commands();
  armours=(object *)who->QueryProp(P_ARMOURS);
  weapon=(object)who->QueryProp(P_WEAPON);
  if ( previous_object() && object_name(previous_object()) == "/secure/merlin" )
      DEBUG_MSG(sprintf("RENEWING: %O %O\n",newob,who),previous_object());
  else
      DEBUG2_MSG(sprintf("RENEWING: %O %O\n",newob,who),previous_object());
  ob_name=explode(object_name(newob),"#")[0];
  if (strlen(ob_name)>11 && ob_name[0..11]=="/std/shells/")
    ob_name=ob_name[11..];
  ob_name=ob_name+":"+getuid((object)who);
  if (active)
    exec(newob,who);
  if (active && (interactive(who)||!interactive(newob)))
  {
    DEBUG_MSG("ERROR: still active !\n",previous_object());
    newob->remove();
    return 0;
  }
//   newob->start_player(capitalize(getuid(who)),who->_query_my_ip());
  funcall(
          bind_lambda(
                      unbound_lambda( ({'x, 'y}),
                                      ({#'call_other/*'*/,
                                             newob,
                                             "start_player",
                                             'x, 'y
                                             })
                                      ), who
                      ),
          capitalize(getuid(who)), who->_query_my_ip() );

  newob->move(environment(who),M_TPORT|M_NOCHECK|M_NO_SHOW|M_SILENT
              |M_NO_ATTACK);
  obs=all_inventory(who);
  foreach(object tob: all_inventory(who)) {
    if (!tob->QueryProp(P_AUTOLOADOBJ))
    {
      // kein Autoloader...
      foreach(object ob: deep_inventory(tob))
      {
        // aber enthaltene Autoloader entsorgen...
        if (ob->QueryProp(P_AUTOLOADOBJ))
        {
          catch(ob->remove();publish);
          if (ob) destruct(ob);
        }
      }
      // objekt ohne die AL bewegen.
      catch(tob->move(newob,M_NOCHECK);publish);
    }
    else {
      // Inhalt von Autoloadern retten.
      // neue instanz des ALs im neuen Objekt.
      object new_al_instance = present_clone(tob, newob);
      foreach(object ob: deep_inventory(tob)) {
        if (ob->QueryProp(P_AUTOLOADOBJ)) {
            // autoloader in Autoloadern zerstoeren...
            catch(ob->remove(1);publish);
            if (ob) destruct(ob);
        }
        // alle nicht autoloader in die AL-Instanz im neuen Objekt oder
        // notfalls ins Inv.
        else {
          if (objectp(new_al_instance))
            catch(ob->move(new_al_instance, M_NOCHECK);publish);
          else
            catch(ob->move(newob, M_NOCHECK);publish);
        }
      }
      // Autoloader zerstoeren. Wird nicht vom Spielerobjekt im remove()
      // gemacht, wenn nicht NODROP.
      catch(tob->remove(1);publish);
      if (tob) destruct(tob);
    }
  }
  who->remove();
  if ( objectp(who) )
      destruct(who);
  rename_object(newob,ob_name);
  newob->__reload_explore();
  tp=this_player();
  efun::set_this_player(newob);
  if (objectp(weapon))
    weapon->DoWield();
  if (pointerp(armours))
    for (i=sizeof(armours)-1;i>=0;i--)
      if (objectp(armours[i]))
        armours[i]->do_wear("alles");
  efun::set_this_player(tp);
  //Rueckgabewert noetig, weil Funktion vom Typ 'int'
  return(1);
}

mixed __query_variable(object ob, string var)
{
  if (!PO || !IS_ARCH(geteuid(PO)) || !this_interactive() ||
      !IS_ARCH(this_interactive()) || getuid(ob)==ROOTID )
  {
    write("ILLEGAL\n");
    return;
  }
  if (query_once_interactive(ob) && (PO!=ob) &&
     (var=="history" || var=="hist2"))
     CHMASTER->send("Snoop", previous_object(), sprintf("%s -> %s (history).",
                    capitalize(getuid(PO)),capitalize(getuid(ob))));

  funcall(symbol_function('log_file/*'*/), "ARCH/QV",
                          sprintf("%s: %O inquires var %s in %O\n",
                                  ctime(time()),this_interactive(),var,ob) );
  return
    funcall(bind_lambda(
                        unbound_lambda(
                                       ({}),
                                       ({#'funcall,({#'symbol_variable,var})})),
                        ob));
}

void RestartBeats()
{
  int i,size,counter;
  object ob;
  mixed *list;
  string file,obname,fun;

  "/secure/simul_efun"->StopCallOut(0);
  write("CALL_OUT-Restart in progress !\n");
  filter(users(),#'tell_object,"CALL_OUT-Restart in progress !\n");
  size=file_size("log/call_out_stop");
  if (size<=0)
    return;
  file="";
  counter=0;
  while (counter<size)
  {
    file+=read_bytes("log/call_out_stop",counter,
                     (size-(counter+=40000)>0?counter:size));
  }
  list=explode(file,"__(CUT HERE)__\n");
  list=list[<1..];
  list=explode(list[0],"\n")-({""});
  for (i=sizeof(list)-1;i>=0;i--)
    if (sscanf(list[i],"%s \"%s\"",obname,fun)==2 && ob=find_object(obname))
    {
      write(sprintf("%O -> %s\n",ob,fun));
      catch(ob->__restart(fun);publish);
    }
  write("CALL_OUT-Restart completed !\n");
  filter(users(),#'tell_object,"CALL_OUT-Restart completed !\n");
  rename("log/call_out_stop","log/call_out_stop.old");
}

