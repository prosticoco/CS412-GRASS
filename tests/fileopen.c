#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


int main(){
    FILE * fileptr = fopen("direct/ory/test2.txt","r");
    if(fileptr == NULL){
        printf("Filepointer is null \n");
    }
    int error = access("direct/ory/test2.txt",F_OK);
    if(error){
        printf("error is %d \n",error);
    }

    error = access("direct/ory/tst2.txt",F_OK);
    if(error){
        printf("nice this did not work as expected\n");
    }
    return 0;
}