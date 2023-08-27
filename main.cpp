#include <fstream>
#include <iostream>
#include <sstream>
#include "sql/SQLWriter.hpp"
#include "infra/Schema.hpp"
#include "compiler/SaneQLCompiler.hpp"
//---------------------------------------------------------------------------
using namespace std;
using namespace saneql;
//---------------------------------------------------------------------------
// (c) 2023 Thomas Neumann
//---------------------------------------------------------------------------
static string readFiles(unsigned count, char* files[]) {
   ostringstream output;
   for (unsigned i = 0; i != count; i++) {
      ifstream in(files[i]);
      if (!in.is_open()) {
         cerr << "unable to read " << files[i] << endl;
         exit(1);
      }
      output << in.rdbuf();
      output << "\n";
   }
   return output.str();
}
//---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
   if (argc < 2) {
      cerr << "usage: " << argv[0] << " file..." << endl;
      return 1;
   }

   Schema schema;
   schema.populateSchema(); // use TPC-H schema
   SQLWriter sql;
   SaneQLCompiler compiler(schema, sql);

   try {
      string query = readFiles(argc - 1, argv + 1);
      string sqlQuery = compiler.compile(query);
      cout << sqlQuery << endl;
   } catch (const exception& e) {
      cerr << "error: " << e.what() << endl;
      return 1;
   }

   return 0;
}
