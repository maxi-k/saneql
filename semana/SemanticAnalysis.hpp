#ifndef H_saneql_SemanticAnalysis
#define H_saneql_SemanticAnalysis
//---------------------------------------------------------------------------
#include "infra/Schema.hpp"
#include "semana/Functions.hpp"
#include <memory>
#include <unordered_map>
#include <variant>
//---------------------------------------------------------------------------
// SaneQL
// (c) 2023 Thomas Neumann
// SPDX-License-Identifier: BSD-3-Clause
//---------------------------------------------------------------------------
namespace saneql {
//---------------------------------------------------------------------------
namespace ast {
class AST;
class Access;
class BinaryExpression;
class Call;
class Cast;
class FuncArg;
class LetEntry;
class Literal;
class Type;
class UnaryExpression;
}
//---------------------------------------------------------------------------
namespace algebra {
class Expression;
class IU;
class Operator;
}
//---------------------------------------------------------------------------
/// Collate info
enum class Collate : uint8_t {
   None
};
//---------------------------------------------------------------------------
/// Information about collation and ordering
class OrderingInfo {
   /// The collate
   Collate collate = Collate::None;
   /// Descending?
   bool descending = false;

   /// Explicit constructor
   constexpr OrderingInfo(Collate collate, bool descending) : collate(collate), descending(descending) {}

   public:
   /// Default constructor
   constexpr OrderingInfo() = default;

   /// Mark as ascending
   void markAscending() { descending = false; }
   /// Mark as descending
   void markDescending() { descending = true; }
   /// Is descending?
   bool isDescending() const { return descending; }
   /// Get the collate
   Collate getCollate() const { return collate; }
   /// Change the collate
   void setCollate(Collate newCollate) { collate = newCollate; }

   /// Construct the default order
   static OrderingInfo defaultOrder() { return {}; }
   /// Lookup a collate. Throws if not found
   static Collate lookupCollate(const std::string& name);
};
//---------------------------------------------------------------------------
/// Semantic analysis for saneql queries
class SemanticAnalysis {
   public:
   /// Binding information
   class BindingInfo {
      public:
      /// Helper for group by
      class GroupByScope;

      /// A column description
      struct Column {
         /// The name
         std::string name;
         /// The IU
         const algebra::IU* iu;
      };

      private:
      /// Scope information
      struct Scope {
         /// The columns
         std::unordered_map<std::string, const algebra::IU*> columns;
         /// Is the scope ambiguous?
         bool ambiguous = false;
      };
      /// The well defined column order
      std::vector<Column> columns;
      /// Mapping from column name to IU
      std::unordered_map<std::string, const algebra::IU*> columnLookup;
      /// Scoped columns
      std::unordered_map<std::string, Scope> scopes;
      /// The arguments
      std::unordered_map<std::string, std::pair<const ast::AST*, const BindingInfo*>> arguments;
      /// The parent scope for function calls (if any)
      const BindingInfo* parentScope = nullptr;
      /// The group by scope (if any)
      GroupByScope* gbs = nullptr;

      friend class SemanticAnalysis;

      public:
      /// Marker for ambiguous IUs
      static const algebra::IU* const ambiguousIU;

      /// Access all columns
      const auto& getColumns() const { return columns; }
      /// Add a new scope, mark it as ambiguous if it already exists
      Scope* addScope(const std::string& name);
      /// Add a binding
      void addBinding(Scope* scope, const std::string& column, const algebra::IU* iu);

      /// Lookup a column
      const algebra::IU* lookup(const std::string& name) const;
      /// Lookup a column
      const algebra::IU* lookup(const std::string& binding, const std::string& name) const;

      /// Register an argument
      void registerArgument(const std::string& name, const ast::AST* ast, const BindingInfo* scope);
      /// Check for an argument
      std::pair<const ast::AST*, const BindingInfo*> lookupArgument(const std::string& name) const;

      /// Merge after a join
      void join(const BindingInfo& other);

      /// Get the group by scope
      GroupByScope* getGroupByScope() const { return gbs; }
   };

   private:
   /// The schema
   const Schema& schema;

   /// An expression container
   struct ExpressionResult {
      /// Content for scalar expressions
      struct ScalarInfo {
         /// The expression
         std::unique_ptr<algebra::Expression> expression;
         /// Collation and ordering
         OrderingInfo ordering;
      };
      /// Content for table expressions
      struct TableInfo {
         /// The operator
         std::unique_ptr<algebra::Operator> op;
         /// The column bindings
         BindingInfo binding;
      };
      std::variant<ScalarInfo, TableInfo> content;

      public:
      /// Constructor
      ExpressionResult(std::unique_ptr<algebra::Expression> expression, OrderingInfo ordering);
      /// Constructor
      ExpressionResult(std::unique_ptr<algebra::Operator> op, BindingInfo binding);
      /// Constructor
      ExpressionResult(ExpressionResult&&) = default;
      /// Destructor
      ~ExpressionResult();

      ExpressionResult& operator=(ExpressionResult&&) = default;

      /// Do we have a scalar result?
      bool isScalar() const { return content.index() == 0; }
      /// Access the scalar value
      auto& scalar() { return std::get<0>(content).expression; }
      /// Access the ordering
      auto& accessOrdering() { return std::get<0>(content).ordering; }
      /// Access the ordering
      OrderingInfo getOrdering() const { return std::get<0>(content).ordering; }
      /// Do we have a table result?
      bool isTable() const { return content.index() == 1; }
      /// Access the table value
      auto& table() { return std::get<1>(content).op; }
      /// Access the binding
      BindingInfo& accessBinding() { return std::get<1>(content).binding; }
      /// Access the binding
      const BindingInfo& getBinding() const { return std::get<1>(content).binding; }
   };
   /// Information about an extended type
   struct ExtendedType {
      /// The content
      std::variant<Type> content;

