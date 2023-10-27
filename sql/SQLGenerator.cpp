#include "SQLGenerator.hpp"
#include "SQLWriter.hpp"
#include "algebra/Expression.hpp"
#include "algebra/Operator.hpp"
#include <utility>
//---------------------------------------------------------------------------
using namespace std;
using namespace saneql::algebra;
//---------------------------------------------------------------------------
namespace saneql {
//---------------------------------------------------------------------------
// Operators 
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const TableScan& op) {
   out.write("(select ");
   bool first = true;
   for (auto& c : op.columns) {
      if (first)
         first = false;
      else
         out.write(", ");
      out.writeIdentifier(c.name);
      out.write(" as ");
      out.writeIU(c.iu.get());
   }
   out.write(" from ");
   out.writeIdentifier(op.name);
   out.write(")");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const Select& op) {
   out.write("(select * from ");
   op.input->traverse(*this);
   out.write(" s where ");
   op.condition->traverse(*this);
   out.write(")");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const Map& op) {
   out.write("(select *");
   for (auto& c : op.computations) {
      out.write(", ");
      c.value->traverse(*this);
      out.write(" as ");
      out.writeIU(c.iu.get());
   }
   out.write(" from ");
   op.input->traverse(*this);
   out.write(" s)");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const SetOperation& op) {
   using enum SetOperation::Op;
   auto dumpColumns = [this](const vector<unique_ptr<Expression>>& columns) {
      if (columns.empty()) {
         out.write("1");
      } else {
         bool first = true;
         for (auto& c : columns) {
            if (first)
               first = false;
            else
               out.write(", ");
            c->traverse(*this);
         }
      }
   };
   out.write("(select * from ((select ");
   dumpColumns(op.leftColumns);
   out.write(" from ");
   op.left->traverse(*this);
   out.write(" l) ");
   switch (op.op) {
      case Union: out.write("union"); break;
      case UnionAll: out.write("union all"); break;
      case Except: out.write("except"); break;
      case ExceptAll: out.write("except all"); break;
      case Intersect: out.write("intersect"); break;
      case IntersectAll: out.write("intersect all"); break;
   }
   out.write(" (select ");
   dumpColumns(op.rightColumns);
   out.write(" from ");
   op.right->traverse(*this);
   out.write(" r)) s");
   if (!op.resultColumns.empty()) {
      out.write("(");
      bool first = true;
      for (auto& c : op.resultColumns) {
         if (first)
            first = false;
         else
            out.write(", ");
         out.writeIU(c.get());
      }
      out.write(")");
   }
   out.write(")");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const Join& op) {
   using enum Join::JoinType;
   switch (op.joinType) {
      case Inner:
         out.write("(select * from ");
         op.left->traverse(*this);
         out.write(" l inner join ");
         op.right->traverse(*this);
         out.write(" r on ");
         op.condition->traverse(*this);
         out.write(")");
         break;
      case LeftOuter:
         out.write("(select * from ");
         op.left->traverse(*this);
         out.write(" l left outer join ");
         op.right->traverse(*this);
         out.write(" r on ");
         op.condition->traverse(*this);
         out.write(")");
         break;
      case RightOuter:
         out.write("(select * from ");
         op.left->traverse(*this);
         out.write(" l right outer join ");
         op.right->traverse(*this);
         out.write(" r on ");
         op.condition->traverse(*this);
         out.write(")");
         break;
      case FullOuter:
         out.write("(select * from ");
         op.left->traverse(*this);
         out.write(" l full outer join ");
         op.right->traverse(*this);
         out.write(" r on ");
         op.condition->traverse(*this);
         out.write(")");
         break;
      case LeftSemi:
         out.write("(select * from ");
         op.left->traverse(*this);
         out.write(" l where exists(select * from ");
         op.right->traverse(*this);
         out.write(" r where ");
         op.condition->traverse(*this);
         out.write("))");
         break;
      case RightSemi:
         out.write("(select * from ");
         op.right->traverse(*this);
         out.write(" r where exists(select * from ");
         op.left->traverse(*this);
         out.write(" l where ");
         op.condition->traverse(*this);
         out.write("))");
         break;
      case LeftAnti:
         out.write("(select * from ");
         op.left->traverse(*this);
         out.write(" l where not exists(select * from ");
         op.right->traverse(*this);
         out.write(" r where ");
         op.condition->traverse(*this);
         out.write("))");
         break;
      case RightAnti:
         out.write("(select * from ");
         op.right->traverse(*this);
         out.write(" r where not exists(select * from ");
         op.left->traverse(*this);
         out.write(" l where ");
         op.condition->traverse(*this);
         out.write("))");
         break;
   }
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const GroupBy& op) {
   using enum GroupBy::Op;
   out.write("(select ");
   bool first = true;
   for (auto& g : op.groupBy) {
      if (first)
         first = false;
      else
         out.write(", ");
      g.value->traverse(*this);
      out.write(" as ");
      out.writeIU(g.iu.get());
   }
   for (auto& a : op.aggregates) {
      if (first)
         first = false;
      else
         out.write(", ");
      switch (a.op) {
         case CountStar: out.write("count(*)"); break;
         case Count: out.write("count("); break;
         case CountDistinct: out.write("count(distinct "); break;
         case Sum: out.write("sum("); break;
         case SumDistinct: out.write("sum(distinct "); break;
         case Avg: out.write("avg("); break;
         case AvgDistinct: out.write("avg(distinct "); break;
         case Min: out.write("min("); break;
         case Max: out.write("max("); break;
      }
      if (a.op != CountStar) {
         a.value->traverse(*this);
         out.write(")");
      }
      out.write(" as ");
      out.writeIU(a.iu.get());
   }
   out.write(" from ");
   op.input->traverse(*this);
   out.write(" s group by ");
   if (op.groupBy.empty()) {
      out.write("true");
   } else {
      for (unsigned index = 0, limit = op.groupBy.size(); index < limit; ++index) {
         if (index) out.write(", ");
         out.write(to_string(index + 1));
      }
   }
   out.write(")");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const Sort& op) {
   out.write("(select * from ");
   op.input->traverse(*this);
   out.write(" s");
   if (!op.order.empty()) {
      out.write(" order by ");
      bool first = true;
      for (auto& o : op.order) {
         if (first)
            first = false;
         else
            out.write(", ");
         o.value->traverse(*this);
         if (o.collate != Collate{}) out.write(" collate TODO"); // TODO
         if (o.descending) out.write(" desc");
      }
   }
   if (op.limit.has_value()) {
      out.write(" limit ");
      out.write(to_string(*op.limit));
   }
   if (op.offset.has_value()) {
      out.write(" offset ");
      out.write(to_string(*op.offset));
   }
   out.write(")");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const Window& op) {
   using WindowOp = Window::WindowOp;
   using Op = Window::Op;
   auto aggr = [this](const char* name, const Window::Aggregation& a, bool distinct = false) {
      out.write(name);
      out.write("(");
      if (distinct) out.write("distinct ");
      a.value->traverse(*this);
      out.write(")");
   };
   out.write("(select *");
   for (auto& a : op.aggregates) {
      out.write(", ");
      switch (static_cast<WindowOp>(a.op)) {
         case Op::CountStar: out.write("count(*)"); break;
         case Op::Count: aggr("count", a); break;
         case Op::CountDistinct: aggr("count", a, true); break;
         case Op::Sum: aggr("sum", a); break;
         case Op::SumDistinct: aggr("sum", a, true); break;
         case Op::Avg: aggr("avg", a); break;
         case Op::AvgDistinct: aggr("avg", a, true); break;
         case Op::Min: aggr("min", a); break;
         case Op::Max: aggr("max", a); break;
         case Op::RowNumber: out.write("row_number()"); break;
      }
      out.write(" over (");
      if (!op.partitionBy.empty()) {
         out.write("partition by ");
         bool first = true;
         for (auto& p : op.partitionBy) {
            if (first)
               first = false;
            else
               out.write(", ");
            p->traverse(*this);
         }
      }
      if (!op.orderBy.empty()) {
         if (!op.partitionBy.empty()) out.write(" ");
         out.write("order by ");
         bool first = true;
         for (auto& o : op.orderBy) {
            if (first)
               first = false;
            else
               out.write(", ");
            o.value->traverse(*this);
            if (o.collate != Collate{}) out.write(" collate TODO"); // TODO
            if (o.descending) out.write(" desc");
         }
      }
      out.write(") as ");
      out.writeIU(a.iu.get());
   }
   out.write(" from ");
   op.input->traverse(*this);
   out.write(" s)");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const InlineTable& op) {
   out.write("(select * from (values");
   if (op.rowCount) {
      for (unsigned index = 0; index != op.rowCount; ++index) {
         if (index) out.write(",");
         if (!op.columns.empty()) {
            out.write("(");
            for (unsigned index2 = 0, limit2 = op.columns.size(); index2 != limit2; ++index2) {
               if (index2) out.write(", ");
               op.values[index * limit2 + index2]->traverse(*this);
            }
            out.write(")");
         } else {
            // PostgreSQL does not support empty tuples in values, add a dummy value
            out.write("(NULL)");
         }
      }
   } else {
      if (!op.columns.empty()) {
         out.write("(");
         for (unsigned index2 = 0, limit2 = op.columns.size(); index2 != limit2; ++index2) {
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
   for (auto& c : op.columns) {
      if (first)
         first = false;
      else
         out.write(", ");
      out.writeIU(c.get());
   }
   out.write(")");
   if (!op.rowCount) out.write(" limit 0");
   out.write(")");
};
//---------------------------------------------------------------------------
// Expressions 
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const IURef& expr) {
   out.writeIU(expr.iu);
}
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const ConstExpression& expr) {
   if (expr.null) {
      out.write("NULL");
   } else {
      auto type = expr.getType();
      if ((type.getType() != Type::Char) && (type.getType() != Type::Varchar) && (type.getType() != Type::Text)) {
         out.write("cast(");
         out.writeString(expr.value);
         out.write(" as ");
         out.writeType(type);
         out.write(")");
      } else {
         out.writeString(expr.value);
      }
   }
}
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const CastExpression& expr) {
   out.write("cast(");
   expr.input->traverse(*this);
   out.write(" as ");
   out.writeType(expr.getType());
   out.write(")");
}
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const ComparisonExpression& expr) {
   using enum ComparisonExpression::Mode;
   generateOperand(*expr.left);
   switch (expr.mode) {
      case Equal: out.write(" = "); break;
      case NotEqual: out.write(" <> "); break;
      case Is: out.write(" is not distinct from "); break;
      case IsNot: out.write(" is distinct from "); break;
      case Less: out.write(" < "); break;
      case LessOrEqual: out.write(" <= "); break;
      case Greater: out.write(" > "); break;
      case GreaterOrEqual: out.write(" >= "); break;
      case Like: out.write(" like "); break;
   }
   generateOperand(*expr.right);
}
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const BetweenExpression& expr){
   generateOperand(*expr.base);
   out.write(" between ");
   generateOperand(*expr.lower);
   out.write(" and ");
   generateOperand(*expr.upper);
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const InExpression& expr) {
   generateOperand(*expr.probe);
   out.write(" in (");
   bool first = true;
   for (auto& v : expr.values) {
      if (first)
         first = false;
      else
         out.write(", ");
      v->traverse(*this);
   }
   out.write(")");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const BinaryExpression& expr) {
   using enum BinaryExpression::Operation;
   generateOperand(*expr.left);
   switch (expr.op) {
      case Plus: out.write(" + "); break;
      case Minus: out.write(" - "); break;
      case Mul: out.write(" * "); break;
      case Div: out.write(" / "); break;
      case Mod: out.write(" % "); break;
      case Power: out.write(" ^ "); break;
      case Concat: out.write(" || "); break;
      case And: out.write(" and "); break;
      case Or: out.write(" or "); break;
   }
   generateOperand(*expr.right);
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const UnaryExpression& expr) {
   using enum UnaryExpression::Operation;
   switch (expr.op) {
      case Plus: out.write("+"); break;
      case Minus: out.write("-"); break;
      case Not: out.write(" not "); break;
   }
   generateOperand(*expr.input);
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const ExtractExpression& expr){
   using enum ExtractExpression::Part;
   out.write("extract(");
   switch (expr.part) {
      case Year: out.write("year"); break;
      case Month: out.write("month"); break;
      case Day: out.write("day"); break;
   }
   out.write(" from ");
   generateOperand(*expr.input);
   out.write(")");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const SubstrExpression& expr){
   out.write("substring(");
   expr.value->traverse(*this);
   if (expr.from) {
      out.write(" from ");
      expr.from->traverse(*this);
   }
   if (expr.len) {
      out.write(" for ");
      expr.len->traverse(*this);
   }
   out.write(")");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const SimpleCaseExpression& expr) {
   out.write("case ");
   generateOperand(*expr.value);
   for (auto& c : expr.cases) {
      out.write(" when ");
      c.first->traverse(*this);
      out.write(" then ");
      c.second->traverse(*this);
   }
   out.write(" else ");
   expr.defaultValue->traverse(*this);
   out.write(" end");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const SearchedCaseExpression& expr){
   out.write("case");
   for (auto& c : expr.cases) {
      out.write(" when ");
      c.first->traverse(*this);
      out.write(" then ");
      c.second->traverse(*this);
   }
   out.write(" else ");
   expr.defaultValue->traverse(*this);
   out.write(" end");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const Aggregate& expr) {
   using enum Aggregate::Op;
   out.write("(select ");
   expr.computation->traverse(*this);
   if (!expr.aggregates.empty()) {
      out.write(" from (select ");
      bool first = true;
      for (auto& a : expr.aggregates) {
         if (first)
            first = false;
         else
            out.write(", ");
         switch (a.op) {
            case CountStar: out.write("count(*)"); break;
            case Count: out.write("count("); break;
            case CountDistinct: out.write("count(distinct "); break;
            case Sum: out.write("sum("); break;
            case SumDistinct: out.write("sum(distinct "); break;
            case Avg: out.write("avg("); break;
            case AvgDistinct: out.write("avg(distinct "); break;
            case Min: out.write("min("); break;
            case Max: out.write("max("); break;
         }
         if (a.op != CountStar) {
            a.value->traverse(*this);
            out.write(")");
         }
         out.write(" as ");
         out.writeIU(a.iu.get());
      }
      out.write(" from ");
      expr.input->traverse(*this);
      out.write(" s");
      out.write(") s");
   }
   out.write(")");
};
//---------------------------------------------------------------------------
void SQLGenerator::visit(const const ForeignCall& expr) {
   using enum ForeignCall::CallType;
   switch (expr.callType) {
      case Function: {
         out.write(expr.name);
         out.write("(");
         bool first = true;
         for (auto& a : expr.arguments) {
            if(!std::exchange(first, false)) out.write(", ");
            a->traverse(*this);
         }
         out.write(")");
         break;
      }
      case LeftAssocOperator: { // ((a op b) op c) op d
         for (auto i = 0u; i != expr.arguments.size() - 2; ++i) {
            out.write("(");
         }
         generateOperand(*expr.arguments[0]);
         for (auto i = 1u; i != expr.arguments.size(); ++i) {
            out.write(" ");
            out.write(expr.name);
            out.write(" ");
            generateOperand(*expr.arguments[i]);
            if (i != expr.arguments.size() - 1) {
               out.write(")");
            }
         }
         break;
      }
      case RightAssocOperator: { // a op (b op (c op d))
         for (auto i = 0u; i != expr.arguments.size(); ++i) {
            generateOperand(*expr.arguments[i]);
            if (i != expr.arguments.size() - 1) {
               out.write(" ");
               out.write(expr.name);
               out.write(" ");
               out.write("(");
            }
         }
         for (auto i = 0u; i != expr.arguments.size() - 2; ++i) {
            out.write(")");
         }
         break;
      }
   }
};
//---------------------------------------------------------------------------
void SQLGenerator::generateOperand(const Operator& op) {
   out.write("(");
   op.traverse(*this);
   out.write(")");
}
//---------------------------------------------------------------------------
void SQLGenerator::generateOperand(const Expression& e) {
   out.write("(");
   e.traverse(*this);
   out.write(")");
}
//---------------------------------------------------------------------------
}  // namespace saneql
//---------------------------------------------------------------------------
