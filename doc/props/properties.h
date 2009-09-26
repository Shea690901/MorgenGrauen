
armour.h:
---------
P_AC                           "ac"                          
 *  Numerischer Wert fuer die Abwehrstaerke der Ruestung.

P_WORN                         "worn"                        
 *  Flag, ob die Ruestung im Moment getragen wird. Falls ja,
 *  enthaelt die Property das Traegerobjekt.

P_ARMOUR_TYPE                  "armour_type"                 
 *  String fuer Art der Ruestung; welcher Koerperteil wird
 *  geschuetzt?

P_DEFEND_FUNC                  "defend_func"                 
 *  Enthaelt das Objekt, das eine DefendFunc() definiert
 *  (fuer Ruestungen)

P_WEAR_FUNC                    "wear_func"                   
 *  Enthaelt das Objekt, das eine WearFunc() definiert
 *  (fuer Ruestungen)

P_REMOVE_FUNC                  "remove_func"                 
 *  Enthaelt das Objekt, das eine RemoveFunc() definiert
 *  (fuer Ruestungen)


bank.h:
-------
P_SHOP_PERCENT_LEFT            "shop_percent_left"           
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_STORE_PERCENT_LEFT           "store_percent_left"          
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_CURRENT_MONEY                "current_money"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_STORE_CONSUME                "store_consume"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_MIN_STOCK                    "min_stock"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***


combat.h:
---------
P_NO_GLOBAL_ATTACK             "no_global_attack"            
 *  Falls diese Property in einem NPC gesetzt ist, wird dieser bei einem
 *  "toete alle" nicht mit angegriffen
 *  (das ist zB. bei BegleitNPCs ganz nuetzlich)

P_FRIEND                       "friend"                      
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_LAST_COMBAT_TIME             "last_combat_time"            
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_DAMAGED                      "item_damaged"                
 *  Grad der Beschaedigung einer Waffe/Ruestung.
 *  Die Property sollte so gesetzt sein, dass P_DAMAGED + aktuelles P_WC/AC
 *  die urspruengliche P_WC/AC ergibt

P_EFFECTIVE_WC                 "effective_wc"                
 *  Die durchschnittliche Wirkung einer Waffe, basierend auf P_WC und
 *  einer ggf. vorhandenen HitFunc().

P_EFFECTIVE_AC                 "effective_ac"                
 *  Die durchschnittliche Wirkung einer Ruestung, basierend auf P_AC
 *  und einer ggf. vorhandenen DefendFunc().

P_QUALITY                      "quality"                     
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_RESTRICTIONS                 "restrictions"                
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_EQUIP_TIME                   "equip_time"                  
 *** KEINE BESCHREIBUNG VORHANDEN ***


container.h:
------------
P_MAX_WEIGHT                   "max_weight"                  
 *  Maximales Gewicht in Gramm, das in dem Container verstaut werden
 *  kann.

P_CONTENTS                     "contents"                    
 *  *** OBSOLET! ***

P_CNT_STATUS                   "cnt_status"                  
 *  Status des Containers (offen, geschlossen, abgeschlossen)
 *  siehe auch /sys/container.h

P_TRANSPARENT                  "transparent"                 
 *  ist != 0, wenn hinein oder hinausgeschaut werden kann.


doorroom.h:
-----------
P_DOOR_INFOS                   "door_infos"                  
 *  Hat was mit den Tueren zu tun -> Muss Rochus mal dokumentieren


fishing.h:
----------
P_WATER                        "water"                       
 *  Gewaessertyp. Erlaetuerungen in fishing.h

P_FISH                         "fish"                        
 *  Fischdichte. Erlaeuterungen in fishing.h

P_LIQUID                       "w_max_wasserfuellmenge"      
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_LONG_EMPTY                   "w_longdesc_empty"            
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_LONG_FULL                    "w_longdesc_full"             
 *** KEINE BESCHREIBUNG VORHANDEN ***


guard.h:
--------
P_GUARD                        "guard"                       
 *** KEINE BESCHREIBUNG VORHANDEN ***


inpc/boozing.h:
---------------
P_I_HATE_ALCOHOL               "i_hate_alcohol"              
 *** KEINE BESCHREIBUNG VORHANDEN ***


inpc/eval.h:
------------
P_EVAL_OFFSETS                 "inpc_eval_offsets"           
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_EVAL_FACTORS                 "inpc_eval_factors"           
 *** KEINE BESCHREIBUNG VORHANDEN ***


inpc/walking.h:
---------------
P_INPC_LAST_PLAYER_CONTACT     "inpc_last_player_contact"    
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_INPC_LAST_ENVIRONMENT        "inpc_last_environment"       
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_INPC_WALK_MODE               "inpc_walk_mode"              
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_INPC_WALK_DELAYS             "inpc_walk_delay"             
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_INPC_WALK_FLAGS              "inpc_walk_flags"             
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_INPC_WALK_AREA               "inpc_walk_area"              
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_INPC_WALK_ROUTE              "inpc_walk_route"             
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_INPC_HOME                    "inpc_home"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***


language.h:
-----------
P_ARTICLE                      "article"                     
 *  Gibt an, ob in der Beschreibung ein Artikel ausgegeben werden soll
 *  oder nicht.

P_GENDER                       "gender"                      
 *  Grammatikalisches Geschlecht des Objektes:
 *  (Definiert in language.h) MALE, FEMALE oder NEUTER


living/attributes.h:
--------------------
P_ATTRIBUTES                   "attributes"                  
 *  Mapping mit den Attributen des Wesens.

P_ATTRIBUTES_OFFSETS           "attributes_offsets"          
 *  Mapping mit Offsets, die zu den Attributen addiert werden (koennen auch
 *  negativ sein) - zB Rassenboni.

P_ATTRIBUTES_MODIFIER          "attributes_modifier"         
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_X_ATTR_MOD                   "extern_attributes_modifier"  
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_X_HEALTH_MOD                 "extern_health_modifier"      
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_ABILITIES                    "abilities"                   
 *  *** OBSOLET! ***
 *  Siehe P_NEWSKILLS.


living/combat.h:
----------------
P_RESISTANCE_STRENGTHS         "resistance_strengths"        
 *  Mapping mit Schadensfaktoren minus 1.0 fuer jeden Schadenstyp
 *  -0.5 entspricht also Resistance (Faktor 0.5)
 *  1.0 entspricht also Vulnerability (Faktor 2.0)

P_RESISTANCE                   "resistance"                  
 *  Array mit Angriffsarten, gegen die das Lebewesen teilweise
 *  resistent ist.

P_VULNERABILITY                "vulnerability"               
 *  Array mit Angriffsarten, gegen die das Lebewesen empfindlich
 *  ist.

P_TOTAL_AC                     "total_ac"                    
 *  Numerischer Wert der Abwehrstaerke des Wesens.

