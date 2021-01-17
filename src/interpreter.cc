#include "interpreter.h"
#include <cstring>
#include <cmath>
#include <algorithm>

namespace lasm {
    Interpreter::Interpreter(BaseError &onError, BaseInstructionSet &is, InterpreterCallback *callback):
        onError(onError), instructions(is), callback(callback),
        globals(std::make_shared<Enviorment>(Enviorment())), enviorment(globals) {
        initGlobals();
    }

    void Interpreter::initGlobals() {
        auto hi = LasmObject(CALLABLE_O, std::static_pointer_cast<Callable>(std::shared_ptr<NativeHi>(new NativeHi())));
        globals->define("hi", hi);

        auto lo = LasmObject(CALLABLE_O, std::static_pointer_cast<Callable>(std::shared_ptr<NativeLo>(new NativeLo())));
        globals->define("lo", lo);

        auto address = LasmObject(CALLABLE_O, std::static_pointer_cast<Callable>(
                    std::shared_ptr<NativeAddress>(new NativeAddress())));
        globals->define("_A", address);
    }

    std::vector<InstructionResult> Interpreter::interprete(std::vector<std::shared_ptr<Stmt>> stmts, int pass) {
        try {
            for (auto stmt : stmts) {
                execute(stmt);
            }
        } catch (LasmException &e) {
            onError.onError(e.getType(), e.getToken(), &e);
        }
        return code;
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
            case BIN_AND:
                if (right.isScalar() && left.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() & right.toNumber());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, right.getType(), expr->op);
                }
            case BIN_OR:
                if (right.isScalar() && left.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() | right.toNumber());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, right.getType(), expr->op);
                }
            case BIN_XOR:
                if (right.isScalar() && left.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() ^ right.toNumber());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, right.getType(), expr->op);
                }
            case BIN_SHIFT_LEFT:
                if (right.isScalar() && left.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() << right.toNumber());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, right.getType(), expr->op);
                }
            case BIN_SHIFT_RIGHT:
                if (right.isScalar() && left.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() >> right.toNumber());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, right.getType(), expr->op);
                }
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
            case BIN_NOT:
                if (right.isScalar()) {
                    return LasmObject(NUMBER_O, ~right.toNumber());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, right.getType(), expr->op);
                }
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

    std::any Interpreter::visitLogical(LogicalExpr *expr) {
        auto left = evaluate(expr->left);

        if (expr->op->getType() == OR) {
            if (left.isTruthy()) {
                return left;
            }
        } else {
            if (!left.isTruthy()) {
                return left;
            }
        }

        return evaluate(expr->right);
    }

    std::any Interpreter::visitCall(CallExpr *expr) {
        auto callee = evaluate(expr->callee);

        std::vector<LasmObject> arguments;
        for (auto arg : expr->arguments) {
            arguments.push_back(evaluate(arg));
        }

        if (callee.getType() != CALLABLE_O) {
            throw LasmNotCallable(expr->paren);
        }

        auto function = callee.toCallable();

        if (function->getArity() != arguments.size()) {
            throw LasmArityError(expr->paren);
        }

        return function->call(this, arguments);
    }

    std::any Interpreter::visitList(ListExpr *expr) {
        auto values = std::make_shared<std::vector<LasmObject>>(std::vector<LasmObject>());

        // evaluate all array members
        for (auto init : expr->list) {
            values->push_back(evaluate(init));
        }

        return LasmObject(LIST_O, values);
    }

    std::any Interpreter::visitIndex(IndexExpr *expr) {
        auto value = evaluate(expr->object);
        auto index = evaluate(expr->index);

        if (!index.isNumber()) {
            throw LasmTypeError(std::vector<ObjectType> {NUMBER_O}, index.getType(), expr->token);
        }

        LasmObject result(NIL_O, nullptr);
        if (value.isString()) {
            if ((unsigned long)value.toString().length() < (unsigned long)index.toNumber()) {
                throw LasmException(INDEX_OUT_OF_BOUNDS, expr->token);
            }
            return LasmObject(NUMBER_O, (lasmNumber)value.toString().at(index.toNumber()));
        } else if (value.isList()) {
            if ((unsigned long)value.toList()->size() < (unsigned long)index.toNumber()) {
                throw LasmException(INDEX_OUT_OF_BOUNDS, expr->token);
            }
            return value.toList()->at(index.toNumber());
        } else {
            throw LasmTypeError(std::vector<ObjectType> {STRING_O, LIST_O}, value.getType(), expr->token);
        }

        return result;
    }

    std::any Interpreter::visitIndexAssign(IndexAssignExpr *expr) {
        auto value = evaluate(expr->value);
        auto index = evaluate(expr->index);
        auto object = evaluate(expr->object);
        if (!index.isNumber()) {
            throw LasmTypeError(std::vector<ObjectType> {NUMBER_O}, index.getType(), expr->token);
        }

        if (object.isList()) {
            if ((unsigned long)object.toList()->size() < (unsigned long)index.toNumber()) {
                throw LasmException(INDEX_OUT_OF_BOUNDS, expr->token);
            }
            object.toList()->at(index.toNumber()) = value;
            return value;
        } else {
            throw LasmTypeError(std::vector<ObjectType> {STRING_O, LIST_O}, object.getType(), expr->token);
        }

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

    std::any Interpreter::visitIf(IfStmt *stmt) {
        if (evaluate(stmt->condition).isTruthy()) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch.get()) {
            execute(stmt->elseBranch);
        }
        return std::any();
    }

    std::any Interpreter::visitWhile(WhileStmt *stmt) {
        while (evaluate(stmt->condition).isTruthy()) {
            execute(stmt->body);
        }
        return std::any();
    }

    std::any Interpreter::visitFunction(FunctionStmt *stmt) {
        auto fn = std::make_shared<LasmFunction>(LasmFunction(stmt));
        LasmObject obj(CALLABLE_O, std::static_pointer_cast<Callable>(fn));
        enviorment->define(stmt->name->getLexeme(), obj);
        return std::any();
    }

    std::any Interpreter::visitReturn(ReturnStmt *stmt) {
        LasmObject value(NIL_O, nullptr);

        if (stmt->value.get()) {
            value = evaluate(stmt->value);
        }
        // TODO this is kinda ugly but functional
        throw Return(value);
    }

    std::any Interpreter::visitInstruction(InstructionStmt *stmt) {
        // TODO catch unresolved labels, return a deep clone of the current enviorment
        // and set unresolved flag along with the expression.
        // after assembly ends do a second pass and attempt to
        // resolve again
        code.push_back(instructions.generate(this, stmt->info, stmt));
        return std::any();
    }

    std::any Interpreter::visitAlign(AlignStmt *stmt) {
        auto alignTo = evaluate(stmt->alignTo);
        auto fillValue = evaluate(stmt->fillValue);

        if (!alignTo.isScalar()) {
            throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, alignTo.getType(), stmt->token);
        } else if (!fillValue.isScalar()) {
            throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, fillValue.getType(), stmt->token);
        } else if (fillValue.toNumber() > 0xFF) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->token);
        }

        unsigned long size = 0;
        // fill until address % fillValue == 0
        while ((address % (unsigned long)alignTo.toNumber()) != 0) {
            address++;
            size++;
        }

        // make byte array with fill value as instruction result
        std::shared_ptr<char[]> data(new char[size]);
        memset(data.get(), fillValue.toNumber(), size);
        code.push_back(InstructionResult(data, size, getAddress()-size, stmt->token));

        return std::any();
    }

    std::any Interpreter::visitFill(FillStmt *stmt) {
        auto fillTo = evaluate(stmt->fillAddress);
        auto fillValue = evaluate(stmt->fillValue);

        if (!fillTo.isScalar()) {
            throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, fillTo.getType(), stmt->token);
        } else if (!fillValue.isScalar()) {
            throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, fillValue.getType(), stmt->token);
        } else if (fillValue.toNumber() > 0xFF || (unsigned long)fillTo.toNumber() <= address) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->token);
        }

        // make data of address-fillTo
        unsigned long size = (unsigned long)fillTo.toNumber() - address;
        std::shared_ptr<char[]> data(new char[size]);
        memset(data.get(), fillValue.toNumber(), size);
        address += size;
        code.push_back(InstructionResult(data, size, getAddress()-size, stmt->token));

        return std::any();
    }

    std::any Interpreter::visitOrg(OrgStmt *stmt) {
        auto address = evaluate(stmt->address);
        if (!address.isScalar()) {
            throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, address.getType(), stmt->token);
        }
        this->address = address.toNumber();
        return std::any();
    }

