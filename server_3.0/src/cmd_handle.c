#include "factory.h"

int cmdHandle(int newFd)
{
    //连接数据库，创建一个数据库连接
    MYSQL *conn;
    mysqlConnect(&conn); 

    int ret;
    int dataLen; 
    int commond_code;
    char buf[1000] = {0};  //dataLen、commond_code、buf用来保存客户端发来的小火车数据
    char downloadFilePath[1024] = {0};  //从客户端上传的文件的保存地址
    sprintf(downloadFilePath, "%s%s%s", getcwd(NULL, 0), "/", "Downloads");

    //日志记录文件
    int logFd = open("netdisk.log", O_WRONLY|O_APPEND|O_CREAT, 0666);
    char logInfo[1024] = {0};
    time_t now;

    //验证用户登录操作
    char username[20] = {0};
    char password[20] = {0};
    while(1)
    {
        memset(buf, 0, sizeof(buf));
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        recv(newFd, &dataLen, 4, MSG_WAITALL);
        //判断用户是登陆还是注册，0是注册，1是登录
        recv(newFd, &commond_code, 4, MSG_WAITALL);
        //接受用户名
        recv(newFd, username, dataLen, MSG_WAITALL);
        if(0 == commond_code)
        {
            //用户注册
            userRegister(newFd, conn, username);
        }else if(1 == commond_code){
            //用户登录
            
        }

        //如果用户名不存在
        /**if(NULL == (sp=getspnam(username)))
        {
            dataLen = 0;
            send(newFd, &dataLen, 4, 0);
            //将不存在的用户记录的日志
            now = time(NULL);
            sprintf(logInfo, "%s %s %s\n", username, "don't exist and try to log in", ctime(&now));
            write(logFd, logInfo, strlen(logInfo));
            continue;
        }
        //从sp得到salt
        get_salt(salt, sp->sp_pwdp);

        //密码验证
        if(!strcmp(sp->sp_pwdp, crypt(password, salt)))
        {
            dataLen = 1;
            send(newFd, &dataLen, 4, 0);
            printf("%s login\n", username);
            //将登录成功记录到日志
            now = time(NULL);
            sprintf(logInfo, "%s %s %s\n", username, "login success", ctime(&now));
            write(logFd, logInfo, strlen(logInfo));
            break;
        }else{ //密码错误
            dataLen = 0;
            send(newFd, &dataLen, 4, 0);
            //将用户密码错误记录到日志
            now = time(NULL);
            sprintf(logInfo, "%s %s %s\n", username, "password error", ctime(&now));
            write(logFd, logInfo, strlen(logInfo));
            continue;
        }**/
    }
    while(1)
    {
        dataLen = commond_code = 0;
        memset(buf, 0, sizeof(buf));
        ret = recv(newFd, &dataLen, 4, MSG_WAITALL);
        if(0 == ret)
        {
            close(logFd);
            return -1;
        }
        recv(newFd, &commond_code, 4, MSG_WAITALL);
        if(dataLen > 0)   //如果是pwd命令，则buf数据为空,不接受buf数据,ls命令数据可能为空
        {
            recv(newFd, buf, dataLen, MSG_WAITALL);
            printf("buf=%s\n", buf);
        }
        
        //将用户操作记录到日志
        //opToLog(newFd, logInfo, username, commond_code, buf);        
        switch(commond_code)
        {
        case 1:   //相应用户发来的cd命令
            //将用户操作记录到日志
            now = time(NULL);
            sprintf(logInfo, "%s %s %s %s\n", username, "cd", buf, ctime(&now));
            write(logFd, logInfo, strlen(logInfo));

            changeDir(buf);
            break;
        case 2:  //响应客户端发来ls命令
            //将用户操作记录到日志
            now = time(NULL);
            sprintf(logInfo, "%s %s %s %s\n", username, "ls", buf, ctime(&now));
            write(logFd, logInfo, strlen(logInfo));

            getCurrentList(newFd, dataLen, buf);
            break;
        case 3:  //响应用户puts上传请求，接收文件
            //将用户操作记录到日志
            now = time(NULL);
            sprintf(logInfo, "%s %s %s %s\n", username, "puts", buf, ctime(&now));
            write(logFd, logInfo, strlen(logInfo));

            sprintf(downloadFilePath,"%s%s%s", downloadFilePath, "/", buf);
            getFile(newFd, downloadFilePath);
            break;
        case 4:  //向客户端发送文件，响应gets下载请求
            //将用户操作记录到日志
            now = time(NULL);
            sprintf(logInfo, "%s %s %s %s\n", username, "gets", buf, ctime(&now));
            write(logFd, logInfo, strlen(logInfo));

            tranFile(newFd, buf);  //tran_file文件下
            break;
        case 5:  //响应用户的remove命令
            //将用户操作记录到日志
            now = time(NULL);
            sprintf(logInfo, "%s %s %s %s\n", username, "remove", buf, ctime(&now));
            write(logFd, logInfo, strlen(logInfo));
            
            removeFile(newFd, buf);
            break;
        case 6:  //响应pwd命令
            //将用户操作记录到日志
            now = time(NULL);
            sprintf(logInfo, "%s %s %s %s\n", username, "pwd", buf, ctime(&now));
            write(logFd, logInfo, strlen(logInfo));

            getCurrentPath(newFd);
            break;
        default: //客户端请求退出
            break;
        }
    }
    //关闭日志文件
    close(logFd);
    return 0;
}

