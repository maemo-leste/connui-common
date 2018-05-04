#include <glib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "connui-log.h"

gchar *
gateway_pin_random_digit_string(int digits)
{
  int fd;
  char buf[17];


  if (!digits)
    return NULL;

  fd = open("/dev/urandom", O_RDONLY);

  if (fd == -1)
  {
    CONNUI_ERR("could not open '/dev/urandom'");
    return NULL;
  }

  if (digits > 16)
    digits = 16;

  if (digits == read(fd, buf, digits))
  {
    int i = 0;
    close(fd);

    while (i < digits)
    {
      buf[i] = (((unsigned char)buf[i]) % 10) + '0';
      i++;
    }

    buf[i] = 0;
    return g_strdup(buf);
  }

  CONNUI_ERR("could not read '%d' digits", digits);

  close(fd);
  return NULL;
}
