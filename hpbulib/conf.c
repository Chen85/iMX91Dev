#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "type_def.h"
#include "hicc_config.h"
#include "conf.h"

#define BUF_LEN         10*1024
#define MAX_ARGV_CNT    256

static char *null_str = "";
//==============================================================================
static int load_ini_file(const char *file, char *buf, int buf_size)
{
    FILE *fp = NULL;
    int read_size;

    if(file == NULL || buf == NULL)
        return -1;

    fp = fopen(file, "r");
    if(fp == NULL)
        return 0;

    flock(fileno(fp), LOCK_SH);

    //load initialization file
    read_size = fread(buf, 1, buf_size, fp);
    if(read_size == buf_size)
        buf[buf_size - 1] = '\0';
    else
        buf[read_size] = '\0';

    flock(fileno(fp), LOCK_UN);

    fclose(fp);
    return read_size;
}
//==============================================================================
char *trim(char *str)
{
    char *p = str;
    int len;

    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p ++;

    str = p;

    len = strlen(str);
    if(str == 0)
        return str;

    p = str + len - 1;

    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        -- p;

    *(p + 1) = '\0';

    return str;
}
//==============================================================================
int readConf(const char *file, char *argv[][2], int argc, char *buf, int buf_len)
{
    int file_len, i;
    int argc_cnt;

    file_len = load_ini_file(file, buf, buf_len);
    if(file_len < 0)
        return -1;

    if(file_len == 0)
       return 0;

    argc_cnt = 0;
    for(i = 0; i < file_len; i++)
    {
        if(buf[i] == '\n' || buf[i] == '\r' || buf[i] == ' ')
            continue;

        if(buf[i] == '#')
        {
            while((buf[i] != '\n') && (buf[i] != '\r') && (i < file_len))
            {
                i++;
            }
            continue;
        }

        argv[argc_cnt][0] = &buf[i];

        while((buf[i] != '=') && (i < file_len))
        {
            if(buf[i] == '\n' || buf[i] == '\r')
                break;
            i++;
        }

        if(i >= file_len)
            break;

        if(buf[i] == '\n' || buf[i] == '\r')
             continue;

        buf[i] = '\0';

        i++;
        argv[argc_cnt][1] = &buf[i];

        while((buf[i] != '\n') && (buf[i] != '\r') && (i < file_len))
        {
            i++;
        }

        buf[i] = '\0';

        argv[argc_cnt][0] = trim(argv[argc_cnt][0]);

        if(strlen(argv[argc_cnt][0]) == 0)
            continue;

        argv[argc_cnt][1] = trim(argv[argc_cnt][1]);
        argc_cnt++;

        if(argc_cnt >= argc)
            break;
    }

    return argc_cnt;
}
//==============================================================================
char *findArgValue(const char *name, char *argv[][2], int argc)
{
    int i;

    for(i = 0; i < argc; i++)
    {
        if(strcmp(argv[i][0], name) == 0)
            return argv[i][1];
    }

    return null_str;
}
//==============================================================================
char *readArgValue(const char *file, const char *name, char *buf, int buf_len)
{
    char *argv[MAX_ARGV_CNT][2];
    int argc;

    argc = readConf(file, argv, MAX_ARGV_CNT, buf, buf_len);
    if(argc <= 0)
        return null_str;

    return findArgValue(name, argv, argc);
}
//==============================================================================
int checkArgValue(const char *file, const char *arg, const char *value, char *buf, int buf_len)
{
    char *argv[MAX_ARGV_CNT][2];
    int argc;
    char *arg_val = "";

    argc = readConf(file, argv, MAX_ARGV_CNT, buf, buf_len);
    if(argc <= 0)
        return -2;

    arg_val = findArgValue(arg, argv, argc);

    if(strcmp(arg_val, value) == 0)
        return 0;
    else
        return -1;
}
//==============================================================================
int writeConf(const char *file, char *argv[][2], int argc)
{
    FILE *fp = NULL;
    int fd = -1;
    int file_len, out_file_len;
    int argc_cnt;
    int i, j, i_tmp, copy_len;
    char *old_name, *old_value;
    char *buf = NULL, *in_buf, *out_buf;
    char is_new_argv[MAX_ARGV_CNT];
    int ret = -1;

    buf = malloc(3 * BUF_LEN);
    if(buf == NULL)
        goto error;

    in_buf = buf + BUF_LEN;
    out_buf = in_buf + BUF_LEN;

    file_len = load_ini_file(file, buf, BUF_LEN);
    if(file_len < 0)
        goto error;

    fd = open(file, O_RDONLY|O_NOCTTY|O_CREAT, 0666);
    if (fd < 0)
        goto error;

    flock(fd, LOCK_EX);

    fp = fopen(file, "w");
    if(fp == NULL)
        goto error;

    if(file_len > 0)
        memcpy(&in_buf[0], &buf[0], file_len);

    if(argc > ARRAY_SIZE(is_new_argv))
    {
        DEBUGP("too much argc=%d\n", argc);
        argc = ARRAY_SIZE(is_new_argv);
    }

    memset(is_new_argv, 'Y', argc);

    argc_cnt = 0;
    out_file_len = 0;
    i_tmp = 0;
    for(i = 0; i < file_len; i++)
    {
        if(i_tmp != i)
        {
            DEBUGP("i=%d\ni_tmp=%d\nout_file_len=%d\n", i, i_tmp, out_file_len);
            copy_len = i - i_tmp;
            if(out_file_len + copy_len > BUF_LEN)
                goto error;

            memcpy(&out_buf[out_file_len], &in_buf[i_tmp], copy_len);
            out_file_len += copy_len;
            i_tmp = i;
        }

        if(buf[i] == '\n' || buf[i] == '\r' || buf[i] == ' ')
            continue;

        if(buf[i] == '#')
        {
            while((buf[i] != '\n') && (buf[i] != '\r') && (i < file_len))
            {
                i++;
            }
            continue;
        }

        old_name = &buf[i];

        while((buf[i] != '=') && (i < file_len))
        {
            if(buf[i] == '\n' || buf[i] == '\r')
                break;
            i++;
        }

        if(i >= file_len)
            break;

        if(buf[i] == '\n' || buf[i] == '\r')
             continue;

        buf[i] = '\0';

        i++;
        old_value = &buf[i];

        while((buf[i] != '\n') && (buf[i] != '\r') && (i < file_len))
        {
            i++;
        }

        buf[i] = '\0';

        old_name = trim(old_name);

        if(strlen(old_name) == 0)
            continue;

        for(j = 0; j < argc; j++)
        {
            if(argv[j][0] != NULL)
            {
                if(strcmp(argv[j][0], old_name) == 0)
                {
                    out_file_len += snprintf(&out_buf[out_file_len], BUF_LEN-out_file_len, "%s=%s", argv[j][0], argv[j][1]);
                    DEBUGP("new value=%s\n", argv[j][1]);
                    i_tmp = i;
                    is_new_argv[j] = 'N';
                    break;
                }
            }
        }

        old_value = trim(old_value);
        DEBUGP("%s=%s\n", old_name, old_value);
        argc_cnt++;
    }

    if(i_tmp != i)
    {
        DEBUGP("i=%d\ni_tmp=%d\nout_file_len=%d\n", i, i_tmp, out_file_len);
        copy_len = i - i_tmp;
        if(out_file_len + copy_len < BUF_LEN)
        {
            memcpy(&out_buf[out_file_len], &in_buf[i_tmp], copy_len);
            out_file_len += copy_len;
        }
        else
            goto error;
    }

    if(file_len <= 0)
    {
        for(i = 0; i < argc; i++)
            fprintf(fp, "%s=%s\n", argv[i][0], argv[i][1]);
    }
    else
    {
        fwrite(out_buf, 1, out_file_len, fp);

        if(out_buf[out_file_len-1] != '\n')
        {
            fprintf(fp, "\n");
        }

        for(j = 0; j < argc; j++)
        {
            if((argv[j][0] != NULL) && (is_new_argv[j] == 'Y'))
            {
                fprintf(fp, "%s=%s\n", argv[j][0], argv[j][1]);
                DEBUGP("new name=%s\n", argv[j][0]);
                DEBUGP("new value=%s\n", argv[j][1]);
            }
        }
    }

    ret = 0;

end:

    if(fp != NULL)
        fclose(fp);

    if(fd >= 0)
    {
        flock(fd, LOCK_UN);
        close(fd);
    }

    if(buf != NULL)
        free(buf);

    return ret;

error:
    ret = -1;
    goto end;
}
//==============================================================================
int setArgValue(const char *file, const char *arg, const char *value)
{
    char *args[1][2];
    int argc;

    args[0][0] = (char *)arg;
    args[0][1] = (char *)value;
    argc = 1;

    return writeConf(file, args, argc);
}
//==============================================================================
void printShellValue(char *val)
{
    while (*val) {
        switch (*val) {
        case '"':
        case '\\':
        case '`':
        case '$':
            putc ('\\', stdout);
        default:
            putc (*val,  stdout);
            break;
        }

        val++;
    }
}
//==============================================================================
void printShellArg(char *name, char *val)
{
    printf("%s=\"", name);
    printShellValue(val);
    printf("\"\n");
}
//==============================================================================
void printJsonValue(char *val)
{
    while (*val) {
        switch (*val) {
        case '"':
        case '\\':
            putc ('\\', stdout);
        default:
            putc (*val,  stdout);
            break;
        }

        val++;
    }
}
//==============================================================================
/* Split String but haven't select range of exception characters
 * e.g. Str=aa bb cc "dd ee" gg, split=' ', exception='\"'; argv[0]=aa, argv[1]=bb, argv[2]=cc, argv[3]=dd ee, argv[4]=gg
 */
