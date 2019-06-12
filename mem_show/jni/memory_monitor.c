/* 
 * This program show the infomation in /proc/meminfo 
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void examine()
{
    char s[256];
    freopen("/proc/meminfo", "r", stdin);
    fgets(s, 256, stdin);
    fgets(s, 256, stdin);
    fgets(s, 256, stdin);
    fgets(s, 256, stdin);
    fgets(s, 256, stdin);
    fgets(s, 256, stdin);
    printf("%s", s);
    fgets(s, 256, stdin);
    printf("%s", s);
}

int main(int argc, char *argv[])
{
    int i;
    for (i = 0; i <= 20; ++i)
    {
        printf("%d meminfo show\n", i);
        examine();
        sleep(1);
    }
    return 0;
}
