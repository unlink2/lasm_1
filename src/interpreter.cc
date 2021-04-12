#include "interpreter.h"
#include <cstring>
#include <cmath>
#include <algorithm>
#include "scanner.h"
#include "parser.h"

namespace lasm {
    Interpreter::Interpreter(BaseError &onError, BaseInstructionSet &is, InterpreterCallback *callback,
            FileReader *reader):
        onError(onError), instructions(is), callback(callback),
        globals(std::make_shared<Environment>(Environment())), environment(globals),
        globalLabels(std::make_shared<Environment>(Environment())), labels(globalLabels), reader(reader) {
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

        auto ord = LasmObject(CALLABLE_O, std::static_pointer_cast<Callable>(
                    std::shared_ptr<NativeOrd>(new NativeOrd())));
        globals->define("ord", ord);

        auto len = LasmObject(CALLABLE_O, std::static_pointer_cast<Callable>(
                    std::shared_ptr<NativeLen>(new NativeLen())));
        globals->define("len", len);

        auto setEnvName = LasmObject(CALLABLE_O, std::static_pointer_cast<Callable>(
                    std::shared_ptr<NativeSetEnvName>(new NativeSetEnvName())));
        globals->define("setScopeName", setEnvName);
    }

    std::vector<InstructionResult> Interpreter::interprete(std::vector<std::shared_ptr<Stmt>> stmts,
            bool abortOnError, int passes) {
        for (int i = 0; i < passes && (!onError.didError() || !abortOnError); i++) {
            execPass(stmts);
        }
        return code;
    }

