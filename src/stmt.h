#ifndef __STMT_H__
#define __STMT_H__

#include <iostream>
#include <any>
#include <memory>
#include "expr.h"

namespace lasm {
    enum StmtType {
        EXPRESSION_STMT,
        LET_STMT,
        BLOCK_STMT
    };

    class StmtVisitor;

    class Stmt {
        public:
            Stmt(StmtType type):
                type(type) {}

            template<typename T>
            T* castTo() {
                return static_cast<T>(this);
            }

            virtual std::any accept(StmtVisitor *visitor) { return std::any(nullptr); }

            StmtType getType() {
                return type;
            }
        private:
            StmtType type;
    };

    class ExpressionStmt: public Stmt {
        public:
            ExpressionStmt(std::shared_ptr<Expr> expr):
                Stmt::Stmt(EXPRESSION_STMT), expr(expr) {}

            virtual std::any accept(StmtVisitor *visitor);

            std::shared_ptr<Expr> expr;
    };

    class LetStmt: public Stmt {
        public:
            LetStmt(std::shared_ptr<Token> name, std::shared_ptr<Expr> init):
                Stmt::Stmt(LET_STMT), name(name), init(init) {}

            virtual std::any accept(StmtVisitor *visitor);

            std::shared_ptr<Token> name;
            std::shared_ptr<Expr> init;
    };

    class BlockStmt: public Stmt {
        public:
            BlockStmt(std::vector<std::shared_ptr<Stmt>> statements):
                Stmt::Stmt(BLOCK_STMT), statements(statements) {}

            virtual std::any accept(StmtVisitor *visitor);

            std::vector<std::shared_ptr<Stmt>> statements;
    };

    class StmtVisitor {
        public:
            virtual ~StmtVisitor () {}

            virtual std::any visitExpression(ExpressionStmt *stmt) { return std::any(nullptr); };
            virtual std::any visitLet(LetStmt *stmt) { return std::any(nullptr); };
            virtual std::any visitBlock(BlockStmt *stmt) { return std::any(nullptr); };
    };
}

#endif 
