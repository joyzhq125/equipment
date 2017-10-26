
#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>

#ifdef __cplusplus 
extern "C" {
#endif 
extern MYSQL g_dbconn;

extern bool g_dbConnected;

void dbConnect();
#ifdef __cplusplus 
} /* extern "C" */
#endif 

#endif // DB_H
