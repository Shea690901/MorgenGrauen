// MorgenGrauen MUDlib
//
// player/quests.c -- quest handler
//
// $Id: quests.c 6813 2008-03-31 21:13:00Z Zesstra $

// Dieses Modul enhaelt die Quest-spezifischen Teile der Playerobjekte.
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

#define NEED_PROTOTYPES
#include <player/life.h>
#include <player/quest.h>
#include <thing/properties.h>
#include <player/base.h>
#include <living/life.h>
#undef NEED_PROTOTYPES

#include "/secure/questmaster.h"
#include <wizlevels.h>
#include <daemon.h>
#include <language.h>
#include <mail.h>
#include <defines.h>
#include <new_skills.h>
#include <properties.h>
#include <events.h>

mixed quests;
private nosave int qp;

int QueryQuest(string questname);
// local properties prototype
static mixed _query_quests();
static int   _set_questpoints(int qp);
static int   _query_questpoints();

void create()
{
  Set(P_QUESTS, NOSETMETHOD, F_SET_METHOD);
  Set(P_QUESTS, quests = ({}));
  Set(P_QUESTS, SECURED, F_MODE);
  Set(P_QP, SAVE, F_MODE);
  Set(P_QP, SECURED, F_MODE);
}

varargs int GiveQuest(string questname, string message)
{
  mixed *quest;
  string guild;

  quest=QM->QueryQuest(questname);
  // Questname ungueltig
  if (!quest||!pointerp(quest)||quest==({})) return GQ_KEY_INVALID;
  // Unbefugter Zugriff auf deaktivierte Quest
  if (!quest[6]&&!IS_ARCH(this_interactive())) return GQ_IS_INACTIVE;
  // Unbefugter Zugriff
  if (member(quest[2], load_name(previous_object()))==-1 &&
      !IS_ARCH(this_interactive()))
    return GQ_ILLEGAL_OBJ;
  
  // Gilde wird in jedem Fall informiert.
  guild=GUILD_DIR+QueryProp(P_GUILD);
  if (find_object(guild) || file_size(guild+".c")>-1)
    catch( call_other(guild, "NotifyGiveQuest", ME, questname);publish );

  // Quest bereits gesetzt
  if (QueryQuest(questname)) return GQ_ALREADY_SET;
  Set(P_QP, QueryProp(P_QP)+quest[0]);
  AddExp(quest[1]);
  if (quests)
    quests+=({({questname,quest[0]})});
  else
    quests=({({questname,quest[0]})});
  force_save();
  // Event ausloesen
  EVENTD->TriggerEvent(EVT_LIB_QUEST_SOLVED,([
             E_OBJECT: this_object(),
             E_PLNAME: getuid(this_object()),
             E_ENVIRONMENT: environment(),
             E_QUESTNAME: questname,
             E_QP_GRANTED: quest[0] ]) );

  if (message && message!="")
    if (message!="__silent__")
      message=implode(explode(message,"@@name@@"),
		      capitalize(query_real_name()));
    else
      message="";
  else
    message=capitalize(query_real_name())
      +" hat gerade ein Abenteuer bestanden: "+ questname+"\n";
  if(message!="")
    catch("/secure/questmaster"->Channel(message);publish);
  catch("/secure/questmaster"->SendMail(questname, quest,
	                                this_object());publish);
  return OK;
}

int DeleteQuest(string questname)
{
  mixed *quest;
  int i;
  
  // Quest ist nicht gesetzt
  if(!QueryQuest(questname)) return DQ_NOT_SET;
  quest=QM->QueryQuest(questname);
  // Questname ungueltig
  if (!quest||!pointerp(quest)||quest==({})) return DQ_KEY_INVALID;
  // Unbefugter Zugriff
  if (!IS_ARCH(this_interactive())) return DQ_ILLEGAL_OBJ;
  Set(P_QP, QueryProp(P_QP)-quest[0]);
  AddExp(-quest[1]);
  for(i=sizeof(quests)-1;i>=0;i--)
    if(quests[i][0]==questname)
      break;
  quests=quests[0..i-1]+quests[i+1..];
  force_save();
  return OK;
}

int QueryQuest(string questname)
{
  int i,dummy;

  if( sscanf( getuid(), "gast%d", dummy ) == 1 )
    return QQ_GUEST;

  if(!questname || !stringp(questname) || questname == "")
    return QQ_KEY_INVALID;

  foreach(mixed quest: quests) {
    if (questname==quest[0])
      return OK;
  }
  return QQ_KEY_INVALID;
}


static mixed *_check_questpoints( mixed *eintrag, closure QueryQ )
{
    int qpoints;
    
    if ( stringp(eintrag) )
        eintrag = ({ eintrag });

    if ( (qpoints = funcall( QueryQ, eintrag[0] )) >= 0 ){
        qp += qpoints;
        return ({ eintrag[0], qpoints });
    }
    else
        return ({});
}


// **** local property methods

static int _query_questpoints()
{
    int i, old_qp, qpoints;
	
    old_qp = Query(P_QP);
    
    if ( !pointerp(quests) || !sizeof(quests) ){
        if ( this_interactive() == this_object() && old_qp ){
            SetProp( P_QP, 0 );
            save_me(1);
        }
        
        return 0;
    }
    
	quests -= ({ ({}) });
    qp = 0;

    quests = map( quests, "_check_questpoints", ME,
                        symbol_function( "QueryQuestPoints", QM ) );

    quests -= ({ ({}) });

    if( this_interactive() == ME && old_qp != qp ){
        Set( P_QP, qp );
        save_me(1);
    }

    return qp;
}

static int _set_questpoints(int qp)
{
  if (!IS_ARCH(this_interactive())) 
    return -1;
  return Set(P_QP, qp);
}

static mixed _query_quests() {
  return deep_copy(quests);
}

