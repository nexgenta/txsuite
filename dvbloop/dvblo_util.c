#include <linux/errno.h>

#include "dvblo_util.h"

static short ns_h2i(char c)
{
   if (c >= '0' && c <= '9')
      return (short) (c - '0');
   if (c >= 'A' && c <= 'F')
      return (short) (c - 'A' + 10);
   if (c >= 'a' && c <= 'f')
      return (short) (c - 'a' + 10);
   return -1;
}

int dvblo_parse_mac(const char *macstr, u8 *mac_out)
{
   int i, j;
   short byte1, byte0;

   if(macstr == NULL || macstr == NULL)
      return -EINVAL;
   j = 0;
   for (i = 0; i < 6; i++)
   {
      if((byte1 = ns_h2i(macstr[j++])) < 0)
         return -EINVAL;
      if((byte0 = ns_h2i(macstr[j++])) < 0)
         return -EINVAL;
      mac_out[i] = (unsigned char) (byte1 * 16 + byte0);
      if (i < 5)
      {
        if(macstr[j] == ':' || macstr[j] == '-')
		j++;
	else
            return -EINVAL;
      }
   }
   return 0;
}




