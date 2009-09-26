#include <defines.h>
#ifndef PO
#define PO previous_object()
#endif

#define SECURE(x) "/gilden/files.dunkelelfen/secure/"+x
#define KRAEUTERVCSAVEFILE SECURE("kraeuterVC")
#define PLANTMASTER        SECURE("plantmaster")
#define PLANTDIR           "/p/service/padreic/kraeuter/"
#define STDPLANT           PLANTDIR+"plant"
#define KRAEUTERVC         PLANTDIR+"virtual_compiler"
#define PLANT_ROOMDETAIL   "Padreic:Plant:RoomDetail"

#define INGREDIENT_ID           0
#define INGREDIENT_DEMON        1
#define INGREDIENT_GENDER       2
#define INGREDIENT_NAME         3
#define INGREDIENT_ADJ          4
#define INGREDIENT_LONG         5
#define INGREDIENT_ROOMDETAIL   6

#define is_plant(x) (member(inherit_list(x), "/p/service/padreic/kraeuter/plant.c")>=0)
