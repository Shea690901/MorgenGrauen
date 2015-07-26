// (c) by Padreic (Padreic@mg.mud.de)

#include <defines.h>
#include <properties.h>
#include <v_compiler.h>
#include "../files.h"

inherit "std/virtual/v_compiler";
inherit "std/thing/language";
inherit "std/thing/description";

#define HEADERFILE "/p/service/padreic/kraeuter/plant.h"

// mit Hilfe dieses mappings kommt man sowohl an die ID und die Eigenschaften,
// als auch an die Liste der Raeume in denen das Kraut mit dem filenamen room
// gefunden werden kann.
private mapping map_ldfied; // ([ "key": ({ ({eigenschaften}), ([raeume]) }) ])
private mapping validfiles;

// mit dieser Funktion kann ein neuer aktueller Plantheader erzeugt werden
void update_header_file()
{
   int i, si;
   string *keys;
   mapping tmp;

   // Daten update durchfuehrn
   if (!restore_object(KRAEUTERVCSAVEFILE))
      raise_error("Data not available!\n");

   keys=m_indices(map_ldfied);
   si=sizeof(keys);
   validfiles=([]);
   for (i=si-1; i>=0; i--) validfiles+=([ keys[i] ]);

   // validfiles speichern...
   tmp=map_ldfied; map_ldfied=([]); save_object(object_name()); map_ldfied=tmp;

   if (si<=0) raise_error("Data not available!\n");
   keys=sort_array(keys, #'<);
   rm(HEADERFILE);
   write_file(HEADERFILE,
     "// Automatisch generiertes File, nicht von Hand editieren!\n"
    +"// Erzeugendes File: "+object_name()+"\n\n"
    +"#define PLANTCOUNT "+to_string(si)+"\n\n"
    +"#define PLANT(x) \"/p/service/padreic/kraeuter/\"+x\n\n"
   );
   for (i=si-1; i>=0; i--)
     write_file(HEADERFILE, sprintf("#define %-30s PLANT(\"%s\")\n", upperstring(keys[i]), keys[i]));
}

#define KATEGORYS ({ "klee", "rebe", "hahnenfuss", "rettich", "kraut", "wurz",\
                     "moos", "enzian", "rautenwicke", "pilz", "nelke",\
                     "lichtnelke"})

