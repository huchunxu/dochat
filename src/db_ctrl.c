/*==============================================================
 *     FileName: db_ctrl.c
 *         Desc: 数据库操作
 *       Author: Hu Chunxu
 *        Email: huchunxu@hust.edu.cn
 *      Version: 0.0.1
 *   LastChange: 2014-05-12 21:21:38
 *      History:
 *=============================================================*/
#include "db_ctrl.h"

#define MYSQL_DBNAME "dochat"
#define MYSQL_USER   "root"
#define MYSQL_PSW    "hcxxxr"
#define MYSQL_IP     "localhost"
static MYSQL my_connection;
static int dbconnected = 0;

/**
 * @brief 数据库初始化并且连接
 *
 * @param name  数据库的用户名
 * @param pwd   数据库的密码
 *
 * @return 
 */
int database_start(char *name, char *pwd)
{
    if(dbconnected)
        return 1;
    
    mysql_init(&my_connection);
    if(!mysql_real_connect(&my_connection, MYSQL_IP, name, pwd, MYSQL_DBNAME, 0, NULL, 0))
    {
        fprintf(stderr, "Database connection failure: %d, %s\n", mysql_errno(&my_connection), mysql_error(&my_connection));
        return ERR_DB;
    }

    dbconnected = 1;

    return 1;
}

/**
 * @brief 数据库断开连接
 */
void database_end()
{
    if(dbconnected)
    {
        mysql_close(&my_connection);
        dbconnected = 0;
    }
}

/**
 * @brief 搜索用户
 *
 * @param user_name 需要搜索的用户
 *
 * @return -1 没有找到
 *         0  数据库执行语句错误
 *         >0 用户id    
 */
int search_user(char *user_name)
{
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;

    int res;
    char qs[250];
    char es[250];
    int user_id = ERR_USER;

    //查询是否在数据库中存在
    mysql_escape_string(es, user_name, strlen(user_name));     //格式化
    sprintf(qs, "select userno from users where name = '%s'", es);

    res = mysql_query(&my_connection, qs);
    if(res)
    {
        fprintf(stderr, "select error: %s\n", mysql_error(&my_connection));
        return ERR_DB;
    }
    else
    {
        res_ptr = mysql_store_result(&my_connection);
        if(res_ptr)
        {
            if(mysql_num_rows(res_ptr) > 0)
            {
                if((mysqlrow = mysql_fetch_row(res_ptr)))
                {
                    sscanf(mysqlrow[0], "%d", &user_id);
                }
            }
            mysql_free_result(res_ptr);
        }
    }

    return user_id;
}

/**
 * @brief 创建用户 
 *
 * @param user_name 需要创建的用户名
 * @param pwd       用户密码
 *
 * @return 0 数据库执行错误 
 *         >0 新创建用户的用户名
 */
int create_user(char *user_name, char *pwd)
{
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;

    int res;
    char is[250];
    char es[250];
    char ps[250];

    int user_id = ERR_USER;

    //如果用户不存在，创建用户
    mysql_escape_string(es, user_name, strlen(user_name));     //格式化用户名
    mysql_escape_string(ps, pwd, strlen(pwd));                 //格式化密码
    sprintf(is, "insert into users(name, pwd) values('%s', '%s')", es, ps);
    res = mysql_query(&my_connection, is);    //不需要返回的语句

    if(res)
    {
        fprintf(stderr, "insert error %d: %s\n", mysql_errno(&my_connection), mysql_error(&my_connection));
        return ERR_DB;
    }

    res = mysql_query(&my_connection, "select last_insert_id()");
    if(res)
    {
        printf("SELECT error: %s\n", mysql_error(&my_connection));
        return ERR_DB;
    }
    else
    {
        res_ptr = mysql_use_result(&my_connection);
        if(res_ptr)
        {
            if((mysqlrow = mysql_fetch_row(res_ptr)))
            {
                sscanf(mysqlrow[0], "%d", &user_id);
            }
            mysql_free_result(res_ptr);
        }
    }

    return user_id;
}