P_HANDS                        "hands"                       
 *  3-elem. Array
 *  1. Elem.: String mit der Meldung, wenn ohne Waffen angegriffen wird.
 *  2. Elem.: Weaponclass, wenn ohne Waffen angegriffen wird.
 *  3. Elem.: Angriffs-typ, default ist DT_BLUDGEON

P_MAX_HANDS                    "max_hands"                   
 *  Anzahl der Haende, die ein Wesen hat.

P_USED_HANDS                   "used_hands"                  
 *  Anzahl der Haende in Benutztung

P_FREE_HANDS                   "free_hands"                  
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_HANDS_USED_BY                "hands_used_by"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_ATTACK_BUSY                  "attack_busy"                 
 *  Kann der Spieler noch Spezialwaffen (zB Flammenkugel) einsetzen?

P_NO_ATTACK                    "no_attack"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_PREFERED_ENEMY               "pref_enemy"                  
 *  Array: 1. Element: Wahrscheinlichkeit, dass einer der bevorzugten
 *  Feinde (restliche Elemente) genommen wird.

P_SHOW_ATTACK_MSG              "show_attack_msg"             
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_LAST_DAMTYPES                "last_damtypes"               
 *  Schadenstypen, mit denen das letzte do_damage() von Defend() aus
 *  ausgeloest wurde (kann von Leichen abgefragt werden).
 *  Entweder ein Array mit den Schadenstypen oder 0.

P_RESISTANCE_STRENGHTS         P_RESISTANCE_STRENGTHS        
 *** KEINE BESCHREIBUNG VORHANDEN ***


living/description.h:
---------------------
P_EXTRA_LOOK                   "extralook"                   
 *  String, der einen zusaetzlichen Text in der long()-Beschreibung
 *  eines Spielers erzeugt.


living/life.h:
--------------
P_AGE                          "age"                         
 *  Alter des Spielers in Heart-Beats (1 HB == 2 Sekunden)

P_ALIGN                        "align"                       
 *  Numerischer Wert fuer Gut- oder Boesheit des Wesens.

P_DEADS                        "deads"                       
 *  Anzahl der Tode des Spielers seit Einfuehrung dieser Property (irgendwann
 *  im Dezember 94)

P_GHOST                        "ghost"                       
 *  Gesetzt, wenn der Spieler tot ist.

P_FROG                         "frog"                        
 *  Gesetzt, wenn der Spieler ein Frosch ist.

P_FOOD                         "food"                        
 *  Numerischer Wert fuer Saettigungsgrad des Wesens.

P_MAX_FOOD                     "max_food"                    
 *  Numerischer Wert fuer die maximale Saettigung des Wesens.

P_DRINK                        "drink"                       
 *  Numerischer Wert fuer Saettigung des Wesens mit Getraenken.

P_MAX_DRINK                    "max_drink"                   
 *  Numerischer Wert fuer die maximale 'Wassermenge' im Wesen.

P_ALCOHOL                      "alcohol"                     
 *  Num. Wert fuer Besoffenheit.

P_MAX_ALCOHOL                  "max_alcohol"                 
 *  Numerischer Wert fuer die Alkoholvertraeglichkeit des Wesens.

P_HP                           "hp"                          
 *  Anzahl der Lebenspunkte des Wesens.

P_MAX_HP                       "max_hp"                      
 *  Maximale Anzahl der Lebenspunkte.

P_SP                           "sp"                          
 *  Anzahl der Magiepunkte des Wesens.

P_MAX_SP                       "max_sp"                      
 *  Maximale Anzahl der Magiepunkte.

P_XP                           "xp"                          
 *  Anzahl der Erfahrungspunkte.

P_FOOD_DELAY                   "food_delay"                  
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_DRINK_DELAY                  "drink_delay"                 
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_ALCOHOL_DELAY                "alcohol_delay"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_HP_DELAY                     "hp_delay"                    
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_SP_DELAY                     "sp_delay"                    
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_POISON_DELAY                 "poison_delay"                
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_NO_REGENERATION              "no_regeneration"             
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_ENEMY_DAMAGE                 "enemy_damage"                
 *  Gibt eine Kopie des Mappings zurueck, in dem vermerkt wird, wer
 *  diesem Lebewesen welchen Schaden zugefuegt hat.


living/team.h:
--------------
P_TEAM                         "team"                        
 *  Teamobjekt, falls Spieler in einem Team ist.

P_TEAM_NEWMEMBER               "potential_team_member"       
 *  Spieler moechte ins Team von diesem Objekt aufgenommen werden.

P_TEAM_ATTACK_CMD              "team_attack_cmd"             
 *  Angriffsbefehl des Spielers, nicht setzbar.

P_TEAM_AUTOFOLLOW              "team_autofollow"             
 *  Folgewunsch des Spielers, nicht setzbar.

P_TEAM_WANTED_ROW              "team_wanted_row"             
 *  Gewuenschte Reihe des Spielers (von 1 bis MAX_TEAMROWS)

P_TEAM_WIMPY_ROW               "team_wimpy_row"              
 *  Fluchtreihe des Spielers (von 1 bis MAX_TEAMROWS)
 *  Wenn die Fluchtreihe <=1 ist ist die Flucht in eine hintere Reihe
 *  deaktiviert.

P_TEAM_LEADER                  "team_leader"                 
 *  Teamobjekt, falls Spieler Anfuehrer eines Teams ist.

P_TEAM_ASSOC_MEMBERS           "team_assoc_members"          
 *  Array mit den zugeordneten NPCs des Spielers bzw.
 *  der Spieler, dem dieser NPC zugeordnet ist.
 *  Zugeordnete NPCs sind automatisch im Team des Spielers.
 *  Der Zugriff auf diese Property sollte ueber AssocMember()
 *  bzw. DeAssocMember() erfolgen.

P_TEAM_COLORS                  "team_colors"                 
 *  Grenzwerte fuer farbige Anzeige im Teaminfo.
 *  Array mit 4 Werten ({ lp_rot, lp_gelb, sp_rot, sp_gelb })


new_skills.h:
-------------
P_VALID_GUILDS                 "valid_guilds"                
 *  Enthaelt die zugelassenen Gilden und ist nur fuer den Gildenmaster
 *  von Bedeutung.

P_GUILD_SKILLS                 "guild_skills"                
 *  Gildenproperty
 *  Enthaelt ein Mapping mit allen Spruechen und Faehigkeiten der Gilde.
 *  Wird mit AddSkill()/AddSpell() modifiziert.

P_GUILD_RESTRICTIONS           "guild_rest"                  
 *  Gildenproperty
 *  Enthaelt ein Mapping mit den Eintrittbeschraenkungen einer Gilde.

P_GUILD_DEFAULT_SPELLBOOK      "guild_sb"                    
 *  Gildenproperty
 *  Der Name des Spellbooks, das von der Gilde standardmaessig verwendet
 *  wird (kann bei Spells mit SI_SPELLBOOK ueberschrieben werden).

