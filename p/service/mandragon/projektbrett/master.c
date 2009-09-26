// Projectmaster
// Verwaltung 
// 01.02.04 Bugfix Feigling (siehe Feigling Aenderung Start)


#include "projectmaster.h"
#include "/secure/wizlevels.h"

#define NEWMAP(x) ([ x: 0;"";({});" ";"";"";0;" ";0;0;0])

mapping data;
int projectcount;
int changetime;
static mapping modifying;
static string helptext;

static void EnterTeam(int projnum,mapping newdata);
static void ChangeDate(int index,mapping newdata);
static void ChangeSupervisor(int index,mapping newdata);
static void ChangeProjNum(int index,mapping newdata);
static void ShowMenu(int index,mapping newdata);
static void EnterLong(int projnum, mapping newdata);
static void EnterSize(int projnum,mapping newdata);
static void EnterWizLevel(int projnum,mapping newdata);
static void EnterType(int projnum, mapping newdata);
static void EnterType3(int projnum, mapping newdata, string type);
static void EnterType5(int projnum, mapping newdata, string type);
static void EnterJob(int projnum, mapping newdata);
static void EnterJob3(int projnum, mapping newdata, string job);
static void EnterJob5(int projnum, mapping newdata, string job);
static void EnterStatus(int projnum,mapping newdata);

int filter_fun(int key)
{
  if (!data[key,PM_Change]) data[key,PM_Change]=data[key,PM_Date];
  return 1;
}

