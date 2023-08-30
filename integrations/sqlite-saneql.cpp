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
#include "compiler/SaneQLCompiler.hpp"
#include "infra/Schema.hpp"
#include "sql/SQLWriter.hpp"
#include <string.h>
#include <assert.h>

#include <string>
#include <fstream>
#include <ostream>
#include <stdexcept>

/// C++ std allocator using sqlite3_malloc and sqlite3_free
template <typename T>
struct saneql_allocator {
   using value_type = T;

   // standard allocator methods

   saneql_allocator() = default;
   template <typename U>
   saneql_allocator(const saneql_allocator<U>&) {}

   T* allocate(std::size_t n) {
      void* result = sqlite3_malloc(n * sizeof(T));
      if (result == nullptr) { throw std::bad_alloc(); }
      return reinterpret_cast<T*>(result);
   }

   void deallocate(T* p, std::size_t) noexcept {
      sqlite3_free(p);
   }

   // static utility methods

   template <typename... Args>
   static T* make(Args&&... args) {
      saneql_allocator<T> alloc;
      return new (alloc.allocate(1)) T(std::forward<Args>(args)...);
   }

   static void destroy(T*&& p) {
      saneql_allocator<T> alloc;
      p->~T();
      alloc.deallocate(p, 1);
   }
};

#ifndef NDEBUG
static std::ostream& get_log_file() {
  static std::ofstream log_file("/tmp/sqlite-saneql.log");
  return log_file;
};

template<typename... Args>
static void saneql_log(const char* fmt, Args&&... args) {
  static char buf[1024];
  auto& stream = get_log_file();
  snprintf(buf, sizeof(buf), fmt, std::forward<Args>(args)...);
  stream << buf << std::endl;
}
#else
template<typename... Args>
static void saneql_log(const char*, Args&&...) {}
#endif

using str = std::basic_string<char, std::char_traits<char>, saneql_allocator<char>>;
template<typename T>
using vec = std::vector<T, saneql_allocator<T>>;


struct saneql_table_data {
   int nrow{0}, ncol{0};
   vec<str> colnames;
   vec<vec<str>> table;

   saneql_table_data() = default;
   saneql_table_data(saneql_table_data&&) = default;
   saneql_table_data& operator=(saneql_table_data&&) = default;
   // disallow copy
   saneql_table_data(const saneql_table_data&) = delete;
   saneql_table_data& operator=(const saneql_table_data&) = delete;

   void initialize(int ncol, char** names) {
      this->ncol = ncol;
      colnames.resize(ncol);
      table.resize(ncol);
      for (int i = 0; i < ncol; i++) {
         colnames[i] = names[i];
      }
   }

   vec<str>& column(int i) { return table[i]; }
   const vec<str>& column(int i) const { return table[i]; }
};

/* saneql_vtab is a subclass of sqlite3_vtab which is
** underlying representation of the virtual table
*/
struct saneql_vtab {
  sqlite3_vtab base{nullptr, 0, nullptr};  /* Base class - must be first */
  const str saneQuery;
  const str compiledQuery;
  const saneql_table_data table;

  saneql_vtab(str&& saneQuery, str&& compiledQuery, saneql_table_data&& table)
    : saneQuery(std::move(saneQuery))
    , compiledQuery(std::move(compiledQuery))
    , table(std::move(table)) {}
};

