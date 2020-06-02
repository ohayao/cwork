#include "ign.h"


void f(void * p) {
    for(int i=0;i<20000;i++) {
        GetMsgID();
    }
    printf("msgid=[%u]\n", GetMsgID());
}


int main() {

    thread_type thr[10];
    for(int n = 0; n < 10; ++n)
        pthread_create(&thr[n], NULL, f, NULL);
    for(int n = 0; n < 10; ++n)
        pthread_join(thr[n], NULL);

    printf("end, msgid=[%u]\n", GetMsgID());
}