void create()
{
  if (IS_CLONE(TO)) return; // Keine Clones
  seteuid(getuid());
  data=m_allocate(0,PM_MAX+1);
  restore_object(PROJSAVEFILE);
  modifying=([]);
  filter_indices(data,#'filter_fun);
  if (!projectcount) projectcount=0;
  if (!changetime) changetime=time();
  if (!(helptext=read_file(PROJHELPFILE)))
    helptext = "Die Hilfe ist derzeit nicht verfuegbar.\n"
               "Bitte umgehend Mandragon verstaendigen.\n%s\n";
}

static string ConvertDate(int timeint)
{
  string timestring;
  timestring=dtime(timeint);
  return sprintf("%s %s",timestring[5..11],timestring[15..16]);
}

static string ListEntry(int index,int showlist)
{
  return sprintf(" %2.2d. %-48.48s %-1.1s %-10.10s %-10.10s\n",
                 index,data[index,PM_Short],
                 (showlist?""+data[index,PM_Status]:" "),
                 capitalize(data[index,PM_Supervisor]),
                 ConvertDate(data[index,PM_Change]));
}

static int AllWizlevelFilter(int index)
{
  if (getuid(this_interactive())==data[index,PM_Supervisor]) return 1;
  if (query_wiz_level(this_interactive())<data[index,PM_Wizlevel]) return 0;
  return 1;
}

static int CurrentWizlevelFilter(int index)
{
  if (data[index,PM_Status]) return 0;
  if (getuid(this_interactive())==data[index,PM_Supervisor]) return 1;
  if (query_wiz_level(this_interactive())<data[index,PM_Wizlevel]) return 0;
  return 1;
}

varargs void ShowList(mixed selection,int showlist)
{
  string text;
  mixed indices;
  int i;
  if (!mappingp(selection))
  {
    if (!selection)
      selection=filter_indices(data,#'CurrentWizlevelFilter/*'*/);
    else
      selection=filter_indices(data,#'AllWizlevelFilter/*'*/);	
  } 
  if (!sizeof(selection))
    text="Leider passen keine Projekte auf Deine Beschreibung.\n";
  else
  {
    indices=m_indices(selection);
    indices=sort_array(indices,#'>);	
    text="Die folgenden Projekte passen auf Deine Beschreibung:\n\n"
      "============================================================"
      "=================\n"
      " Nr. Kurzbeschreibung des Projekts                    "+
      (showlist?"S":" ") + " Magier     Datum\n--------------------"
      "---------------------------------------------------------\n";
    for (i=0;i<sizeof(indices);i++) text+=ListEntry(indices[i],showlist);
    text+="========================================================"
      "=====================\n";
    if (showlist)
      text+="Status:  0: ausgeschrieben   2: abgeschlossen   4: in Planung\n"
        "         1: vergeben         3: abgebrochen     5: unterbrochen\n";
    text+="\n  WOW! Da bekommst Du richtig Lust, auch etwas zu machen!\n";
  }
  TP->More(text);
}

static string BuildLong(int index)
{
  string wizards, work, size, special, curwizname, *wiz,curwizards,who;
  int i,wizlevel;
  work=""; special=""; wizards="";
  switch(data[index,PM_Size])
  {	
  case PMS_Medium: size="mittelgross"; break;
  case PMS_Large:  size="gross"; break;
  default: size="klein"; break;
  }
  if (data[index,PM_Type]!=" ")
  {
    if (test_bit(data[index,PM_Type],PMT_Guild)) work+="Gilde, ";
    if (test_bit(data[index,PM_Type],PMT_Mudlib)) work+="Mudlib, ";
    if (test_bit(data[index,PM_Type],PMT_Region)) work+="Region";
    else work=work[0..<3];
    work+=".";
  }
  if (data[index,PM_Job]!=" ")
  {
    if (test_bit(data[index,PM_Job],PMJ_Rework))
      special+="Ueberarbeiten, ";
    if (test_bit(data[index,PM_Job],PMJ_Details))
      special+="Details einfuegen, ";
    if (test_bit(data[index,PM_Job],PMJ_Debug))
      special+="Debuggen";
    else special=special[0..<3];
    special+=".";
  }
  wiz=data[index,PM_Wizards];
  if (sizeof(wiz)>0)
  {
    i=sizeof(wiz);
    curwizards="";
    while(i--)
    {
      curwizname=capitalize(wiz[i]);
      if (strlen(curwizname)+strlen(curwizards)>58)
      {
        wizards+=sprintf("|               %-60.60s|\n",curwizards);
        curwizards="";
      }
      curwizards=curwizards+curwizname+", ";
    }
    wizards+=sprintf("|               %-60.60s|\n",curwizards[0..<3]+".");
    wizards[2..13]="Mitarbeiter:";
  }
  if (!(wizlevel=data[index,PM_Wizlevel])) who="alle Spieler und Magier.";
  else if (wizlevel==1) who="Seher und Magier.";
  else who="Magier ab Level " + wizlevel + ".";
  return 
    sprintf("+%'-'75.75s+\n"
            "| Projekt Nr. %-55.55s       |\n"
            "|      Eintrag von %-55.55s  |\n"
            "|      Status: %-60.60s |\n",
            "-",""+index+": "+data[index,PM_Short],
            capitalize(data[index,PM_Supervisor])+ " am " +
            ConvertDate(data[index,PM_Date])+ " fuer " +who,
            upperstring(PMU_States[data[index,PM_Status]])+".") +
    (ConvertDate(data[index,PM_Change])==ConvertDate(data[index,PM_Date])?
     "":sprintf("|      Letzte Aenderung: %-50.50s |\n",
                ConvertDate(data[index,PM_Change])))+
    (work==""?"":sprintf("| Arbeit an: %-55.55s        |\n",work))+
    (special==""?"":sprintf("| Besonderheiten: %-55.55s   |\n",special))+
    sprintf("| Der Umfang des Projektes ist %-45.45s|\n%s+%'-'75.75s+\n",
             size+".",wizards,"-");
}

void ShowLong(int index)
{
  string text;
  if (!member(data,index)||
      (getuid(this_interactive())!=data[index,PM_Supervisor]&&
       data[index,PM_Wizlevel]>query_wiz_level(this_interactive())))
  {
    TP->More("Ein Projekt mit dieser Nummer existiert nicht.\n");
    return;
  }
  text="Du beschliesst, Dir das Projekt Nummer " + index + " genauer anzusehen.\n" +
    "Ueber dieses Projekt befinden sich die folgenden Informationen am Brett:\n";
  text=text + BuildLong(index) + break_string(data[index,PM_Long],77,0,BS_LEAVE_MY_LFS)+
    "+---------------------------------------------------------------------------+\n\n";
  TP->More(text);
  return;
}

void BoardHelp()
{
  TP->More(BS(sprintf(helptext,IS_LEARNER(this_interactive())?
   "\n|      komplett             Auch inaktive Projekte anzeigen   "
   "              |\n|      level <nummer>       Nur Projekte bis "
   "Level <nummer> zeigen          |":"")));              
  return;
}

static int FilterFunc(int index, string filtertype, int invert, mixed extra)
{
  int ret;
  switch (filtertype)
  {
  case "mudlib":  ret=test_bit(data[index,PM_Type],PMT_Mudlib); break;
  case "region":  ret=test_bit(data[index,PM_Type],PMT_Region); break;
  case "gilde":   ret=test_bit(data[index,PM_Type],PMT_Guild); break;
  case "details": ret=test_bit(data[index,PM_Job],PMJ_Details); break;
  case "debug":   ret=test_bit(data[index,PM_Job],PMJ_Debug); break;
  case "neu":     ret=(data[index,PM_Job]==" "?1:0); break;
  case "ueberarbeiten": ret=test_bit(data[index,PM_Job],PMJ_Rework); break;
  case "gross":   ret=(data[index,PM_Size]==PMS_Large); break;
  case "mittel":  ret=(data[index,PM_Size]==PMS_Medium); break;
  case "klein":   ret=(data[index,PM_Size]==PMS_Small); break;
  case "neueintrag":  ret=(data[index,PM_Date]>(time()-1209600)); break;
  case "magier":  ret=(data[index,PM_Supervisor]==extra); break;
  case "level":   ret=(data[index,PM_Wizlevel]<=extra); break;
  default: ret=1; break;
  }
  return (invert==1?!ret:ret);
} 

void ParseArgs(string arg)
{
  mixed args;
  string filter_ldfied;
  int index,argfail,invert,comp,showstatus;
  mapping filterdata;
  args=old_explode(arg," ");
  index=0; invert=0; argfail=0;
  if (!sizeof(args)) argfail=1;
  if ((comp=member(args,"komplett"))==-1)
    filterdata=filter_indices(data,#'CurrentWizlevelFilter/*'*/);
  else
  {
    showstatus=1;
    if (comp>0&&args[comp-1]=="nicht")
    {
      tell_object(TP,
                  BS("Der Filter 'komplett' kann nicht invertiert werden."));
      return;
    }
    args-= ({ "komplett" });
    if (IS_LEARNER(this_interactive()))
      filterdata=filter_indices(data,#'AllWizlevelFilter/*'*/);
    else
      filterdata=filter_indices(data,#'CurrentWizlevelFilter/*'*/);
  }	
  while (index<sizeof(args))
  {
    switch (args[index])
    {
      case "nicht": invert=(invert==0?1:0); break;
    case "magier": //Feigling Aenderung Start
      if ((index+1)<sizeof(args)) 
	filterdata=filter_indices(filterdata,"FilterFunc",TO,
				  args[index],invert,args[index+1]); 
      else tell_object(TP, 
		       BS("Der Filter 'magier' benoetigt weitere Parameter."));
	     //Feigling Aenderung Ende
                     invert=0; index++; break;
      case "level" : filterdata=filter_indices(filterdata,"FilterFunc",TO,
                                           args[index],invert,to_int(args[index+1]));
                     invert=0; index++; break;
      case "mudlib": case "region": case "gilde": case "details":
      case "debug" : case "neu": case "ueberarbeiten": case "neueintrag" :
      case "gross" : case "klein":
      case "mittel": filterdata=filter_indices(filterdata,"FilterFunc",TO,
                     args[index],invert,""); invert=0; break;
      case "komplett" : invert=0; break; 
      default:    argfail=1;break;
    }
    if (argfail) break;
    index++;
  }
  if (argfail) tell_object(TP,
              BS("Der Filter '"+args[index]+"' ist nicht implementiert."));
  else ShowList(filterdata,showstatus);	
  return;
}

static void ChangeDate2(string arg, int index, mapping newdata)
{
  if (arg&&stringp(arg))
  {
    if (arg=="heute")
    {
      newdata[index,PM_Date]=time();
      ShowMenu(index, newdata);
      return;
    }
    if (arg=="q"||arg=="Q")
    {
      ShowMenu(index,newdata);
      return;
    }
    if (to_int(arg)&&arg==to_string(to_int(arg)))
    {
      if (to_int(arg)>time())
      {
        tell_object(TP,
             "Also in der Vergangenheit sollte das Datum schon liegen.\n");
        return 0;
      }
      newdata[index,PM_Date]=to_int(arg);
      ShowMenu(index,newdata);
      return;
    }
  }
  tell_object(TP,"Eingabe ungueltig!\n");
  ChangeDate(index,newdata);
  return;
}

static void ChangeDate(int index,mapping newdata)
{
  tell_object(TP,BS("Moechtest Du das Datum auf den heutigen Tag setzen (" +
                ConvertDate(time())+"), dann gib 'heute' ein, "
                "ansonsten gib das Datum in Sekunden seit dem 1.1.1970 an."));
  input_to("ChangeDate2",0,index,newdata);
  return;
}

static void ChangeSupervisor2(string arg, int index, mapping newdata)
{
  if (arg&&stringp(arg))
  {
    if (IS_LEARNER(lowerstring(arg)))
    {
      newdata[index,PM_Supervisor]=lowerstring(arg);
      ShowMenu(index,newdata);
      return;
    }
    if (arg=="q"||arg=="Q")
    {
      ShowMenu(index,newdata);
      return;
    }	
  }
  tell_object(TP,"Du musst den Namen eines Magiers angeben.\n");
  ChangeSupervisor(index,newdata);
  return;	
}

static void ChangeSupervisor(int index,mapping newdata)
{
  tell_object(TP,"Wer soll ab sofort verantwortlich sein fuer das Projekt?\n");
  input_to("ChangeSupervisor2",0,index,newdata);
  return;
}

static void ChangeProjNum2(string text,int index,mapping newdata)
{
  int fail, i,new_index;
  mapping replacemap;
  if (!text||!stringp(text)) fail=1;
  if (!fail&&(text=="q"||text=="Q"))
  {
    ShowMenu(index,newdata);
    return;
  }
  if (fail||(!fail&&(!(new_index=to_int(text))||
                     (text!=to_string(new_index))||(new_index<1))))
  {
    tell_object(TP,"Die neue Nummer muss eine ganze Zahl groesser 0 sein.\n");
    ChangeProjNum(index,newdata);
    return;
  }
  if (member(data,new_index))
  {
    tell_object(TP,
        "Es existiert schon ein Projekt mit der Nummer " + new_index + ".\n");
    ShowMenu(index,newdata);
    return;
  }
  replacemap=NEWMAP(new_index);
  for (i=PM_MAX;i>=0;i--) replacemap[new_index,i]=data[index,i];
  data-=([ index ]); data+=replacemap;
  replacemap=NEWMAP(new_index);
  for (i=PM_MAX;i>=0;i--) replacemap[new_index,i]=newdata[index,i];
  modifying+=([ new_index : modifying[index] ]);
  modifying-=([ index ]);
  ShowMenu(new_index,replacemap);		
  return;
}

static void ChangeProjNum(int index,mapping newdata)
{
  tell_object(TP,"Wie soll die neue Projektnummer lauten?\n");
  input_to("ChangeProjNum2",0,index,newdata);
  return;
}

static int AddRecord(mapping newdata)
{
  mixed nums;
  nums=m_indices(data);
  if ((projectcount++)<0) projectcount=1;
  for (;member(nums,projectcount)!=-1;) projectcount++;
  data+=([projectcount:newdata[0,PM_Date];newdata[0,PM_Supervisor];newdata[0,PM_Wizards];
          newdata[0,PM_Type];newdata[0,PM_Short];newdata[0,PM_Long];newdata[0,PM_Size];
          newdata[0,PM_Job];newdata[0,PM_Wizlevel];newdata[0,PM_Status]]);
  return projectcount;
}

static void EnterTeam2(string team,int projnum,mapping newdata)
{
  if (team&&stringp(team))
  {
    team=lowerstring(team);
    if (team=="q"||team=="Q")
    {
      ShowMenu(projnum,newdata);
      return;
    }
    else if (!("/secure/master"->get_userinfo(team)))
    {
      tell_object(TP,"Bitte den Namen des betreffenden Spielers angeben.\n");
      EnterTeam(projnum,newdata);
      return;
    }
    if (member(newdata[projnum,PM_Wizards],team)!=-1)
      newdata[projnum,PM_Wizards]-=({ team });
    else
      newdata[projnum,PM_Wizards]+=({ team });
    ShowMenu(projnum,newdata);
  }
  else
  {
    tell_object(TP,"Bitte den Namen des betreffenden Spielers angeben.\n");
    EnterTeam(projnum,newdata);
  }
  return;
}

static void EnterTeam(int projnum,mapping newdata)
{
  tell_object(TP,"Welcher Magier oder Spieler soll hinzugefuegt/geloescht werden?\n");
  input_to("EnterTeam2",0,projnum, newdata);
  return;
}

static void EnterQuit(string answer,int projnum, mapping newdata)
{
  if (!answer||!stringp(answer)) answer="";
  if (answer=="j"||answer=="J") return;
  if (answer=="n"||answer=="N")
  {
    EnterLong(projnum,newdata);
    return;
  }	
  tell_object(TP,"Projekterstellung abbrechen? (j/n)\n");
  input_to("EnterQuit",0,projnum,newdata);
  return;
}


static void EnterLong2(string newtext, int projnum, mapping newdata, string oldtext)
{
  if (!newtext||!stringp(newtext)) newtext="";
  if (newtext=="~q")
  {
    if (projnum) 
    {
      ShowMenu(projnum,newdata);
      return;
    }
    tell_object(TP,"Projekterstellung abbrechen? (j/n)\n");
    input_to("EnterQuit",0,projnum,newdata);
    return;
  }
  if (newtext=="**"||newtext==".")
  {
    newdata[projnum,PM_Long]=oldtext;
    if (projnum) ShowMenu(projnum,newdata);
    else 
    {
      newdata[projnum,PM_Supervisor]=getuid(TP);
      newdata[projnum,PM_Date]=time();
      newdata[projnum,PM_Change]=time();
      newdata[projnum,PM_Status]=PMU_Planned;
      projnum=AddRecord(newdata);
      changetime=time();
      save_object(PROJSAVEFILE);
      tell_object(TP,
                  BS("Das Projekt wurde erstellt und eingetragen. "
                     "Weitere Optionen sind bei der Projektmodifikation "
                     "verfuegbar.\nDas Projekt hat die Nummer >> " + projnum +
                     " << "));	
    }
    return;	
  }
  newtext=oldtext+newtext+"\n";
  tell_object(TP,"]");
  input_to("EnterLong2",0,projnum,newdata,newtext);
  return;
}

static void EnterLong(int projnum, mapping newdata)
{
  tell_object(TP,"Bitte beschreibe das Projekt nun genau und detailliert!\n"
              "(Ende mit . oder **, Abbruch mit ~q)\n]");
  input_to("EnterLong2",0,projnum, newdata,"");
  return;
}

static void EnterJob6(string answer,int projnum, mapping newdata, string job)
{
  int fail;
  fail=0;
  if (!answer||!stringp(answer))
    fail=1;
  else
  {
    if (answer=="q") 
    {
      if (projnum) ShowMenu(projnum,newdata);
      return;
    }
    if (answer!="j"&&answer!="J"&&answer!="n"&&answer!="N") fail=1;
  }
  if (fail) 
  {
    tell_object(TP,"Als Antwort sind nur 'j','J','n', 'N' und 'q' zulaessig.\n");
    EnterJob5(projnum,newdata,job);
    return;
  }
  if (!fail&&(answer=="j"||answer=="J")) job=set_bit(job,PMJ_Rework);
  newdata[projnum,PM_Job]=job;
  if (!projnum) EnterWizLevel(projnum,newdata);
  else ShowMenu(projnum,newdata);
  return;	
  
}

static void EnterJob5(int projnum, mapping newdata, string job)
{
  tell_object(TP,"Soll etwas ueberarbeitet werden? (j/n/q)\n");
  input_to("EnterJob6",0,projnum,newdata,job);
  return;
}
static void EnterJob4(string answer,int projnum, mapping newdata, string job)
{
  int fail;
  fail=0;
  if (!answer||!stringp(answer)) 
    fail=1;
  else
  {
    if (answer=="q") 
    {
      if (projnum) ShowMenu(projnum,newdata);
      return;
    }
    if (answer!="j"&&answer!="J"&&answer!="n"&&answer!="N") fail=1;
  }
  if (fail) 
  {
    tell_object(TP,"Als Antwort sind nur 'j','J','n','N' und 'q' zulaessig.\n");
    EnterJob3(projnum,newdata,job);
    return;
  }
  if (!fail&&(answer=="j"||answer=="J"))	job=set_bit(job,PMJ_Details);
  EnterJob5(projnum,newdata,job);
  return;	
  
}

static void EnterJob3(int projnum, mapping newdata, string job)
{
  tell_object(TP,"Sollen Details hinzugefuegt werden? (j/n/q)\n");
  input_to("EnterJob4",0,projnum,newdata,job);
  return;
}

static void EnterJob2(string answer,int projnum, mapping newdata)
{
  int fail;
  string job;
  fail=0;
  job=" ";
  if (!answer||!stringp(answer)) 
    fail=1;
  else
  {
    if (answer=="q") 
    {
      if (projnum) ShowMenu(projnum,newdata);
      return;
    }
    if (answer!="j"&&answer!="J"&&answer!="n"&&answer!="N") fail=1;
  } 
  if (fail) 
  {
    tell_object(TP,"Als Antwort sind nur 'j','J','n','N' und 'q' zulaessig.\n");
    EnterJob(projnum,newdata);
    return;
  }
  if (!fail&&(answer=="j"||answer=="J"))	job=set_bit(job,PMJ_Debug);
  EnterJob3(projnum, newdata, job);
  return;
}

static void EnterJob(int projnum, mapping newdata)
{
  tell_object(TP,"Handelt es sich um Debugging? (j/n/q)\n");
  input_to("EnterJob2",0,projnum,newdata);
  return;
} 

static void EnterSize2(string sizestr, int projnum, mapping newdata)
{
  int size;
  if (strlen(sizestr)&&(sizestr=="0"||sizestr=="1"||sizestr=="2"))
    size=to_int(sizestr);
  else
  {
    if(strlen(sizestr)&&sizestr=="q")
    {
      if (projnum) ShowMenu(projnum,newdata);
      return;
    }
    tell_object(TP,"Als Groessenangabe sind nur die Zahlen 0-2 zulaessig.\n");
    EnterSize(projnum, newdata);
    return;
  }
  newdata[projnum,PM_Size]=size;
  if (projnum) ShowMenu(projnum,newdata);
  else EnterJob(projnum,newdata);
  return;
}

static void EnterSize(int projnum,mapping newdata)
{
  tell_object(TP,"Welchen Umfang hat das Projekt?\n"+
              "Klein  [0]:  Wenige Objekte\n"+
              "Mittel [1]:  Ein kleines Gebiet\n"+
              "Gross  [2]:  Ein grosses Gebiet\n"+
              "Bitte gib die passende Zahl an:\n");
  input_to("EnterSize2",0,projnum,newdata);
  return;
}

static void EnterType6(string answer,int projnum, mapping newdata, string type)
{
  int fail;
  fail=0;
  if (!answer||!stringp(answer))
    fail=1;
  else
  {
    if (answer=="q") 
    {
      if (projnum) ShowMenu(projnum,newdata);
      return;
    }
    if (answer!="j"&&answer!="J"&&answer!="n"&&answer!="N") fail=1;
  }
  if (fail) 
  {
    tell_object(TP,"Als Antwort sind nur 'j','J','n', 'N' und 'q' zulaessig.\n");
    EnterType5(projnum,newdata,type);
    return;
  }
  if (!fail&&(answer=="j"||answer=="J"))	type=set_bit(type,PMT_Guild);
  newdata[projnum,PM_Type]=type;
  if (!projnum) EnterSize(projnum,newdata);
  else ShowMenu(projnum,newdata);
  return;	
}

static void EnterType5(int projnum, mapping newdata, string type)
{
  tell_object(TP,"Hat das Projekt mit einer Gilde zu tun? (j/n/q)\n");
  input_to("EnterType6",0,projnum,newdata,type);
  return;
}
static void EnterType4(string answer,int projnum, mapping newdata, string type)
{
  int fail;
  fail=0;
  if (!answer||!stringp(answer)) 
    fail=1;
  else
  {
    if (answer=="q") 
    {
      if (projnum) ShowMenu(projnum,newdata);
      return;
    }
    if (answer!="j"&&answer!="J"&&answer!="n"&&answer!="N") fail=1;
  }
  if (fail) 
  {
    tell_object(TP,"Als Antwort sind nur 'j','J','n', 'N' und 'q' zulaessig.\n");
    EnterType3(projnum,newdata,type);
    return;
  }
  if (!fail&&(answer=="j"||answer=="J"))	type=set_bit(type,PMT_Region);
  EnterType5(projnum,newdata,type);
  return;	
  
}

static void EnterType3(int projnum, mapping newdata, string type)
{
  tell_object(TP,"Wird etwas in einer Region gearbeitet? (j/n/q)\n");
  input_to("EnterType4",0,projnum,newdata,type);
  return;
}

static void EnterType2(string answer,int projnum, mapping newdata)
{
  int fail;
  string type;
  fail=0;
  type=" ";
  if (!answer||!stringp(answer)) 
    fail=1;
  else
  {
    if (answer=="q") 
    {
      if (projnum) ShowMenu(projnum,newdata);
      return;
    }
    if (answer!="j"&&answer!="J"&&answer!="n"&&answer!="N") fail=1;
  } 
  if (fail) 
  {
    tell_object(TP,"Als Antwort sind nur 'j','J','n','N' und 'q' zulaessig.\n");
    EnterType(projnum,newdata);
    return;
  }
  if (!fail&&(answer=="j"||answer=="J")) type=set_bit(type,PMT_Mudlib);
  EnterType3(projnum, newdata, type);
  return;
}

static void EnterType(int projnum, mapping newdata)
{
  tell_object(TP,"Handelt es sich um Mudlib-Programmierung? (j/n/q)\n");
  input_to("EnterType2",0,projnum,newdata);
  return;
} 

static void EnterShort2(string text,int projnum,mapping newdata)
{
  if (strlen(text)<1||strlen(text)>48||
      ((strlen(text)==48)&&(text[<1..<1]!=".")))
  {
    tell_object(TP,"Die Kurzbeschreibung darf maximal 48 Zeichen "
                "lang sein.\nBitte nocheinmal eingeben:\n");
    input_to("EnterShort2",0,projnum,newdata);
    return;	
  }
  if (text=="q")
  { 
    if (projnum) ShowMenu(projnum,newdata);
    return;
  }
  if (text[<1..<1]!=".") text=text+".";
  newdata[projnum,PM_Short]=text;
  if (!projnum) EnterType(projnum,newdata);
  else ShowMenu(projnum,newdata);
  return;
}

static void EnterShort(int projnum,mapping newdata)
{
  tell_object(TP,"Wie soll die Kurzbeschreibung des Projektes lauten? (Abbruch mit 'q')\n");
  input_to("EnterShort2",0,projnum,newdata);
  return;
}

static void EnterWizLevel2(string text,int projnum, mapping newdata)
{
  int lv;
  if (stringp(text)&&(text=="q"))
  { 
    if (projnum) ShowMenu(projnum,newdata);
    return;
  }
  if (!(text&&stringp(text)&&(to_string(lv=to_int(text))==text)&&lv>=0&&lv<112))
  {
    tell_object(TP,"Bitte gib eine ganze Zahl von 0 bis 111 (oder 'q') an.\n");
    EnterWizLevel(projnum, newdata);
    return;
  }
  newdata[projnum,PM_Wizlevel]=lv;
  if (projnum) ShowMenu(projnum,newdata);
  else EnterLong(projnum,newdata);
  return;
  
}

static void EnterWizLevel(int projnum,mapping newdata)
{

  tell_object(TP,
              BS("Welchen Magierlevel muss ein Spieler haben, um dieses Projekt sehen zu koennen?"));
  input_to("EnterWizLevel2",0,projnum,newdata);
  return;
}


static void EnterStatus2(string statusstr, int projnum, mapping newdata)
{
  int state;
  if (stringp(statusstr)&&(statusstr=="q"))
  { 
    if (projnum) ShowMenu(projnum,newdata);
    return;
  }
  
  if (statusstr&&stringp(statusstr)&&
      (statusstr=="0"||statusstr=="1"||statusstr=="2"||
       statusstr=="3"||statusstr=="4"||statusstr=="5")) state=to_int(statusstr);
  else
  {        
    tell_object(TP,"Als Statusangabe sind nur die Zahlen 0-5 zulaessig.\n");
    EnterStatus(projnum, newdata);
    return;
  }
  newdata[projnum,PM_Status]=state;
  ShowMenu(projnum,newdata);
  return;
}

static void EnterStatus(int projnum,mapping newdata)
{
  tell_object(TP,"Welchen Status hat das Projekt?\n"+
              "[0] Ausgeschrieben\n"+
              "[1] Vergeben\n"+
              "[2] Abgeschlossen\n"+
              "[3] Abgebrochen\n"+
              "[4] Planung\n"+
              "[5] Unterbrochen\n"+
              "Bitte gib die passende Zahl an:\n");
  input_to("EnterStatus2",0,projnum,newdata);
  return;
}

void ChangeData(int projnum)
{
  mapping newdata;
  int i;
  if (projnum<0)
  {
    tell_object(TP,"Projekte koennen nur positive Nummern haben.\n");
    return;
  }
  if (projnum&&!member(data,projnum))
  {
    tell_object(TP,"Ein Projekt mit der Nummer " + projnum + " existiert nicht.\n");
    return;
  }
  if (!projnum)
    if (IS_LEARNER(this_interactive())) EnterShort(projnum,NEWMAP(0));
    else tell_object(TP,"Das duerfen nur Magier!\n"); 
  else 
  {
    if (IS_ARCH(this_interactive())||getuid(this_interactive())==data[projnum,PM_Supervisor]||
        getuid(this_interactive())==MAGIERNAME)
    {
      if (member(modifying,projnum))
      {
        tell_object(TP,BS("Das Projekt wird gerade bearbeitet. Bitte warte, bis " +
                          capitalize(modifying[projnum]) + " fertig ist."));
        return;
      }
      newdata=NEWMAP(projnum);
      for (i=PM_MAX;i>=0;i--) newdata[projnum,i]=data[projnum,i];
      modifying+=([ projnum : getuid(TP) ]);
      ShowMenu(projnum,newdata);
    }
    else tell_object(TP,"Du darfst an diesem Projekt nichts aendern.\n");
  }
  return;
}

static void DeleteData2(string bool, int index)
{
  if (bool&&stringp(bool)&&(bool=="j"||bool=="J")) 
  {
    data-=([ index ]);
    save_object(PROJSAVEFILE);
    tell_object(TP,"Projekt unwideruflich geloescht!\n");
  }
  return;
}

void DeleteData(int projnum)
{
  if (projnum<1)
  {
    tell_object(TP,"Projekte koennen nur positive Nummern haben.\n");
    return;
  }
  if (!member(data,projnum)) 
  {
    tell_object(TP,"Ein Projekt mit der Nummer " + projnum + " existiert nicht.\n");
    return;
  }
  if (!IS_ARCH(this_interactive())&&getuid(this_interactive())!=data[projnum,PM_Supervisor]&&
      getuid(this_interactive())!=MAGIERNAME)
  {
    tell_object(TP,"Dieses Projekt darfst Du nicht loeschen.\n");
    return;
  }
  tell_object(TP,"\nWillst Du das folgende Projekt wirklich loeschen ???\n\n"+BuildLong(projnum)+
              "\n\n(j/n) ");
  input_to("DeleteData2",0,projnum);
  return;
}

static void ShowMenu2(string selection, int index, mapping newdata)
{
  int fail;
  fail=0;
  if (!selection||!stringp(selection)) fail=1;
  else
  {
    switch (selection)
    {
      case "s": 
      case "S":
        newdata[index,PM_Change]=time();
        data-=([ index ]);
        data+=newdata;
        modifying-=([ index ]);
        changetime=time();
        tell_object(TP,"Aenderungen gespeichert.\n");
        break;
      case "l": 
      case "L": TP->More("Die Langbeschreibung lautet wie folgt:\n" +
                         break_string(newdata[index,PM_Long],77,0,BS_LEAVE_MY_LFS));
        ShowMenu(index,newdata);
        break;
      case "q": case "Q":
        tell_object(TP,"OK, Aenderungen verworfen!\n");
        modifying-=([ index ]);
        break;
      case "1": ChangeProjNum(index,newdata);break;
      case "2": ChangeDate(index,newdata);break;
      case "3": ChangeSupervisor(index,newdata);break;
      case "4": EnterTeam(index,newdata);break;
      case "5": EnterType(index,newdata);break;
      case "6": EnterJob(index,newdata);break;
      case "7": EnterSize(index,newdata);break;
      case "8": EnterWizLevel(index,newdata);break;
      case "9": EnterStatus(index,newdata);break;
      case "10": EnterShort(index,newdata);break;
      case "11": EnterLong(index,newdata);break;
      default: fail=1; break;
    }
  }
  if (fail)
  {
    tell_object(TP,"Bitte eine der unten angegebenen Wahlmoeglichkeiten verwenden.\n");
    ShowMenu(index,newdata);
  }
  return;
}

static void ShowMenu(int index,mapping newdata)
{
  string wizards, work, size, special, curwizname, *wiz,curwizards;
  int i;
  work=""; special=""; wizards="";
  switch(newdata[index,PM_Size])
  {	
  case PMS_Medium: size="mittelgross"; break;
  case PMS_Large:  size="gross"; break;
  default: size="klein"; break;
  }
  if (newdata[index,PM_Type]!=" ")
  {
    if (test_bit(data[index,PM_Type],PMT_Guild)) work+="Gilde, ";
    if (test_bit(data[index,PM_Type],PMT_Mudlib)) work+="Mudlib, ";
    if (test_bit(data[index,PM_Type],PMT_Region)) work+="Region";
    else work=work[0..<3];
  }
  else work="Unbestimmt";
  if (newdata[index,PM_Job]!=" ")
  {
    if (test_bit(data[index,PM_Job],PMJ_Rework)) special+="Ueberarbeiten, ";
    if (test_bit(data[index,PM_Job],PMJ_Details)) special+="Details einfuegen, ";
    if (test_bit(data[index,PM_Job],PMJ_Debug)) special+="Debuggen";
    else special=special[0..<3];
  }
  else special="komplett neues Projekt";
  wiz=newdata[index,PM_Wizards];
  if (sizeof(wiz)>0)
  {
    i=0;
    curwizards="";
    for(i=sizeof(wiz)-1;i>=0;i--)
    {
      curwizname=capitalize(wiz[i]);
      if (strlen(curwizname)+strlen(curwizards)>48)
      {
        wizards=wizards+"                             "+sprintf("%-48.48s\n",curwizards);
        curwizards="";
      }
      curwizards=curwizards+curwizname+", ";
    }
    wizards=wizards+"                             "+sprintf("%-48.48s",curwizards[0..<3]+".\n");
    wizards=wizards[29..];
  }
  else wizards="Noch keiner.\n";
  tell_object(TP,
              "-----------------------------------------------------------------------------\n" +
              "Das Projekt hat zur Zeit die folgenden Eigenschaften:\n"+
              "[ 1] Projektnummer          : " + index + "\n" +
              "[ 2] Erstellungsdatum       : " + ConvertDate(newdata[index,PM_Date])+ "\n" +
              "[ 3] Verantwortlicher Magier: " + capitalize(newdata[index,PM_Supervisor]) + "\n" +
              "[ 4] Mitarbeiter            : " + wizards +
              "[ 5] Projekttyp             : " + work+ ".\n" +
              "[ 6] Arbeitstyp             : " + special + ".\n" +
              "[ 7] Projektumfang          : " + size + ".\n" +
              "[ 8] Benoetigter Magierlevel: " + newdata[index,PM_Wizlevel] + ".\n" +
              "[ 9] Status                 : " + upperstring(PMU_States[newdata[index,PM_Status]]) + "\n" + 
              "[10] Kurzbeschreibung       :\n>>> " + newdata[index,PM_Short]+ "\n" +
              "[11] Langbeschreibung (Anzeige mit L)\n\n" +
              "Du hast folgende Moeglichkeiten:\n" +
              "[S] Die oben angegebenen Eigenschaften speichern\n" +
              "[Q] Aenderungen verwerfen\n" +
              "[L] Langbeschreibung anzeigen\n" +
              "[1] - [11] Eigenschaften aendern\n" +
              "-----------------------------------------------------------------------------\n>>");
  input_to("ShowMenu2",0,index,newdata);
  return;
}

int remove(int silent)
{
  save_object(PROJSAVEFILE);
  destruct(this_object());
  return 1;
}

int ChangeTime()
{
  return changetime;
}
