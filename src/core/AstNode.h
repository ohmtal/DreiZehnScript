//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// AstNode
//-----------------------------------------------------------------------------
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "core/Lexer.h"
#include "core/Value.h"

namespace DreiZehn {

class Environment;

enum class FlowSignal {
    None,
    Break,
    Return,
    Continue
};

enum class NodeType {
    BaseNode,
    Expression,
    BlockStatement,
    LiteralExpression,
    ValueExpression,
    VariableExpression,
    ArrayVariableExpression,
    CallExpression,
    MethodExpression,
    AssignStatement,
    BinaryExpression,
    BinaryOpExpression,
    BinaryInlineExpression,
    IfStatement,
    ElseMarkerNode,
    ElIfStatement,
    FunctionDefineStartNode,
    EndNode,
    ForStatement,
    BreakStatement,
    ContinueStatement,
    ReturnStatement,
    WhileStatement,
    AssignOPStatement,
    ObjectFieldExpression,
    RangeStatement,
    TernaryExpression
    // ,CurlyArgumentsExpression
    ,ForEachStatement
};


constexpr const char* NodeTypeToString(NodeType type) {
    switch (type) {
        case NodeType::BaseNode:                return "BaseNode";
        case NodeType::Expression:              return "Expression";
        case NodeType::BlockStatement:          return "BlockStatement";
        case NodeType::LiteralExpression:       return "LiteralExpression";
        case NodeType::ValueExpression:         return "ValueExpression";
        case NodeType::VariableExpression:      return "VariableExpression";
        case NodeType::ArrayVariableExpression:   return "ArrayVariableExpression";
        case NodeType::CallExpression:          return "CallExpression";
        case NodeType::MethodExpression:        return "MethodExpression";
        case NodeType::AssignStatement:         return "AssignStatement";
        case NodeType::BinaryExpression:        return "BinaryExpression";
        case NodeType::BinaryOpExpression:      return "BinaryOpExpression";
        case NodeType::BinaryInlineExpression:  return "BinaryInlineExpression";
        case NodeType::IfStatement:             return "IfStatement";
        case NodeType::ElseMarkerNode:          return "ElseMarkerNode";
        case NodeType::ElIfStatement:           return "ElIfStatement";

        case NodeType::FunctionDefineStartNode: return "FunctionDefineStartNode";
        case NodeType::EndNode:                 return "EndNode";
        case NodeType::ForStatement:            return "ForStatement";
        case NodeType::BreakStatement:          return "BreakStatement";
        case NodeType::ContinueStatement:       return "ContinueStatement";
        case NodeType::ReturnStatement:         return "ReturnStatement";
        case NodeType::WhileStatement:          return "WhileStatement";
        case NodeType::AssignOPStatement:       return "AssignOPStatement";
        case NodeType::ObjectFieldExpression:   return "ObjectFieldExpression";
        case NodeType::RangeStatement:          return "RangeStatement";
        case NodeType::TernaryExpression:       return "TernaryExpression";
        // case NodeType::CurlyArgumentsExpression: return "CurlyArgumentsExpression";
        case NodeType::ForEachStatement:       return "ForEachStatement";
    }
    return "UnknownNodeType";
}

inline  uint32_t ReturnValueSymbol = SymbolTable::insert("__return_value__");

// base node -------------------------------------------------------------------
struct ASTNode {
    NodeType mNodeType = NodeType::BaseNode;
    virtual ~ASTNode() = default;
};

// base expression -------------------------------------------------------------
struct Expression : public ASTNode {
    Expression() {
        mNodeType  = NodeType::Expression;
    }
    virtual Value evaluate(Environment& env) = 0;
    // fetch a Ptr to a value instead of a value
    virtual Value* evaluatePtr(Environment& env) {
        return nullptr;
    }
};


// FlowBaseStatement --------------------------------------------------------
struct FlowBaseStatement: public ASTNode {
    virtual FlowSignal execute(Environment& env) = 0;
};


// block statement --------------------------------------------------------------
class BlockStatement : public FlowBaseStatement {
public:
    std::vector<std::shared_ptr<ASTNode>> mBody;
    BlockStatement() {
        mNodeType = NodeType::BlockStatement;
    }
    FlowSignal execute(Environment& env) override;
};


// constants -------------------------------------------------------------------
struct LiteralExpression : public Expression {
    TokenType mType;
    std::string mRawValue;

