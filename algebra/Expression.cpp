#include "algebra/Expression.hpp"
#include "algebra/Operator.hpp"
#include "sql/SQLWriter.hpp"
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
void Expression::generateOperand(SQLWriter& out)
// Generate SQL in a form that is suitable as operand
{
   out.write("(");
   out.write(*this);
   out.write(")");
}
//---------------------------------------------------------------------------
IURef::IURef(const IU* iu)
   : Expression(iu->getType()), iu(iu)
// Constructor
{
}
//---------------------------------------------------------------------------
void IURef::generate(SQLWriter& out) const
// Generate SQL
{
   out.writeIU(iu);
}
//---------------------------------------------------------------------------
void ConstExpression::generate(SQLWriter& out) const
// Generate SQL
{
   if (null) {
      out.write("NULL");
   } else {
      auto type = getType();
      bool needsCast = (type.getType() != Type::Char) && (type.getType() != Type::Varchar) && (type.getType() != Type::Text);
      if (needsCast) { out.write("cast("); }
      out.writeString(value);
      if (needsCast) {
         out.write(" as ");
         out.writeType(type);
         out.write(")");
      }
   }
}
//---------------------------------------------------------------------------
void CastExpression::generate(SQLWriter& out) const
// Generate SQL
{
   out.write("cast(");
   input->generateOperand(out);
   out.write(" as ");
   out.writeType(getType());
   out.write(")");
}
//---------------------------------------------------------------------------
void CastExpression::generate(SQLiteWriter& out) const
// Generate SQL
{
   auto type = getType();
   if (type.getType() == Type::Date) {
      out.write("unixepoch(");
      out.write(*input);
      out.write(")");
   } else if (type.getType() == Type::Interval) {
      out.write("unixepoch(0, ");
      out.write(*input);
      out.write(")");
   } else {
      generate(static_cast<SQLWriter&>(out));
   }
}
//---------------------------------------------------------------------------
ComparisonExpression::ComparisonExpression(unique_ptr<Expression> left, unique_ptr<Expression> right, Mode mode, Collate collate)
   : Expression(Type::getBool().withNullable((mode != Mode::Is) && (mode != Mode::IsNot) && (left->getType().isNullable() || right->getType().isNullable()))), left(move(left)), right(move(right)), mode(mode), collate(collate)
// Constructor
{
}
//---------------------------------------------------------------------------
void ComparisonExpression::generate(SQLWriter& out) const
// Generate SQL
{
   left->generateOperand(out);
   switch (mode) {
      case Mode::Equal: out.write(" = "); break;
      case Mode::NotEqual: out.write(" <> "); break;
      case Mode::Is: out.write(" is not distinct from "); break;
      case Mode::IsNot: out.write(" is distinct from "); break;
      case Mode::Less: out.write(" < "); break;
      case Mode::LessOrEqual: out.write(" <= "); break;
      case Mode::Greater: out.write(" > "); break;
      case Mode::GreaterOrEqual: out.write(" >= "); break;
   }
   right->generateOperand(out);
}
//---------------------------------------------------------------------------
BinaryExpression::BinaryExpression(unique_ptr<Expression> left, unique_ptr<Expression> right, Type resultType, Operation op)
   : Expression(resultType), left(move(left)), right(move(right)), op(op)
// Constructor
{
}
//---------------------------------------------------------------------------
void BinaryExpression::generate(SQLWriter& out) const
// Generate SQL
{
   left->generateOperand(out);
   switch (op) {
      case Operation::Plus: out.write(" + "); break;
      case Operation::Minus: out.write(" - "); break;
      case Operation::Mul: out.write(" * "); break;
      case Operation::Div: out.write(" / "); break;
      case Operation::Mod: out.write(" % "); break;
      case Operation::Power: out.write(" ^ "); break;
      case Operation::Concat: out.write(" || "); break;
      case Operation::And: out.write(" and "); break;
      case Operation::Or: out.write(" or "); break;
   }
   right->generateOperand(out);
}
//---------------------------------------------------------------------------
UnaryExpression::UnaryExpression(unique_ptr<Expression> input, Type resultType, Operation op)
   : Expression(resultType), input(move(input)), op(op)
// Constructor
{
}
//---------------------------------------------------------------------------
void UnaryExpression::generate(SQLWriter& out) const
// Generate SQL
{
   switch (op) {
      case Operation::Plus: out.write("+"); break;
      case Operation::Minus: out.write("-"); break;
      case Operation::Not: out.write(" not "); break;
   }
   input->generateOperand(out);
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