P_GUILD_MALE_TITLES            "guild_male_titles"           
 *  Gildenproperty
 *  Ein Mapping mit den Stufenbezeichnungen fuer maennliche Gildenmitglieder.
 *  Als Key dient der Gildenlevel.

P_GUILD_FEMALE_TITLES          "guild_female_titles"         
 *  Gildenproperty
 *  Ein Mapping mit den Stufenbezeichnungen fuer weibliche Gildenmitglieder.
 *  Als Key dient der Gildenlevel.

P_GUILD_LEVELS                 "guild_levels"                
 *  Gildenproperty
 *  Ein Mapping mit den Stufenbeschraenkungen fuer die Gildenlevel.
 *  Als Key dient der jeweilige Gildenlevel.

P_SB_SPELLS                    "sb_spells"                   
 *  Spellbookproperty
 *  Hier sind saemtliche Spells des Spellbooks vermerkt (wird mit AddSpell()
 *  veraendert).

P_GLOBAL_SKILLPROPS            "sm_global"                   
 *  Gilden- und Spellbookproperty
 *  Eigenschaften, die ALLE Spells eines Spellbooks bzw. ALLE Skills und
 *  Spells einer Gilde haben sollen.

#define                        P_GUILD                       
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_GUILD_LEVEL                  "guild_level"                 
 *  Gildenlevel des Spielers (nicht unbedingt identisch mit dem Spielerlevel)

P_GUILD_TITLE                  "guild_title"                 
 *  Gildentitel, der dem Gildenlevel des Spielers entspricht. Wird nur
 *  angezeigt, wenn P_TITLE=0 ist.

P_GUILD_RATING                 "guild_rating"                
 *  Einstufung des Spielers in der Gilde (zwischen 0 und 10000).
 *  Wie sich die Einstufung zusammensetzt, ist Sache der jeweiligen Gilde.

P_NEWSKILLS                    "newskills"                   
 *  Hier sind saemtliche Skills und Spells vermerkt, die der Spieler kennt.

P_NEXT_SPELL_TIME              "next_spell"                  
 *  Wann kann das naechste Mal gezaubert werden?

P_TMP_ATTACK_HOOK              "attack_hook"                 
 *  Mindestens 3-elementiges Array ({zeitpunkt, objekt, funktion, ...}).
 *  Die Funktion wird im 'objekt' mit dem Feind des Lebewesens als Parameter
 *  zu Beginn von Attack() (des Lebewesens) aufgerufen, wenn der 'zeitpunkt'
 *  noch nicht ueberschritten ist. Wenn die Funktion 0 zurueckgibt, wird
 *  Attack() abgebrochen. Ueber optionale Arrayelemente koennen der Funktion
 *  weitere Parameter uebergeben werden.

P_TMP_ATTACK_MOD               "attack_mod"                  
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_TMP_DEFEND_HOOK              "defend_hook"                 
 *  Mindestens 3-elementiges Array ({zeitpunkt, objekt, funktion, ...}).
 *  Die Funktion wird im 'objekt' mit den gleichen Parametern wie Defend()
 *  zu Beginn von Defend() (des Lebewesens) aufgerufen, wenn der 'zeitpunkt'
 *  noch nicht ueberschritten ist. Wenn die Funktion 0 zurueckgibt, wird
 *  Defend() abgebrochen, ansonsten wird als Rueckgabe ein 3-elementiges
 *  Array ({schaden, schadenstypen, spell}) erwartet, die anstelle der
 *  Defend() uebergebenen Werte verwendet werden. Ueber optionale Array-
 *  elemente koennen der Funktion weitere Parameter uebergeben werden.

P_TMP_DIE_HOOK                 "die_hook"                    
 *  Mindestens 3-elementiges Array ({zeitpunkt, objekt, funktion, ...}).
 *  Die Funktion wird im 'objekt' mit dem gleichen Parameter wie die()
 *  zu Beginn von die() (des Lebewesens) aufgerufen, wenn der 'zeitpunkt'
 *  noch nicht ueberschritten ist. Wenn die Funktion 1 zurueckgibt, wird
 *  die() abgebrochen. Ueber optionale Arrayelemente koennen der Funktion
 *  weitere Parameter uebergeben werden.

P_TMP_MOVE_HOOK                "move_hook"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_DEFENDERS                    "defenders"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_PREPARED_SPELL               "prepared_spell"              
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_DEFAULT_GUILD                "default_guild"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_MAGIC_RESISTANCE_OFFSET      "mag_res_offset"              
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_SKILL_ATTRIBUTES             "skill_attr"                  
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_SKILL_ATTRIBUTE_OFFSETS      "skill_attr_offsets"          
 *** KEINE BESCHREIBUNG VORHANDEN ***


player.h:
---------
P_LAST_COMMAND_ENV             "last_command_env"            
 *  Der Raum, in dem das letzte Kommando eingegeben wurde.

P_HISTMIN                      "histmin"                     
 *  Minimale Laenge, die eine Zeile haben muss, um in die History zu kommen

P_SHOW_ALIAS_PROCESSING        "show_alias_processing"       
 *  Arbeit des Parsers beobachten (debugging)

P_DEFAULT_NOTIFY_FAIL          "default_notify_fail"         
 *  Welche Fehlermeldung kommt, wenn kein Objekt ein notify_fail macht?

P_NETDEAD_INFO                 "netdead_info"                
 *  Hier kann ein Raum Informationen in einem Spieler zwischenspeichern, wenn
 *  dieser in den Netztotenraum gemoved wird.

P_IP_NAME                      "ip_name"                     
 *  Rechnername des Interactives

P_AUTH_INFO                    "auth_info"                   
 *  Username des Spielers, wenn bei ihm ein AUTHD laeuft

P_LAST_KILLER                  "last_killer"                 
 *  Letzter Moerdes des Wesens

P_ACTUAL_NOTIFY_FAIL           "actual_notify_fail"          
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_LEP                          "lep"                         
 *  Levelpunkte eines Spielers
 *  NICHT VON HAND SETZEN!!!


player/base.h:
--------------
P_LAST_LOGIN                   "last_login"                  
 *  Zeitpunkt des letzen Logins

P_LAST_LOGOUT                  "last_logout"                 
 *  Zeitpunkt des letzen Logouts

P_DAILY_PLAYTIME               "daily_playtime"              
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_IGNORE                       "ignore"                      
 *  Array der Spieler, deren Mitteilungen ignoriert werden.

P_SHOW_EXITS                   "show_exits"                  
 *  Gesetzt, wenn der Spieler die offensichtlichen Ausgaenge
 *  immer automatisch sehen will.

P_WANTS_TO_LEARN               "wants_to_learn"              
 *  Gesetzt, wenn der Magier die Filenamen sehen will.
 *  (Nur fuer Magier). Wird diese Property auf 0 gesetzt, gehen auch
 *  einige andere Dinge nicht mehr - verfolge zB. Eigentlich sollten
 *  dann auch die Magierbefehle wie "goto" usw unterbunden werden -
 *  das kommt vielleicht noch.