/* saneql_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
typedef struct saneql_cursor saneql_cursor;
struct saneql_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_int64 current_row{0}, nrow{0};

  saneql_cursor(sqlite3_vtab* pVtab, sqlite3_int64 current_row = 0)
    : base{pVtab}
    , current_row(current_row)
    , nrow(vtab().table.nrow){}

  bool isEof() const { return current_row >= nrow; }

  saneql_vtab& vtab() {
    return *reinterpret_cast<saneql_vtab*>(base.pVtab);
  }

  const saneql_vtab& vtab() const {
    return *reinterpret_cast<const saneql_vtab*>(base.pVtab);
  }
};

static int saneql_exec_callback(void *pCtx, int argc, char **argv, char **colnames) noexcept {
   saneql_table_data& result = *reinterpret_cast<saneql_table_data*>(pCtx);
   if (result.ncol != argc) { // initialize result struct
       result.initialize(argc, colnames); }
   // TODO can we get the column types somehow?
   result.nrow++;
   for (int i = 0; i < argc; i++) {
      result.table[i].emplace_back(argv[i]);
   }
   return SQLITE_OK;
};

static str saneql_compile_query(const char* saneQuery) noexcept(false) {
   using namespace saneql;
   TPCHSchema schema; // TODO: use actual database schema
   SQLWriter sql;
   SaneQLCompiler compiler(schema, sql);

   str compiled(compiler.compile(saneQuery));
   return compiled;
}

static str saneql_build_vtable_string(const saneql_table_data& eval_result) {
   // the table name itself is not important,
   // see https://www.sqlite.org/vtab.html#the_xcreate_method
   static str vtable_prefix = "CREATE TABLE x(";
   str vtable_string = vtable_prefix;
   for (auto& colname : eval_result.colnames) {
      vtable_string += colname + " TEXT,";
   }
   // replace last comma with closing paren
   vtable_string.back() = ')';
   saneql_log("vtable string: %s", vtable_string.c_str());
   return vtable_string;
}

static saneql_vtab* saneql_compile_and_build_vtab(
  sqlite3& db,
  [[maybe_unused]] void *pAux,
  int argc, const char * const * argv) {
  for (auto i = 0; i != argc; ++i) {
    saneql_log("compile argv[%d] = %s", i, argv[i]);
  }
  /// result code and error string for sqlite functions
  int rc; char* result_error{nullptr};
  /// compile the saneql query
  const char* sane_query = argv[1];
  saneql_log("sane query: %s", sane_query);

  str compiled_query = saneql_compile_query(sane_query);
  saneql_log("compiled query: %s\b", compiled_query.c_str());

  /// execute the saneql query
  saneql_table_data eval_result;
  rc = sqlite3_exec(&db, compiled_query.c_str(), saneql_exec_callback, &eval_result, &result_error);
  if (rc != SQLITE_OK) {
    saneql_log("error executing saneql query: %s", result_error);
    throw std::runtime_error(result_error);
  }

  /// declare the virtual table
  str vtabledef = saneql_build_vtable_string(eval_result);

  rc = sqlite3_declare_vtab(&db, vtabledef.c_str());
  if (rc != SQLITE_OK) {
    saneql_log("error declaring virtual table: %s", sqlite3_errmsg(&db));
    throw std::runtime_error(sqlite3_mprintf("error declaring virtual table: %s", sqlite3_errmsg(&db)));
  }

  return saneql_allocator<saneql_vtab>::make(
    std::move(sane_query),
    std::move(compiled_query),
    std::move(eval_result));
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
  int argc, const char * const *argv,
  sqlite3_vtab **ppVtab,
  char **pzErr) noexcept {
  try {
    saneql_vtab* vtab = saneql_compile_and_build_vtab(*db, pAux, argc, argv);
    *ppVtab = &(vtab->base);
    return SQLITE_OK;
  } catch (const std::exception& e) {
    *pzErr = sqlite3_mprintf("error compiling saneql query: %s", e.what());
    return SQLITE_ERROR;
  }
}

static int saneqlCreate(
  sqlite3 *db,
  void *pAux,
  int argc, const char * const *argv,
  sqlite3_vtab **ppVtab,
  char **pzErr) noexcept {
  for (auto i = 0; i != argc; ++i) {
    saneql_log("create argv[%d] = %s", i, argv[i]);
  }
  try {
    saneql_vtab* vtab = saneql_compile_and_build_vtab(*db, pAux, argc, argv);
    *ppVtab = &(vtab->base);
    return SQLITE_OK;
  } catch (const std::exception& e) {
    *pzErr = sqlite3_mprintf("error compiling saneql query: %s", e.what());
    return SQLITE_ERROR;
  }
}

/*
** This method is the destructor for saneql_vtab objects.
*/
static int saneqlDisconnect(sqlite3_vtab *pVtab) noexcept {
  auto* vtab = reinterpret_cast<saneql_vtab*>(pVtab);
  saneql_allocator<saneql_vtab>::destroy(std::move(vtab));
  return SQLITE_OK;
}

