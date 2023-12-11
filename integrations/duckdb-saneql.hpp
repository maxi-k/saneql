#pragma once
//---------------------------------------------------------------------------
#include "duckdb.hpp"
#include <fstream>
//---------------------------------------------------------------------------
namespace duckdb {
//---------------------------------------------------------------------------
class SaneQLExtension : public Extension {
   public:
   void Load(DuckDB& db) override;
   std::string Name() override { return name(); }
   static constexpr std::string name() { return "saneql"; }
};
//---------------------------------------------------------------------------
struct SaneAST;
struct SaneQLParseData : public ParserExtensionParseData {
   shared_ptr<SaneAST> ast;
   explicit SaneQLParseData(shared_ptr<SaneAST> ast) : ast(ast) {}

   unique_ptr<ParserExtensionParseData> Copy() const override;
};
//---------------------------------------------------------------------------
struct SaneQLParserExtension : public ParserExtension {
   SaneQLParserExtension();

   /// parses saneql into an AST.
   static ParserExtensionParseResult saneql_parse(ParserExtensionInfo*, const std::string& query);
   // annoyingly, we can only return a 'table function' here, so we have to
   // implement an operator extension (that gets called upon when the plan
   // function throws). There, we can return a logical operator from the parsed
   // SQL without having to implement a table function.
   static ParserExtensionPlanResult saneql_plan(ParserExtensionInfo*, ClientContext&, unique_ptr<ParserExtensionParseData>);
};
//---------------------------------------------------------------------------
struct SaneQLOperatorExtension : public OperatorExtension {
   SaneQLOperatorExtension() : OperatorExtension() { Bind = saneql_bind; }
   ~SaneQLOperatorExtension() = default;

   std::string GetName() override { return SaneQLExtension::name(); }
   unique_ptr<LogicalExtensionOperator> Deserialize(LogicalDeserializationState&, FieldReader&) override {
      throw std::runtime_error("saneql operator deserialization not implemented");
   };

   private:
   static BoundStatement saneql_bind(ClientContext& context, Binder& binder, OperatorExtensionInfo* info, SQLStatement& statement);
};
//---------------------------------------------------------------------------
} // namespace duckdb
//---------------------------------------------------------------------------