P_AUTOLOADOBJ                  "autoloadobj"                 
 *  Mit dieser Property werden Autoloadobjekte verwaltet.
 *  Der Inhalt der Property sind die permanenten Eigenschaften des
 *  Objektes, die der Spieler uebers ausloggen hinweg beibehaelt.
 *  Beim Einloggen werden sie automatisch neu gesetzt. (ACHTUNG:
 *  Die Property muss intern selbst verwaltet werden.)
 *  Autoloadobjekte werden beim Ausloggen nicht fallengelassen!

P_AUTOLOAD                     "autoload"                    
 *  Mapping mit der Menge der Autoloadobjekte und den zugeh.
 *  Properties.

P_MAILADDR                     "mailaddr"                    
 *  EMailadresse des Spielers.

P_HOMEPAGE                     "homepage"                    
 *  Die Homepage eines Spielers (mit dem Befehl 'url' zu setzen).

P_FOLLOW_SILENT                "follow_silent"               
 *  Wenn diese Property 1 ist, wird der MOVE vom verfolge Silent ausge-
 *  fuehrt.

P_INVIS                        "invis"                       
 *  Die Property P_INVIS dient dazu, Objekte (insbesondere Magier) als
 *  unsichtbar zu kennzeichnen. Man sollte drei Arten von unsichtbaren
 *  Objekten unterscheiden:
 *  - Gegenstaende
 *    Gegenstaende macht man unsichtbar, indem man in ihnen die Property
 *    P_SHORT auf 0 setzt; will man ein Objekt unsichtbar machen, ohne
 *    seine Kurzbeschreibung zu aendern, kann man aber auch P_INVIS auf
 *    einen Wert ungleich 0 setzen.
 *  - NPCs
 *    NPCs macht man ebenfalls unsichtbar, indem man in ihnen die Property
 *    P_SHORT auf 0 setzt. GGf. kann man auch noch die Property P_INVIS auf
 *    1 setzen.
 *    Der Unterschied: Bei gesetztem P_INVIS wird als Name 'Jemand' ver-
 *    wendet, ansonsten der normale Name des NPCs.
 *  - Spieler / Magier
 *    Spieler und Magier macht man unsichtbar, indem man ihnen die Property
 *    P_INVIS auf einen Wert <>0 setzt.
 *    Spieler duerfen nicht unsichtbar gemacht werden!			 !
 *    Wird ein Magier unsichtbar gemacht, muss man ihm die Property	 !
 *    P_INVIS auf den Wert setzen, den die Property P_AGE zu diesem	 !
 *    Zeitpunkt hat (keine F_QUERY_METHOD !).				 !
 *    Setzt man die Property auf den Wert 1, so erhaelt ein Spieler,
 *    wenn er den entsp. Magier fingert, die Ausgabe: Alter: 00:00:02,
 *    was genauso verraeterisch ist, wie ein Alter, dass bei einem
 *    scheinbar nicht eingeloggten Magier immer weiter hochgezaehlt
 *    wird.

P_SECOND                       "second"                      
 *  Spieler ist Zweitie. Eingetragen ist die uid des Erstspielers.

P_SECOND_MARK                  "second_mark"                 
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_TESTPLAYER                   "testplayer"                  
 *  Gesetzt, wenn der Spieler nicht in der Bestenliste auftauchen soll.

P_TTY                          "tty"                         
 *  Name der Terminalemulation, die der Spieler nutzt.
 *  NOT YET IMPLEMENTED.

P_START_HOME                   "start_home"                  
 *  Raum, in dem der Spieler nach dem Einloggen landen soll

P_CMSG                         "clonemsg"                    
 *  *** OBSOLET! *** Siehe P_CLONE_MSG

P_DMSG                         "destmsg"                     
 *  *** OBSOLET! *** Siehe P_DESTRUCT_MSG

P_CLONE_MSG                    "clone_msg"                   
 *  Meldung, die beim Clonen eines Obj ausgegegen wird (nur Magier)

P_DESTRUCT_MSG                 "destruct_msg"                
 *  Meldung, die beim Destructen Obj ausgegegen wird (nur Magier)

P_CARRIED_VALUE                "carried"                     
 *  Entschaedigung, die der Spieler beim Einloggen erhaelt.

P_PROMPT                       "prompt"                      
 *  Das Prompt (Nur fuer Magier).

P_SCREENSIZE                   "screensize"                  
 *  Bildschirmgroesse in Zeilen (fuer More)

P_MORE_FLAGS                   "more_flags"                  
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_CAN_FLAGS                    "can_flags"                   
 *  Flags, ob Spieler echoen/emoten kann

P_LAST_QUIT                    "last_quit"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_READ_NEWS                    "read_news"                   
 *  Welche Artikel bereits gelesen wurde (frueher: in der MPA)

P_NEEDED_QP                    "needed_qp"                   
 *  APs, die man fuer den Seherstatus braucht


player/can.h:
-------------
P_CAN_FLAGS                    "can_flags"                   
 *  Flags, ob Spieler echoen/emoten kann


player/comm.h:
--------------
P_INTERMUD                     "intermud"                    
 *  Obsolet?

P_BUFFER                       "buffer"                      
 *  Enthaelt Kobold-Nachrichten

P_DEAF                         "deaf"                        
 *  Der Spieler ist taub. Falls hier ein String steht, wird dieser bei
 *  "teile ... mit" an den Mitteilenden ausgegeben, ansonsten kommt nur
 *  "Soundso ist leider gerade taub.\n"

P_PERM_STRING                  "perm_string"                 
 *  Fuer Sprachflueche. In dem Objekt, das in P_PERM_STRING vermerkt ist,
 *  wird bei Sprachbefehlen permutate_string(str) aufgerufen und der
 *  zurueckgegebene String statt dessen ausgegeben.


player/description.h:
---------------------
P_EXTRA_LOOK                   "extralook"                   
 *  String, der einen zusaetzlichen Text in der long()-Beschreibung
 *  eines Spielers erzeugt.

P_PRESAY                       "presay"                      
 *  Presay des Spielers. Erscheint vor dem Namen in Kurz/Langbeschreibung.
 *  Erscheint auch in name(), also in sag, ruf, teile mit usw.

P_TITLE                        "title"                       
 *  Titel des Spielers. Erscheint hinter dem Namen in Kurz/Langbeschreibung.

P_AVERAGE_SIZE                 "average_size"                
 *  Durchschnittliche Groesse eines Wesens dieser Rasse (derzeit nur Player)

P_AVERAGE_WEIGHT               "average_weight"              
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_REFERENCE_OBJECT             "reference_object"            
 *** KEINE BESCHREIBUNG VORHANDEN ***


player/moving.h:
----------------
P_MSGIN                        "msgin"                       
 *  String mit der Meldung, die beim Verlassen eines Raumes mit M_GO
 *  an die uebrigen Anwesenden ausgegeben wird.

