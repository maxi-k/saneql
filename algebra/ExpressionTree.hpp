#pragma once
//---------------------------------------------------------------------------
#include <stdexcept>
namespace saneql {
//---------------------------------------------------------------------------
namespace algebra {
//---------------------------------------------------------------------------
// Expressions
//---------------------------------------------------------------------------
class Expression;
class IURef;
class ConstExpression;
class CastExpression;
class ComparisonExpression;
class BetweenExpression;
class InExpression;
class BinaryExpression;
class UnaryExpression;
class ExtractExpression;
class SubstrExpression;
class SimpleCaseExpression;
class SearchedCaseExpression;
class Aggregate;
class ForeignCall;
//---------------------------------------------------------------------------
struct ExpressionVisitor {
   virtual ~ExpressionVisitor() = default;
   virtual void visit(Expression&) {
      throw std::runtime_error("not implemented");
   }

   virtual void visit(IURef& expr) = 0;
   virtual void visit(ConstExpression& expr) = 0;
   virtual void visit(CastExpression& expr) = 0;
   virtual void visit(ComparisonExpression& expr) = 0;
   virtual void visit(BetweenExpression& expr) = 0;
   virtual void visit(InExpression& expr) = 0;
   virtual void visit(BinaryExpression& expr) = 0;
   virtual void visit(UnaryExpression& expr) = 0;
   virtual void visit(ExtractExpression& expr) = 0;
   virtual void visit(SubstrExpression& expr) = 0;
   virtual void visit(SimpleCaseExpression& expr) = 0;
   virtual void visit(SearchedCaseExpression& expr) = 0;
   virtual void visit(Aggregate& expr) = 0;
   virtual void visit(ForeignCall& expr) = 0;
};
//---------------------------------------------------------------------------
struct ConstExpressionVisitor {
   virtual ~ConstExpressionVisitor() = default;
   virtual void visit(const Expression&) {
      throw std::runtime_error("not implemented");
   }

   virtual void visit(const IURef& expr) = 0;
   virtual void visit(const ConstExpression& expr) = 0;
   virtual void visit(const CastExpression& expr) = 0;
   virtual void visit(const ComparisonExpression& expr) = 0;
   virtual void visit(const BetweenExpression& expr) = 0;
   virtual void visit(const InExpression& expr) = 0;
   virtual void visit(const BinaryExpression& expr) = 0;
   virtual void visit(const UnaryExpression& expr) = 0;
   virtual void visit(const ExtractExpression& expr) = 0;
   virtual void visit(const SubstrExpression& expr) = 0;
   virtual void visit(const SimpleCaseExpression& expr) = 0;
   virtual void visit(const SearchedCaseExpression& expr) = 0;
   virtual void visit(const Aggregate& expr) = 0;
   virtual void visit(const ForeignCall& expr) = 0;
};
//---------------------------------------------------------------------------
}  // namespace algebra
//---------------------------------------------------------------------------
}  // namespace saneql
//---------------------------------------------------------------------------
