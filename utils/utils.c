#include <sys/time.h>
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
pthread_mutex_t mutex_debug = PTHREAD_MUTEX_INITIALIZER;

long int pma_start_time = 0;
char* PRINTTIME(struct timeval timeinfo){
    char* timestr = (char*)malloc(100);
    memset(timestr,0x00,100);
    struct tm timefo;
    localtime_r(&(timeinfo.tv_sec),&timefo);
    sprintf(timestr,"%4d-%02d-%02d %02d:%02d:%02d",\
     timefo.tm_year+1900,\
     timefo.tm_mon+1,\
     timefo.tm_mday,\
     timefo.tm_hour,\
     timefo.tm_min,\
     timefo.tm_sec);
    printf("%s",timestr);
    return timestr;
}