/**
 * @brief 通过用户名和密码匹配搜索
 *
 * @param user_name 用户名
 * @param pwd       密码
 *
 * @return -2 不存在用户
 *         -1 存在用户，但是密码不对
 *         0  数据库执行错误
 *         >0 找到的用户id
 */
int match_user(char *user_name, char *pwd)
{    
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;

    int user_id = ERR_PWD;
    int res;
    char qs[250];
    char es[250];
    char ps[250];

    if(search_user(user_name) > 0)
    {
        mysql_escape_string(es, user_name, strlen(user_name));     //格式化用户名
        mysql_escape_string(ps, pwd, strlen(pwd));                 //格式化密码

        sprintf(qs, "select userno from users where name = '%s' and pwd = '%s'", es, ps);
        res = mysql_query(&my_connection, qs);
        if(res)
        {
            fprintf(stderr, "select error: %s\n", mysql_error(&my_connection));
            return ERR_DB;
        }
        else
        {
            res_ptr = mysql_store_result(&my_connection);
            if(res_ptr)
            {
                if(mysql_num_rows(res_ptr) > 0)
                {
                    if((mysqlrow = mysql_fetch_row(res_ptr)))
                    {
                        sscanf(mysqlrow[0], "%d", &user_id);
                    }
                }
                mysql_free_result(res_ptr);
            }
        }

        return user_id;
    }
    else
        return ERR_USER;   //不存在用户
}

/**
 * @brief 搜索所有用户名
 *
 * @param name[][]
 *
 * @return 
 */
static int list_user(char *name[MAXLINK])
{
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;

    int res;
    int count = 0;

    //查询数据库
    res = mysql_query(&my_connection, "select name from users");
    if(res)
    {
        fprintf(stderr, "select error: %s\n", mysql_error(&my_connection));
        return ERR_DB;
    }
    else
    {
        res_ptr = mysql_store_result(&my_connection);
        if(res_ptr)
        {
            if(mysql_num_rows(res_ptr) > 0)
            {
                while((mysqlrow = mysql_fetch_row(res_ptr))>0)
                {
                    name[count] = strdup(mysqlrow[0]);
                    count++;
                }
            }
            mysql_free_result(res_ptr);
        }
    }

    return count;
}

/**
 * @brief 查询所有用户
 *
 * @param user_name[MAXLINK]
 *
 * @return 
 */
int get_all_user(char *user_name[MAXLINK])
{
    int count = 0;
    int ret = 0;

    if(!database_start(MYSQL_USER, MYSQL_PSW))
    {
        ret = ERR_DB;  //数据库连接错误
        return ret;
    }

    if((count = list_user(user_name)))
        ret = count;
    
    database_end();

    return count;
}



/**
 * @brief 检查数据库中的用户名和密码是否正确
 *
 * @param user_name 用户名
 * @param pwd   密码
 *
 * @return   ERR_DB   数据库连接错误
 *           ERR_USER 不存在用户
 *           ERR_PWD  密码错误
 *           user_id  找到的用户id
 */
int check_user(char *user_name, char *pwd)
{
    int user_id = 0;
    int ret = 0;

    if(!database_start(MYSQL_USER, MYSQL_PSW))
    {
        ret = ERR_DB;  //数据库连接错误
        return ret;
    }

    if((user_id = match_user(user_name, pwd)))
        ret = user_id;
    
    database_end();

    return ret;
}

/**
 * @brief 在数据库中注册用户
 *
 * @param user_name 用户名
 * @param pwd       密码
 *
 * @return ERR_DB   数据库连接错误
 *         ERR_USER 用户已经存在
 *         >0       用户id
 */
int register_user(char *user_name, char *pwd)
{
    int user_id = 0;
    int ret = 0;

    if(!database_start(MYSQL_USER, MYSQL_PSW))
    {
        ret = ERR_DB;  //数据库连接错误
        return ret;
    }

    if((user_id = match_user(user_name, pwd)) > 0)
    {
        ret = ERR_USER;
    }
    else
    {
        ret = create_user(user_name, pwd);
    }
    
    database_end();

    return ret;
}


