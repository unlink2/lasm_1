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
#include "callable.h"
#include "instruction.h"

namespace lasm {
    class InterpreterCallback {
        public:
            ~InterpreterCallback() {}

            virtual void onStatementExecuted(LasmObject *object) {}
    };

    class Interpreter: public ExprVisitor, public StmtVisitor {
        public:
            Interpreter(BaseError &onError, BaseInstructionSet &is, InterpreterCallback *callback=nullptr);

            void initGlobals();

            // TODO first pass:
            // resolve labels by adding them to a speical enviorment
            // each variable that was not resolvable the first time around gets a pointer to said enviorment
            // for loops each iteration creates a new label enviorment
            // each block creates a new label enviormnent
            // variable names shadow labels
            // second pass:
            // now all variables should be resolved
            std::vector<InstructionResult> interprete(std::vector<std::shared_ptr<Stmt>> stmts, int passes=2);

            void execPass(std::vector<std::shared_ptr<Stmt>> stmts);

            void execute(std::shared_ptr<Stmt> stmt);

            LasmObject evaluate(std::shared_ptr<Expr> expr);

            std::any visitBinary(BinaryExpr *expr);
            std::any visitUnary(UnaryExpr *expr);
            std::any visitLiteral(LiteralExpr *expr);
            std::any visitGrouping(GroupingExpr *expr);
            std::any visitVariable(VariableExpr *expr);
            std::any visitAssign(AssignExpr *expr);
            std::any visitLogical(LogicalExpr *expr);
            std::any visitCall(CallExpr *expr);
            std::any visitList(ListExpr *expr);
            std::any visitIndex(IndexExpr *expr);
            std::any visitIndexAssign(IndexAssignExpr *expr);

            std::any visitExpression(ExpressionStmt *stmt);
            std::any visitLet(LetStmt *stmt);
            std::any visitBlock(BlockStmt *stmt);
            std::any visitIf(IfStmt *stmt);
            std::any visitWhile(WhileStmt *stmt);
            std::any visitFunction(FunctionStmt *stmt);
            std::any visitReturn(ReturnStmt *stmt);
            std::any visitInstruction(InstructionStmt *stmt);
            std::any visitAlign(AlignStmt *stmt);
            std::any visitFill(FillStmt *stmt);
            std::any visitOrg(OrgStmt *stmt);
            std::any visitDefineByte(DefineByteStmt *stmt);
            std::any visitBss(BssStmt *stmt);
            std::any visitLabel(LabelStmt *stmt);

            void executeBlock(std::vector<std::shared_ptr<Stmt>> statements, std::shared_ptr<Enviorment> enviorment,
                    std::shared_ptr<Enviorment> labels=std::make_shared<Enviorment>(Enviorment()));

            unsigned long getAddress() { return address; }
            void setAddress(unsigned long newAddress) { address = newAddress; }

            std::vector<InstructionResult> getCode() { return code; }
            unsigned int getPass() { return pass; }
        private:
            BaseError &onError;
            BaseInstructionSet &instructions;
            InterpreterCallback *callback;

            // interpreter enviorment chain
            std::shared_ptr<Enviorment> globals;
            std::shared_ptr<Enviorment> enviorment;

            // interpreter label chain
            std::shared_ptr<Enviorment> globalLabels;
            std::shared_ptr<Enviorment> labels;

            unsigned long address = 0;
            unsigned short pass = 0;

            std::vector<InstructionResult> code;
    };
}

#endif