/**
 * uncomment relevant line at compile time
 */
#define NATIVE_BO LITTLE
// #define NATIVE_BO BIG

    // TODO test endianess
    std::any Interpreter::visitDefineByte(DefineByteStmt *stmt) {
        // loop all exprs. each entry gets a node as code
        for (auto value : stmt->values) {
            auto evaluated = evaluate(value);
            std::shared_ptr<char[]> data;

            if (evaluated.isString()) {
                data = std::shared_ptr<char[]>(new char[evaluated.toString().length()+1]);
                // for string we ignore endianess anyway
                strncpy(data.get(), evaluated.toString().c_str(), evaluated.toString().length()+1);
                code.push_back(InstructionResult(data, evaluated.toString().length()+1, getAddress(), stmt->token));
                address += evaluated.toString().length()+1;
            } else if (evaluated.isScalar() || evaluated.isBool()) {
                data = std::shared_ptr<char[]>(new char[stmt->size]);
                switch (stmt->size) {
                    case 1: {
                                char value = 0;
                                if (evaluated.isBool()) {
                                    value = evaluated.toBool();
                                } else if (evaluated.isNumber()) {
                                    value = evaluated.toNumber();
                                } else {
                                    throw LasmException(VALUE_OUT_OF_RANGE, stmt->token);
                                }
                                memcpy(data.get(), &value, stmt->size);
                                break;
                            }
                    case 2: {
                                short value = 0;
                                if (evaluated.isBool()) {
                                    value = evaluated.toBool();
                                } else if (evaluated.isNumber()) {
                                    value = evaluated.toNumber();
                                } else {
                                    throw LasmException(VALUE_OUT_OF_RANGE, stmt->token);
                                }
                                memcpy(data.get(), &value, stmt->size);
                                break;
                            }
                    case 4: {
                                int value = 0;
                                if (evaluated.isBool()) {
                                    value = evaluated.toBool();
                                } else if (evaluated.isNumber()) {
                                    value = evaluated.toNumber();
                                } else if (evaluated.isReal()) {
                                    float evalFloat = evaluated.toReal();
                                    memcpy(&value, &evalFloat, stmt->size);
                                } else {
                                    throw LasmException(VALUE_OUT_OF_RANGE, stmt->token);
                                }
                                memcpy(data.get(), &value, stmt->size);
                                break;
                            }
                    case 8: {
                                long value = 0;
                                if (evaluated.isBool()) {
                                    value = evaluated.toBool();
                                } else if (evaluated.isNumber()) {
                                    value = evaluated.toNumber();
                                } else if (evaluated.isReal()) {
                                    double evalFloat = evaluated.toReal();
                                    memcpy(&value, &evalFloat, stmt->size);
                                } else {
                                    throw LasmException(VALUE_OUT_OF_RANGE, stmt->token);
                                }
                                memcpy(data.get(), &value, stmt->size);
                                break;
                            }
                    default:
                        throw LasmException(VALUE_OUT_OF_RANGE, stmt->token);
                }

                // is the required endianess the same as the native endianess?
                // TODO test this! maybe on a powerpc machine?
                if (stmt->endianess != NATIVE_BO) {
                    // swap time!
                    std::shared_ptr<char[]> swapped(new char[stmt->size]);
                    std::reverse_copy(data.get(), data.get()+stmt->size, swapped.get());
                    data = swapped;
                }

                code.push_back(InstructionResult(data, stmt->size, getAddress(), stmt->token));
                address += stmt->size;
            } else {
                throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O, BOOLEAN_O, STRING_O},
                        evaluated.getType(), stmt->token);
            }
        }

        return std::any();
    }

    std::any Interpreter::visitBss(BssStmt *stmt) {
        auto startAddress = evaluate(stmt->startAddress);
        if (!startAddress.isNumber()) {
            throw LasmTypeError(std::vector<ObjectType> {NUMBER_O}, startAddress.getType(), stmt->token);
        }

        // define every value
        for (auto declaration : stmt->declarations) {
            auto value = evaluate(declaration->init);
            if (!value.isNumber()) {
                throw LasmTypeError(std::vector<ObjectType> {NUMBER_O}, value.getType(), declaration->name);
            }
            // add start address to it
            enviorment->define(declaration->name->getLexeme(), startAddress);
            startAddress = LasmObject(NUMBER_O, value.toNumber() + startAddress.toNumber());
        }

        return std::any();
    }

    std::any Interpreter::visitLabel(LabelStmt *stmt) {
        stmt->value = LasmObject(NUMBER_O, (lasmNumber)address);
        enviorment->define(stmt->name->getLexeme().substr(0, stmt->name->getLexeme().length()-1), stmt->value);
        return std::any();
    }
}
