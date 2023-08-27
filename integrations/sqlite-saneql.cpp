#include "compiler/SaneQLCompiler.hpp"
#include "infra/Schema.hpp"
#include "sql/SQLWriter.hpp"
#include <string>
/*
** 2018-04-19
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements a template virtual-table.
** Developers can make a copy of this file as a baseline for writing
** new virtual tables and/or table-valued functions.
**
** Steps for writing a new virtual table implementation:
**
**     (1)  Make a copy of this file.  Perhaps call it "mynewvtab.c"
**
**     (2)  Replace this header comment with something appropriate for
**          the new virtual table
**
**     (3)  Change every occurrence of "saneql" to some other string
**          appropriate for the new virtual table.  Ideally, the new string
**          should be the basename of the source file: "mynewvtab".  Also
**          globally change "SANEQL" to "MYNEWVTAB".
**
**     (4)  Run a test compilation to make sure the unmodified virtual
**          table works.
**
**     (5)  Begin making incremental changes, testing as you go, to evolve
**          the new virtual table to do what you want it to do.
**
** This template is minimal, in the sense that it uses only the required
** methods on the sqlite3_module object.  As a result, saneql is
** a read-only and eponymous-only table.  Those limitation can be removed
** by adding new methods.
**
** This template implements an eponymous-only virtual table with a rowid and
** two columns named "a" and "b".  The table as 10 rows with fixed integer
** values. Usage example:
**
**     SELECT rowid, a, b FROM saneql;
*/
#if !defined(SQLITEINT_H)
#include "sqlite3ext.h"
#endif
SQLITE_EXTENSION_INIT1
#include <string.h>
#include <assert.h>

/// C++ std allocator using sqlite3_malloc and sqlite3_free
template <typename T>
struct saneql_allocator {
    using value_type = T;
    saneql_allocator() = default;
    template <typename U>
    saneql_allocator(const saneql_allocator<U>&) {}
    T* allocate(std::size_t n) {
        return reinterpret_cast<T*>(sqlite3_malloc(n * sizeof(T)));
    }
    void deallocate(T* p, std::size_t) {
        sqlite3_free(p);
    }
};

using str = std::basic_string<char, std::char_traits<char>, saneql_allocator<char>>;
template<typename T>
using vec = std::vector<T, saneql_allocator<T>>;


/* saneql_vtab is a subclass of sqlite3_vtab which is
** underlying representation of the virtual table
*/
typedef struct saneql_vtab saneql_vtab;
struct saneql_vtab {
  sqlite3_vtab base;  /* Base class - must be first */
  str saneQuery;
  str compiledQuery;
  /* Add new fields here, as necessary */
};

/* saneql_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
typedef struct saneql_cursor saneql_cursor;
struct saneql_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  /* Insert new fields here.  For this saneql we only keep track
  ** of the rowid */
  sqlite3_int64 iRowid;      /* The rowid */
};

struct saneql_table_data {
    int nrow{0}, ncol{0};
    vec<str> colnames;
    vec<vec<str>> table;

    void initialize(int ncol, char** names) {
        this->ncol = ncol;
        colnames.resize(ncol);
        table.resize(ncol);
        for (int i = 0; i < ncol; i++) {
           colnames[i] = names[i];
        }
    }
};

static int saneql_exec_callback(void *pCtx, int argc, char **argv, char **colnames) {
   saneql_table_data& result = *reinterpret_cast<saneql_table_data*>(pCtx);
   if (result.ncol != argc) { // initialize result struct
       result.initialize(argc, colnames);
   }
   result.nrow++;
   for (int i = 0; i < argc; i++) {
      result.table[i].emplace_back(argv[i]);
   }
};

static int saneql_compile_query(const char* saneQuery, char** result, char** error) {
   using namespace saneql;
   Schema schema;
   schema.populateSchema(); // use TPC-H schema TODO: use schema from sqlite3
   SQLWriter sql;
   SaneQLCompiler compiler(schema, sql);
   try {
      str result(compiler.compile(saneQuery));
      char* target = reinterpret_cast<char*>(sqlite3_malloc(result.size() + 1));
      strcpy(target, result.c_str());
   } catch (const std::exception& e) {
       const char* errstr = e.what();
       *error = reinterpret_cast<char*>(sqlite3_malloc(strlen(errstr) + 1));
       strcpy(*error, errstr);
       return SQLITE_ERROR;
   }
   return SQLITE_OK;
}

int saneql_build_vtable_string(char** result, saneql_table_data* eval_result, char** error) {
    // TODO stub
   str vtable_string = "CREATE TABLE x(a,b)";
   char* target = reinterpret_cast<char*>(sqlite3_malloc(vtable_string.size() + 1));
   strcpy(target, vtable_string.c_str());
   *result = target;
   return SQLITE_OK;
}

