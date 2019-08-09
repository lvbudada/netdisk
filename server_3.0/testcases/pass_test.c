#include <func.h>

void get_salt(char *salt, char *passwd)
{
    int i, j;

    //取出salt，i记录密码字符下标，j记录$出现的次数
    for(i=0,j=0; passwd[i]&&j!=3; ++i)
    {
        if(passwd[i] == '$')
        {
            j++;
        }
    }
    strncpy(salt, passwd, i-1);
}

int main(int argc, char *argv[])
{
    //argv[1] 是密码
    ARGS_CHECK(argc, 2);
    struct spwd *sp;
    char *passwd;
    char salt[512] = {0};

    //输入密码
    passwd = getpass("请输入密码：");
    //判断用户是否存在
    if((sp = getspnam(argv[1])) == NULL)
    {
        printf("用户名或密码错误\n");
        ERROR_CHECK(sp, NULL, "getspnam");
        return -1;
    }
    //ERROR_CHECK(sp, NULL, "getspnam");

    //从sp得到salt
    get_salt(salt, sp->sp_pwdp);

    //密码验证
    if(!strcmp(sp->sp_pwdp, crypt(passwd, salt)))
    {
        printf("用户名和密码正确\n");
    }else{
        printf("用户名或者密码错误\n");
    }

    return 0;
}