      /// Constructor
      ExtendedType(Type type) : content(type) {}

      /// Is a basic type?
      bool isBasic() const { return content.index() == 0; }
      /// Get the contained basic type
      Type getBasicType() const { return std::get<0>(content); }
   };
   /// Information about a let
   struct LetInfo {
      /// The signature (if any)
      Functions::Signature signature;
      /// The default values (if any)
      std::vector<const ast::AST*> defaultValues;
      /// The body of the let
      const ast::AST* body;
   };

   /// All lets
   std::vector<LetInfo> lets;
   /// Lookup of lets by name
   std::unordered_map<std::string, unsigned> letLookup;
   /// Visibility limit for lets
   unsigned letScopeLimit = ~0u;
   /// The next symbol id
   unsigned nextSymbolId = 1;

   /// Change the let scope limit
   class SetLetScopeLimit {
      SemanticAnalysis* semana;
      unsigned oldLimit;

      public:
      SetLetScopeLimit(SemanticAnalysis* semana, unsigned newLimit) : semana(semana), oldLimit(semana->letScopeLimit) { semana->letScopeLimit = newLimit; }
      ~SetLetScopeLimit() { semana->letScopeLimit = oldLimit; }
   };

   public:
   /// Report an error
   [[noreturn]] void reportError(std::string message);
   /// Invalid AST node
   [[noreturn]] void invalidAST();
   /// Extract a string value
   std::string extractString(const ast::AST* token);
   /// Extract a symbol name
   std::string extractSymbol(const ast::AST* token);
   /// Analyze a type
   ExtendedType analyzeType(const ast::Type& type);

   private:
   /// Recognize gensym calls. Returns an empty string otherwise
   std::string recognizeGensym(const ast::AST* ast);
   /// Analyze a literal
   ExpressionResult analyzeLiteral(const ast::Literal& literal);
   /// Analyze access
   ExpressionResult analyzeAccess(const BindingInfo& scope, const ast::Access& ast);
   /// Analyze a binary expression
   ExpressionResult analyzeBinaryExpression(const BindingInfo& scope, const ast::BinaryExpression& ast);
   /// Analyze a unary expression
   ExpressionResult analyzeUnaryExpression(const BindingInfo& scope, const ast::UnaryExpression& ast);
   /// Analyze a case computation
   ExpressionResult analyzeCase(const BindingInfo& scope, const std::vector<const ast::FuncArg*>& args);
   /// Analyze a join computation
   ExpressionResult analyzeJoin(ExpressionResult& input, const std::vector<const ast::FuncArg*>& args);
   /// Analyze a groupby computation
   ExpressionResult analyzeGroupBy(ExpressionResult& input, const std::vector<const ast::FuncArg*>& args);
   /// Analyze an aggregate computation
   ExpressionResult analyzeAggregate(ExpressionResult& input, const std::vector<const ast::FuncArg*>& args);
   /// Analyze a set computation
   ExpressionResult analyzeSetOperation(Functions::Builtin builtin, ExpressionResult& input, const std::vector<const ast::FuncArg*>& args);
   /// Analyze an orderby computation
   ExpressionResult analyzeOrderBy(ExpressionResult& input, const std::vector<const ast::FuncArg*>& args);
   /// Analyze a map or project computation
   ExpressionResult analyzeMap(ExpressionResult& input, const std::vector<const ast::FuncArg*>& args, bool project);
   /// Handle a symbol argument
   std::string symbolArgument(const std::string& funcName, const std::string& argName, const ast::FuncArg* arg);
   /// Handle a constant boolean argument
   bool constBoolArgument(const std::string& funcName, const std::string& argName, const ast::FuncArg* arg);
   /// Handle a scalar argument
   ExpressionResult scalarArgument(const BindingInfo& scope, const std::string& funcName, const std::string& argName, const ast::FuncArg* arg);
   /// Handle a list of scalar arguments
   std::vector<ExpressionResult> scalarArgumentList(const BindingInfo& scope, const std::string& funcName, const std::string& argName, const ast::FuncArg* arg);
   /// Handle a table argument
   ExpressionResult tableArgument(const BindingInfo& scope, const std::string& funcName, const std::string& argName, const ast::FuncArg* arg);
   /// Expression argument
   struct ExpressionArg {
      /// The name (if any)
      std::string name;
      /// The expression
      ExpressionResult value;
   };
   /// Handle expression list arguments
   std::vector<ExpressionArg> expressionListArgument(const BindingInfo& scope, const ast::FuncArg* arg);
   /// Make sure two values are comparable
   void enforceComparable(ExpressionResult& a, ExpressionResult& b);
   /// Make sure two values are comparable
   void enforceComparable(std::unique_ptr<algebra::Expression>& sa, std::unique_ptr<algebra::Expression>& sb);
   /// Analyze a call expression
   ExpressionResult analyzeCall(const BindingInfo& scope, const ast::Call& ast);
   /// Analyze a cast expression
   ExpressionResult analyzeCast(const BindingInfo& scope, const ast::Cast& ast);
   /// Analyze a table construction expression
   ExpressionResult analyzeTableConstruction(const BindingInfo& scope, const ast::FuncArg* ast);
   /// Analyze a token
   ExpressionResult analyzeToken(const BindingInfo& scope, const ast::AST* exp);
   /// Analyze an expression
   ExpressionResult analyzeExpression(const BindingInfo& scope, const ast::AST* exp);
   /// Analyze a let construction
   void analyzeLet(const ast::LetEntry& ast);

   public:
   /// Constructor
   explicit SemanticAnalysis(Schema& schema) : schema(schema) {}

   /// Analyze a query
   ExpressionResult analyzeQuery(const ast::AST* query);
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
