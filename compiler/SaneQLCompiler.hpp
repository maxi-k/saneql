#ifndef H_saneql_Compiler
#define H_saneql_Compiler
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
namespace saneql {
//---------------------------------------------------------------------------
class Schema;
class SQLWriter;
namespace ast { class AST; }
//---------------------------------------------------------------------------
class SaneQLCompiler {
    /// the schema to compile against
    const Schema& schema;
    SQLWriter& sql;

    public:
    SaneQLCompiler(const Schema& schema, SQLWriter& sqlWriter);
    std::string compile(const std::string& sqlQuery);
    std::string compile(const ast::AST* saneqlAst);
};
}
//---------------------------------------------------------------------------
#endif // H_saneql_Compiler
