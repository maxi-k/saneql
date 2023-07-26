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
class SQLWriter;
//---------------------------------------------------------------------------
namespace algebra {
//---------------------------------------------------------------------------
/// An information unit
class IU {
   /// The type
   Type type;

   public:
   /// Constructor
   explicit IU(Type type) : type(type) {}

   /// Get the type
   const Type& getType() const { return type; }
};
//---------------------------------------------------------------------------
/// Base class for operators
class Operator : public SQLGenerator {
   public:
   /// Destructor
   virtual ~Operator();
};
//---------------------------------------------------------------------------
/// A table scan operator
class TableScan : public Operator {
   public:
   /// A column entry
   struct Column {
      /// The name
      std::string name;
      /// The IU
      std::unique_ptr<IU> iu;
   };

   private:
   /// The table name
   std::string name;
   /// The columns
   std::vector<Column> columns;

   public:
   /// Constructor
   TableScan(std::string name, std::vector<Column> columns);

   // Generate SQL
   void generate(SQLWriter& out) const override;
};
//---------------------------------------------------------------------------
/// A select operator
class Select : public Operator {
   /// The input
   std::unique_ptr<Operator> input;
   /// The filter condition
   std::unique_ptr<Expression> condition;

   public:
   /// Constructor
   Select(std::unique_ptr<Operator> input, std::unique_ptr<Expression> condition);

   // Generate SQL
   void generate(SQLWriter& out) const override;
};
//---------------------------------------------------------------------------
/// A map operator
class Map : public Operator {
   public:
   /// A regular computation
   struct Entry {
      /// The expression
      std::unique_ptr<algebra::Expression> value;
      /// The result IU
      std::unique_ptr<algebra::IU> iu;
   };

   private:
   /// The input
   std::unique_ptr<Operator> input;
   /// The computations
   std::vector<Entry> computations;

   public:
   /// Constructor
   Map(std::unique_ptr<Operator> input, std::vector<Entry> computations);

   // Generate SQL
   void generate(SQLWriter& out) const override;
};
//---------------------------------------------------------------------------
/// A join operator
class Join : public Operator {
   public:
   /// Join types
   enum class JoinType {
      Inner,
      LeftOuter,
      RightOuter,
      FullOuter,
      LeftSemi,
      RightSemi,
      LeftAnti,
      RightAnti
   };

   private:
   /// The input
   std::unique_ptr<Operator> left, right;
   /// The join condition
   std::unique_ptr<Expression> condition;
   /// The join type
   JoinType joinType;

   public:
   /// Constructor
   Join(std::unique_ptr<Operator> left, std::unique_ptr<Operator> right, std::unique_ptr<Expression> condition, JoinType joinType);

   // Generate SQL
   void generate(SQLWriter& out) const override;
};
//---------------------------------------------------------------------------
/// A group by operator
class GroupBy : public Operator {
   public:
   using Entry = Map::Entry;
   /// Known aggregation functions
   enum class Op {
      CountStar,
      Count,
      Sum,
      Min,
      Max,
      Avg
   };
   /// An aggregation
   struct Aggregation {
      /// The expression
      std::unique_ptr<algebra::Expression> value;
      /// The result IU
      std::unique_ptr<algebra::IU> iu;
      /// The operation
      Op op;
   };

   private:
   /// The input
   std::unique_ptr<Operator> input;
   /// The group by expressions
   std::vector<Entry> groupBy;
   /// The aggregates
   std::vector<Aggregation> aggregates;

   public:
   /// Constructor
   GroupBy(std::unique_ptr<Operator> input, std::vector<Entry> groupBy, std::vector<Aggregation> aggregates);

   // Generate SQL
   void generate(SQLWriter& out) const override;
};
//---------------------------------------------------------------------------
/// A sort operator
class Sort : public Operator {
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

   // Generate SQL
   void generate(SQLWriter& out) const override;
};
//---------------------------------------------------------------------------
/// An inline table definition
class InlineTable : public Operator {
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

   // Generate SQL
   void generate(SQLWriter& out) const override;
};
//---------------------------------------------------------------------------
}
}
//---------------------------------------------------------------------------
#endif
