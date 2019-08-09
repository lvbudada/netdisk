#include <func.h>

int main(int argc, char* argv[])
{
    FILE *fp = fopen(argv[1], "rb");
    char buf[128] = {0};
    while(fgets(buf, 128, fp)!=NULL)
    {
        printf("%s", buf);
    }
    return 0;
}

