#ifndef H_saneql_Operator
#define H_saneql_Operator
//---------------------------------------------------------------------------
#include "algebra/Expression.hpp"
#include "infra/Schema.hpp"
#include <memory>
#include <optional>
//---------------------------------------------------------------------------
// SaneQL
// (c) 2023 Thomas Neumann
// SPDX-License-Identifier: BSD-3-Clause
//---------------------------------------------------------------------------
namespace saneql {
//---------------------------------------------------------------------------
struct OperatorVisitor;
struct ConstOperatorVisitor;
//---------------------------------------------------------------------------
namespace algebra {
//---------------------------------------------------------------------------
/// An information unit
struct IU {
   /// The type
   Type type;

   public:
   /// Constructor
   explicit IU(Type type) : type(type) {}

   /// Get the type
   const Type& getType() const { return type; }
};
//---------------------------------------------------------------------------
/// Base struct for operators
struct Operator {
   public:
   /// Destructor
   virtual ~Operator();

   // Traverse Operator
   virtual void traverse(OperatorVisitor& visitor) = 0;
   virtual void traverse(ConstOperatorVisitor& visitor) const = 0;
};
//---------------------------------------------------------------------------
/// A table scan operator
struct TableScan : public Operator {
   public:
   /// A column entry
   struct Column {
      /// The name
      std::string name;
      /// The IU
      std::unique_ptr<IU> iu;
   };

   /// The table name
   std::string name;
   /// The columns
   std::vector<Column> columns;

   public:
   /// Constructor
   TableScan(std::string name, std::vector<Column> columns);

   // Traverse Operator
   void traverse(OperatorVisitor& visitor) override;
   void traverse(ConstOperatorVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A select operator
struct Select : public Operator {
   /// The input
   std::unique_ptr<Operator> input;
   /// The filter condition
   std::unique_ptr<Expression> condition;

   public:
   /// Constructor
   Select(std::unique_ptr<Operator> input, std::unique_ptr<Expression> condition);

   // Traverse Operator
   void traverse(OperatorVisitor& visitor) override;
   void traverse(ConstOperatorVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A map operator
struct Map : public Operator {
   public:
   using Entry = AggregationLike::Entry;

   /// The input
   std::unique_ptr<Operator> input;
   /// The computations
   std::vector<Entry> computations;

   public:
   /// Constructor
   Map(std::unique_ptr<Operator> input, std::vector<Entry> computations);

   // Traverse Operator
   void traverse(OperatorVisitor& visitor) override;
   void traverse(ConstOperatorVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A set operation operator
struct SetOperation : public Operator {
   public:
   /// Operation types
   enum struct Op {
      Union,
      UnionAll,
      Except,
      ExceptAll,
      Intersect,
      IntersectAll
   };

   /// The input
   std::unique_ptr<Operator> left, right;
   /// The input columns
   std::vector<std::unique_ptr<Expression>> leftColumns, rightColumns;
   /// The result columns
   std::vector<std::unique_ptr<IU>> resultColumns;
   /// The operation
   Op op;

   public:
   /// Constructor
   SetOperation(std::unique_ptr<Operator> left, std::unique_ptr<Operator> right, std::vector<std::unique_ptr<Expression>> leftColumns, std::vector<std::unique_ptr<Expression>> rightColumns, std::vector<std::unique_ptr<IU>> resultColumns, Op op);

   // Traverse Operator
   void traverse(OperatorVisitor& visitor) override;
   void traverse(ConstOperatorVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A join operator
struct Join : public Operator {
   public:
   /// Join types
   enum struct JoinType {
      Inner,
      LeftOuter,
      RightOuter,
      FullOuter,
      LeftSemi,
      RightSemi,
      LeftAnti,
      RightAnti
   };

   /// The input
   std::unique_ptr<Operator> left, right;
   /// The join condition
   std::unique_ptr<Expression> condition;
   /// The join type
   JoinType joinType;

   public:
   /// Constructor
   Join(std::unique_ptr<Operator> left, std::unique_ptr<Operator> right, std::unique_ptr<Expression> condition, JoinType joinType);

   // Traverse Operator
   void traverse(OperatorVisitor& visitor) override;
   void traverse(ConstOperatorVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A group by operator
struct GroupBy : public Operator, public AggregationLike {
   /// The input
   std::unique_ptr<Operator> input;
   /// The group by expressions
   std::vector<Entry> groupBy;
   /// The aggregates
   std::vector<Aggregation> aggregates;

   public:
   /// Constructor
   GroupBy(std::unique_ptr<Operator> input, std::vector<Entry> groupBy, std::vector<Aggregation> aggregates);

   // Traverse Operator
   void traverse(OperatorVisitor& visitor) override;
   void traverse(ConstOperatorVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A sort operator
struct Sort : public Operator {
   public:
   struct Entry {
      /// The value to order by
      std::unique_ptr<algebra::Expression> value;
      /// The collate
      Collate collate;
      /// Descending?
      bool descending;
   };

   /// The input
   std::unique_ptr<Operator> input;
   /// The order
   std::vector<Entry> order;
   /// View
   std::optional<uint64_t> limit, offset;

   public:
   /// Constructor
   Sort(std::unique_ptr<Operator> input, std::vector<Entry> order, std::optional<uint64_t> limit, std::optional<uint64_t> offset);

   // Traverse Operator
   void traverse(OperatorVisitor& visitor) override;
   void traverse(ConstOperatorVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// A window operator
struct Window : public Operator, public AggregationLike {
   public:
   using Op = WindowOp;

   /// The input
   std::unique_ptr<Operator> input;
   /// The aggregates
   std::vector<Aggregation> aggregates;
   /// The partition by expressions
   std::vector<std::unique_ptr<Expression>> partitionBy;
   /// The order by expression
   std::vector<Sort::Entry> orderBy;

   public:
   /// Constructor
   Window(std::unique_ptr<Operator> input, std::vector<Aggregation> aggregates, std::vector<std::unique_ptr<Expression>> partitionBy, std::vector<Sort::Entry> orderBy);

   // Traverse Operator
   void traverse(OperatorVisitor& visitor) override;
   void traverse(ConstOperatorVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
/// An inline table definition
struct InlineTable : public Operator {
   public:
   /// The columns
   std::vector<std::unique_ptr<algebra::IU>> columns;
   /// The values
   std::vector<std::unique_ptr<algebra::Expression>> values;
   /// The row count
   unsigned rowCount;

   public:
   /// Constructor
   InlineTable(std::vector<std::unique_ptr<algebra::IU>> columns, std::vector<std::unique_ptr<algebra::Expression>> values, unsigned rowCount);

   // Traverse Operator
   void traverse(OperatorVisitor& visitor) override;
   void traverse(ConstOperatorVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
}
}
//---------------------------------------------------------------------------
#endif
