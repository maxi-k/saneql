#ifndef H_saneql_Expression
#define H_saneql_Expression
//---------------------------------------------------------------------------
#include "infra/Schema.hpp"
#include "semana/Functions.hpp"
#include <memory>
//---------------------------------------------------------------------------
// SaneQL
// (c) 2023 Thomas Neumann
// SPDX-License-Identifier: BSD-3-Clause
//---------------------------------------------------------------------------
namespace saneql {
//---------------------------------------------------------------------------
enum class Collate : uint8_t;
//---------------------------------------------------------------------------
namespace algebra {
//---------------------------------------------------------------------------
class IU;
class Operator;
class ExpressionVisitor;
class ConstExpressionVisitor;
//---------------------------------------------------------------------------
/// Base struct for expressions
struct Expression {
   private:
   /// The type
   Type type;

   public:
   /// Constructor
   explicit Expression(Type type) : type(type) {}
   /// Destructor
   virtual ~Expression();

   /// Get the result type
   Type getType() const { return type; }

   /// Traverse Expressions
   virtual void traverse(ExpressionVisitor& visitor) = 0;
   virtual void traverse(ConstExpressionVisitor& visitor) const = 0;
};
//---------------------------------------------------------------------------
/// An IU reference
struct IURef : public Expression {
   /// The IU
   const IU* iu;

   public:
   /// Constructor
   IURef(const IU* iu);

   /// Get the IU
   const IU* getIU() const { return iu; }

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A constant value
struct ConstExpression : public Expression {
   /// The raw value
   std::string value;
   /// NULL?
   bool null;

   public:
   /// Constructor for non-null values
   ConstExpression(std::string value, Type type) : Expression(type), value(std::move(value)), null(false) {}
   /// Constructor for NULL values
   ConstExpression(std::nullptr_t, Type type) : Expression(type), null(true) {}

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A cast expression
struct CastExpression : public Expression {
   /// The input
   std::unique_ptr<Expression> input;

   public:
   /// Constructor
   CastExpression(std::unique_ptr<Expression> input, Type type) : Expression(type), input(move(input)) {}

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A comparison expression
struct ComparisonExpression : public Expression {
   public:
   /// Possible modes
   enum Mode {
      Equal,
      NotEqual,
      Is,
      IsNot,
      Less,
      LessOrEqual,
      Greater,
      GreaterOrEqual,
      Like
   };
   /// The input
   std::unique_ptr<Expression> left, right;
   /// The mode
   Mode mode;
   /// The collation
   Collate collate;

   public:
   /// Constructor
   ComparisonExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, Mode mode, Collate collate);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A between expression
struct BetweenExpression : public Expression {
   public:
   /// The input
   std::unique_ptr<Expression> base, lower, upper;
   /// The collation
   Collate collate;

   public:
   /// Constructor
   BetweenExpression(std::unique_ptr<Expression> base, std::unique_ptr<Expression> lower, std::unique_ptr<Expression> upper, Collate collate);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// An in expression
struct InExpression : public Expression {
   public:
   /// The input
   std::unique_ptr<Expression> probe;
   /// The values to check against
   std::vector<std::unique_ptr<Expression>> values;
   /// The collation
   Collate collate;

   public:
   /// Constructor
   InExpression(std::unique_ptr<Expression> probe, std::vector<std::unique_ptr<Expression>> values, Collate collate);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A binary expression
struct BinaryExpression : public Expression {
   public:
   /// Possible operations
   enum Operation {
      Plus,
      Minus,
      Mul,
      Div,
      Mod,
      Power,
      Concat,
      And,
      Or
   };
   /// The input
   std::unique_ptr<Expression> left, right;
   /// The mode
   Operation op;

   public:
   /// Constructor
   BinaryExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, Type resultType, Operation op);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// An unary expression
struct UnaryExpression : public Expression {
   public:
   /// Possible operations
   enum Operation {
      Plus,
      Minus,
      Not
   };
   /// The input
   std::unique_ptr<Expression> input;
   /// The mode
   Operation op;