    bool  mEvaluated = false;
    Value mEvaluatedValue;

    LiteralExpression(TokenType t, std::string val)
        : mType(t), mRawValue(std::move(val)) {
            mNodeType  = NodeType::LiteralExpression;
        }

    Value evaluate(Environment& env) override;
};

// Values directly pushed in (ConstantsMap)-------------------------------------------------------------------
struct ValueExpression : public Expression {
    Value mValue;

    ValueExpression(const Value& value ) : mValue(value) {
        mNodeType  = NodeType::ValueExpression;
    }

    inline Value evaluate(Environment& env) override {
        return mValue;
    }
};

// Variables -------------------------------------------------------------------

struct VariableExpression : public Expression {
    uint32_t mVariableNameSymbolId = 0;

    VariableExpression(uint32_t n) : mVariableNameSymbolId(n) {
        mNodeType  = NodeType::VariableExpression;
    }
    Value evaluate(Environment& env) override;
    Value* evaluatePtr(Environment& env) override;
};

// ArrayVariableExpression --------------------------------------------------------------
struct ArrayVariableExpression : public Expression {
    std::unique_ptr<Expression> mVarExpr; //left variable expression
    std::unique_ptr<Expression> mIndexExpr;

    ArrayVariableExpression(std::unique_ptr<Expression> varExpr, std::unique_ptr<Expression>  idxExpr) :
        mVarExpr(std::move(varExpr)),mIndexExpr(std::move(idxExpr)) {
        mNodeType  = NodeType::ArrayVariableExpression;
    }
    Value evaluate(Environment& env) override;
    Value* evaluatePtr(Environment& env) override;
};

// ObjectFieldExpression --------------------------------------------------------------
struct ObjectFieldExpression : public Expression {
    std::unique_ptr<Expression> mVarExpr; //left variable expression
    uint32_t mFieldSymbolId = 0;
    ObjectFieldExpression(std::unique_ptr<Expression> varExpr, uint32_t fieldId)
        : mVarExpr(std::move(varExpr)), mFieldSymbolId(fieldId)
    {
        mNodeType  = NodeType::ObjectFieldExpression;
    }
    Value evaluate(Environment& env) override;
    Value* evaluatePtr(Environment& env) override;
};
// function calls --------------------------------------------------------------
struct CallExpression : public Expression {
    uint32_t mFuncSymbolId = 0;
    std::vector<std::unique_ptr<Expression>> arguments;

    CallExpression(uint32_t funcSymbolID, std::vector<std::unique_ptr<Expression>> args)
    : mFuncSymbolId(funcSymbolID), arguments(std::move(args)) {
        mNodeType  = NodeType::CallExpression;
    }

    Value evaluate(Environment& env) override;
};

// method/field on Pointer  calls --------------------------------------------------------------
struct MethodExpression : public Expression {
    std::unique_ptr<Expression> mVarExpr; //left variable expression
    uint32_t mMethodNameSymbolId;

    std::vector<std::unique_ptr<Expression>> mArguments;

    MethodExpression(std::unique_ptr<Expression> varExpr, uint32_t methodNameSymId,
        std::vector<std::unique_ptr<Expression>> args)
        :mVarExpr(std::move(varExpr)), mMethodNameSymbolId(methodNameSymId), mArguments(std::move(args))
    {
        mNodeType  = NodeType::MethodExpression;
    }

    Value evaluate(Environment& env) override;
    Value* evaluatePtr(Environment& env) override;
};


// CurlyArgumentsExpression  --------------------------------------------------------------
// Nonsense !!
// struct CurlyArgumentsExpression : public Expression {
//     std::vector<std::unique_ptr<Expression>> arguments;
//
//     CurlyArgumentsExpression( std::vector<std::unique_ptr<Expression>> args)
//     :  arguments(std::move(args)) {
//         mNodeType  = NodeType::CurlyArgumentsExpression;
//     }
//
//     Value evaluate(Environment& env) override {
//         //FIXME
//         Tools::errorf("FIXME CurlyArgumentsExpression evaluate\n");
//         return Value();
//     }
// };

// Assingment ------------------------------------------------------------------

// struct AssignStatement : public AssignBaseStatement {
struct AssignStatement : public Expression {
    std::unique_ptr<Expression> mVarExpr; //left variable expression
    std::unique_ptr<Expression> mRhs; // Right-Hand Side

