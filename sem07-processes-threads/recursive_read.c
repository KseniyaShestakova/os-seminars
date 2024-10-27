
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
 
void* start_routine(void* arg)
{
     int32_t curr;
     if (scanf("%d", &curr) == EOF) {
         return NULL;
     }
     pthread_t next;
     pthread_create(&next, NULL, &start_routine, NULL);
     pthread_join(next, NULL);
     printf("%d ", curr);
     return NULL;
}

int main()
{
     pthread_t init;
     pthread_create(&init, NULL, &start_routine, NULL);
     pthread_join(init, NULL);
     return 0;
}