//用户注册
int userRegister(int newFd, MYSQL *conn, char *username)
{
    //先验证用户名是否存在
    char query[100] = "select username from student where username = '";
    sprintf(query, "%s%s%s", query, username, "'");
    int ret = mysql_query(conn, query);
    MYSQL_RES *res;
    if(ret)
    {
        printf("Error making query:%s\n", mysql_error(conn));
    }else{
        res = mysql_use_result(conn);
        if(res)
        {
            if(mysql_fetch_row(res) != NULL)
            {
                printf("注册的用户名已存在\n");
                ret = 0;
                send(newFd, &ret, 4, 0);
                return -1;
            }else{
                //回复确认信息，要注册的用户名不存在，可以注册
                ret = 1;
                send(newFd, &ret, 4, 0);
            }
        }else{
            printf("Error making use data\n");
        }
        mysql_free_result(res);
    }

    //用户名不存在，用户注册
    train_t t;
    memset(&t, 0, sizeof(t));
    char salt[20] = {0};
    char str[STR_LEN+1] = {0};
    Generate(str);
    //生成salt
    sprintf(salt, "%s%s", "$6$", str);
    printf("salt=%s\n", salt);
    //将salt值发送给客户端，客户端加密
    strcpy(t.buf, salt);
    t.dataLen = strlen(t.buf);
    send(newFd, &t, 8+t.dataLen, 0);

    //接受客户端发来的密文
    memset(&t, 0, sizeof(t));
    recv(newFd, &t.dataLen, 4, MSG_WAITALL);
    recv(newFd, &t.command_code, 4, MSG_WAITALL);
    recv(newFd, t.buf, t.dataLen, MSG_WAITALL);
    printf("密文=%s\n", t.buf);

    //插入数据库数据库
    char insert[300] = "insert into user (username, salt, password) values('";
    sprintf(insert, "%s%s%s%s%s%s%s", insert, username, "','", salt, "','", t.buf, "')");
    ret = mysql_query(conn, insert);
    if(ret)
    {
        printf("Error making query:%s\n", mysql_error(conn));
    }else{
        printf("insert success\n");
    }
    return 0;
}

//生成随机数
void Generate(char *str)
{
    int i, flag;
    srand(time(NULL));
    for(i=0; i<STR_LEN; i++)
    {
        flag = rand() % 3;
        switch(flag)
        {
            case 0:
                str[i] = rand()%26 + 'a';
                break;
            case 1:
                str[i] = rand()%26 + 'A';
                break;
            case 2:
                str[i] = rand()%10 + '0';
                break;
        }
    }
}

//从/etc/shadow中取得的密文中取得salt
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

