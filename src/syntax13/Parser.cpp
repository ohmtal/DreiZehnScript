//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Parser: Pratt (top-down operator precedence parser /Precedence Climbing Parser)
//-----------------------------------------------------------------------------
#include "Parser.h"
#include "core/FunctionMap.h"
#include <ScriptLoader.h>

namespace DreiZehn {

    enum class BlockType : U8 { Function, ForLoop, WhileLoop, IfBlock, MainBlock };

    struct OpenBlock {
        BlockType mType;
        uint32_t mFuncNameSymbolId;
        BlockStatement* mBlockNodePointer;
        U32 mLineNumber;
    };


    namespace ParserFrame {
        // -- Function Vars ---------------
        std::vector<uint32_t> mFunctionVariables;
        bool mBlockInFunction = false;
        std::vector<TokenType> mBlockOpenTypes;
        // -------------------------------------------------------------------------
        bool isFunctionVar(uint32_t symId) {
            return std::find(mFunctionVariables.begin(), mFunctionVariables.end(), symId) != mFunctionVariables.end();
        }
        void insertFunctionVar(uint32_t symId) {
            // Tools::printf(" ** VAR %s insertFunctionVar {block:%d}\n",
            //               SymbolTable::getName(symId).c_str(),
            //               mBlockInFunction);

            if (!mBlockInFunction) return;
            if (!isFunctionVar(symId))
                mFunctionVariables.push_back(symId);
        }

        bool guessGlobalVarAndInsertLocalIfNot(uint32_t symId) {
            bool foundAGlobal = Globals::isGlobalVar(symId);
            bool foundFunctionVar = isFunctionVar(symId);
            // Tools::printf(" ** VAR %s check foundAGlobal:%d foundFunc:%d {block:%d}\n",
            //               SymbolTable::getName(symId).c_str(), foundAGlobal,foundFunctionVar,
            //               mBlockInFunction);
            if (!mBlockInFunction) return foundAGlobal;
            if (!foundFunctionVar && foundAGlobal) return true;
            insertFunctionVar(symId);
            return false;
        }

        void startFunc(uint32_t funcSymId) {
            // Tools::printf("StartFunc %s ......\n", SymbolTable::getName(funcSymId).c_str());
            mFunctionVariables.clear();
            mBlockInFunction = true;
        }
        void endFunc(uint32_t funcSymId) {
            // Tools::printf("..... EndFunc %s {block:%d} \n"
            //     , SymbolTable::getName(funcSymId).c_str(), mBlockInFunction);
            mFunctionVariables.clear();
            mBlockInFunction = false;
        }

    } //namespace


    // ----------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    // for error message:
    std::string Parser::getLine(int byLine) {
        if (byLine < 1) byLine = (int)lineNumber;
        if (byLine == 0 || !mCurrentCodeStream) {
            return "";
        }
        mCurrentCodeStream->clear();
        mCurrentCodeStream->seekg(0, std::ios::beg);

        for (std::size_t currentLine = 1; currentLine < byLine; ++currentLine) {
            mCurrentCodeStream->ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (!mCurrentCodeStream) {
                return "";
            }
        }
        std::string line;
        if (std::getline(*mCurrentCodeStream, line)) {
            return line;
        }
        return "";
    }

    void Parser::PrintParseError(const std::string& message
    , const std::string& p1 ,const std::string& p2 ,const std::string& p3 ) {
        Tools::errorf("[Parser-ERROR LINE:%d]: %s %s %s %s\n%s\n",lineNumber
            , message.c_str(),p1.c_str(),p2.c_str(),p3.c_str()
            , getLine().c_str());
    }
    void Parser::PrintParseInfo(const std::string& message
    , const std::string& p1 ,const std::string& p2 ,const std::string& p3 ) {
        Tools::printf("[Parser-INFO LINE:%d]: %s %s %s %s\n%s\n",lineNumber
            , message.c_str(),p1.c_str(),p2.c_str(),p3.c_str()
            , getLine().c_str());
    }

