// brett.h <2005-01-11>
//
// Include-File fuer das Partybrett
//
// Zook

// $Id: brett.h 7216 2009-05-26 20:17:41Z Zesstra $ 

#ifndef PARTYDIR
#include "party.h"
#endif

#define ORGANISATOR "srabi"

#define INFOFILE       (PARTYDIR+"info.txt")
#define MOREINFOFILE   (PARTYDIR+"moreinfo.txt")
#define BESTAETIGFILE  (PARTYDIR+"bestaetigung.txt")


#define Q_KEY 1
#define Q_TEXT 2
#define Q_CHOICES 3
#define Q_DEFAULT 4
#define Q_BOOLEAN 5

#define KEY_QUERY_NUMBER "query_number"
#define KEY_BESTAETIGUNG "bestaetigt"
#define KEY_DATUM "datum"


mapping *Fragen() {
          return ({
    ([Q_KEY: "ort",
      Q_TEXT:"Abfahrtsort:"]),

    ([Q_KEY:"fahrt",
      Q_TEXT:"Wie kommst Du her (Bahn, Auto, ...)?",
      Q_DEFAULT:"auto"]),

    ([Q_KEY:"mfg",
      Q_TEXT:"Kannst Du noch Leute mitnehmen? (Falls ja, wieviele?)",
      Q_DEFAULT:"nein"]),

    ([Q_KEY:"ankunft",
      Q_TEXT:"Ankunft",
      Q_CHOICES:(["20":"Montag",
                  "21":"Dienstag",
                  "22":"Mittwoch",
                  "23":"Donnerstag",
                  "24":"Freitag",
                  "25":"Samstag", ]),
      Q_DEFAULT:"20"]),

/*    ([Q_KEY:"verpflegung",
      Q_TEXT:"Verpflegung (momentan wird HP fuer alle erwogen).",
      Q_CHOICES:(["hp":"Halbpension",
                  "vp":"Vollpension", ]),
      Q_DEFAULT:"hp"]),
*/
    ([Q_KEY:"uebernachtung",
      Q_TEXT:"Uebernachtungswunsch (Zelt, Ferienhaus, Hotel, selbstorganisiert)",
      Q_CHOICES:({"zelt","ferienhaus","hotel", "selbstorganisiert"}),
      Q_DEFAULT:"zelt"]),

     ([Q_KEY:"vegetarier",
      Q_TEXT:"Vegetarier:",
      Q_DEFAULT:"nein",
      Q_BOOLEAN:1]),

    ([Q_KEY:"seminar",
      Q_TEXT:"Falls ein Seminar oder Vortrag stattfindet, moechtest du "
             "teilnehmen?",
      Q_DEFAULT:"ja",
      Q_BOOLEAN:1]),

//        ([Q_KEY:"sonntagessen",
//        Q_TEXT:"Moechtest Du am Sonntag zu einem Abschieds-Mittagessen?",
//        Q_DEFAULT:"ja",
//        Q_BOOLEAN:1]),

//      ([Q_KEY:"tshirt",
//      Q_TEXT:"Moechtest Du ein T-Shirt (falls es eins gibt)?
// (nein,S,M,L,XL,XXL,XXXL)",
//      Q_CHOICES:({"nein","s","m","l","xl","xxl","xxxl"}),
//      Q_DEFAULT:"nein"]),


    ([Q_KEY:"familie",
      Q_TEXT:"Wenn du mit Kind und Kegel kommst, hier bitte Familiennamen "
             "als ID + Anzahl der Kinder + evtl. Partner (falls nicht "
             "selber angemeldet) notieren.",
      Q_DEFAULT:"keine"]),

    ([Q_KEY:"bemerkungen",
      Q_TEXT:"Bemerkungen:",
      Q_DEFAULT:"keine"]),
      });
}
