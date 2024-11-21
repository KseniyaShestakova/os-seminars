
#include <signal.h>
#include <stdio.h>

void f(int) {
    printf("SIGTERM received\n");
}
 
int main(void)
{
    /* ignoring the signal */
    signal(SIGTERM, &f);
    raise(SIGTERM);
    printf("Exit main()\n");
}
