#include<stdio.h>

#define F_PATH "test.txt"

int main(void)
{
    FILE*fp=NULL;//需要注意
    fp=fopen("test.txt", "r");
    if(NULL==fp)
    {
        return -1;//要返回错误代码
    }
    printf("open!");
    fclose(fp);
    fp=NULL;//需要指向空，否则会指向原打开文件地址
    return 0;
}
