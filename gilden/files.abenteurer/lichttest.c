#define LICHTKUGEL "/gilden/files.abenteurer/lichtkugel"
#define LICHTMASTER "/gilden/files.abenteurer/lichtmaster"

create()
{
   object l;
   int i;

   for (i = 0; i < 10; i++)
   {
      l = clone_object(LICHTKUGEL);
      l->move("/room/void");
      LICHTMASTER->add_kugel(l, 2 + random(20));
   }
   LICHTMASTER->debug();
}