    // ----------------------------------------------------------------------------
    int Parser::getPrecedence(TokenType type) {
        if (isInlineMathType(type)) return 100;
        if (type == TokenType::Mul || type == TokenType::Div ||
            type == TokenType::Modulo || type == TokenType::Not) return 90;
        if (type == TokenType::Plus || type == TokenType::Minus) return 80;
        if (type == TokenType::SHL || type == TokenType::SHR) return 70;

        if (type == TokenType::Less || type == TokenType::Greater ||
            type == TokenType::LowerEqual || type == TokenType::GreaterEqual) return 60;

        if (type == TokenType::Equal || type == TokenType::NotEqual) return 50;

        if (type == TokenType::BitAnd) return 40;
        if (type == TokenType::BitXOr) return 30;
        if (type == TokenType::BitOr)  return 20;

        if (type == TokenType::And || type == TokenType::Or) return 10;

        if (type == TokenType::Question) return 0;
        if (type == TokenType::Assign) return 0;
        if (type == TokenType::Dot) return 0;
        if (type == TokenType::Arrow) return 0;
        if (type == TokenType::KeyValue) return 0;
        if (type == TokenType::DotDot) return 0;
        if (type == TokenType::DotAssign) return 0;
        if (isMathAssignOperatorType(type)) return 0;

        return -1;
    }

    // ----------------------------------------------------------------------------
    std::unique_ptr<Expression> Parser::parseIdentifier() {
        std::unique_ptr<Expression> currentExpression = nullptr;
        Token nameToken = advance();
        uint32_t nameTokenSymbolId = SymbolTable::insert( nameToken.mValue);
        Value* constansPointer = FunctionMap::getConstants(nameTokenSymbolId);
        if (constansPointer != nullptr) {
            return std::make_unique<ValueExpression>((*constansPointer));
        } else
            if (FunctionMap::IsFunction(nameTokenSymbolId)) {
                std::vector<std::unique_ptr<Expression>> args;
                while (isContinueToken(peek())) {
                    size_t lastPos = mPos;
                    auto arg = parseMath();
                    if (arg.get()) args.push_back(std::move(arg));
                    if (checkCommaBreak(args)) break; //TEST

                    if (lastPos == mPos) {
                        PrintParseError("In function call:");
                        break;
                    }
                }
                return std::make_unique<CallExpression>(nameTokenSymbolId, std::move(args));
            } else {
                // we got a variable
                bool isGlobal = peekOffset(-2).mType == TokenType::Global;
                if (isGlobal) Globals::insertGlobalVar(nameTokenSymbolId);
                else {
                    isGlobal = ParserFrame::guessGlobalVarAndInsertLocalIfNot(nameTokenSymbolId);
                }
                currentExpression = std::make_unique<VariableExpression>(nameTokenSymbolId, isGlobal);
            }

            while (true) {
                if (peek().mType  == TokenType::Dot && peekNext().mType == TokenType::Identifier ) {
                    advance(); // eat DOT
                    Token fieldToken = advance();
                    uint32_t fieldSymbolId = SymbolTable::insert(fieldToken.mValue);
                    currentExpression = std::make_unique<ObjectFieldExpression>(std::move(currentExpression), fieldSymbolId);
                }
                else  if (peek().mType  == TokenType::Arrow && peekNext().mType == TokenType::Identifier ) {
                    advance(); // eat ->
                    Token methodToken = advance();
                    uint32_t methodSymbolId = SymbolTable::insert(methodToken.mValue);

                    std::vector<std::unique_ptr<Expression>> args;
                    while (isContinueToken(peek())) {
                        size_t lastPos = mPos;
                        auto arg = parseMath();
                        if (arg) args.push_back(std::move(arg));

                        if (checkCommaBreak(args)) break; //TEST

                        if (lastPos == mPos) {
                            PrintParseError("In method call:");
                            break;
                        }
                    }
                    currentExpression = std::make_unique<MethodExpression>(std::move(currentExpression), methodSymbolId, std::move(args));
                }
                else if (peek().mType == TokenType::Dot) {
                    PrintParseError("Field expected after  '.'");
                    advance();
                    break;
                }
                else if (peek().mType == TokenType::Arrow) {
                    PrintParseError("Method expected after '->'");
                    advance(); // eat '->'
                    break;
                }
                else if (peek().mType == TokenType::LSquare) {
                    advance(); // eat '['
                    std::unique_ptr<Expression> indexExpr = parseMath();

                    if (peek().mType != TokenType::RSquare) {
                        PrintParseError("expected ']' after array index!");
                        break;
                    }
                    advance(); // eat ']'
                    currentExpression = std::make_unique<ArrayVariableExpression>(
                        std::move(currentExpression), std::move(indexExpr));

                }
                else {
                    break;
                }
            }
            return currentExpression;

    }
    // ----------------------------------------------------------------------------
    std::unique_ptr< DreiZehn::Expression > Parser::parsePrimary() {
        Token t = peek();
        TokenType curType = peek().mType;

        switch (curType) {
            // ~~~~ LineBreak ~~~~
            case TokenType::EOLToken:
                lineNumber++;
            case TokenType::Semicolon: {
                advance();
                break;
            }
            // ~~~~ UnaryMinusExpression ~~~~
            case TokenType::Minus: {
                advance(); // eat '-'
                auto expr = parseMath(9);
                return std::make_unique<UnaryMinusExpression>(std::move(expr));
                break;
            }
            // ~~~~ LParen ~~~~~~
            case TokenType::LParen: {
                advance();
                auto expr = parseMath();
                if (peek().mType == TokenType::RParen) {
                    advance(); //  ')'
                } else {
                    PrintParseError("Error: Missing closing bracket");
                }
                return expr;
            }

            case TokenType::LCurly: {
                advance();
                std::vector<std::unique_ptr<Expression>> args;
                while (isContinueToken(peek())) {
                    size_t lastPos = mPos;
                    args.push_back(parseMath());
                    if (checkCommaBreak(args)) break; //TEST
                    if (lastPos == mPos) {
                        PrintParseError("In Curly Brackets:");
                        break;
                    }
                }
                if (peek().mType == TokenType::RCurly) {
                    advance();

                    return std::make_unique<ListExpression>(std::move(args));
                } else {
                    PrintParseError("Error: Missing closing curly bracket\n");
                }
                return nullptr;
            }

            case TokenType::Not: {
                advance(); //eat Not
                auto expr = parseMath();
                return std::make_unique<BinarySingleRightOpExpression>( TokenType::Not, std::move(expr));
            }
            case TokenType::Number:
            case TokenType::StringLiteral: {
                Token t = advance();
                return std::make_unique<LiteralExpression>(t.mType, t.mValue);
            }
            case TokenType::BitAnd:  {
                Token t = advance();
                auto expr = parseMath();
                if (!expr || (expr.get()->mNodeType != NodeType::VariableExpression &&
                    expr.get()->mNodeType != NodeType::ObjectFieldExpression)
                ) {
                    PrintParseError("Get by Reference (&) need a Variable!");
                    return nullptr;
                }

                return std::make_unique<VariableReferenceExpression>(std::move(expr));
            }
            case TokenType::Mul: {
                Token t = advance();
                auto expr = parseExpression();
                if (!expr || (expr.get()->mNodeType != NodeType::VariableExpression &&
                    expr.get()->mNodeType != NodeType::ObjectFieldExpression)
                ) {
                    PrintParseError("Get by Reference (&) need a Variable!");
                    return nullptr;
                }

                return std::make_unique<VariablePointerExpression>(std::move(expr));
            }
            case  TokenType::Identifier: {
                return parseIdentifier();
            } // TokenType::Identifier
            default:
                //not! PrintParseError("parsePrimary :: Unhandled Token:", tokenTypeToString(curType));
                return nullptr;
            break;
        } //switch


        return nullptr;
    }