P_MSGOUT                       "msgout"                      
 *  String mit der Meldung, die beim Betreten eines Raumes mit M_GO
 *  an die uebrigen Anwesenden ausgegeben wird.

P_MMSGIN                       "mmsgin"                      
 *  String mit der Meldung, die beim Verlassen eines Raumes mit M_TPORT
 *  an die uebrigen Anwesenden ausgegeben wird.

P_MMSGOUT                      "mmsgout"                     
 *  String mit der Meldung, die beim Betreten eines Raumes mit M_TPORT
 *  an die uebrigen Anwesenden ausgegeben wird.


player/potion.h:
----------------
P_POTIONROOMS                  "potionrooms"                 
 *  Alist mit den Nummern der Raeume, in denen der Spieler noch Zauber-
 *  traenke finden kann.

P_KNOWN_POTIONROOMS            "known_potionrooms"           
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_HEALPOTIONS                  "healpotions"                 
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_TRANK_FINDEN                 "trank_finden"                
 *  Wenn die Property auf 1 steht kann immer ein Zaubertrank gefunden
 *  werden, auch wenn er nicht in der Liste des Spielers steht.

P_VISITED_POTIONROOMS          "visited_potionrooms"         
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_BONUS_POTIONS                "bonus_potions"               
 *** KEINE BESCHREIBUNG VORHANDEN ***


player/quest.h:
---------------
P_QUESTS                       "quests"                      
 *  Liste der geloesten Quests.

P_QP                           "questpoints"                 
 *  Anzahl der Questpunkte, die ein Spieler hat.


player/skills.h:
----------------
P_SKILLS                       "skills"                      
 *  *** OBSOLET! ***
 *  Siehe P_NEWSKILLS.


player/viewcmd.h:
-----------------
P_BLIND                        "blind"                       
 *  TRUE, wenn der Spieler nichts sehen kann.

P_BRIEF                        "brief"                       
 *  Ist gesetzt, wenn der Spieler nur die Kurzbeschreibung sehen will.


properties.h:
-------------
P_ORIG_NAME                    "original_name"               
 *  In einer Leiche der Name des Gestorbenen. (name(RAW))

P_KILLER                       "killer"                      
 *  ???

P_MURDER_MSG                   "murder_msg"                  
 *  Welche Meldung soll auf dem Moerder-Kanal erscheinen?

P_FORCE_MURDER_MSG             "force_murder_msg"            
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_KILL_MSG                     "kill_msg"                    
 *  Meldung auf dem Todeskanal, wenn jemand von uns geotetet wird

P_KILL_NAME                    "kill_name"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_CORPSE                       "corpse"                      
 *  Welchen Corpse hinterlassen wir?

P_ENEMY_DEATH_SEQUENCE         "enemy_death_sequence"        
 *  Wenn man jemanden toetet, wird dieses in seine Todessequenz eingefuegt.

P_LIGHT                        "light"                       
 *  Greift auf den Lichtlevel zu. Handle with care !!!

P_TOTAL_LIGHT                  "total_light"                 
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_CLONER                       "cloner"                      
 *  Enthaelt einen String mit dem Namen desjenigen, der das Objekt gecloned hat.

P_LAST_CONTENT_CHANGE          "last_content_change"         
 *  Wann wurde zum letzten Mal was ins Obj gestopft oder rausgenommen?
 *  Wichtig fuer den Weight-Cache

P_VALUE                        "value"                       
 *  Wert des Objektes in Goldmuenzen. Diesen Wert erhaelt man beim
 *  Verkauf. Kaufen kostet ein Vielfaches hiervon.

P_INFO                         "info"                        
 *  Geheime Information, die ggf. ueber einen Zauberspruch
 *  von Spielern ermittelt werden kann.

P_READ_MSG                     "read_msg"                    
 *  Hier koennen Informationen gespeichert werden, die beim Lesen
 *  des Objektes ausgegeben werden.

P_FW_ALWAYS_READABLE           "fw_always_readable"          
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_NOBUY                        "nobuy"                       
 *  Wenn diese Property gesetzt ist, wird das Objekt nach einem
 *  Verkauf im Laden zerstoert, damit es nicht wieder von einem Spieler
 *  gekauft werden kann.

P_NEVERDROP                    "neverdrop"                   
 *  Objekte mit dieser Property werden beim Tod des Spielers nicht
 *  in den Leichnam gelegt.
 *  P_NODROP wird automatisch mitgesetzt.

P_MAGIC                        "magic"                       
 *  Dieses Objekt ist magisch.

P_WEIGHT_PERCENT               "weight_percent"              
 *  Diese Property gibt an, wieviel Prozent des Gewichts des Inhaltes
 *  "nach aussen" wiedergegeben werden.

P_DETAILS                      "details"                     
 *  Diese Property enthaelt ein mapping, in der Objekte im Raum
 *  definiert werden und Beschreibungen, die ausgegeben werden,
 *  wenn man sich diese Details anschaut.

P_SPECIAL_DETAILS              "special_details"             
 *  Mapping von Details, die beim Anschauen eine Funktion starten.

P_READ_DETAILS                 "read_details"                
 *  Details, die durch Lesen ermittelt werden koennen.

P_SMELLS                       "smell_details"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_SOUNDS                       "sound_details"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_DOORS                        "doors"                       
 *  *** OBSOLET! ***
 *  Siehe P_DOOR_INFOS

P_DOORS2                       "doors2"                      
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_ITEMS                        "items"                       
 *  Definition von Gegenstaenden, die in dem Raum liegen sollen.
 *  Erklaerung in einem Extrafile.

P_NO_TPORT                     "tport"                       
 *  Kann folgende Werte annnehmen (definiert in moving.h):
 *  NO_TPORT_IN	= Man kann nicht in den Raum hinein teleportieren.
 *  NO_TPORT_OUT = Man kann nicht aus dem Raum hinaus teleportieren.
 *  NO_TPORT	= Weder noch.

P_TPORT_COST_IN                "tport_cost_in"               
 *  In einem Raum mit Sehertor: Kostenanteil, um sich in den Raum zu
 *  teleportieren

P_TPORT_COST_OUT               "tport_cost_out"              
 *  In einem Raum mit Sehertor: Kostenanteil, sich aus dem Raum heraus
 *  zu teleportieren

P_INDOORS                      "indoors"                     
 *  Gesetzt, wenn von dem Raum aus der Himmel nicht sichtbar ist.
 *  Dinge wie Wetter oder Mondaufgaenge werden in solchen Raeumen
 *  nicht gezeigt.
 *  Ausserdem

P_NOMAGIC                      "nomagic"                     
 *  Angabe in Prozent, mit welcher Wahrscheinlichkeit in einem
 *  Raum nicht gezaubert werden kann. Bei NPC's zeigt es die
 *  Resistenz gegen Magie an.
 *  (Noch nicht implementiert)

P_ORAKEL                       "orakel"                      
 *  Wenn diese Property gesetzt ist, kann der Wanderer in diesen
 *  Raum hinein.

