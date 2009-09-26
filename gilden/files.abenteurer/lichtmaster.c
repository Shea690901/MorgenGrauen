#define DEBUGGER "boing"

mixed* kugel_array;

create()
{
   kugel_array = ({ });
}


add_kugel(kugel, zeit)
{
   kugel_array += ({ ({ kugel, time() + zeit }) });
   kugel_array = sort_array(kugel_array, "sortieren", this_object());
   update_call_out();
}


remove_kugel()
{
   object kugel;

   kugel = kugel_array[0][0];
   if (kugel)
   {
      kugel->remove();
   }
   kugel_array = kugel_array[1..];

   update_call_out();
}


sortieren(a, b)
{
   if (a[1] > b[1])
      return 1;
   else
      return 0;
}


update_call_out()
{
   int diff;
   while (remove_call_out("remove_kugel") != -1)
      ; 

   if (sizeof(kugel_array) == 0)
      return;

   diff = kugel_array[0][1] - time();
   if (diff < 0)
      diff = 0;

   call_out("remove_kugel", diff);
}


debug()
{
   int i;
   int t;
   tell_object(find_player(DEBUGGER), sprintf("%O\n", kugel_array));
   tell_object(find_player(DEBUGGER), sprintf("naechster callout: %O\n", 
					     find_call_out("remove_kugel")));

   t = time();
   for (i=0; i<sizeof(kugel_array); i++)
   {
      tell_object(find_player(DEBUGGER), sprintf("Element %d: %d\n", i, 
						kugel_array[i][1] - t));
   }
}
