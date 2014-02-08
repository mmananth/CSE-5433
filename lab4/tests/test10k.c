#include <stdlib.h>
#include <unistd.h>

#define SIZE (10000)
#define SYS_CP_RANGE (285)
#define cp_range(x,y) (syscall(SYS_CP_RANGE, x, y))

int main() {
  int *data = (int*) malloc(SIZE*sizeof(int));
  memset(data, 0x00, SIZE);
  
  cp_range(data, data+SIZE-1);

  memset(data, 0xaa, SIZE);
  cp_range(data, data+SIZE-1);

  return 0;
}
