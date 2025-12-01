#ifndef __CONF_H__
#define __CONF_H__
//==============================================================================
int readConf(const char *file, char *argv[][2], int argc, char *buf, int buf_len);
char *findArgValue(const char *name, char *argv[][2], int argc);
int checkArgValue(const char *file, const char *arg, const char *value, char *buf, int buf_len);
int writeConf(const char *file, char *argv[][2], int argc);
int setArgValue(const char *file, const char *arg, const char *value);
char *readArgValue(const char *file, const char *name, char *buf, int buf_len);
//------------------------------------------------------------------------------
void printShellArg(char *name, char *val);
void printShellValue(char *val);
void printJsonValue(char *val);
void printJsonArg(char *name, char *val, u8_t last);
bool_t isArray(char *arg_val);
//==============================================================================
#endif /* __CONF_H__ */