static s32_t splitStrPlus(int argc, char *argv[], char *str, char split, char exception)
{
    int i;
    int argc_cnt;

    u32_t str_len;

    argc_cnt = 0;
    i = 0;
    str_len = strlen(str);

    while(i < str_len)
    {
        argv[argc_cnt] = &str[i];

        while((str[i] != split) && (str[i] != '\0'))
        {
            if(str[i] == exception)
            {
                argv[argc_cnt] = &str[i];
                i++;

                while((str[i] != exception) && (str[i] != '\0'))
                    i++;
            }

            i++;
        }

        str[i] = '\0';

        argv[argc_cnt] = trim(argv[argc_cnt]);
        argc_cnt++;

        if(argc_cnt >= argc)
            break;

        i++;
    }

    return argc_cnt;
}

#define IS_NUMBER(x) ((x >= '0' && x <= '9') ? TRUE : FALSE)
static bool_t checkArrayValue(char *val, s32_t len)
{
    char buf[1024];
    char *argv[MAX_ARGV_CNT];
    s32_t argc, i, j;

    if(len > (sizeof(buf) - 1))
        return FALSE;

    memcpy(buf, val, len);
    buf[len] = '\0';

    argc = splitStrPlus(ARRAY_SIZE(argv), argv, buf, ',', '\"');
    if(argc == 0)
        return TRUE;

    for(i = 0; i < argc; i++)
    {
        len = strlen(argv[i]);
        if(len == 0)
            return FALSE;

        // check string
        if((len >= 2) && (argv[i][0] == '\"') && (argv[i][len-1] == '\"'))
        {
            continue;
        }

        // check number valid
        if((argv[i][0] < '1') || (argv[i][0] > '9'))
            return FALSE;

        for(j = 1; j < len; j++)
        {
            if(IS_NUMBER(argv[i][j]) == FALSE)
                return FALSE;
        }
    }

    return TRUE;
}
//==============================================================================
bool_t isArray(char *arg_val)
{
    u32_t arg_val_len;

    if(arg_val[0] != '[')
        return FALSE;

    arg_val_len = strlen(arg_val);

    if((arg_val_len >= 2) &&
       (arg_val[arg_val_len-1] == ']'))
    {
        return checkArrayValue(arg_val + 1, arg_val_len - 2);
    }
    else
    {
        return FALSE;
    }
}
//==============================================================================
void printJsonArg(char *name, char *val, u8_t last)
{
    if(isArray(val) == TRUE)
    {
        printf("\"%s\": %s", name, val);
    }
    else
    {
        printf("\"%s\": \"", name);
        printJsonValue(val);
        printf("\"");
    }

    if(last == TRUE)
        printf("\n");
    else
        printf(",\n");
}
//==============================================================================