P_RACE                         "race"                        
 *  String mit der Rasse des Wesens.

P_TOTAL_WC                     "total_wc"                    
 *  Numerischer Wert der Angriffsstaerke des Wesens.

P_ZAP_MSG                      "zap_msg"                     
 *  Dreielementiges Array:
 *  1.) Meldung, die der Magier beim Zappen bekommt.
 *  2.) Meldung, die die Spieler im Raum beim Zappen bekommen.
 *  3.) Meldung, die das Opfer beim Zappen bekommt.
 *  Mit @@wer@@, @@wessen@@, ... kann der Name des Opfers und mit
 *  @@ich@@ der Name des Magiers in die Meldung eingewebt werden.

P_AWAY                         "away"                        
 *  String der ausgegeben wird, wenn man weg ist und eine Mitteilung bekommt.

P_NO_TOPLIST                   "no_toplist"                  
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_WEAPON                       "weapon"                      
 *  Momentan gezueckte Waffe.

P_COMBATCMDS                   "combatcmds"                  
 *  Fuer den Kampf gebrauchbare Befehle spezieller Objekte (damit auch
 *  Monster sie automatisch richtig anwenden koennen)
 *  Der Inhalt von P_COMBATCMDS ist ein Mapping, der Key ist das Kommando,
 *  um den Gegenstand zu benutzen (also z.B. "wirf flammenkugel"), und der
 *  Value ein weiteres Mapping mit Zusatzinfos (definiert in /sys/combat.h).
 *  Folgende Keys sind definiert:
 *  - C_MIN, C_AVG, C_MAX:
 *    minimaler, mittlerer und maximaler Schaden, den das
 *    Objekt macht. Alle Angaben in LEBENSPUNKTEN, d.h. Defend-Einheiten/10.
 *    Bei einem Aufruf wie 'enemy->Defend(200+random(200), ...)' ist dann
 *    C_MIN=20, C_AVG=30, C_MAX=40.
 *  - C_DTYPES:
 *    Array mit dem Schadenstyp oder den Schadenstypen. Beim Eisstab
 *    wuerde der Eintrag dann 'C_DTYPES:({DT_COLD})' lauten.
 *  - C_HEAL:
 *    Sollte das Kampfobjekt ueber die Moeglichkeit verfuegen, den Anwender
 *    irgendwie zu heilen, so wird hier die Heilung in LP/MP eingetragen.
 *    Das funktioniert auch bei Objekten, die nur heilen, also sonst
 *    nichts mit Kampf zu tun haben.
 *    Im Lupinental z.B. gibt es Pfirsiche, die beim Essen 5LP heilen. Da
 *    kann man dann 'SetProp(P_COMBATCMDS, (["iss pfirsich":([C_HEAL:5])]))'
 *    eintragen.
 *  Es sind auch mehrere Kommandos moeglich, z.B. bei Objekten, die sowohl
 *  heilen als auch Kampfwirkung haben.

P_ARMOURS                      "armours"                     
 *  Liste der getragenen Schutzbekleidungen.

P_LAST_USE                     "last_object_use"             
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_NPC                          "is_npc"                      
 *  Gesetzt bei Monstern.

P_WIMPY                        "wimpy"                       
 *  Numerischer Wert. Das Wesen flieht, wenn die Lebenspunkte
 *  unter diesen Wert sinken.

P_WIMPY_DIRECTION              "wimpy_dir"                   
 *  Fluchtrichtung

P_HEAL                         "heal"                        
 *  Numerischer Wert, der beim Verzehr dieses Objektes zu den
 *  Lebenspunkten hinzugezaehlt wird. (kann auch negativ sein)
 *  Der Wert sollte zwischen +4 und -4 liegen und bei leichten
 *  Monstern 0 sein.

P_POISON                       "poison"                      
 *  Wie stark wir vergiftet sind (0-11)

P_MAX_POISON                   "max_poison"                  
 *  Maximale Vergiftung

P_DISABLE_ATTACK               "disable_attack"              
 *  Das Lebewesen kann nicht angreifen.

P_DIE_MSG                      "die_msg"                     
 *  String mit der Meldung, die ausgegeben wird, wenn das Wesen stirbt.
 *  ist die Property nicht gesetzt, so nehme " faellt tot zu Boden.\n".

P_KILLS                        "playerkills"                 
 *  Anzahl der Spieler, die dieser Spieler schon getoetet hat.
 *  Unerlaubte Manipulation ist ein SCHWERES VERGEHEN gegen
 *  die Mudreglen.

P_CALLED_FROM_IP               "called_from_ip"              
 *  Letzte IP-Adr, von der aus sich der Spieler eingeloggt hat.

P_DESCRIPTION                  "description"                 
 *  Beschreibung des Spielers

P_GUILD                        "guild"                       
 *  Gilde, der der Spieler angehoert.
 *  Der Name der Gilde ist hier als String definiert. Bei Spielern
 *  ist der Wert dieser Property niemals 0 (Defaultmaessig gehoert
 *  ein Spieler der Abenteurergilde an).

P_VISIBLE_GUILD                "visible_guild"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_LEVEL                        "level"                       
 *  Spieler-Level (!= Magierlevel)

P_CAP_NAME                     "cap_name"                    
 *  Name des Spielers, der dekliniert und ausgegen wird.
 *  NOT YET IMPLEMENTED.

P_EARMUFFS                     "earmuffs"                    
 *  Shouts von Magiern mit Level < earmuffs werden abgeblockt
 *  (Nur fuer Magier)

P_MARRIED                      "married"                     
 *  Enthaelt einen String mit der uid des Partners
 *  (sofern vorhanden)

P_AMOUNT                       "amount"                      
 *  Anzahl der Objekte, fuer die das Objekt steht.

P_VALUE_PER_UNIT               "value_per_unit"              
 *  Wert in Goldstuecken pro Untereinheit.

P_WEIGHT_PER_UNIT              "weight_per_unit"             
 *  Gewicht in Gramm pro Untereinheit.

P_FUEL                         "fuel"                        
 *  Numerischer Wert fuer die Zeitdauer, die die Lichtquelle noch
 *  leuchten kann.

P_LIGHTDESC                    "lightdesc"                   
 *  String mit der Bezeichnung des Leuchtvorgangs, den die Licht-
 *  quelle ausfuehrt (leuchtend, brennend, gluehend ...)

P_DO_DESTRUCT                  "do_destruct"                 
 *  Flag, ob sich die Lichtquelle am Ende der Leuchtzeit selbst
 *  zerstoert. (Oder sogar mit String fuer die Meldung, die bei
 *  der Zerstoerung angegeben wird?)

P_LIGHTED                      "lighted"                     
 *  Flag, ob die Lichtquelle in Betrieb ist.

P_CHATS                        "chats"                       
 *  Alist mit Strings, die das Monster zufaellig ausgibt.