int _createfile(string filename)
{
   int i;
   mixed *arr;
   string str, short, long, gender, *name, roomdetail;
   string *ids;
   string plantfile;

   if (object_name(previous_object())!=PLANTMASTER) {
      write("Illegal usage of _createfile()!\n");
      return 1;   
   }
   if (!pointerp(arr=map_ldfied[filename])) {
      write("Unknown Plant '"+filename+"'.\n");
      return 1;
   }
   if (file_size(PLANTDIR+filename+".c")>=0) {
      write("error: file "+PLANTDIR+filename+".c allready exists.\n");
      return 1;
   }

   // Kurzbeschreibung erzeugen
   SetProp(P_NAME,     arr[0][INGREDIENT_NAME]);
   SetProp(P_NAME_ADJ, arr[0][INGREDIENT_ADJ]);
   SetProp(P_GENDER,   arr[0][INGREDIENT_GENDER]);
   if (arr[0][INGREDIENT_DEMON]==RAW) {
       SetProp(P_ARTICLE, 0);
       short=Name(WER);
       SetProp(P_ARTICLE, 1);
   }
   else short=Name(WER,arr[0][INGREDIENT_DEMON]);
   ids = ({ lowerstring(arr[0][INGREDIENT_NAME]) });
   // bei zusammengesetzten Namen, auch den hauptnamen akzeptieren
   str=lowerstring(arr[0][INGREDIENT_NAME]);
   name=explode(str, "-");
   if (sizeof(name)>1) ids += ({ name[<1] });
   name=explode(str, " ");
   if (sizeof(name)>1) ids += ({ name[<1] });
   for (i=sizeof(KATEGORYS)-1; i>=0; i--) {
       if (strstr(str, KATEGORYS[i], 0)==-1) continue;
       ids += ({ KATEGORYS[i] });
       break;
   }
   switch(arr[0][INGREDIENT_GENDER]) {
     case MALE:   gender="MALE"; break;
     case FEMALE: gender="FEMALE"; break;
     case NEUTER: gender="NEUTER"; break;
     default: gender=arr[0][INGREDIENT_GENDER];
   }
   long="    \""+implode(old_explode(arr[0][INGREDIENT_LONG], "\n"), "\\n\"\n   +\"")+"\\n\"";
   roomdetail="    \""+implode(old_explode(arr[0][INGREDIENT_ROOMDETAIL], "\n"), "\\n\"\n   +\"")+"\\n\"";
   plantfile=
 "// (c) 2001 by Padreic (Padreic@mg.mud.de)\n\n"
+"#include <properties.h>\n"
+"#include \"../files.h\"\n"
+"#include \"/p/service/padreic/kraeuter/plant.h\"\n\n"
+"inherit PLANT(\"plant\");\n\n"
+"void create()\n"
+"{\n"
+"  ::create("+upperstring(filename)+");\n"
+"  SetProp(P_NAME,     \""+arr[0][INGREDIENT_NAME]+"\");\n"
+"  SetProp(P_NAME_ADJ, \""+(arr[0][INGREDIENT_ADJ]||"")+"\");\n"
+"  SetProp(P_GENDER,   "+gender+");\n"
+"  SetProp(P_LONG,     \n"
+long+");\n"
+"  SetProp(PLANT_ROOMDETAIL, \n"
+roomdetail+");\n"
+"  SetProp(P_SHORT,    \""+short+"\");\n";
  plantfile+="  AddId(({";
  for (i=sizeof(ids)-1; i>=0; i--)
     plantfile+=" \""+ids[i]+"\",";
  plantfile[<1]=' ';
  plantfile+="}));\n";
  // Adjective vorher deklinieren
  if (stringp(short=arr[0][INGREDIENT_ADJ])) {
     short=DeclAdj(lowerstring(short), WEN, 0)[0..<2];
     plantfile+="  AddAdjective(\""+short+"\");\n";
  }
  plantfile+="}\n";
  write_file(PLANTDIR+filename+".c", plantfile);
  write(PLANTDIR+filename+".c created.\n");
  return 1;
}

void create()
{
   if (clonep(this_object())) {
      destruct(this_object());
      raise_error("Don't clone VirtualCompiler!\n");
   }
   seteuid(getuid());
   // Daten update durchfuehrn
   if (!restore_object(object_name()))
      raise_error("Data not available!\n");
   if (!restore_object(KRAEUTERVCSAVEFILE)) map_ldfied=([]);
   v_compiler::create();
   description::create();

   SetProp(P_COMPILER_PATH, "/p/service/padreic/kraeuter");
   SetProp(P_STD_OBJECT,    "/p/service/padreic/kraeuter/plant.c");
}

mixed compile_object(string file)
{
   string str;
   // es gibt einige files auch einzeln...
   if (file_size(SECURE(str=explode(file, "/")[<1]+".c"))>0)
      return clone_object(SECURE(str));
   return ::compile_object(file);
}

string Validate(string file)
{
   if (!stringp(file)) return 0;
   file = (string)::Validate(explode(file, "#")[0]);
   return (member(validfiles, file) ? file : 0);
}

// fuer den Master kann man einfach master() nehmen. Zesstra, 7.1.10
//private nosave object master;
private nosave object simul_efun;

// fuer SIMUL_EFUN_FILE
#include <config.h>
nomask static private string get_cloner()
{
   int i;
   object po;
   /*
   if (!master) {
      master=find_object("/secure/master.c");
      if (!master) raise_error("inkonsistent system!\n");
   }
   */
   // Es kann nicht passieren, dass es keine simul_efun oder master gibt, denn
   // dann wird das Mud runtergefahren. Ausserdem sollte man die Pfade nicht
   // hart-kodieren.
   /*
   if (!simul_efun) {
      if (!(simul_efun=find_object("/secure/simul_efun")))
         simul_efun=find_object("/secure/spare_simul_efun.c");
      if (!simul_efun) raise_error("inkonsistent system!\n");
   }
   */
   if (!simul_efun) {
      if (!(simul_efun=find_object(SIMUL_EFUN_FILE)))
         simul_efun=find_object(SPARE_SIMUL_EFUN_FILE);
   }
   // wenn sie jetzt nicht existiert - auch gut, dann gibt es halt keine
   // sefuns.

   for (i=0; po=previous_object(i); i++) {
      if (po==master() || po==simul_efun || po==ME || po==previous_object())
         continue;
      else return BLUE_NAME(po);
   }
   return 0;
}

