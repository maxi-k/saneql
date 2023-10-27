#include "algebra/Expression.hpp"
#include "algebra/Operator.hpp"
#include "algebra/ExpressionTree.hpp"
#include <algorithm>
#include <utility>
//---------------------------------------------------------------------------
// (c) 2023 Thomas Neumann
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace saneql::algebra {
//---------------------------------------------------------------------------
Expression::~Expression()
// Destructor
{
}
//---------------------------------------------------------------------------
IURef::IURef(const IU* iu)
   : Expression(iu->getType()), iu(iu)
// Constructor
{
}
//---------------------------------------------------------------------------
void IURef::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void IURef::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
ComparisonExpression::ComparisonExpression(unique_ptr<Expression> left, unique_ptr<Expression> right, Mode mode, Collate collate)
   : Expression(Type::getBool().withNullable((mode != Mode::Is) && (mode != Mode::IsNot) && (left->getType().isNullable() || right->getType().isNullable()))), left(move(left)), right(move(right)), mode(mode), collate(collate)
// Constructor
{
}
//---------------------------------------------------------------------------
void ComparisonExpression::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void ComparisonExpression::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
BetweenExpression::BetweenExpression(unique_ptr<Expression> base, unique_ptr<Expression> lower, unique_ptr<Expression> upper, Collate collate)
   : Expression(Type::getBool().withNullable(base->getType().isNullable() || lower->getType().isNullable() || upper->getType().isNullable())), base(move(base)), lower(move(lower)), upper(move(upper)), collate(collate)
// Constructor
{
}
//---------------------------------------------------------------------------
void BetweenExpression::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void BetweenExpression::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
InExpression::InExpression(unique_ptr<Expression> probe, vector<unique_ptr<Expression>> values, Collate collate)
   : Expression(Type::getBool().withNullable(probe->getType().isNullable() || any_of(values.begin(), values.end(), [](auto& e) { return e->getType().isNullable(); }))), probe(move(probe)), values(move(values)), collate(collate)
// Constructor
{
}
//---------------------------------------------------------------------------
void InExpression::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void InExpression::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
BinaryExpression::BinaryExpression(unique_ptr<Expression> left, unique_ptr<Expression> right, Type resultType, Operation op)
   : Expression(resultType), left(move(left)), right(move(right)), op(op)
// Constructor
{
}
//---------------------------------------------------------------------------
void BinaryExpression::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void BinaryExpression::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
UnaryExpression::UnaryExpression(unique_ptr<Expression> input, Type resultType, Operation op)
   : Expression(resultType), input(move(input)), op(op)
// Constructor
{
}
//---------------------------------------------------------------------------
void UnaryExpression::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void UnaryExpression::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
ExtractExpression::ExtractExpression(unique_ptr<Expression> input, Part part)
   : Expression(Type::getInteger().withNullable(input->getType().isNullable())), input(move(input)), part(part)
// Constructor
{
}
//---------------------------------------------------------------------------
void ExtractExpression::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void ExtractExpression::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
SubstrExpression::SubstrExpression(unique_ptr<Expression> value, unique_ptr<Expression> from, unique_ptr<Expression> len)
   : Expression(value->getType().withNullable(value->getType().isNullable() || (from ? from->getType().isNullable() : false) || (len ? len->getType().isNullable() : false))), value(move(value)), from(move(from)), len(move(len))
// Constructor
{
}
//---------------------------------------------------------------------------
void SubstrExpression::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void SubstrExpression::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
SimpleCaseExpression::SimpleCaseExpression(unique_ptr<Expression> value, Cases cases, unique_ptr<Expression> defaultValue)
   : Expression(defaultValue->getType()), value(move(value)), cases(move(cases)), defaultValue(move(defaultValue))
// Constructor
{
}
//---------------------------------------------------------------------------
void SimpleCaseExpression::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void SimpleCaseExpression::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
SearchedCaseExpression::SearchedCaseExpression(Cases cases, unique_ptr<Expression> defaultValue)
   : Expression(defaultValue->getType()), cases(move(cases)), defaultValue(move(defaultValue))
// Constructor
{
}
//---------------------------------------------------------------------------
void SearchedCaseExpression::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void SearchedCaseExpression::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
Aggregate::Aggregate(unique_ptr<Operator> input, vector<Aggregation> aggregates, unique_ptr<Expression> computation)
   : Expression(computation->getType()), input(move(input)), aggregates(move(aggregates)), computation(move(computation))
// Constructor
{
}
//---------------------------------------------------------------------------
void Aggregate::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void Aggregate::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
ForeignCall::ForeignCall(string name, Type returnType, vector<unique_ptr<Expression>> arguments, CallType callType)
   : Expression(returnType), name(std::move(name)), arguments(std::move(arguments)), callType(callType)
// Constructor
{
}
//---------------------------------------------------------------------------
void ForeignCall::traverse(ExpressionVisitor& visitor) { visitor.visit(*this); }
void ForeignCall::traverse(ConstExpressionVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