P_CHAT_CHANCE                  "chat_chance"                 
 *  Wahrscheinlichkeit, mit der die Chats ausgegeben werden.

P_ACHATS                       "achats"                      
 *  Chats, die das Monster im Kampf ausgibt.

P_ACHAT_CHANCE                 "achat_chance"                
 *  Wahrscheinlichkeit fuer die Attack-Chat-Ausgabe.

P_BODY                         "body"                        
 *  Numerischer Wert fuer die Abwehrstaerke des blanken Koerpers
 *  des Wesens.

P_HB                           "hb"                          
 *  Diese Property wird gesetzt, wenn das Monster immer einen
 *  heart_beat haben soll. (VORSICHT, nur wenn es WIRKLICH sein muss!)

P_AGGRESSIVE                   "aggressive"                  
 *  Gesetzt, wenn das Wesen von sich aus Angriffe startet.

P_NOCORPSE                     "nocorpse"                    
 *  Gesetzt, wenn im Todesfall kein Leichnam erzeugt werden soll.

P_REJECT                       "reject"                      
 *  Ein Array aus 2 Elementen, das bestimmt, das der npc mit Dingen
 *  tuen soll, die ihm gegeben werden.
 *  Default, der npc behaelt die Dinge einfach.
 *  Wenn gesetzt:
 *  1.Element: Art der Handlung. (aus <moving.h>)
 * 	REJECT_GIVE: Der NPC gibt das Objekt zurueck.
 * 	REJECT_DROP: Der NPC laesst das Objekt fallen.
 * 	REJECT_KEEP: Der NPC behaelt das Objekt doch.
 *  2.Arrayelement: Meldung, mit der der NPC die Handlung kommentiert.

P_RACE_DESCRIPTION             "racedescr"                   
 *  Beschreibung der Vor/Nachteile einer Rasse.

P_RACESTRING                   "racestring"                  
 *  Gibt eine dem Geschlecht angepasste Beschreibung der Rasse zurueck
 *  ("Zwerg" oder "Zwergin" etc.)

P_CONTAINER                    "container"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_FW_UNDERSTAND                "fw_understand"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_TRAY                         "tray"                        
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_DEFAULT_INFO                 "default_info"                
 *  NPC-Info-System: Default-Antwort auf dumme Fragen

P_LOG_INFO                     "log_info"                    
 *  Wenn diese Property gesetzt ist wird jede Frage, die ein
 *  Monster nicht beantworten kann, im Report-File des
 *  zustaendigen Magiers geloggt.

P_LOG_FILE                     "log_file"                    
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_PURSUERS                     "pursuers"                    
 *  Enthaelt Verfolger - nicht von Hand manipulieren!

P_HP_HOOKS                     "hp_hooks"                    
 *  Welche Objekte sollen bei HP-Aenderungen benachrichtigt werden?

P_CURSED                       "cursed"                      
 *  Verfluchte Waffen/Ruestungen kann man nicht ausziehen/wegstecken

P_KEEP_ON_SELL                 "keep_on_sell"                
 *  Bei "verkaufe alles" wird das Objekt behalten.

P_SPELLS                       "spells"                      
 *  NPC-Spells

P_SPELLRATE                    "spellrate"                   
 *  NPC-Spellrate (in %)

P_INFORMME                     "informme"                    
 *  Informieren ueber Spieler, die ins Mud kommen/aus dem Mud gehen?

P_WAITFOR                      "waitfor"                     
 *  Die Erwarte-Liste

P_WAITFOR_REASON               "waitfor_reason"              
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_LOCALCMDS                    "localcmds"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_CLOCKMSG                     "clockmsg"                    
 *  Die Meldung wird zur vollen Stunde ausgegeben

P_PARA                         "para"                        
 *  Nummer der Parallelwelt in der ein Spieler ist.

P_SIZE                         "size"                        
 *  Groesse des Lebewesens (in cm)

P_NO_STD_DRINK                 "no_std_drink"                
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_KEEPER                       "shop_keeper"                 
 *** KEINE BESCHREIBUNG VORHANDEN ***


room/description.h:
-------------------
P_INT_SHORT                    "int_short"                   
 *  Kurzbeschreibung, wenn man sich im Inneren des Containers
 *  befindet.

P_INT_LONG                     "int_long"                    
 *  Beschreibung, die man bekommt, wenn man sich in dem Container
 *  umschaut.

P_ROOM_MSG                     "room_msg"                    
 *  Liste mit Meldungen, die zufaellig im Raum ausgegeben werden.

P_FUNC_MSG                     "func_msg"                    
 *  Liste mit Funktionen, die zufaellig im Raum aufgerufen werden.

P_MSG_PROB                     "msg_prob"                    
 *  Numerischer Wert fuer Wahrscheinlichkeit, mit der die Meldungen
 *  und/oder die Funktionen ausgegeben/aufgerufen werden.

P_HAUS_ERLAUBT                 "haus_erlaubt"                
 *  Hier darf gebaut werden


room/exits.h:
-------------
P_EXITS                        "exits"                       
 *  Mapping aller unmittelbar sichtbaren Ausgaenge mit zugehoerigen
 *  Nachbarraeumen. Sollte nur mittels AddExit() benutzt werden.

P_SPECIAL_EXITS                "special_exits"               
 *  Dito, aber anstatt des Nachbarraums wird eine Funktion (im Raum)
 *  angegebem, die bei Eingabe der Richtung ausgefuehrt wird.

P_HIDE_EXITS                   "hide_exits"                  
 *  Ist diese Property in einem Raum auf einen Wert ungleich 0, gesetzt,
 *  werden einem Spieler fuer diesen Raum keine Ausgaenge angezeigt.


sensitive.h:
------------
P_SENSITIVE_INVENTORY          "sensitive_inv"               
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_SENSITIVE_INVENTORY_TRIGGER  "sensitive_inv_trigger"       
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_SENSITIVE_ATTACK             "sensitive_attack"            
 *** KEINE BESCHREIBUNG VORHANDEN ***


shells.h:
---------
P_CURRENTDIR                   "currentdir"                  
 *  Momentanes Verzeichnis in dem der Spieler ist. (nur fuer
 *  Magier von Belang)

P_SHORT_CWD                    "short_cwd"                   
 *  .readme bei cd ausgeben oder nicht


snooping.h:
-----------
P_SNOOPFLAGS                   "snoopflags"                  
 *** KEINE BESCHREIBUNG VORHANDEN ***


thing/commands.h:
-----------------
P_COMMANDS                     "commands"                    
 *  Mapping von Kommandos, die im Objekt definiert sind.
 *  Sollte nur mittels AddCmd() benutzt werden.


thing/description.h:
--------------------
P_NAME                         "name"                        
 *  In dieser Property wird der Name des Objektes gespeichert. Er
 *  sollte nur aus einem Wort bestehen. Der Name dient dazu, das
 *  Objekt in einem Satz zu erwaehnen und wird auch dekliniert.