varargs CustomizeObject(string file)
{
   int i, legal;
   mixed arr;
   string str, *name;
   closure sp;
   object cloner;

   if (stringp(file))
      file=Validate(file);
   else file=::CustomizeObject();
   if (!file) return 0;
   if (previous_object()->QueryPlantId()) return 0; // bereits initialisiert
   sp=symbol_function("SetProp", previous_object());
   if (pointerp(arr=map_ldfied[file])) {
      // Blueprint oder legal cloner
      cloner = get_cloner();
      legal=(member(arr[1], get_cloner())>=0) || (object_name(cloner)==PLANTMASTER);
      if (BLUE_NAME(previous_object())==STDPLANT ||
          !clonep(previous_object()) || legal)
      {
         if ((arr[0][INGREDIENT_NAME]=="Klee") ||
             (arr[0][INGREDIENT_NAME][<4..]=="klee")) {
            funcall(sp, P_NAME, ({ arr[0][INGREDIENT_NAME],
                                   arr[0][INGREDIENT_NAME]+"s",
                                   arr[0][INGREDIENT_NAME],
                                   arr[0][INGREDIENT_NAME]}));
         }
         else funcall(sp, P_NAME,     arr[0][INGREDIENT_NAME]);
         funcall(sp, P_NAME_ADJ, arr[0][INGREDIENT_ADJ]);
         funcall(sp, P_GENDER,   arr[0][INGREDIENT_GENDER]);
         funcall(sp, P_LONG,     arr[0][INGREDIENT_LONG]);
         funcall(sp, PLANT_ROOMDETAIL, arr[0][INGREDIENT_ROOMDETAIL]);
         if (arr[0][INGREDIENT_DEMON]==RAW) {
            funcall(sp, P_ARTICLE, 0);
            funcall(sp, P_SHORT, previous_object()->Name(WER));
            funcall(sp, P_ARTICLE, 1);
         }
         else funcall(sp, P_SHORT, previous_object()->Name(WER,arr[0][INGREDIENT_DEMON]));
         previous_object()->AddId(lowerstring(arr[0][INGREDIENT_NAME]));
         // bei zusammengesetzten Namen, auch den hauptnamen akzeptieren
         str=lowerstring(arr[0][INGREDIENT_NAME]);
         name=explode(str, "-");
         if (sizeof(name)>1) previous_object()->AddId(name[<1]);
         name=explode(str, " ");
         if (sizeof(name)>1) previous_object()->AddId(name[<1]);
         for (i=sizeof(KATEGORYS)-1; i>=0; i--) {
            if (strstr(str, KATEGORYS[i], 0)==-1) continue;
            previous_object()->AddId(KATEGORYS[i]);
            break;
         }
         // Adjective vorher deklinieren
         if (stringp(str=arr[0][INGREDIENT_ADJ])) {
            str=DeclAdj(lowerstring(str), WEN, 0);
            previous_object()->AddAdjective(str);
         }
         previous_object()->SetPlantId(legal ? arr[0][INGREDIENT_ID] : -1);
         return file;
      }
   }
   funcall(sp, P_NAME,     "Kraut");
   funcall(sp, P_GENDER,   NEUTER);
   funcall(sp, P_SHORT,    "Ein Testkraut ("+capitalize(file)+")");
   funcall(sp, P_LONG,     "Ein nicht naeher spezifiziertes Testkraut.\n");
   funcall(sp, PLANT_ROOMDETAIL, "Ein nicht naeher spezifiziertes Testkraut ("+capitalize(file)+").\n");
   previous_object()->AddId("kraut");
   previous_object()->SetPlantId(-1);
   return file;
}

// ist zwar absoluter Unfug, aber damit Rikus besser schlafen kann...
int NoParaObjects()
{   return 1;   }