/*
** Constructor for a new saneql_cursor object.
*/
static int saneqlOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor) {
  try {
    saneql_cursor *pCur = saneql_allocator<saneql_cursor>::make(p);
    *ppCursor = &pCur->base;
    return SQLITE_OK;
  } catch (std::bad_alloc&) {
    return SQLITE_NOMEM;
  }
}

/*
** Destructor for a saneql_cursor.
*/
static int saneqlClose(sqlite3_vtab_cursor *cur) noexcept {
  saneql_cursor* pCur = reinterpret_cast<saneql_cursor*>(cur);
  saneql_allocator<saneql_cursor>::destroy(std::move(pCur));
  return SQLITE_OK;
}


/*
** Advance a saneql_cursor to its next row of output.
*/
static int saneqlNext(sqlite3_vtab_cursor *cur) noexcept {
  saneql_cursor *pCur = reinterpret_cast<saneql_cursor*>(cur);
  pCur->current_row++;
  return SQLITE_OK;
}

/*
** Return values of columns for the row at which the saneql_cursor
** is currently pointing.
*/
static int saneqlColumn(
   sqlite3_vtab_cursor* cur, /* The cursor */
   sqlite3_context* ctx, /* First argument to sqlite3_result_...() */
   int i /* Which column to return */
) noexcept {
   try {
      saneql_cursor* pCur = reinterpret_cast<saneql_cursor*>(cur);
      auto& table = pCur->vtab().table;
      const str& value = table.column(i)[pCur->current_row];
      // TODO can we assume static here?
      sqlite3_result_text(ctx, value.c_str(), value.size(), SQLITE_STATIC);
      return SQLITE_OK;
   } catch (const std::exception& e) {
      sqlite3_result_error(ctx, e.what(), -1);
      return SQLITE_ERROR;
   }
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static int saneqlRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid) noexcept {
  saneql_cursor *pCur = reinterpret_cast<saneql_cursor*>(cur);
  *pRowid = pCur->current_row;
  return SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static int saneqlEof(sqlite3_vtab_cursor *cur){
  saneql_cursor *pCur = reinterpret_cast<saneql_cursor*>(cur);
  return pCur->isEof();
}

/*
** This method is called to "rewind" the saneql_cursor object back
** to the first row of output.  This method is always called at least
** once prior to any call to saneqlColumn() or saneqlRowid() or
** saneqlEof().
*/
static int saneqlFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  [[maybe_unused]] int idxNum, [[maybe_unused]] const char *idxStr,
  [[maybe_unused]] int argc, [[maybe_unused]] sqlite3_value **argv
){
  for (auto i = 0; i != argc; ++i) {
    saneql_log("filter argv[%d] = %s", i, argv[i]);
  }
  saneql_cursor *pCur = reinterpret_cast<saneql_cursor*>(pVtabCursor);
  pCur->current_row = 0;
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
  auto& vtab = *reinterpret_cast<saneql_vtab*>(tab);
  pIdxInfo->estimatedCost = vtab.table.nrow;
#if SQLITE_VERSION_NUMBER > 3008002
  pIdxInfo->estimatedRows = vtab.table.nrow;
#endif
  return SQLITE_OK;
}

/*
** This following structure defines all the methods for the
** virtual table.
*/
static sqlite3_module saneqlModule = {
  /* iVersion    */ 3,
  /* xCreate     */ saneqlCreate,
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
extern "C" int sqlite3_sqlitesaneql_init(
  sqlite3 *db,
  [[maybe_unused]] char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);
  rc = sqlite3_create_module(db, "saneql", &saneqlModule, 0);
  saneql_log("registered %s extension with rc %d %p", "saneql", rc, saneqlModule);
  return rc;
}