   public:
   /// Constructor
   UnaryExpression(std::unique_ptr<Expression> input, Type resultType, Operation op);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// An extract expression
struct ExtractExpression : public Expression {
   public:
   /// Possible parts
   enum Part {
      Year,
      Month,
      Day
   };
   /// The input
   std::unique_ptr<Expression> input;
   /// The part
   Part part;

   public:
   /// Constructor
   ExtractExpression(std::unique_ptr<Expression> input, Part part);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A substring expression
struct SubstrExpression : public Expression {
   public:
   /// The input
   std::unique_ptr<Expression> value, from, len;

   public:
   /// Constructor
   SubstrExpression(std::unique_ptr<Expression> value, std::unique_ptr<Expression> from, std::unique_ptr<Expression> len);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A simple case expression
struct SimpleCaseExpression : public Expression {
   public:
   using Cases = std::vector<std::pair<std::unique_ptr<algebra::Expression>, std::unique_ptr<algebra::Expression>>>;

   /// The value to search
   std::unique_ptr<Expression> value;
   /// The cases
   Cases cases;
   /// The default result
   std::unique_ptr<Expression> defaultValue;

   public:
   /// Constructor
   SimpleCaseExpression(std::unique_ptr<Expression> value, Cases cases, std::unique_ptr<Expression> defaultValue);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A searched case expression
struct SearchedCaseExpression : public Expression {
   public:
   using Cases = SimpleCaseExpression::Cases;

   /// The cases
   Cases cases;
   /// The default result
   std::unique_ptr<Expression> defaultValue;

   public:
   /// Constructor
   SearchedCaseExpression(Cases cases, std::unique_ptr<Expression> defaultValue);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// Helper for aggregation steps
struct AggregationLike {
   /// A regular computation
   struct Entry {
      /// The expression
      std::unique_ptr<algebra::Expression> value;
      /// The result IU
      std::unique_ptr<algebra::IU> iu;
   };
   /// Known aggregation functions
   enum struct Op {
      CountStar,
      Count,
      CountDistinct,
      Sum,
      SumDistinct,
      Min,
      Max,
      Avg,
      AvgDistinct
   };
   /// Known window functions
   enum struct WindowOp {
      CountStar,
      Count,
      CountDistinct,
      Sum,
      SumDistinct,
      Min,
      Max,
      Avg,
      AvgDistinct,
      RowNumber
   };
   static_assert(static_cast<unsigned>(Op::AvgDistinct) == static_cast<unsigned>(WindowOp::AvgDistinct));

   /// An aggregation
   struct Aggregation {
      /// The expression
      std::unique_ptr<algebra::Expression> value;
      /// The result IU
      std::unique_ptr<algebra::IU> iu;
      /// The operation
      Op op;
   };
};
//---------------------------------------------------------------------------
/// An aggregate expression
struct Aggregate : public Expression, public AggregationLike {
   /// The input
   std::unique_ptr<Operator> input;
   /// The aggregates
   std::vector<Aggregation> aggregates;
   /// The final result computation
   std::unique_ptr<Expression> computation;

   public:
   /// Constructor
   Aggregate(std::unique_ptr<Operator> input, std::vector<Aggregation> aggregates, std::unique_ptr<Expression> computation);

   // Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A foreign call expression
struct ForeignCall : public Expression {
   // Type of the traversed call
   enum struct CallType { Function, LeftAssocOperator, RightAssocOperator };
   static constexpr CallType defaultType() { return CallType::Function; }

   /// The name of the declared function
   std::string name;
   /// The function call arguments
   std::vector<std::unique_ptr<Expression>> arguments;
   /// The call type
   CallType callType;

   public:
   /// Constructor
   ForeignCall(std::string name, Type returnType, std::vector<std::unique_ptr<Expression>> arguments, CallType callType);

   /// Traverse Expressions
   void traverse(ExpressionVisitor& visitor) override;
   void traverse(ConstExpressionVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
}
}
//---------------------------------------------------------------------------
#endif