    AssignStatement( std::unique_ptr<Expression> varExpr, std::unique_ptr<Expression> rightExpr)
    : mVarExpr(std::move(varExpr)), mRhs(std::move(rightExpr)) {
        mNodeType  = NodeType::AssignStatement;
    }

    Value evaluate(Environment& env) override;

};
// Binary ----------------------------------------------------------------------
struct BinaryExpression : public Expression {

    std::unique_ptr<Expression> mLeft;
    TokenType mOp;
    std::unique_ptr<Expression> mRight;

    BinaryExpression(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r)
    : mLeft(std::move(l)), mOp(o), mRight(std::move(r)) {
        mNodeType  = NodeType::BinaryExpression;
    }

    Value evaluate(Environment& env) override;
};
// BinarySingleRightOpExpression  ----------------------------------------------
// used for Not
struct BinarySingleRightOpExpression : public Expression {

    TokenType mOp;
    std::unique_ptr<Expression> mRight;

    BinarySingleRightOpExpression( TokenType o, std::unique_ptr<Expression> r)
    :  mOp(o), mRight(std::move(r)) {
        mNodeType  = NodeType::BinaryExpression;
    }

    Value evaluate(Environment& env) override;
};
// BinaryOP --------------------------------------------------------------------
struct BinaryOpExpression : public Expression {
    std::unique_ptr<Expression> mLeft;
    TokenType mOp;
    std::unique_ptr<Expression> mRight;

    BinaryOpExpression(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r)
    : mLeft(std::move(l)), mOp(o), mRight(std::move(r)) {
        mNodeType  = NodeType::BinaryOpExpression;
    }

    Value evaluate(Environment& env) override;
};
// BinaryInline ----------------------------------------------------------------------
struct BinaryInlineExpression : public Expression {
    std::unique_ptr<Expression> mExpr;
    TokenType mOp;

    BinaryInlineExpression(std::unique_ptr<Expression> expr, TokenType o)
    :  mExpr(std::move(expr)), mOp(o) {
        mNodeType  = NodeType::BinaryInlineExpression;
    }
    Value evaluate(Environment& env) override;
};

// AssingmentOP ------------------------------------------------------------------
// struct AssignOPStatement : public AssignBaseStatement {
struct AssignOPStatement : public Expression {
    std::unique_ptr<Expression> mVarExpr;
    TokenType mOp;
    std::unique_ptr<Expression> mRhs; // Right-Hand Side


    AssignOPStatement( std::unique_ptr<Expression> varExpr, TokenType op, std::unique_ptr<Expression> rightExpr)
    : mVarExpr(std::move(varExpr)), mOp(op),mRhs(std::move(rightExpr)) {
        mNodeType  = NodeType::AssignOPStatement;
    }

     Value evaluate(Environment& env) override;
};
// short if => ? : ---------------------------------------------------------------
struct TernaryExpression : public Expression {
    std::unique_ptr<Expression> mCondition;
    std::unique_ptr<Expression> mTrueBranch;
    std::unique_ptr<Expression> mFalseBranch;

    TernaryExpression(
        std::unique_ptr<Expression> cond,
        std::unique_ptr<Expression> trueBranch,
        std::unique_ptr<Expression> falseBranch
    ) : mCondition(std::move(cond)),
    mTrueBranch(std::move(trueBranch)),
    mFalseBranch(std::move(falseBranch))
    {
        mNodeType = NodeType::TernaryExpression;
    }

    inline Value evaluate(Environment& env) override {
        auto condVal = mCondition->evaluate(env);
        if (condVal.getBool() ) {
            return mTrueBranch->evaluate(env);
        } else {
            return mFalseBranch->evaluate(env);
        }
    }
};
// UnaryMinusExpression -----------------------------------------------------------
struct UnaryMinusExpression : public Expression {
    std::unique_ptr<Expression> mExpr;

    UnaryMinusExpression( std::unique_ptr<Expression> expr) : mExpr(std::move(expr)) { }

    inline Value evaluate(Environment& env) {
        Value v = mExpr->evaluate(env);
        if (v.isInt()) return Value(v.asFastInt() * -1);
        if (v.isDouble()) return Value(v.asFastDouble() * -1);
        Tools::PrintRuntimeError( "Pointer Minus Operation not allowed!");
        return v;
    }
};

// If -------------------------------------------------------------------------
struct IfStatement : public BlockStatement {
    std::unique_ptr<Expression> mCondition;
    // NOTE: body is defined in BlockStatement

