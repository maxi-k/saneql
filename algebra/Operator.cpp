#include "algebra/Operator.hpp"
#include "sql/SQLWriter.hpp"
//---------------------------------------------------------------------------
// (c) 2023 Thomas Neumann
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace saneql::algebra {
//---------------------------------------------------------------------------
Operator::~Operator()
// Destructor
{
}
//---------------------------------------------------------------------------
TableScan::TableScan(string name, vector<Column> columns)
   : name(move(name)), columns(move(columns))
// Constructor
{
}
//---------------------------------------------------------------------------
void TableScan::generate(SQLWriter& out) const
// Generate SQL
{
   out.write("(select ");
   bool first = true;
   for (auto& c : columns) {
      if (first)
         first = false;
      else
         out.write(", ");
      out.writeIdentifier(c.name);
      out.write(" as ");
      out.writeIU(c.iu.get());
   }
   out.write(" from ");
   out.writeIdentifier(name);
   out.write(")");
}
//---------------------------------------------------------------------------
Select::Select(unique_ptr<Operator> input, unique_ptr<Expression> condition)
   : input(move(input)), condition(move(condition))
// Constructor
{
}
//---------------------------------------------------------------------------
void Select::generate(SQLWriter& out) const
// Generate SQL
{
   out.write("(select * from ");
   out.write(*input);
   out.write(" s where ");
   out.write(*condition);
   out.write(")");
}
//---------------------------------------------------------------------------
Map::Map(unique_ptr<Operator> input, vector<Entry> computations)
   : input(move(input)), computations(move(computations))
// Constructor
{
}
//---------------------------------------------------------------------------
void Map::generate(SQLWriter& out) const
// Generate SQL
{
   out.write("(select *");
   for (auto& c : computations) {
      out.write(", ");
      out.write(*c.value);
      out.write(" as ");
      out.writeIU(c.iu.get());
   }
   out.write(" from ");
   out.write(*input);
   out.write(" s)");
}
//---------------------------------------------------------------------------
Join::Join(unique_ptr<Operator> left, unique_ptr<Operator> right, unique_ptr<Expression> condition, JoinType joinType)
   : left(move(left)), right(move(right)), condition(move(condition)), joinType(joinType)
// Constructor
{
}
//---------------------------------------------------------------------------
void Join::generate(SQLWriter& out) const
// Generate SQL
{
   switch (joinType) {
      case JoinType::Inner:
         out.write("(select * from ");
         out.write(*left);
         out.write(" l inner join ");
         out.write(*right);
         out.write(" r on ");
         out.write(*condition);
         out.write(")");
         break;
      case JoinType::LeftOuter:
         out.write("(select * from ");
         out.write(*left);
         out.write(" l left outer join ");
         out.write(*right);
         out.write(" r on ");
         out.write(*condition);
         out.write(")");
         break;
      case JoinType::RightOuter:
         out.write("(select * from ");
         out.write(*left);
         out.write(" l right outer join ");
         out.write(*right);
         out.write(" r on ");
         out.write(*condition);
         out.write(")");
         break;
      case JoinType::FullOuter:
         out.write("(select * from ");
         out.write(*left);
         out.write(" l full outer join ");
         out.write(*right);
         out.write(" r on ");
         out.write(*condition);
         out.write(")");
         break;
      case JoinType::LeftSemi:
         out.write("(select * from ");
         out.write(*left);
         out.write(" l where exists(select * from ");
         out.write(*right);
         out.write(" r where ");
         out.write(*condition);
         out.write("))");
         break;
      case JoinType::RightSemi:
         out.write("(select * from ");
         out.write(*right);
         out.write(" r where exists(select * from ");
         out.write(*left);
         out.write(" l where ");
         out.write(*condition);
         out.write("))");
         break;
      case JoinType::LeftAnti:
         out.write("(select * from ");
         out.write(*left);
         out.write(" l where not exists(select * from ");
         out.write(*right);
         out.write(" r where ");
         out.write(*condition);
         out.write("))");
         break;
      case JoinType::RightAnti:
         out.write("(select * from ");
         out.write(*right);
         out.write(" r where not exists(select * from ");
         out.write(*left);
         out.write(" l where ");
         out.write(*condition);
         out.write("))");
         break;
   }
}
//---------------------------------------------------------------------------
GroupBy::GroupBy(unique_ptr<Operator> input, vector<Entry> groupBy, vector<Aggregation> aggregates)
   : input(move(input)), groupBy(move(groupBy)), aggregates(move(aggregates))
// Constructor
{
}
//---------------------------------------------------------------------------
void GroupBy::generate(SQLWriter& out) const
// Generate SQL
{
   out.write("(select ");
   bool first = true;
   for (auto& g : groupBy) {
      if (first)
         first = false;
      else
         out.write(", ");
      out.write(*g.value);
      out.write(" as ");
      out.writeIU(g.iu.get());
   }
   for (auto& a : aggregates) {
      if (first)
         first = false;
      else
         out.write(", ");
      switch (a.op) {
         case Op::CountStar: out.write("count(*)"); break;
         case Op::Count: out.write("count"); break;
         case Op::Sum: out.write("sum"); break;
         case Op::Avg: out.write("avg"); break;
         case Op::Min: out.write("min"); break;
         case Op::Max: out.write("max"); break;
      }
      if (a.op != Op::CountStar) {
         out.write("(");
         out.write(*a.value);
         out.write(")");
      }
      out.write(" as ");
      out.writeIU(a.iu.get());
   }
   out.write(" from ");
   out.write(*input);
   out.write(" s group by ");
   if (groupBy.empty()) {
      out.write("true");
   } else {
      for (unsigned index = 0, limit = groupBy.size(); index < limit; ++index) {
         if (index) out.write(", ");
         out.write(to_string(index + 1));
      }
   }
   out.write(")");
}
//---------------------------------------------------------------------------
Sort::Sort(unique_ptr<Operator> input, vector<Entry> order, optional<uint64_t> limit, optional<uint64_t> offset)
   : input(move(input)), order(move(order)), limit(limit), offset(offset)
// Constructor
{
}
//---------------------------------------------------------------------------
void Sort::generate(SQLWriter& out) const
// Generate SQL
{
   out.write("(select * from ");
   out.write(*input);
   out.write(" s");
   if (!order.empty()) {
      out.write(" order by ");
      bool first = true;
      for (auto& o : order) {
         if (first)
            first = false;
         else
            out.write(", ");
         out.write(*o.value);
         if (o.collate != Collate{}) out.write(" collate TODO"); // TODO
         if (o.descending) out.write(" desc");
      }
   }
   if (limit.has_value()) {
      out.write(" limit ");
      out.write(to_string(*limit));
   }
   if (offset.has_value()) {
      out.write(" offset ");
      out.write(to_string(*offset));
   }
   out.write(")");
}
//---------------------------------------------------------------------------
InlineTable::InlineTable(vector<unique_ptr<algebra::IU>> columns, vector<unique_ptr<algebra::Expression>> values, unsigned rowCount)
   : columns(move(columns)), values(move(values)), rowCount(move(rowCount))
// Constructor
{
}
//---------------------------------------------------------------------------
void InlineTable::generate(SQLWriter& out) const
// Generate SQL
{
   out.write("(select * from (values");
   if (rowCount) {
      for (unsigned index = 0; index != rowCount; ++index) {
         if (index) out.write(",");
         if (!columns.empty()) {
            out.write("(");
            for (unsigned index2 = 0, limit2 = columns.size(); index2 != limit2; ++index2) {
               if (index2) out.write(", ");
               out.write(*values[index * limit2 + index2]);
            }
            out.write(")");
         } else {
            // PostgreSQL does not support empty tuples in values, add a dummy value
            out.write("(NULL)");
         }
      }
   } else {
      if (!columns.empty()) {
         out.write("(");
         for (unsigned index2 = 0, limit2 = columns.size(); index2 != limit2; ++index2) {
            if (index2) out.write(", ");
            out.write("NULL");
         }
         out.write(")");
      } else {
         // PostgreSQL does not support empty tuples in values, add a dummy value
         out.write("(NULL)");
      }
   }
   out.write(") s(");
   bool first = true;
   for (auto& c : columns) {
      if (first)
         first = false;
      else
         out.write(", ");
      out.writeIU(c.get());
   }
   out.write(")");
   if (!rowCount) out.write(" limit 0");
   out.write(")");
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
