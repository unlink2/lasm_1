#ifndef __EXPR_H__
#define __EXPR_H__

#include "object.h"
#include "token.h"
#include <any>

namespace lasm {
    enum ExprType {
        BINARY_EXPR,
        GROUPING_EXPR,
        LITERAL_EXPR,
        UNARY_EXPR
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
            BinaryExpr(Expr *left=nullptr, Token *op=nullptr, Expr *right=nullptr):
                Expr::Expr(BINARY_EXPR), left(left), op(op), right(right) {}

            virtual std::any accept(ExprVisitor *visitor);

            Expr *left;
            Token *op;
            Expr *right;
    };

    class GroupingExpr: public Expr {
        public:
            GroupingExpr(Expr *expression=nullptr):
                Expr::Expr(GROUPING_EXPR), expression(expression) {}

            virtual std::any accept(ExprVisitor *visitor);

            Expr *expression;
    };

    class LiteralExpr: public Expr {
        public:
            LiteralExpr(LasmLiteral *value=nullptr):
                Expr::Expr(LITERAL_EXPR), value(value) {}

            virtual std::any accept(ExprVisitor *visitor);

            LasmLiteral *value;
    };

    class UnaryExpr: public Expr {
        public:
            UnaryExpr(Token *op=nullptr, Expr *right=nullptr):
                Expr::Expr(UNARY_EXPR), op(op), right(right) {}

            virtual std::any accept(ExprVisitor *visitor);

            Token *op;
            Expr *right;
    };

    class ExprVisitor {
        public:
            virtual ~ExprVisitor () {}

            virtual std::any visitBinary(BinaryExpr *expr) { return std::any(nullptr); };
            virtual std::any visitUnary(UnaryExpr *expr) { return std::any(nullptr); };
            virtual std::any visitLiteral(LiteralExpr *expr) { return std::any(nullptr); };
            virtual std::any visitGrouping(GroupingExpr *expr) { return std::any(nullptr); };
    };
}

#endif
