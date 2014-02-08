#include <unistd.h>

#define SIZE (100000)
#define SYS_INC_CP_RANGE (286)
#define inc_cp_range(x,y) (syscall(SYS_INC_CP_RANGE,x,y))

int main()
{
    int *i;
    int *data = (int *) malloc(SIZE*sizeof(int));
    memset(data, 0x00, SIZE);

    unsigned long end = 0xffffffff;
    //inc_cp_range(0, end);

    inc_cp_range(data, data+SIZE-1);
    /*printf("data: %d\n", *data);
    printf("data+SIZE: %d\n", *(data+SIZE));

    //memset((int*)data+2, 1, 1);   

    printf("data: %d\n", *(data+2));*/
    
    for(i = data; i < data+SIZE; i+=40000){ 
       memset(i, 1, 1);   
       printf("val: %d\n", *i);
       //inc_cp_range(data, end);
       inc_cp_range(data, data+SIZE-1);
    }
}