/*
** The saneqlConnect() method is invoked to create a new
** template virtual table.
**
** Think of this routine as the constructor for saneql_vtab objects.
**
** All this routine needs to do is:
**
**    (1) Allocate the saneql_vtab object and initialize all fields.
**
**    (2) Tell SQLite (via the sqlite3_declare_vtab() interface) what the
**        result set of queries against the virtual table will look like.
*/
static int saneqlConnect(
  sqlite3 *db,
  void *pAux,
  int argc, char **argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  assert(argc >= 1);
  int rc;
  /// compile the saneql query
  const char* sane_query = argv[0];
  char* compiled_query = nullptr;
  char* result_error = nullptr;

  rc = saneql_compile_query(sane_query, &compiled_query, &result_error);
  if (rc != SQLITE_OK) {
      *pzErr = result_error;
      return rc;
  }

  /// execute the saneql query
  saneql_table_data eval_result;
  rc = sqlite3_exec(db, compiled_query, saneql_exec_callback, &eval_result, &result_error);
  if (rc != SQLITE_OK) {
      *pzErr = result_error;
      return rc;
  }

  /// declare the virtual table
  char* vtable_string = nullptr;
  rc = saneql_build_vtable_string(&vtable_string, &eval_result, &result_error);
  if (rc != SQLITE_OK) {
     *pzErr = result_error;
     return rc;
  }

  rc = sqlite3_declare_vtab(db, vtable_string);
  sqlite3_free(vtable_string);
  if (rc != SQLITE_OK) {
     *pzErr = sqlite3_mprintf("error declaring virtual table: %s", sqlite3_errmsg(db));
     return rc;
  }

    /// allocate the vtable data struct TODO
  void* pNew = sqlite3_malloc(sizeof(saneql_vtab));
  *ppVtab = (sqlite3_vtab*) pNew;
  if (pNew == 0) return SQLITE_NOMEM;
  memset(pNew, 0, sizeof(saneql_vtab));

  return rc;
}

/*
** This method is the destructor for saneql_vtab objects.
*/
static int saneqlDisconnect(sqlite3_vtab *pVtab){
  saneql_vtab *p = (saneql_vtab*)pVtab;
  sqlite3_free(p);
  return SQLITE_OK;
}

/*
** Constructor for a new saneql_cursor object.
*/
static int saneqlOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  saneql_cursor *pCur;
  pCur = sqlite3_malloc( sizeof(*pCur) );
  if( pCur==0 ) return SQLITE_NOMEM;
  memset(pCur, 0, sizeof(*pCur));
  *ppCursor = &pCur->base;
  return SQLITE_OK;
}

/*
** Destructor for a saneql_cursor.
*/
static int saneqlClose(sqlite3_vtab_cursor *cur){
  saneql_cursor *pCur = (saneql_cursor*)cur;
  sqlite3_free(pCur);
  return SQLITE_OK;
}


/*
** Advance a saneql_cursor to its next row of output.
*/
static int saneqlNext(sqlite3_vtab_cursor *cur){
  saneql_cursor *pCur = (saneql_cursor*)cur;
  pCur->iRowid++;
  return SQLITE_OK;
}

/*
** Return values of columns for the row at which the saneql_cursor
** is currently pointing.
*/
static int saneqlColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  int i                       /* Which column to return */
){
  saneql_cursor *pCur = (saneql_cursor*)cur;
  switch( i ){
    case SANEQL_A:
      sqlite3_result_int(ctx, 1000 + pCur->iRowid);
      break;
    default:
      assert( i==SANEQL_B );
      sqlite3_result_int(ctx, 2000 + pCur->iRowid);
      break;
  }
  return SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static int saneqlRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  saneql_cursor *pCur = (saneql_cursor*)cur;
  *pRowid = pCur->iRowid;
  return SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static int saneqlEof(sqlite3_vtab_cursor *cur){
  saneql_cursor *pCur = (saneql_cursor*)cur;
  return pCur->iRowid>=10;
}

/*
** This method is called to "rewind" the saneql_cursor object back
** to the first row of output.  This method is always called at least
** once prior to any call to saneqlColumn() or saneqlRowid() or
** saneqlEof().
*/
static int saneqlFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  saneql_cursor *pCur = (saneql_cursor *)pVtabCursor;
  pCur->iRowid = 1;
  return SQLITE_OK;
}

/*
** SQLite will invoke this method one or more times while planning a query
** that uses the virtual table.  This routine needs to create
** a query plan for each invocation and compute an estimated cost for that
** plan.
*/
static int saneqlBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  pIdxInfo->estimatedCost = (double)10;
  pIdxInfo->estimatedRows = 10;
  return SQLITE_OK;
}

/*
** This following structure defines all the methods for the
** virtual table.
*/
static sqlite3_module saneqlModule = {
  /* iVersion    */ 0,
  /* xCreate     */ 0,
  /* xConnect    */ saneqlConnect,
  /* xBestIndex  */ saneqlBestIndex,
  /* xDisconnect */ saneqlDisconnect,
  /* xDestroy    */ 0,
  /* xOpen       */ saneqlOpen,
  /* xClose      */ saneqlClose,
  /* xFilter     */ saneqlFilter,
  /* xNext       */ saneqlNext,
  /* xEof        */ saneqlEof,
  /* xColumn     */ saneqlColumn,
  /* xRowid      */ saneqlRowid,
  /* xUpdate     */ 0,
  /* xBegin      */ 0,
  /* xSync       */ 0,
  /* xCommit     */ 0,
  /* xRollback   */ 0,
  /* xFindMethod */ 0,
  /* xRename     */ 0,
  /* xSavepoint  */ 0,
  /* xRelease    */ 0,
  /* xRollbackTo */ 0,
  /* xShadowName */ 0
};


#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_sqlitesaneql_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);
  rc = sqlite3_create_module(db, "saneql", &saneqlModule, 0);
  return rc;
}


