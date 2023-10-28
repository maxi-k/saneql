#include "SaneQLCompiler.hpp"
#include "algebra/Operator.hpp"
#include "infra/Schema.hpp"
#include "parser/ASTBase.hpp"
#include "parser/SaneQLLexer.hpp"
#include "parser/SaneQLParser.hpp"
#include "semana/SemanticAnalysis.hpp"
#include "sql/SQLWriter.hpp"
#include "sql/SQLGenerator.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
saneql::SaneQLCompiler::SaneQLCompiler(const Schema& schema, SQLWriter& sqlWriter)
   : schema(schema), sql(sqlWriter)
{
}
//---------------------------------------------------------------------------
std::string saneql::SaneQLCompiler::compile(const std::string& sqlQuery) {
   ASTContainer container;
   ast::AST* tree = nullptr;
   tree = SaneQLParser::parse(container, sqlQuery);
   return compile(tree);
}
//---------------------------------------------------------------------------
std::string saneql::SaneQLCompiler::compile(const ast::AST* tree) {
   SemanticAnalysis semana(schema);
   SQLGenerator gen(sql);
   auto res = semana.analyzeQuery(tree);
   if (res.isScalar()) {
      sql.write("select ");
      res.scalar()->traverse(gen);
   } else {
      algebra::Sort* sort = nullptr;
      auto tree = res.table().get();
      if (auto s = dynamic_cast<algebra::Sort*>(tree)) {
         sort = s;
         tree = sort->input.get();
      }
      sql.write("select ");
      bool first = true;
      for (auto& c : res.getBinding().getColumns()) {
         if (first)
            first = false;
         else
            sql.write(", ");
         sql.writeIU(c.iu);
         sql.write(" as ");
         sql.writeIdentifier(c.name);
      }
      sql.write(" from ");
      tree->traverse(gen);
      sql.write(" s");
      if (sort) {
         if (!sort->order.empty()) {
            sql.write(" order by ");
            bool first = true;
            for (auto& o : sort->order) {
               if (first)
                  first = false;
               else
                  sql.write(", ");
               o.value->traverse(gen);
               if (o.collate != Collate{}) sql.write(" collate TODO"); // TODO
               if (o.descending) sql.write(" desc");
            }
         }
         if (sort->limit.has_value()) {
            sql.write(" limit ");
            sql.write(to_string(*(sort->limit)));
         }
         if (sort->offset.has_value()) {
            sql.write(" offset ");
            sql.write(to_string(*(sort->offset)));
         }
      }
   }
   return sql.getResult();
}
//---------------------------------------------------------------------------
