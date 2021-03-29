#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    printf("This message should go to stdout\n");
    fflush(stdout);
    
    if(argc > 1) {
        sleep(5);
    }
    
    fprintf(stderr, "This message should go to stderr\n");
    fflush(stderr);

    return -1;
}