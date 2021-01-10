#ifndef __EXPR_H__
#define __EXPR_H__

#include "object.h"
#include "token.h"
#include <any>
#include <memory>

namespace lasm {
    enum ExprType {
        BINARY_EXPR,
        GROUPING_EXPR,
        LITERAL_EXPR,
        UNARY_EXPR,
        VARIABLE_EXPR,
        ASSIGN_EXPR,
        CALL_EXPR
    };

    class ExprVisitor;

    class Expr {
        public:
            Expr(ExprType type):
                type(type) {}

            template<typename T>
            T* castTo() {
                return static_cast<T>(this);
            }

            virtual std::any accept(ExprVisitor *visitor) { return std::any(nullptr); }

            ExprType getType() {
                return type;
            }
        private:
            ExprType type;
    };

    class BinaryExpr: public Expr {
        public:
            BinaryExpr(std::shared_ptr<Expr> left=std::shared_ptr<Expr>(nullptr),
                    std::shared_ptr<Token> op=std::shared_ptr<Token>(nullptr),
                    std::shared_ptr<Expr> right=std::shared_ptr<Expr>(nullptr)):
                Expr::Expr(BINARY_EXPR), left(left), op(op), right(right) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::shared_ptr<Expr> left;
            std::shared_ptr<Token> op;
            std::shared_ptr<Expr> right;
    };

    class GroupingExpr: public Expr {
        public:
            GroupingExpr(std::shared_ptr<Expr> expression=std::shared_ptr<Expr>(nullptr)):
                Expr::Expr(GROUPING_EXPR), expression(expression) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::shared_ptr<Expr> expression;
    };

    class LiteralExpr: public Expr {
        public:
            LiteralExpr(LasmObject value=LasmObject(NIL_O, nullptr)):
                Expr::Expr(LITERAL_EXPR), value(value) {}

            virtual std::any accept(ExprVisitor *visitor);

            LasmObject value;
    };

    class UnaryExpr: public Expr {
        public:
            UnaryExpr(std::shared_ptr<Token> op=std::shared_ptr<Token>(nullptr),
                    std::shared_ptr<Expr> right=std::shared_ptr<Expr>(nullptr)):
                Expr::Expr(UNARY_EXPR), op(op), right(right) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::shared_ptr<Token> op;
            std::shared_ptr<Expr> right;
    };

    class VariableExpr: public Expr {
        public:
            VariableExpr(std::shared_ptr<Token> name=std::shared_ptr<Token>(nullptr)):
                Expr::Expr(VARIABLE_EXPR), name(name) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::shared_ptr<Token> name;
    };

    class AssignExpr: public Expr {
        public:
            AssignExpr(std::shared_ptr<Token> name, std::shared_ptr<Expr> value):
                Expr::Expr(ASSIGN_EXPR), name(name), value(value) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::shared_ptr<Token> name;
            std::shared_ptr<Expr> value;
    };

    class LogicalExpr: public Expr {
        public:
            LogicalExpr(std::shared_ptr<Expr> left=std::shared_ptr<Expr>(nullptr),
                    std::shared_ptr<Token> op=std::shared_ptr<Token>(nullptr),
                    std::shared_ptr<Expr> right=std::shared_ptr<Expr>(nullptr)):
                Expr::Expr(BINARY_EXPR), left(left), op(op), right(right) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::shared_ptr<Expr> left;
            std::shared_ptr<Token> op;
            std::shared_ptr<Expr> right;
    };

    class CallExpr: public Expr {
        public:
            CallExpr(std::shared_ptr<Expr> callee, std::shared_ptr<Token> paren,
                    std::vector<std::shared_ptr<Expr>> arguments):
                Expr::Expr(CALL_EXPR), callee(callee), paren(paren), arguments(arguments) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::shared_ptr<Expr> callee;
            std::shared_ptr<Token> paren;
            std::vector<std::shared_ptr<Expr>> arguments;
    };

    class ExprVisitor {
        public:
            virtual ~ExprVisitor () {}

            virtual std::any visitBinary(BinaryExpr *expr) { return std::any(nullptr); };
            virtual std::any visitUnary(UnaryExpr *expr) { return std::any(nullptr); };
            virtual std::any visitLiteral(LiteralExpr *expr) { return std::any(nullptr); };
            virtual std::any visitGrouping(GroupingExpr *expr) { return std::any(nullptr); };
            virtual std::any visitVariable(VariableExpr *expr) { return std::any(nullptr); };
            virtual std::any visitAssign(AssignExpr *expr) { return std::any(nullptr); };
            virtual std::any visitLogical(LogicalExpr *expr) { return std::any(nullptr); };
            virtual std::any visitCall(CallExpr *expr) { return std::any(nullptr); };
    };
}

#endif