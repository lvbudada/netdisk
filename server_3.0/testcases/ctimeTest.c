#include <func.h>

int main()
{
    time_t now = time(NULL);
    printf("%s\n", ctime(&now));
    return 0;
}