P_NAME_ADJ                     "name_adj"                    
 *  In dieser Property kann ein Adjektiv abgelegt werden, dass dann
 *  bei der Ausgabe mittels name() mitbenutzt wird.
 *  (In der Regel nicht noetig.)

P_SHORT                        "short"                       
 *  In dieser Property wird die Kurzbeschreibung des Objektes
 *  gespeichert, die beim Umschauen im Raum oder bei der Ausgabe
 *  des Inventars ausgegeben wird. Fuer unsichtbare Objekte
 *  sollte sie 0 sein.

P_LONG                         "long"                        
 *  Unter dieser Property wird die Beschreibung gespeichert, die
 *  bei der Untersuchung des Objektes ausgegeben wird.

P_IDS                          "ids"                         
 *  Hier werden die Bezeichnungen abgespeichert, unter denen sich das
 *  Objekt angesprochen fuehlt.
 *  Sollte nur mittels AddId( id ); gesetzt werden.

P_ADJECTIVES                   "adjectives"                  
 *  Hier werden Adjektive gespeichert, unter denen sich das Objekt
 *  angesprochen fuehlt. So sind Kombinationen der Synonyme mit
 *  mehreren Adjektiven moeglich. Ggf sollte auch der deklinierte
 *  Fall des Adjektives eingegeben werden.
 *  Sollte nur mittels AddAdjective( adjective ); gesetzt werden.

P_SHOW_INV                     "show_inv"                    
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_CLASS                        "class"                       
 *  Enthaelt die Klasse (definiert in class.h), zu der ein Objekt gehoert.
 *  Sollte nur mit AddClass() gesetzt werden.
 *  Das setzen ist nur noetig, wenn die Klasse nicht schon aus den Ids
 *  oder (bei Lebewesen) der Rasse ersichtlich ist.


thing/language.h:
-----------------
P_ARTICLE                      "article"                     
 *  Gibt an, ob in der Beschreibung ein Artikel ausgegeben werden soll
 *  oder nicht.

P_GENDER                       "gender"                      
 *  Grammatikalisches Geschlecht des Objektes:
 *  (Definiert in language.h) MALE, FEMALE oder NEUTER


thing/material.h:
-----------------
P_MATERIAL                     "material"                    
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_MATERIAL_KNOWLEDGE           "material_knowledge"          
 *** KEINE BESCHREIBUNG VORHANDEN ***


thing/moving.h:
---------------
P_NODROP                       "nodrop"                      
 *  Diese Property enthaelt eine Meldung, die ausgegeben wird, wenn
 *  jemand versucht, das Objekt fallen zu lassen. Wird die Prop.  auf einen
 *  nicht-String-Wert gesetzt, so wird eine Defaultmeldung ausgegeben.

P_NOGET                        "noget"                       
 *  Diese Property enthaelt eine Meldung, die ausgegeben wird, wenn
 *  jemand versucht, dieses Objekt zu nehmen. Wird die Prop. auf einen
 *  nicht-String-Wert gesetzt, so wird eine Defaultmeldung ausgegeben.

P_SENSITIVE                    "sensitive"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_LAST_MOVE                    "last_move"                   
 *** KEINE BESCHREIBUNG VORHANDEN ***


thing/properties.h:
-------------------
P_UID                          "uid"                         
 *  Simulation des Zugriffs auf die uid.

P_EUID                         "euid"                        
 *  Simulation des Zugriffs auf die euid.


thing/restrictions.h:
---------------------
P_WEIGHT                       "weight"                      
 *  Das Gewicht eines Objetes in Gramm.

P_TOTAL_WEIGHT                 "total_weight"                
 *  Gewicht incl. Inhalt in Gramm. P_WEIGHT_PERCENT wird beruecksichtigt.


transport.h:
------------
P_ENTERMSG                     "entermsg"                    
 *  Array mit zwei Meldungen, eine fuer den Raum, den der Spieler
 *  verlaesst, und eine fuer den Transporter, in den er geht.

P_LEAVEMSG                     "leavemsg"                    
 *  Array mit zwei Meldungen, eine fuer den Transporter, den er verlaesst,
 *  und eine fuer den Raum in den er kommt.

P_LEAVEFAIL                    "leavefail"                   
 *  Meldung an ein Wesen, wenn es ausserhalb der Anlegezeiten den Transporter
 *  verlassen will. Ist die Prop. ein Array, so wird das erste Element als
 *  meldung an das Wesen, das zweite als Meldung an die Mitspieler im
 *  Transporter geschickt.

P_ENTERFAIL                    "enterfail"                   
 *  Meldung an ein Wesen, wenn den vollen Transporter betreten will.
 *  Ist die Prop. ein Array, so wird das erste Element als
 *  meldung an das Wesen, das zweite als Meldung an die Mitspieler im
 *  Raum geschickt.

P_ARRIVEMSG                    "arrivemsg"                   
 *  Meldung, wenn der Transporter anlegt.

P_DEPARTMSG                    "departmsg"                   
 *  Meldung, mit der ein Transporter ablegt.

P_ENTERCMDS                    "entercmds"                   
 *  Befehlsliste, die zum Betreten des Transporters fuehrt.

P_LEAVECMDS                    "leavecmds"                   
 *  Befehlsliste, die zum Verlassen des Transporters fuehrt.

P_MAX_PASSENGERS               "maxpass"                     
 *  Numerischer Wert fuer die maximale Anzahl von Wesen in dem Transporter.
 *  0 bedeutet unbeschaenkte Spielerzahl.


v_compiler.h:
-------------
P_STD_OBJECT                   "std_object"                  
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_COMPILER_PATH                "compiler_path"               
 *** KEINE BESCHREIBUNG VORHANDEN ***


weapon.h:
---------
P_NR_HANDS                     "nr_hands"                    
 *  Anzahl der Haende, die man zur Benuztung des Objektes benoetigt.

P_WC                           "wc"                          
 *  Numerischer Wert fuer die Staerke der Waffe.

P_MAX_WC                       "max_wc"                      
 *** KEINE BESCHREIBUNG VORHANDEN ***

P_WEAPON_TYPE                  "weapon_type"                 
 *  Art der Waffe

P_DAM_TYPE                     "dam_type"                    
 *  String mit der Art der Verletzung.

P_WIELDED                      "wielded"                     
 *  Flag ob die Waffe gezueckt ist.

P_HIT_FUNC                     "hit_func"                    
 *  Enthaelt das Objekt, das eine HitFunc() definiert
 *  (fuer Waffen)

P_WIELD_FUNC                   "wield_func"                  
 *  Enthaelt das Objekt, das eine WieldFunc() definiert
 *  (fuer Waffen)

P_UNWIELD_FUNC                 "unwield_func"                
 *  Enthaelt das Objekt, das eine UnwieldFunc() definiert
 *  (fuer Waffen)


weapon/description.h:
---------------------
P_DAM_DESC                     "dam_desc"                    
 *  Beschreibung beschaedigter Waffen/Ruestungen.

