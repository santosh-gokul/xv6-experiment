#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char buf[512];

void
cat(int fd)
{
  int n;

  while((n = read(fd, buf, sizeof(buf))) > 0) {
    if (write(1, buf, n) != n) {
      fprintf(2, "cat: write error\n");
      exit(1);
    }
  }
  if(n < 0){
    fprintf(2, "cat: read error\n");
    exit(1);
  }
}

int
main()
{
  int fd, i;

  char *filename = "README";
  if((fd = open(filename, 0)) < 0){
    fprintf(2, "cat: cannot open %s\n", filename);
    exit(1);
  }

  printf("Introducing some delay..\n");
  for(int timer = 0; timer<10000000; timer++){
  }
  cat(fd);
  close(fd);
  exit(0);
}