    void Interpreter::execPass(std::vector<std::shared_ptr<Stmt>> stmts) {
        labels = globalLabels;
        environment = globals;

        environment->clear();
        labelTable.clear();
        labelTable.push_back(globalLabels);
        code.clear();
        initGlobals();
        address = 0;
        try {
            for (auto stmt : stmts) {
                execute(stmt);
            }
        } catch (LasmException &e) {
            onError.onError(e.getType(), e.getToken(), &e);
        }
        pass++;
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
                if (left.isNumber() && right.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() - right.toNumber());
                } else if (left.isReal() && right.isScalar()) {
                    return LasmObject(REAL_O, left.toReal() - right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
                break;
            case SLASH:
                if (left.isNumber() && right.isScalar()) {
                    // integer division by 0 is not valid!
                    if (right.toNumber() == 0) {
                        throw LasmDivisionByZero(expr->op);
                    }
                    return LasmObject(NUMBER_O, left.toNumber() / right.toNumber());
                } else if (left.isReal() && right.isScalar()) {
                    return LasmObject(REAL_O, left.toReal() / right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
                break;
            case PERCENT:
                if (left.isNumber() && right.isNumber()) {
                    if (right.toNumber() == 0) {
                        throw LasmDivisionByZero(expr->op);
                    }
                    return LasmObject(NUMBER_O, left.toNumber() % right.toNumber());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O}, left.getType(), expr->op);
                }
            case STAR:
                // first number decides auto-cast
                if (left.isNumber() && right.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() * right.toNumber());
                } else if (left.isReal() && right.isScalar()) {
                    return LasmObject(REAL_O, left.toReal() * right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
                break;
            case PLUS:
                // first number decides auto-cast
                if (left.isNumber() && right.isScalar()) {
                    return LasmObject(NUMBER_O, left.toNumber() + right.toNumber());
                } else if (left.isReal() && right.isScalar()) {
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
                if (left.isNumber() && right.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toNumber() > right.toNumber());
                } else if (right.isReal() && right.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toReal() > right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
            case LESS:
                if (left.isNumber() && right.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toNumber() < right.toNumber());
                } else if (left.isReal() && right.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toReal() < right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
            case GREATER_EQUAL:
                if (left.isNumber() && right.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toNumber() >= right.toNumber());
                } else if (left.isReal() && right.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toReal() >= right.toReal());
                } else {
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, left.getType(), expr->op);
                }
            case LESS_EQUAL:
                if (left.isNumber() && left.isScalar()) {
                    return LasmObject(BOOLEAN_O, left.toNumber() <= right.toNumber());
                } else if (left.isReal() && left.isScalar()) {
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

        auto sign = -1;
        switch (expr->op->getType()) {
            case PLUS:
                sign = 1;
            case MINUS:
                if (right.isReal()) {
                    return LasmObject(REAL_O, sign * right.toReal());
                } else if (right.isNumber()) {
                    return LasmObject(NUMBER_O, sign * right.toNumber());
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
        // label environment. used for n+1th pass
        // only set if it has not already been assigned
        bool wasFirstPass = false;
        if (!expr->getEnv(address).get() && pass == 0) {
            wasFirstPass = true; // if so do not throw
            expr->setEnv(address, labels);
        }
        try {
            // TODO can we avoid copy constructor? does it matter?
            return LasmObject(environment->get(expr->name).get());
        } catch (LasmUndefinedReference &e) {
            // attempt getting label by name, but only on second+ pass
            if (expr->getEnv(address).get() && !wasFirstPass) {
                return LasmObject(expr->getEnv(address)->get(expr->name).get());
            } else if (!wasFirstPass) {
                throw e; // only re-throw on second+ pass
            }
        }

        return LasmObject(NIL_O, 0);
    }

    std::any Interpreter::visitAssign(AssignExpr *expr) {
        auto value = evaluate(expr->value);

        environment->assign(expr->name, value);
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

        return function->call(this, arguments, expr);
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

        environment->define(stmt->name->getLexeme(), value);
        return std::any();
    }

    std::any Interpreter::visitBlock(BlockStmt *stmt) {
        executeBlock(stmt->statements, std::make_shared<Environment>(Environment(environment)));
        return std::any();
    }

    void Interpreter::executeBlock(std::vector<std::shared_ptr<Stmt>> statements,
            std::shared_ptr<Environment> environment, std::shared_ptr<Environment> labels) {

        if (!labels.get()) {
            labels = std::make_shared<Environment>(Environment(this->labels));
        }

        labelTable.push_back(labels);

        auto previous = this->environment;
        auto previousLabels = this->labels;

        this->environment = environment;
        this->labels = labels;
        for (auto statement : statements) {
            execute(statement);
        }
        this->environment = previous;
        this->labels = previousLabels;
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
        auto previousLabels = labels;
        while (evaluate(stmt->condition).isTruthy()) {
            execute(stmt->body);
        }
        labels = previousLabels;
        return std::any();
    }

    std::any Interpreter::visitFunction(FunctionStmt *stmt) {
        auto fn = std::make_shared<LasmFunction>(LasmFunction(stmt));
        LasmObject obj(CALLABLE_O, std::static_pointer_cast<Callable>(fn));
        environment->define(stmt->name->getLexeme(), obj);
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
        // TODO catch unresolved labels, return a deep clone of the current environment
        // and set unresolved flag along with the expression.
        // after assembly ends do a second pass and attempt to
        // resolve again
        onInstructionResult(instructions.generate(this, stmt->info, stmt));
        return std::any();
    }

    std::any Interpreter::visitDirective(DirectiveStmt *stmt) {
        return stmt->directive->execute(this, stmt);
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
        onInstructionResult(InstructionResult(data, size, getAddress()-size, stmt->token));

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
        onInstructionResult(InstructionResult(data, size, getAddress()-size, stmt->token));

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
                onInstructionResult(InstructionResult(data, evaluated.toString().length()+1, getAddress(), stmt->token));
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
                if (stmt->endianess != getNativeByteOrder()) {
                    // swap time!
                    std::shared_ptr<char[]> swapped(new char[stmt->size]);
                    std::reverse_copy(data.get(), data.get()+stmt->size, swapped.get());
                    data = swapped;
                }

                onInstructionResult(InstructionResult(data, stmt->size, getAddress(), stmt->token));
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
            environment->define(declaration->name->getLexeme(), startAddress);
            startAddress = LasmObject(NUMBER_O, value.toNumber() + startAddress.toNumber());
        }

        return std::any();
    }

    std::any Interpreter::visitLabel(LabelStmt *stmt) {
        LasmObject obj(NUMBER_O, (lasmNumber)address);
        labels->define(stmt->name->getLexeme().substr(0, stmt->name->getLexeme().length()-1), obj);
        return std::any();
    }

    std::any Interpreter::visitIncbin(IncbinStmt *stmt) {
        if (!reader) { return std::any(); }
        // either read file using the included file reader
        // or just use the already buffered value

        if (!stmt->data.get()) {
            auto path = evaluate(stmt->filePath);

            if (path.getType() != STRING_O) {
                throw LasmTypeError(std::vector<ObjectType> {STRING_O}, path.getType(), stmt->token);
            }

            auto stream = reader->openFile(path.toString());
            unsigned long size = 0;
            auto data = reader->readFullFile(stream, &size);
            reader->closeFile(stream);

            stmt->data = data;
            stmt->size = size;
        }
        // return result
        onInstructionResult(InstructionResult(stmt->data, stmt->size, getAddress(), stmt->token));
        address += stmt->size;

        return std::any();
    }

    std::any Interpreter::visitInclude(IncludeStmt *stmt) {
        if (!reader) { return std::any(); }

        // either read file and then interprete or just interprete right now!
        if (!stmt->wasparsed) {
            auto path = evaluate(stmt->filePath);

            if (path.getType() != STRING_O) {
                throw LasmTypeError(std::vector<ObjectType> {STRING_O}, path.getType(), stmt->token);
            }

            stmt->wasparsed = true;

            auto previousPath = reader->getDir();
            reader->changeDir(path.toString(), true);

            auto stream = reader->openFile(path.toString());
            auto source = std::string(reader->readFullFile(stream).get());
            reader->closeFile(stream);

            Scanner scanner(onError, instructions, source, path.toString());
            auto tokens = scanner.scanTokens();

            if (onError.didError()) {
                return std::any();
            }
            Parser parser(onError, tokens, instructions);
            auto ast = parser.parse();

            if (onError.didError()) {
                return std::any();
            }
            stmt->stmts = ast;

            reader->changeDir(previousPath);
        }
        try {
            for (auto stmt : stmt->stmts) {
                execute(stmt);
            }
        } catch (LasmException &e) {
            onError.onError(e.getType(), e.getToken(), &e);
        }

        return std::any();
    }

    void Interpreter::onInstructionResult(InstructionResult result) {
        if (pass != 0) {
            code.push_back(result);
        }
    }

    Endianess Interpreter::getNativeByteOrder() {
        // check endianess
        const unsigned int x = 0x12345678;
        if (*((char*)&x) == 0x78) {
            // little endian
            return LITTLE;
        }
        // big endian
        return BIG;
    }
}
