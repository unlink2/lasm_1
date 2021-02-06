#ifndef __EXPR_H__
#define __EXPR_H__

#include "object.h"
#include "token.h"
#include <any>
#include <memory>
#include "enviorment.h"

namespace lasm {
    enum ExprType {
        BINARY_EXPR,
        GROUPING_EXPR,
        LITERAL_EXPR,
        UNARY_EXPR,
        VARIABLE_EXPR,
        ASSIGN_EXPR,
        CALL_EXPR,
        LIST_EXPR,
        INDEX_EXPR,
    };

    class ExprVisitor;

    class Expr {
        public:
            Expr(ExprType type):
                type(type) {}

            virtual ~Expr() {}

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

            std::shared_ptr<Enviorment> getEnv(unsigned long address);
            void setEnv(unsigned long address, std::shared_ptr<Enviorment> env);

            std::shared_ptr<Token> name;
        private:
            // is non-null if variable literal points to a label name
            // labels mapping address, label enviormnet
            std::map<unsigned long, std::shared_ptr<Enviorment>> labels;
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

    class ListExpr: public Expr {
        public:
            ListExpr(std::vector<std::shared_ptr<Expr>> list, std::shared_ptr<Token> paren):
                Expr::Expr(LIST_EXPR), list(list), paren(paren) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::vector<std::shared_ptr<Expr>> list;
            std::shared_ptr<Token> paren;
    };

    class IndexExpr: public Expr {
        public:
            IndexExpr(std::shared_ptr<Expr> object, std::shared_ptr<Expr> index, std::shared_ptr<Token> token):
                Expr::Expr(INDEX_EXPR), object(object), index(index), token(token) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::shared_ptr<Expr> object;
            std::shared_ptr<Expr> index;
            std::shared_ptr<Token> token;
    };

    class IndexAssignExpr: public IndexExpr {
        public:
            IndexAssignExpr(std::shared_ptr<Expr> object, std::shared_ptr<Expr> index,
                    std::shared_ptr<Expr> value, std::shared_ptr<Token> token):
                IndexExpr::IndexExpr(object, index, token), value(value) {}

            virtual std::any accept(ExprVisitor *visitor);

            std::shared_ptr<Expr> value;
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
            virtual std::any visitList(ListExpr *expr) { return std::any(nullptr); };
            virtual std::any visitIndex(IndexExpr *expr) { return std::any(nullptr); };
            virtual std::any visitIndexAssign(IndexAssignExpr *expr) { return std::any(nullptr); };
    };
}

#endif
