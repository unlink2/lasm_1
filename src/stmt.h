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
        BLOCK_STMT,
        IF_STMT,
        WHILE_STMT,
        FUNCTION_STMT,
        RETURN_STMT
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

    class IfStmt: public Stmt {
        public:
            IfStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch,
                    std::shared_ptr<Stmt> elseBranch):
                Stmt::Stmt(IF_STMT), condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

            virtual std::any accept(StmtVisitor *visitor);

            std::shared_ptr<Expr> condition;
            std::shared_ptr<Stmt> thenBranch;
            std::shared_ptr<Stmt> elseBranch;
    };

    class WhileStmt: public Stmt {
        public:
            WhileStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body):
                Stmt::Stmt(WHILE_STMT), condition(condition), body(body) {}

            virtual std::any accept(StmtVisitor *visitor);

            std::shared_ptr<Expr> condition;
            std::shared_ptr<Stmt> body;
    };

    class FunctionStmt: public Stmt {
        public:
            FunctionStmt(std::shared_ptr<Token> name, std::vector<std::shared_ptr<Token>> params,
                    std::vector<std::shared_ptr<Stmt>> body):
                Stmt::Stmt(FUNCTION_STMT), name(name), params(params), body(body) {}

            virtual std::any accept(StmtVisitor *visitor);

            std::shared_ptr<Token> name;
            std::vector<std::shared_ptr<Token>> params;
            std::vector<std::shared_ptr<Stmt>> body;
    };

    class ReturnStmt: public Stmt {
        public:
            ReturnStmt(std::shared_ptr<Token> keyword, std::shared_ptr<Expr> value):
                Stmt::Stmt(RETURN_STMT), keyword(keyword), value(value) {}

            virtual std::any accept(StmtVisitor *visitor);

            std::shared_ptr<Token> keyword;
            std::shared_ptr<Expr> value;
    };

    class StmtVisitor {
        public:
            virtual ~StmtVisitor () {}

            virtual std::any visitExpression(ExpressionStmt *stmt) { return std::any(nullptr); };
            virtual std::any visitLet(LetStmt *stmt) { return std::any(nullptr); };
            virtual std::any visitBlock(BlockStmt *stmt) { return std::any(nullptr); };
            virtual std::any visitIf(IfStmt *stmt) { return std::any(nullptr); };
            virtual std::any visitWhile(WhileStmt *stmt) { return std::any(nullptr); };
            virtual std::any visitFunction(FunctionStmt *stmt) { return std::any(nullptr); };
            virtual std::any visitReturn(ReturnStmt *stmt) { return std::any(nullptr); };
    };
}

#endif 
