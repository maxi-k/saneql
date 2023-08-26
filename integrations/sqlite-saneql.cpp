#include "sqlite-saneql.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Add your header comment here */
#include <sqlite3ext.h> /* Do not use <sqlite3.h>! */
#include <assert.h>
SQLITE_EXTENSION_INIT1

/*
 * Structure used to accumulate the output
 */
struct EvalResult {
    char* z;              /* Accumulated output */
    const char* zSep;     /* Separator */
    int szSep;            /* Size of the separator string */
    sqlite3_int64 nAlloc; /* Number of bytes allocated for z[] */
    sqlite3_int64 nUsed;  /* Number of bytes of z[] actually used */
};

/*
 * Callback from sqlite_exec() for the eval() function.
 */
static int eval_callback(void* pCtx, int argc, char** argv, char** colnames) {
    struct EvalResult* p = (struct EvalResult*)pCtx;
    int i;
    if (argv == 0) {
        return SQLITE_OK;
    }
    for (i = 0; i < argc; i++) {
        const char* z = argv[i] ? argv[i] : "";
        size_t sz = strlen(z);
        if ((sqlite3_int64)sz + p->nUsed + p->szSep + 1 > p->nAlloc) {
            char* zNew;
            p->nAlloc = p->nAlloc * 2 + sz + p->szSep + 1;
            /* Using sqlite3_realloc64() would be better, but it is a recent
            ** addition and will cause a segfault if loaded by an older version
            ** of SQLite.  */
            zNew = p->nAlloc <= 0x7fffffff ? reinterpret_cast<char*>(sqlite3_realloc64(p->z, p->nAlloc)) : 0;
            if (zNew == 0) {
                sqlite3_free(p->z);
                memset(p, 0, sizeof(*p));
                return SQLITE_NOMEM;
            }
            p->z = zNew;
        }
        if (p->nUsed > 0) {
            memcpy(&p->z[p->nUsed], p->zSep, p->szSep);
            p->nUsed += p->szSep;
        }
        memcpy(&p->z[p->nUsed], z, sz);
        p->nUsed += sz;
    }
    return SQLITE_OK;
}

/*
 * Implementation of the eval(X) and eval(X,Y) SQL functions.
 *
 * Evaluate the SQL text in X. Return the results, using string
 * Y as the separator. If Y is omitted, use a single space character.
 */
static void saneql_func(sqlite3_context* context, int argc, sqlite3_value** argv) {
    const char* zSql;
    sqlite3* db;
    char* zErr = 0;
    int rc;
    struct EvalResult x;

    memset(&x, 0, sizeof(x));
    x.zSep = ",";
    zSql = (const char*)sqlite3_value_text(argv[0]);
    if (zSql == 0) {return;}
    if (argc > 1) {
        x.zSep = (const char*)sqlite3_value_text(argv[1]);
        if (x.zSep == 0) {
            return;
        }
    }
    x.szSep = (int)strlen(x.zSep);
    db = sqlite3_context_db_handle(context);
    rc = sqlite3_exec(db, zSql, eval_callback, &x, &zErr);
    if (rc != SQLITE_OK) {
        sqlite3_result_error(context, zErr, -1);
        sqlite3_free(zErr);
    } else if (x.zSep == 0) {
        sqlite3_result_error_nomem(context);
        sqlite3_free(x.z);
    } else {
        sqlite3_result_text(context, x.z, (int)x.nUsed, sqlite3_free);
    }
}

/// main entry point for sqlite
extern "C" int sqlite3_sqlitesaneql_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  /* Insert here calls to
  **     sqlite3_create_function_v2(),
  **     sqlite3_create_collation_v2(),
  **     sqlite3_create_module_v2(), and/or
  **     sqlite3_vfs_register()
  ** to register the new features that your extension adds.
  */
 (void)pzErrMsg;  /* Unused parameter */
 const int flags = SQLITE_UTF8 | SQLITE_DIRECTONLY;
 int rc = sqlite3_create_function(db, "sane", 1, flags, NULL, saneql_func, NULL, NULL);
 if (rc == SQLITE_OK) {
    rc = sqlite3_create_function(db, "sane", 2, flags, NULL, saneql_func, NULL, NULL);
 }
 return rc;
}
