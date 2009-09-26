// party.h
//
// Include-File fuer Partybretter und -master
//

// $Id: party.h 7160 2009-02-25 22:49:00Z Zesstra $

#ifndef __PARTY_BRETT_H__
#define __PARTY_BRETT_H__

#define PARTYDIR  "/p/service/rochus/party/"

#define SAVEFILE   (PARTYDIR+"daten")
#define PM         (PARTYDIR+"master")
#define PARTYBRETT (PARTYDIR+"brett")

// nur fuer Files, die die Liste brauchen.
#ifdef __PARTY_NEED_ROOMS__
#define BOARD_ROOMS ({\
    "/gilden/abenteurer",\
    "/gilden/bierschuettler",\
    "/gilden/dunkelelfen",\
    "/gilden/zauberer",\
    "/gilden/chaos",\
    "/gilden/karate",\
    "/gilden/katzenkrieger",\
    "/gilden/tanjian",\
    "/d/gebirge/wurzel/room/festung/kapelle",\
    "/d/dschungel/paracelsus/room/fkapelle",\
    "/d/gebirge/room/zkapelle",\
    "/d/wald/room/es_mitte",\
    "/d/wald/gundur/hobbitdorf/schrein",\
    "/d/wueste/durian/room/lash/brunnen",\
    "/d/unterwelt/wurzel/dorf/post",\
                     })
#endif

#endif
