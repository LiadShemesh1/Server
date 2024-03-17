#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define MYSIZE 100

void * memmove(void* dest, const void* src, size_t n);
int MSB_L_2_M(__uint64_t data);
int MSB_M_2_L(__uint64_t data);

int main () 
{
    char *filename;
    time_t t = time(NULL);
    struct tm date;
    if(gmtime_r(&t, &date) == NULL) {t = 0;} //making sure we have some time to write.
    localtime_r(&t, &date);

    char currTime[21]; // maximum length of time in the selected format.
    strftime(currTime, sizeof(currTime), "%d-%m-%Y %T ", &date);
    //strftime(currTime, sizeof(currTime), "%d-%m-%Y--%T", &date);

    int a = 155364;
    int* b = (int*)malloc(sizeof(int));

    b = (int*)memmove((void*)b, (const void*)&a, sizeof(int));

    printf("asas\n");
    printf("asas");

    __uint64_t test = (1UL<<62);
    printf("the position is: %d\n\n",MSB_M_2_L(test)); 



}
int MSB_L_2_M(__uint64_t data)
//will return the MSB checking from Less to Most sagnificent bit 
{
    if(data == 0) return 0;

    int msb_pos = -1;
    while(data > 0)
    {
        data >>= 1;
        msb_pos += 1;
    }
    return msb_pos;
}

int MSB_M_2_L(__uint64_t data)
//will return the MSB checking from Less to Most sagnificent bit 
{
    if(data == 0) return 0;

    int msb_pos = 63;
    while(msb_pos > 0)
    {
        if(data & (1UL<<msb_pos)) return msb_pos;
        --msb_pos;
    }
    return msb_pos; // = 0

}



void * memmove(void* dest, const void* src, size_t n){
    char* c_dest = (char *)dest;
    const char* c_src = (const char *)src;

    char* buff = (char*)malloc(sizeof(char) * n);

    if(buff == NULL)
    {
        return NULL;
    }
    else
    {
        size_t i = 0;
        for(i; i < n; ++i)
            buff[i] = c_src[i];
        
        for (i=0; i < n; ++i)
            c_dest[i] = buff[i];
        
        free(buff);
    }
    return dest;

}