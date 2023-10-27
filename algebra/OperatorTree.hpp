//---------------------------------------------------------------------------
#include <stdexcept>
namespace saneql {
//---------------------------------------------------------------------------
namespace algebra {
//---------------------------------------------------------------------------
// Operators
//---------------------------------------------------------------------------
class Operator;
class TableScan;
class Select;
class Map;
class SetOperation;
class Join;
class GroupBy;
class Sort;
class Window;
class InlineTable;
//---------------------------------------------------------------------------
struct OperatorVisitor {
   virtual ~OperatorVisitor() = default;
   virtual void visit(Operator&) {
      throw std::runtime_error("not implemented");
   }
   virtual void visit(TableScan& op) = 0;
   virtual void visit(Select& op) = 0;
   virtual void visit(Map& op) = 0;
   virtual void visit(SetOperation& op) = 0;
   virtual void visit(Join& op) = 0;
   virtual void visit(GroupBy& op) = 0;
   virtual void visit(Sort& op) = 0;
   virtual void visit(Window& op) = 0;
   virtual void visit(InlineTable& op) = 0;
};
//---------------------------------------------------------------------------
struct ConstOperatorVisitor {
   virtual ~ConstOperatorVisitor() = default;
   virtual void visit(const Operator&) {
      throw std::runtime_error("not implemented");
   }
   virtual void visit(const TableScan& op) = 0;
   virtual void visit(const Select& op) = 0;
   virtual void visit(const Map& op) = 0;
   virtual void visit(const SetOperation& op) = 0;
   virtual void visit(const Join& op) = 0;
   virtual void visit(const GroupBy& op) = 0;
   virtual void visit(const Sort& op) = 0;
   virtual void visit(const Window& op) = 0;
   virtual void visit(const InlineTable& op) = 0;
};
//---------------------------------------------------------------------------
}  // namespace algebra
//---------------------------------------------------------------------------
}  // namespace saneql
//---------------------------------------------------------------------------