    // ----------------------------------------------------------------------------
    std::unique_ptr< DreiZehn::Expression > Parser::parseMath(int minPrecedence) {

        if (peek().mType ==  TokenType::BackSlash) {
            lineNumber++;
            advance();
        }


        auto left = parsePrimary();

        while (isMathType(peek())) {
            Token op = peek();

            if (op.mType ==  TokenType::BackSlash) {
                lineNumber++;
                advance();
                continue;
            }


            int precedence = getPrecedence(op.mType);

            if (precedence < minPrecedence) {
                break;
            }
            advance();

            if (op.mType == TokenType::Assign) {
                if (!left) {
                    PrintParseError("Assignment but no Variable Expression!");
                    return nullptr;
                }
                auto rhs = parseMath(0);
                if (!rhs) {
                    PrintParseError("Assignment but no Assign Expression!");
                    return nullptr;
                }
                left = std::make_unique<AssignStatement>(
                    std::move(left), std::move(rhs)
                );
                continue;
            }
            else  if (isMathAssignOperatorType(op.mType)) {
                if (!left) {
                    PrintParseError("OP Assignment but no Expression!");
                    return nullptr;
                }
                auto rhs = parseMath(0);
                if (!rhs) {
                    PrintParseError("OP Assignment but no Assign Expression!");
                    return nullptr;
                }
                left = std::make_unique<AssignOPStatement>(
                    std::move(left),op.mType, std::move(rhs)
                );
                continue;
            }
            else if (op.mType == TokenType::Question) {
                auto trueBranch = parseMath(0);

                if (peek().mType != TokenType::Colon) {
                    PrintParseError("Expect ':' after '?'");
                    return nullptr;
                }
                advance();
                auto falseBranch = parseMath(precedence);
                left = std::make_unique<TernaryExpression>(
                    std::move(left), std::move(trueBranch), std::move(falseBranch)
                );
                continue;
            }
            else if (isInlineMathType(op.mType)) {
                if (!left) {
                    PrintParseError("Inline OP but no Expression!");
                    return nullptr;
                }
                left = std::make_unique<BinaryInlineExpression>(
                    std::move(left),op.mType
                );
                continue;
            } else
            if (op.mType  == TokenType::Dot && peek().mType == TokenType::Identifier ) {

                Token fieldToken = advance();
                uint32_t fieldSymbolId = SymbolTable::insert(fieldToken.mValue);
                return  std::make_unique<ObjectFieldExpression>(std::move(left), fieldSymbolId);
            } else
            if (op.mType  == TokenType::Arrow && peek().mType == TokenType::Identifier ) {
                Token methodToken = advance();
                uint32_t methodSymbolId = SymbolTable::insert(methodToken.mValue);

                std::vector<std::unique_ptr<Expression>> args;
                while (isContinueToken(peek())) {
                    size_t lastPos = mPos;
                    auto arg = parseMath();
                    if (arg) args.push_back(std::move(arg));

                    if (checkCommaBreak(args)) break; //TEST

                    if (lastPos == mPos) {
                        PrintParseError("In method call:");
                        break;
                    }
                }
                return std::make_unique<MethodExpression>(std::move(left), methodSymbolId, std::move(args));
            } else
            if (op.mType == TokenType::KeyValue) {
                if (!left) {
                    PrintParseError("Key Value Assignment but no Key Expression!");
                    return nullptr;
                }
                auto rhs = parseMath(0);
                if (!rhs) {
                    PrintParseError("Key Value Assignment but no Value Expression!");
                    return nullptr;
                }

                left = std::make_unique<KeyValueExpression>(
                    std::move(left), std::move(rhs)
                );
                continue;
            }
            else
            if (op.mType == TokenType::DotDot) {
                if (!left) {
                    PrintParseError("String Concat Dot Dot but no left!");
                    return nullptr;
                }
                auto rhs = parseMath(0);
                if (!rhs) {
                    PrintParseError("String Concat Dot Dot but no right!");
                    return nullptr;
                }

                left = std::make_unique<StringConcatExpression>(
                    std::move(left), std::move(rhs)
                );
                continue;
            }
            else
                if (op.mType == TokenType::DotAssign) {
                    if (!left) {
                        PrintParseError("String Concat Dot Assign but no left!");
                        return nullptr;
                    }
                    auto rhs = parseMath(0);
                    if (!rhs) {
                        PrintParseError("String Concat Dot Assign bu no right!");
                        return nullptr;
                    }

                    left = std::make_unique<StringConcatAssignExpression>(
                        std::move(left), std::move(rhs)
                    );
                    continue;
                }



            auto right = parseMath(precedence + 1);

            if (isMathOperatorType(op)) {
                left = std::make_unique<BinaryOpExpression>(std::move(left), op.mType, std::move(right));
            } else if (op.mType == TokenType::And || op.mType == TokenType::Or ) {
                left = std::make_unique<BinaryAndOrExpression>(std::move(left), op.mType, std::move(right));
            } else {
                left = std::make_unique<BinaryExpression>(std::move(left), op.mType, std::move(right));
            }
        }
        return left;
    }
    // ----------------------------------------------------------------------------
    std::unique_ptr< DreiZehn::Expression > Parser::parseComparison() {
        auto left = parseMath();

        while (peek().mType == TokenType::Greater
            || peek().mType == TokenType::Less
            || peek().mType == TokenType::Equal
            || peek().mType == TokenType::NotEqual
            || peek().mType == TokenType::LowerEqual
            || peek().mType == TokenType::GreaterEqual
        ) {
            Token op = advance();
            auto right = parseMath();
            left = std::make_unique<BinaryExpression>(std::move(left), op.mType, std::move(right));
        }
        return left;
    }
    // ----------------------------------------------------------------------------
    std::unique_ptr< DreiZehn::Expression > Parser::parseExpression() {

        if (peek().mType == TokenType::LParen) {
            advance(); // skip '('
            auto expr = parseExpression(); // inner statement

            if (peek().mType == TokenType::RParen) {
                advance(); // skip ')'
            } else {
                PrintParseError("Error: Missing closing brackets");
            }
            return expr;
        }

        if (peek().mType == TokenType::Number || peek().mType == TokenType::StringLiteral) {
            Token t = advance();
            return std::make_unique<LiteralExpression>(t.mType, t.mValue);
        }


        if (peek().mType == TokenType::Identifier) {
            Token nameToken = advance();
            uint32_t nameTokenSymbolId = SymbolTable::insert(nameToken.mValue);

            if (FunctionMap::IsFunction(nameTokenSymbolId)) {
                std::vector<std::unique_ptr<Expression>> args;

                while (isContinueToken(peek()))
                {
                    size_t lastPos = mPos;
                    auto arg = parseMath();
                    if (arg) args.push_back(std::move(arg));

                    if (checkCommaBreak(args)) break; //TEST

                    if (lastPos == mPos) {
                        PrintParseError("In function call:");
                        break;
                    }
                }
                return std::make_unique<CallExpression>(nameTokenSymbolId, std::move(args));
            }

            bool isGlobal = peekOffset(-2).mType == TokenType::Global;
            if (isGlobal) Globals::insertGlobalVar(nameTokenSymbolId);
            else {
                isGlobal = ParserFrame::guessGlobalVarAndInsertLocalIfNot(nameTokenSymbolId);
            }
            return std::make_unique<VariableExpression>(nameTokenSymbolId, isGlobal);
        }

        return nullptr;
    }
    // ----------------------------------------------------------------------------
    // ----------------------------------------------------------------------------
    std::unique_ptr< DreiZehn::ASTNode > Parser::parseStatement() {


        TokenType tokenType = peek().mType;
        switch (tokenType) {
            case TokenType::Fn: {
                advance(); //eat fn


                if (peek().mType != TokenType::Identifier) {
                    PrintParseError("function name required after fn\n");
                    return nullptr;
                }


                std::string funcName = advance().mValue;
                uint32_t funcNameSymbolId = SymbolTable::insert(funcName);
                ParserFrame::startFunc(funcNameSymbolId);
                std::vector<std::string> params;

                while (peek().mType == TokenType::Identifier) {
                    std::string varName = advance().mValue;
                    ParserFrame::insertFunctionVar(SymbolTable::insert(varName));

                    params.push_back(varName);
                }
                FunctionMap::RegisteredScriptFunctions[funcNameSymbolId] = { params, {},0,0 };
                ParserFrame::mBlockOpenTypes.push_back(tokenType);
                return std::make_unique<FunctionDefineStartNode>(funcNameSymbolId);
            }
            break;

            case TokenType::forRange: {
                advance(); //eat range

                if (peek().mType != TokenType::Identifier) {
                    PrintParseError("Variable name after range expected");
                    return nullptr;
                }
                std::string varName = advance().mValue;

                auto countExpr = parseMath();
                if (!countExpr) {
                    PrintParseError("forRange invalid count expresssion.");
                    return nullptr;
                }
                ParserFrame::mBlockOpenTypes.push_back(tokenType);

                return std::make_unique<ForRangeStatement>(SymbolTable::insert(varName)
                , std::move(countExpr));
            }
            break;

            case TokenType::forEach: {
                advance(); //eat

                if (peek().mType != TokenType::Identifier) {
                    PrintParseError("variable name after range expected");
                    return nullptr;
                }
                std::string varName = advance().mValue;
                auto varExpr = parseMath();
                if (!varExpr) {
                    PrintParseError("Invalid variable expression");
                    return nullptr;
                }
                ParserFrame::mBlockOpenTypes.push_back(tokenType);
                return std::make_unique<ForEachStatement>(SymbolTable::insert(varName)
                , std::move(varExpr));
            }
            break;

            case TokenType::For: {
                advance();

                if (peek().mType != TokenType::Identifier) {
                    PrintParseError("Variable name after for expected");
                    return nullptr;
                }
                std::string varName = advance().mValue;

                auto start = parseMath();
                auto end = parseMath();
                auto step= parseMath();

                if (!start || !end ){
                    PrintParseError("for invalid  borders:");
                    return nullptr;
                }


                ParserFrame::mBlockOpenTypes.push_back(tokenType);

                return std::make_unique<ForStatement>(SymbolTable::insert(varName),
                            std::move(start),
                            std::move(end),
                            std::move(step)
                );
            }
            break;

            case TokenType::While:  {
                advance();
                auto condition = parseComparison();
                ParserFrame::mBlockOpenTypes.push_back(tokenType);
                return std::make_unique<WhileStatement>(std::move(condition));
            }
            break;

            case TokenType::End:  {
                advance();

                if (ParserFrame::mBlockOpenTypes.size() > 0) {
                    TokenType startToken = ParserFrame::mBlockOpenTypes.back();
                    ParserFrame::mBlockOpenTypes.pop_back();
                    if (startToken == TokenType::Fn) {
                        ParserFrame::endFunc(0);
                    }

                    return std::make_unique<FunctionDefineEndNode>();
                }
            }
            break;

            case TokenType::If:  {
                advance(); // skip "if"
                auto condition = parseComparison();
                ParserFrame::mBlockOpenTypes.push_back(tokenType);
                return std::make_unique<IfStatement>(std::move(condition));
            }
            break;

            case TokenType::ElIf:  {
                advance(); // skip "if"
                auto condition = parseComparison();
                return std::make_unique<ElIfStatement>(std::move(condition));
            }
            break;

            case TokenType::Else:  {
                advance(); // skip "else"
                return std::make_unique<ElseMarkerNode>();
            }
            break;

            case TokenType::Break:  {
                advance(); // eat 'break'
                return std::make_unique<BreakStatement>();
            }
            break;

            case TokenType::Continue: {
                advance(); // eat
                return std::make_unique<ContinueStatement>();
            }
            break;

            case TokenType::Return:  {
                advance(); // eat'return'

                std::unique_ptr<Expression> rhs = nullptr;

                if (peek().mType != TokenType::EOFToken &&
                    peek().mType != TokenType::End &&
                    peek().mType != TokenType::Semicolon &&
                    peek().mType != TokenType::RParen) {
                    rhs = parseComparison();
                    }

                    return std::make_unique<ReturnStatement>(std::move(rhs));
            }
            break;

            // -----------------------------------------------------------------
            case TokenType::Define: {
                advance(); //eat define

                if (peek().mType != TokenType::Identifier) {
                    PrintParseError("variable name after define expected");
                    return nullptr;
                }
                std::string varName = advance().mValue;

                if (peek().mType == TokenType::Assign) advance(); //eat optional "="

                auto expr = parseMath();

                if (!expr) {
                    PrintParseError("Value after define expected");
                    return nullptr;
                }

                if (!mEnvPtr) {
                    PrintParseError("Current enviroment not set!");
                    return nullptr;
                }

                // direct register no token
                Value value = expr->evaluate(*mEnvPtr);
                FunctionMap::RegisterConstants(varName, value);

                return nullptr;
            }
            break;

            case TokenType::Import: {
                advance();

                if (peek().mType != TokenType::Identifier) {
                    PrintParseError("Variable name after import expected");
                    return nullptr;
                }
                std::string modname = advance().mValue;

                if (!mEnvPtr) {
                    PrintParseError("Import: current enviroment not set!");
                    return nullptr;
                }
                if (modname.empty()) {
                    PrintParseError("Import: module name empty.");
                    return nullptr;
                }
                // direct import
                if ( modname == "All") {
                    FunctionMap::ModuleRegistry::LoadAll(*mEnvPtr);
                } else {
                    if (!FunctionMap::ModuleRegistry::Load(modname, *mEnvPtr)) {
                        PrintParseError(("Failed to load Module: " + modname).c_str());
                    }
                }

                return nullptr;
            }
            break;

            // ---- direct include ----
            case TokenType::Include: {
                advance();

                if (peek().mType != TokenType::StringLiteral) {
                    PrintParseError("Filename after include expected");
                    return nullptr;
                }
                std::string filename = advance().mValue;

                if (filename.empty()) {
                    PrintParseError("Include: filename empty.");
                    return nullptr;
                }
                auto noExecCond = parseMath();

                auto script = IncludeScriptFile(*mEnvPtr, filename);
                if (!script) return nullptr;

                return std::make_unique<IncludeStatement>(std::move(script), std::move(noExecCond));
            }

            break;
            // -----------------------------------------------------------------
            case TokenType::Global:  {
                advance(); // skip "
                // return std::make_unique<ElseMarkerNode>();
            }
            break;
            // -----------------------------------------------------------------
            case TokenType::EOLToken:
                lineNumber++;
            case TokenType::Semicolon: {
                advance();
                break;
            }
            // -----------------------------------------------------------------


            default: {
                 return parseComparison();
            }
            break;

        }

        return parseComparison();
    }
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    // std::vector< std::unique_ptr< DreiZehn::ASTNode > > Parser::parse() {
    std::shared_ptr<BlockStatement> Parser::parse() {
        std::vector<std::unique_ptr<ASTNode>> statements;

        std::vector<OpenBlock> blockStack;
        auto mainProgram = std::make_shared<BlockStatement>();
        size_t statment_peek = 0;
        blockStack.push_back({BlockType::IfBlock, 0, mainProgram.get(), 0});


        lineNumber = 1;
        while (peek().mType != TokenType::EOFToken) {

            #ifdef DREIZEHN_DEBUG
            if (Globals::gDumpStateNodes) {
                Tools::printf("---------- new statement ------ Pos:%d \n", (int)mPos);
                for(size_t i = mPos; i < mTokens.size() ; i++) {
                    Tools::printf("Token %d: %d :: %s\n", i, (int)mTokens[i].mType, tokenTypeToString(mTokens[i].mType));
                }
            }
            #endif

            size_t startIndex = mPos;
            auto stmt = parseStatement();


            #ifdef DREIZEHN_DEBUG
            if (Globals::gDumpStateNodes) {
                Tools::printf("---------- LINE parsed ------ Pos:%d next:%d :: %s\n", (int)mPos, (int)mTokens[mPos].mType, tokenTypeToString(mTokens[mPos].mType));
            }
            #endif
            if (stmt) {
                stmt->mLineNumber = lineNumber;
                statements.push_back(std::move(stmt));
            }

            if (mPos == startIndex) {
                Tools::errorf("Syntax-Error: Unexpected token '%s' (%s:%d)\n"
                , tokenTypeToString(mTokens[mPos].mType),getLine().c_str()
                , lineNumber );
                advance();
            }
            //------------------------------------------------------------------
            // Blocks
            //------------------------------------------------------------------
            if (peekPrev().mType == TokenType::EOLToken) {
                for (size_t iter = statment_peek; iter < statements.size(); iter++) {
                    auto& ast = statements[iter];
                    if (!ast) return nullptr;

                    // --- fn ---
                    if (auto* startNode = dynamic_cast<FunctionDefineStartNode*>(ast.get())) {
                        blockStack.push_back({BlockType::Function, startNode->mFnNameSymbolId, nullptr, startNode->mLineNumber});
                        mainProgram->mBody.push_back(std::make_shared<FunctionDefineStartNode>(startNode->mFnNameSymbolId)) ;
                        continue;
                    }


                    bool isIf    = ast->mNodeType == NodeType::IfStatement;
                    bool isFor   = ast->mNodeType == NodeType::ForStatement
                    || ast->mNodeType == NodeType::RangeStatement
                    || ast->mNodeType == NodeType::ForEachStatement;
                    bool isWhile = ast->mNodeType == NodeType::WhileStatement;


                    if (isFor || isWhile || isIf ) {
                        std::shared_ptr<ASTNode> sharedBase = std::move(ast);
                        std::shared_ptr<BlockStatement> sharedLoop = std::static_pointer_cast<BlockStatement>(sharedBase);

                        BlockType bType = isIf ? BlockType::IfBlock : (isFor ? BlockType::ForLoop : BlockType::WhileLoop);
                        BlockStatement* blockPtr = sharedLoop.get();

                        auto& outerBlock = blockStack.back();
                        if (outerBlock.mType == BlockType::Function) {
                            FunctionMap::RegisteredScriptFunctions[outerBlock.mFuncNameSymbolId].body.push_back(sharedLoop);

                        } else
                            // ifBlock
                            if (outerBlock.mType == BlockType::IfBlock) {
                                auto* parentIf = dynamic_cast<IfStatement*>(outerBlock.mBlockNodePointer);
                                if (parentIf && parentIf->mElseBranch != nullptr) {
                                    parentIf->mElseBranch->mBody.push_back(sharedLoop);
                                } else {
                                    outerBlock.mBlockNodePointer->mBody.push_back(sharedLoop);
                                }
                            } else
                                // ForLoop / WhileLoop
                                if (outerBlock.mType == BlockType::ForLoop
                                    || outerBlock.mType == BlockType::WhileLoop
                                ) {
                                    outerBlock.mBlockNodePointer->mBody.push_back(sharedLoop);
                                }

                                blockStack.push_back({bType, 0, blockPtr, sharedLoop->mLineNumber});
                            continue;
                    }
                    // ---- end -----
                    else if (ast->mNodeType == NodeType::EndNode) {
                        if (blockStack.size() <= 1) {
                            PrintParseError("Syntax-Error: 'end' without starting statement.");
                            return nullptr;
                        }

                        blockStack.pop_back();

                        continue;
                    }
                    else if (ast->mNodeType == NodeType::ElseMarkerNode) {
                        if (blockStack.empty() || blockStack.back().mType != BlockType::IfBlock) {
                            PrintParseError("Syntax-Error: 'else' without matching 'if'.");
                            return nullptr;
                        }
                        auto* actualIf = dynamic_cast<IfStatement*>(blockStack.back().mBlockNodePointer);
                        if (actualIf) {
                            auto elseBlock = std::make_shared<BlockStatement>();
                            actualIf->mElseBranch = elseBlock;
                        }
                        continue;
                    }
                    else if (ast->mNodeType == NodeType::ElIfStatement) {
                        if (blockStack.empty() || blockStack.back().mType != BlockType::IfBlock) {
                            PrintParseError("Syntax-Error: 'elif' without matching 'if'.");
                            return nullptr;
                        }

                        auto* parentIf = dynamic_cast<IfStatement*>(blockStack.back().mBlockNodePointer);


                        std::shared_ptr<ElIfStatement> elifNode = std::shared_ptr<ElIfStatement>(static_cast<ElIfStatement*>(ast.release()));
                        parentIf->mElseBranch = elifNode;
                        blockStack.back().mBlockNodePointer = elifNode.get();
                        continue;
                    }

                    // ----------- CURRENT BLOCK -------------
                    auto& currentBlock = blockStack.back();
                    std::shared_ptr<ASTNode> sharedAst = std::move(ast);

                    if (currentBlock.mType == BlockType::Function) {
                        FunctionMap::RegisteredScriptFunctions[currentBlock.mFuncNameSymbolId].body.push_back(sharedAst);
                    } else {
                        auto* actualIf = dynamic_cast<IfStatement*>(currentBlock.mBlockNodePointer);
                        if (currentBlock.mType == BlockType::IfBlock && actualIf && actualIf->mElseBranch != nullptr) {
                            actualIf->mElseBranch->mBody.push_back(sharedAst);
                        } else {
                            currentBlock.mBlockNodePointer->mBody.push_back(sharedAst);
                        }
                    }
                } // for statements
                statment_peek = statements.size();
            }
            //------------------------------------------------------------------
        }


        if (blockStack.size() > 1) {
            Tools::errorf("Syntax-Error: One or more END statements are missing!\n");
            Tools::printf(" │─────────────────────────────────────────────────────────────────────\n");
            Tools::printf(" │ %d open nodes \n", (int)blockStack.size() - 1);
            Tools::printf(" │─────────────────────────────────────────────────────────────────────\n");
            for (size_t i = 1; i < blockStack.size() ; i++) {
                auto& block = blockStack[i];
                Tools::printf(" │ %04d ├─ %s\n", block.mLineNumber,  getLine(block.mLineNumber).c_str());
            }
            Tools::printf(" │\n");
            return nullptr;
        }

        return mainProgram;
    }

} // namespace DreiZehn {



