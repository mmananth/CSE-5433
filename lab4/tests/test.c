#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define SIZE (10)
#define SYS_CP_RANGE (285)
#define cp_range(x,y) (syscall(SYS_CP_RANGE, x, y))

int main() {
  int *data = (int*) malloc(SIZE*sizeof(int));
  memset(data, 0x00, SIZE);
  
  printf("Calling on %lu,%lu\n", data, data+SIZE-1);
  cp_range((unsigned long) data, (unsigned long) data+SIZE-1);

  memset(data, 0xff, SIZE);
  cp_range((unsigned long) data, (unsigned long) data+SIZE-1);

  return 0;
}
