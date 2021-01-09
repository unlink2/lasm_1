#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include <iostream>
#include <memory>
#include <vector>
#include <any>
#include "instruction.h"
#include "expr.h"
#include "object.h"
#include "error.h"
#include "stmt.h"
#include "enviorment.h"

namespace lasm {
    class InterpreterCallback {
        public:
            ~InterpreterCallback() {}

            virtual void onStatementExecuted(LasmObject *object) {}
    };

    class Interpreter: public ExprVisitor, public StmtVisitor {
        public:
            Interpreter(BaseError &onError, BaseInstructionSet &is, InterpreterCallback *callback=nullptr);

            // TODO make this return a list of opcode results
            // this can then be consumed by a code generator
            // or be converted to a list file
            void interprete(std::vector<std::shared_ptr<Stmt>> stmts);

            void execute(std::shared_ptr<Stmt> stmt);

            LasmObject evaluate(std::shared_ptr<Expr> expr);

            std::any visitBinary(BinaryExpr *expr);
            std::any visitUnary(UnaryExpr *expr);
            std::any visitLiteral(LiteralExpr *expr);
            std::any visitGrouping(GroupingExpr *expr);
            std::any visitVariable(VariableExpr *expr);
            std::any visitAssign(AssignExpr *expr);

            std::any visitExpression(ExpressionStmt *stmt);
            std::any visitLet(LetStmt *stmt);
            std::any visitBlock(BlockStmt *stmt);
        private:
            void executeBlock(std::vector<std::shared_ptr<Stmt>> statements, std::shared_ptr<Enviorment> enviorment);

            BaseError &onError;
            BaseInstructionSet &instructions;
            InterpreterCallback *callback;
            std::shared_ptr<Enviorment> enviorment;
    };
}

#endif
