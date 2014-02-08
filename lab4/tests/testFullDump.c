#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE (10)
#define INC_SYS_CP_RANGE (286)
#define inc_cp_range(x,y) (syscall(INC_SYS_CP_RANGE, x, y))

int main() {
  printf("Doing full dump of system memory....\n");
  unsigned long end = 0xffffffff;
  inc_cp_range(0, end);
}
