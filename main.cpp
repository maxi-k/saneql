#include "algebra/Operator.hpp"
#include "infra/Schema.hpp"
#include "parser/ASTBase.hpp"
#include "parser/SaneQLLexer.hpp"
#include "parser/SaneQLParser.hpp"
#include "semana/SemanticAnalysis.hpp"
#include "sql/SQLWriter.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
//---------------------------------------------------------------------------
using namespace std;
using namespace saneql;
//---------------------------------------------------------------------------
// (c) 2023 Thomas Neumann
//---------------------------------------------------------------------------
static string readFile(const string& fileName) {
   ifstream in(fileName);
   if (!in.is_open()) {
      cerr << "unable to read " << fileName << endl;
      exit(1);
   }
   ostringstream str;
   str << in.rdbuf();
   return str.str();
}
//---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
   if (argc != 2) {
      cerr << "usage: " << argv[0] << " file" << endl;
      return 1;
   }

   Schema schema;
   schema.populateSchema();

   string query = readFile(argv[1]);
   ASTContainer container;
   ast::AST* tree = nullptr;
   try {
      tree = SaneQLParser::parse(container, query);
   } catch (const exception& e) {
      cerr << e.what() << endl;
      return 1;
   }

   SemanticAnalysis semana(schema);
   try {
      auto res = semana.analyzeQuery(tree);
      auto sql = getenv("dialect") && getenv("dialect") == string("sqlite") ? make_unique<SQLiteWriter>() : make_unique<SQLWriter>();
      if (res.isScalar()) {
         sql->write("select ");
         sql->write(*res.scalar());
      } else {
         algebra::Sort* sort = nullptr;
         auto tree = res.table().get();
         if (auto s = dynamic_cast<algebra::Sort*>(tree)) {
            sort = s;
            tree = sort->input.get();
         }
         sql->write("select ");
         bool first = true;
         for (auto& c : res.getBinding().getColumns()) {
            if (first)
               first = false;
            else
               sql->write(", ");
            sql->writeIU(c.iu);
            sql->write(" as ");
            sql->writeIdentifier(c.name);
         }
         sql->write(" from ");
         sql->write(*tree);
         sql->write(" s");
         if (sort) {
            if (!sort->order.empty()) {
               sql->write(" order by ");
               bool first = true;
               for (auto& o : sort->order) {
                  if (first)
                     first = false;
                  else
                     sql->write(", ");
                  sql->write(*o.value);
                  if (o.collate != Collate{}) sql->write(" collate TODO"); // TODO
                  if (o.descending) sql->write(" desc");
               }
            }
            if (sort->limit.has_value()) {
               sql->write(" limit ");
               sql->write(to_string(*(sort->limit)));
            }
            if (sort->offset.has_value()) {
               sql->write(" offset ");
               sql->write(to_string(*(sort->offset)));
            }
         }
      }
      cout << sql->getResult() << endl;
   } catch (const exception& e) {
      cerr << e.what() << endl;
      return 1;
   }

   return 0;
}