    std::shared_ptr<BlockStatement> mElseBranch = nullptr;

    IfStatement(std::unique_ptr<Expression> cond) : mCondition(std::move(cond)) {
        mNodeType  = NodeType::IfStatement;
    }

    FlowSignal execute(Environment& env) override;

};
struct ElseMarkerNode: public ASTNode {
    ElseMarkerNode() {
        mNodeType = NodeType::ElseMarkerNode;
    }
};
struct ElIfStatement: public IfStatement {
    ElIfStatement(std::unique_ptr<Expression> cond): IfStatement(std::move(cond)) {
        mNodeType = NodeType::ElIfStatement;
    }

};
// fn --------------------------------------------------------------------------
struct FunctionDefineStartNode : public ASTNode {
    uint32_t mFnNameSymbolId;
    FunctionDefineStartNode(uint32_t symId) : mFnNameSymbolId(symId) {
        mNodeType  = NodeType::FunctionDefineStartNode;
    }
};
// end -------------------------------------------------------------------------
struct FunctionDefineEndNode : public ASTNode {
   FunctionDefineEndNode() {
        mNodeType  = NodeType::EndNode;
  }
};
// for -------------------------------------------------------------------------
struct ForStatement : public BlockStatement {
    uint32_t mIteratorVarNameSymbolId = 0;
    std::unique_ptr<Expression> mStartExpr;
    std::unique_ptr<Expression> mEndExpr;

    ForStatement(uint32_t nameSymId, std::unique_ptr<Expression> start, std::unique_ptr<Expression> end)
    : mIteratorVarNameSymbolId(nameSymId), mStartExpr(std::move(start)), mEndExpr(std::move(end)) {
            mNodeType  = NodeType::ForStatement;
    }

    FlowSignal execute(Environment& env) override;
};
// forRange -------------------------------------------------------------------------
struct ForRangeStatement : public BlockStatement {
    uint32_t mIteratorVarNameSymbolId = 0;
    std::unique_ptr<Expression> mCountExpr;

    ForRangeStatement(uint32_t nameSymId, std::unique_ptr<Expression> count)
    : mIteratorVarNameSymbolId(nameSymId), mCountExpr(std::move(count)) {
        mNodeType  = NodeType::RangeStatement;
    }

    FlowSignal execute(Environment& env) override;
};

// forEach -------------------------------------------------------------------------
struct ForEachStatement : public BlockStatement {
    uint32_t mIteratorVarNameSymbolId = 0;
    std::unique_ptr<Expression> mVarExpr;

    ForEachStatement(uint32_t nameSymId, std::unique_ptr<Expression> varExpr)
    : mIteratorVarNameSymbolId(nameSymId), mVarExpr(std::move(varExpr)) {
        mNodeType  = NodeType::ForEachStatement;
    }

    FlowSignal execute(Environment& env) override;
};

// break -------------------------------------------------------------------------
struct BreakStatement : public FlowBaseStatement {
    BreakStatement() {
        mNodeType  = NodeType::BreakStatement;
    }
    inline FlowSignal execute(Environment& env) override {
        return FlowSignal::Break;
    };
};
// continue  -------------------------------------------------------------------------
struct ContinueStatement : public FlowBaseStatement {
    ContinueStatement() {
        mNodeType  = NodeType::ContinueStatement;
    }
    inline FlowSignal execute(Environment& env) override {
        return FlowSignal::Continue;
    };
};
// return -------------------------------------------------------------------------
struct ReturnStatement : public FlowBaseStatement {
    std::unique_ptr<Expression> mExpression;
    ReturnStatement(std::unique_ptr<Expression> expr) : mExpression(std::move(expr)) {
        mNodeType  = NodeType::ReturnStatement;
    }
    FlowSignal execute(Environment& env) override;
};
// While -------------------------------------------------------------------------
struct WhileStatement : public BlockStatement {
    std::unique_ptr<Expression> mCondition;
    WhileStatement(std::unique_ptr<Expression> cond) : mCondition(std::move(cond)) {
        mNodeType  = NodeType::WhileStatement;
    }
    FlowSignal execute(Environment& env) override;
};
} //namespace
