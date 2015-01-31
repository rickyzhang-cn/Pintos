#include <syscall.h>

int main (int, char *[]);
void _start (int argc, char *argv[]);

void
_start (int argc, char *argv[]) 
{
  //printf("will call _start()\n");
  exit (main (argc, argv));
}
