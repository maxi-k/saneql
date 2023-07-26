#ifndef H_saneql_SQLGenerator
#define H_saneql_SQLGenerator
//---------------------------------------------------------------------------
#include "SQLWriter.hpp"
//---------------------------------------------------------------------------
// SaneQL
// (c) 2023 Thomas Neumann
// SPDX-License-Identifier: BSD-3-Clause
//---------------------------------------------------------------------------
namespace saneql {
//---------------------------------------------------------------------------
class SQLGenerator {
   friend class SQLWriter;
   friend class SQLiteWriter;
   protected:
   /// Generate SQL
   virtual void generate(SQLWriter& out) const = 0;
   virtual void generate(SQLiteWriter& out) const {
      generate(static_cast<SQLWriter&>(out));
   }
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif // H_saneql_SQLWriter
