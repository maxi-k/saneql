#ifndef H_saneql_SQLWriter
#define H_saneql_SQLWriter
//---------------------------------------------------------------------------
#include <string>
#include <string_view>
#include <unordered_map>
//---------------------------------------------------------------------------
// SaneQL
// (c) 2023 Thomas Neumann
// SPDX-License-Identifier: BSD-3-Clause
//---------------------------------------------------------------------------
namespace saneql {
//---------------------------------------------------------------------------
class Type;
class SQLGenerator;
//---------------------------------------------------------------------------
namespace algebra {
class IU;
}
//---------------------------------------------------------------------------
/// Helper class to generate SQL
class SQLWriter {
   private:
   /// The result buffer
   std::string result;
   /// The current target
   std::string* target;
   /// All assigned IU names
   std::unordered_map<const algebra::IU*, std::string> iuNames;

   public:
   /// Constructor
   SQLWriter();
   /// Destructor
   virtual ~SQLWriter();

   /// Write a SQL fragment
   virtual void write(std::string_view sql);
   /// Write an identifier, quoting as needed
   virtual void writeIdentifier(std::string_view identifier);
   /// Write an IU
   virtual void writeIU(const algebra::IU* iu);
   /// Write a string literal
   virtual void writeString(std::string_view str);
   /// Write a type
   virtual void writeType(Type type);

   /// Get the result
   std::string getResult() const { return result; }

   /// Generate SQL
   virtual void write(const SQLGenerator& gen);
};
//---------------------------------------------------------------------------
// We can support different SQL dialects
class SQLiteWriter : public SQLWriter {
   public:
   using SQLWriter::write;
   /// Generate SQL
   virtual void write(const SQLGenerator& gen) override;
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
