#include "algebra/OperatorTree.hpp"
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
void TableScan::traverse(OperatorVisitor& visitor) { visitor.visit(*this); }
void TableScan::traverse(ConstOperatorVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
Select::Select(unique_ptr<Operator> input, unique_ptr<Expression> condition)
   : input(move(input)), condition(move(condition))
// Constructor
{
}
//---------------------------------------------------------------------------
void Select::traverse(OperatorVisitor& visitor) { visitor.visit(*this); }
void Select::traverse(ConstOperatorVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
Map::Map(unique_ptr<Operator> input, vector<Entry> computations)
   : input(move(input)), computations(move(computations))
// Constructor
{
}
//---------------------------------------------------------------------------
void Map::traverse(OperatorVisitor& visitor) { visitor.visit(*this); }
void Map::traverse(ConstOperatorVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
SetOperation::SetOperation(unique_ptr<Operator> left, unique_ptr<Operator> right, vector<unique_ptr<Expression>> leftColumns, vector<unique_ptr<Expression>> rightColumns, vector<unique_ptr<IU>> resultColumns, Op op)
   : left(move(left)), right(move(right)), leftColumns(move(leftColumns)), rightColumns(move(rightColumns)), resultColumns(move(resultColumns)), op(op)
// Constructor
{
}
//---------------------------------------------------------------------------
void SetOperation::traverse(OperatorVisitor& visitor) { visitor.visit(*this); }
void SetOperation::traverse(ConstOperatorVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
Join::Join(unique_ptr<Operator> left, unique_ptr<Operator> right, unique_ptr<Expression> condition, JoinType joinType)
   : left(move(left)), right(move(right)), condition(move(condition)), joinType(joinType)
// Constructor
{
}
//---------------------------------------------------------------------------
void Join::traverse(OperatorVisitor& visitor) { visitor.visit(*this); }
void Join::traverse(ConstOperatorVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
GroupBy::GroupBy(unique_ptr<Operator> input, vector<Entry> groupBy, vector<Aggregation> aggregates)
   : input(move(input)), groupBy(move(groupBy)), aggregates(move(aggregates))
// Constructor
{
}
//---------------------------------------------------------------------------
void GroupBy::traverse(OperatorVisitor& visitor) { visitor.visit(*this); }
void GroupBy::traverse(ConstOperatorVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
Sort::Sort(unique_ptr<Operator> input, vector<Entry> order, optional<uint64_t> limit, optional<uint64_t> offset)
   : input(move(input)), order(move(order)), limit(limit), offset(offset)
// Constructor
{
}
//---------------------------------------------------------------------------
void Sort::traverse(OperatorVisitor& visitor) { visitor.visit(*this); }
void Sort::traverse(ConstOperatorVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
Window::Window(unique_ptr<Operator> input, vector<Aggregation> aggregates, vector<unique_ptr<Expression>> partitionBy, vector<Sort::Entry> orderBy)
   : input(move(input)), aggregates(move(aggregates)), partitionBy(move(partitionBy)), orderBy(move(orderBy))
// Constructor
{
}
//---------------------------------------------------------------------------
void Window::traverse(OperatorVisitor& visitor) { visitor.visit(*this); }
void Window::traverse(ConstOperatorVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
InlineTable::InlineTable(vector<unique_ptr<algebra::IU>> columns, vector<unique_ptr<algebra::Expression>> values, unsigned rowCount)
   : columns(move(columns)), values(move(values)), rowCount(move(rowCount))
// Constructor
{
}
//---------------------------------------------------------------------------
void InlineTable::traverse(OperatorVisitor& visitor) { visitor.visit(*this); }
void InlineTable::traverse(ConstOperatorVisitor& visitor) const { visitor.visit(*this); }
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