//下载文件
int getFile(int socketFd, char *fileName)
{
    int dataLen;
    //char buf[1000]={0}; 
    int command_code;
    //首先接受文件名
    //recv(socketFd, &dataLen, 4, MSG_WAITALL);
    //recv(socketFd, &command_code, 4, MSG_WAITALL);
    //recv(socketFd, buf, dataLen, MSG_WAITALL);
    printf("recvFile=%s\n", fileName);

    //接受文件大小
    off_t fileSize;
    recv(socketFd, &dataLen, 4, MSG_WAITALL);
    recv(socketFd, &command_code, 4, MSG_WAITALL);
    recv(socketFd, &fileSize, dataLen, MSG_WAITALL);
    printf("fileSize=%ld\n", fileSize);

    int fd = open(fileName, O_RDWR|O_CREAT, 0666);
    ERROR_CHECK(fd, -1, "open");
    //printf("%s\n", getcwd(NULL, 0));
    
    ftruncate(fd, fileSize);
    char *pMap = (char*)mmap(NULL, fileSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    ERROR_CHECK(pMap, (char*)-1, "mmap");

    int ret = recvCycle(socketFd, pMap, fileSize);
    if(-1 == ret)
    {
        printf("客户端断开\n");
        return -1;
    }
    munmap(pMap, fileSize);
    printf("上传成功\n");

    close(fd);
    return 0;
}

//响应用户的remove命令
int removeFile(int newFd, char *buf)
{
    train_t t;
    memset(&t, 0, sizeof(t));
    int ret;
    ret = remove(buf);
    if(0 == ret)
    {
        strcpy(t.buf, "delete success");
        t.dataLen = strlen(t.buf);
        t.command_code = 5;
        send(newFd, &t, 8+t.dataLen, 0);
    }
    return 0;
}

//得到当前路径并发送给客户端,pwd命令
int getCurrentPath(int newFd)
{
    train_t t;   //保存发送给客户端的小火车数据
    memset(&t, 0, sizeof(t));
    t.command_code = 6;
    strcpy(t.buf, getcwd(NULL, 0));
    t.dataLen = strlen(t.buf);
    send(newFd, &t, 8+t.dataLen, 0);
    return 0;
}

//文件类型判断
int fileMode(int mode, char *str)
{
    if(S_ISLNK(mode))
    {
        str[0] = 'l';
    }
    if(S_ISREG(mode))
    {
        str[0] = '-';
    }
    if(S_ISDIR(mode))
    {
        str[0] = 'd';
    }
    return 0;
}

//列出相应目录文件
int getCurrentList(int newFd, int dataLen, char *buf)
{  
    DIR *dir;
    if(!strcmp(buf, "~"))
    {
        strcpy(buf, getenv("HOME"));
    }else if(dataLen ==  0)
    {
        strcpy(buf, getcwd(NULL, 0));
    }
    dir = opendir(buf);
    printf("%s\n", buf);
    ERROR_CHECK(dir, NULL, "opendir");
    struct dirent *p;
    struct stat fileStauts;
    int ret;
    char path[1024] = {0};  //路径拼接
    train_t t;
    char mode;

    while((p=readdir(dir)) != NULL)
    {
        if(!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
        {
            continue;
        }
        memset(path, 0, sizeof(path));
        //路径拼接
        sprintf(path, "%s%s%s", buf, "/", p->d_name);
        //获取文件信息
        ret = stat(path, &fileStauts);
        ERROR_CHECK(ret, -1, "stat");

        fileMode(fileStauts.st_mode, &mode);
        memset(&t, 0, sizeof(t));
        sprintf(t.buf, "%c  %-40s %-ld", mode, p->d_name, fileStauts.st_size);
        t.dataLen = strlen(t.buf);
        t.command_code = 2;
        send(newFd, &t, 8+t.dataLen, 0);
        //sprintf(t.buf, "%s%s %-15s%ld\n", t.buf, mode, p->d_name, fileStauts.st_size);
    }
    //发送结束标志
    t.dataLen = 0;
    send(newFd, &t, 4, 0);
    //t.dataLen = strlen(t.buf);
    //t.command_code = 2;
    //send(newFd, &t, 8+t.dataLen, 0);
    closedir(dir);  //关闭目录文件
    return 0;
}

//相应用户发来的cd命令
int changeDir(char *buf)
{
    if(strlen(buf)){
        if(!strcmp(buf, "~"))
        {
            strcpy(buf, getenv("HOME"));
        }
        chdir(buf);
    }
    //客户端发送cd命令后，接着调用了clientPWD函数，接着发送了pwd命令
    //getCurrentPath(newFd);
    return 0;
}
