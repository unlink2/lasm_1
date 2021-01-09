#include "interpreter.h"

namespace lasm {
    Interpreter::Interpreter(BaseError &onError, BaseInstructionSet &is, InterpreterCallback *callback):
        onError(onError), instructions(is), callback(callback) {
            enviorment = std::make_shared<Enviorment>(Enviorment());
    }


    void Interpreter::interprete(std::vector<std::shared_ptr<Stmt>> stmts) {
        try {
            for (auto stmt : stmts) {
                execute(stmt);
            }
        } catch (LasmException &e) {
            onError.onError(e.getType(), e.getToken(), &e);
        }
    }

    void Interpreter::execute(std::shared_ptr<Stmt> stmt) {
        stmt->accept(this);
    }

    LasmObject Interpreter::evaluate(std::shared_ptr<Expr> expr) {
        return std::any_cast<LasmObject>(expr->accept(this));
    }

    std::any Interpreter::visitBinary(BinaryExpr *expr) {
        auto left = evaluate(expr->left);
        auto right = evaluate(expr->right);

        switch (expr->op->getType()) {
            case MINUS:
                // first number decides auto-cast
                if (left.isNumber() && left.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() - right.toNumber());
                } else if (left.isReal() && left.isScalar()) {
                    return LasmObject(REAL_O, left.toReal() - right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
                break;
            case SLASH:
                if (left.isNumber() && left.isScalar()) {
                    // integer division by 0 is not valid!
                    if (right.toNumber() == 0) {
                        throw LasmDivisionByZero(expr->op);
                    }
                    return LasmObject(NUMBER_O, left.toNumber() / right.toNumber());
                } else if (left.isReal() && left.isScalar()) {
                    return LasmObject(REAL_O, left.toReal() / right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
                break;
            case STAR:
                // first number decides auto-cast
                if (left.isNumber() && left.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() * right.toNumber());
                } else if (left.isReal() && left.isScalar()) {
                    return LasmObject(REAL_O, left.toReal() * right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
                break;
            case PLUS:
                // first number decides auto-cast
                if (left.isNumber() && left.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() + right.toNumber());
                } else if (left.isReal() && left.isScalar()) {
                    return LasmObject(REAL_O, left.toReal() + right.toReal());
                } else if (left.isString() && right.isString()) {
                    // string cat
                    return LasmObject(STRING_O, left.toString() + right.toString());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O, STRING_O}, left.getType(), expr->op);
                }
                break;

            // comparison
            case GREATER:
                if (left.isNumber() && left.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toNumber() > right.toNumber());
                } else if (right.isReal() && left.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toReal() > right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
            case LESS:
                if (left.isNumber() && left.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toNumber() < right.toNumber());
                } else if (right.isReal() && left.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toReal() < right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
            case GREATER_EQUAL:
                if (left.isNumber() && left.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toNumber() >= right.toNumber());
                } else if (right.isReal() && left.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toReal() >= right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
            case LESS_EQUAL:
                if (left.isNumber() && left.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toNumber() <= right.toNumber());
                } else if (right.isReal() && left.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toReal() <= right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
            case BANG_EQUAL:
                return LasmObject(BOOLEAN_O, !left.isEqual(right));
            case EQUAL_EQUAL:
                return LasmObject(BOOLEAN_O, left.isEqual(right));
            default:
                break;
        }

        // should be unreacbable
        return LasmObject(NIL_O, nullptr);
    }

    std::any Interpreter::visitUnary(UnaryExpr *expr) {
        auto right = evaluate(expr->right);

        switch (expr->op->getType()) {
            case MINUS:
                if (right.isReal()) {
                    return LasmObject(REAL_O, -right.toReal());
                } else if (right.isNumber()) {
                    return LasmObject(NUMBER_O, -right.toNumber());
                } else {
                    // type error!
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, right.getType(), expr->op);
                }
                break;
            case BANG:
                return LasmObject(BOOLEAN_O, !right.isTruthy());
            default:
                break;
        }

        // should be unreacbable
        return LasmObject(NIL_O, nullptr);
    }

    std::any Interpreter::visitLiteral(LiteralExpr *expr) {
        return expr->value;
    }

    std::any Interpreter::visitGrouping(GroupingExpr *expr) {
        return evaluate(expr->expression);
    }

    std::any Interpreter::visitVariable(VariableExpr *expr) {
        // TODO can we avoid copy constructor? does it matter?
        return LasmObject(enviorment->get(expr->name).get());
    }

    std::any Interpreter::visitAssign(AssignExpr *expr) {
        auto value = evaluate(expr->value);

        enviorment->assign(expr->name, value);
        return value;
    }

    std::any Interpreter::visitExpression(ExpressionStmt *stmt) {
        auto obj = std::any_cast<LasmObject>(evaluate(stmt->expr));

        if (callback) {
            callback->onStatementExecuted(&obj);
        }

        return std::any();
    }

    std::any Interpreter::visitLet(LetStmt *stmt) {
        LasmObject value = LasmObject(NIL_O, nullptr);
        if (stmt->init.get() != nullptr) {
            value = evaluate(stmt->init);
        }

        enviorment->define(stmt->name->getLexeme(), value);
        return std::any();
    }

    std::any Interpreter::visitBlock(BlockStmt *stmt) {
        executeBlock(stmt->statements, std::make_shared<Enviorment>(Enviorment(enviorment)));
        return std::any();
    }

    void Interpreter::executeBlock(std::vector<std::shared_ptr<Stmt>> statements,
            std::shared_ptr<Enviorment> enviorment) {
        auto previous = this->enviorment;

        this->enviorment = enviorment;
        for (auto statement : statements) {
            execute(statement);
        }
        this->enviorment = previous;
    }
}
