#pragma once
//---------------------------------------------------------------------------
#include "algebra/ExpressionTree.hpp"
#include "algebra/OperatorTree.hpp"
//---------------------------------------------------------------------------
namespace saneql {
//---------------------------------------------------------------------------
struct SQLWriter;
//---------------------------------------------------------------------------
struct SQLGenerator final : algebra::ConstExpressionVisitor, algebra::ConstOperatorVisitor {
   SQLWriter& out;
   SQLGenerator(SQLWriter& out) : out(out) {}
   ~SQLGenerator() override = default;

   /// Operators
   void visit(const algebra::TableScan& op) override;
   void visit(const algebra::Select& op) override;
   void visit(const algebra::Map& op) override;
   void visit(const algebra::SetOperation& op) override;
   void visit(const algebra::Join& op) override;
   void visit(const algebra::GroupBy& op) override;
   void visit(const algebra::Sort& op) override;
   void visit(const algebra::Window& op) override;
   void visit(const algebra::InlineTable& op) override;

   /// Expressions
   void visit(const algebra::IURef& expr) override;
   void visit(const algebra::ConstExpression& expr) override;
   void visit(const algebra::CastExpression& expr) override;
   void visit(const algebra::ComparisonExpression& expr) override;
   void visit(const algebra::BetweenExpression& expr) override;
   void visit(const algebra::InExpression& expr) override;
   void visit(const algebra::BinaryExpression& expr) override;
   void visit(const algebra::UnaryExpression& expr) override;
   void visit(const algebra::ExtractExpression& expr) override;
   void visit(const algebra::SubstrExpression& expr) override;
   void visit(const algebra::SimpleCaseExpression& expr) override;
   void visit(const algebra::SearchedCaseExpression& expr) override;
   void visit(const algebra::Aggregate& expr) override;
   void visit(const algebra::ForeignCall& expr) override;

private:

   void generateOperand(const algebra::Operator& expr);
   void generateOperand(const algebra::Expression& expr);
};
//---------------------------------------------------------------------------
} // namespace saneql
//---------------------------------------------------------------------------
