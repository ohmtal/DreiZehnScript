//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Loader
//-----------------------------------------------------------------------------
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

#include "Environment.h"
#include "bytecode/VM.h"
#include "toolbox/Tools.h"
#include "syntax13/Parser.h"

namespace DreiZehn {

    // enum class BlockType { Function, ForLoop, WhileLoop, IfBlock };
    //
    // struct OpenBlock {
    //     BlockType mType;
    //     uint32_t mFuncNameSymbolId;
    //     BlockStatement* mBlockNodePointer;
    //     U32 mLineNumber;
    // };



    std::shared_ptr<BlockStatement> ParseScriptToAST(Environment& env,std::istream& stream) {
        // std::string line;
        // int lineCount = 0;

        // auto mainProgram = std::make_shared<BlockStatement>();

        // std::vector<OpenBlock> blockStack;
        // blockStack.push_back({BlockType::IfBlock, 0, mainProgram.get(), 0});

        // concat Backslash lines !!!
        // std::string fullLine = "";
        // int logicalLineStart = 0;

        // while (std::getline(stream, line)) {
            // lineCount++;

            // if (fullLine.empty()) {
                // logicalLineStart = lineCount;
            // }

            // size_t lastRealChar = line.find_last_not_of(" \t\r\n");
            // if (lastRealChar != std::string::npos && line[lastRealChar] == '\\') {
            //     fullLine += line.substr(0, lastRealChar);
            //     continue;
            // } else {
            //     fullLine += line;
            // }

            // Globals::currentScriptLineNumber = logicalLineStart;

            // size_t firstRealChar = fullLine.find_first_not_of(" \t\r\n");
            // if (firstRealChar == std::string::npos) {
            //     fullLine.clear();
            //     continue;
            // }
            //
            // // Lua Style for Kate with lua syntax highlight
            // if (fullLine[firstRealChar] == '-' &&
            //     firstRealChar + 1 < fullLine.length() &&
            //     fullLine[firstRealChar + 1] == '-') {
            //     fullLine.clear();
            //     continue;
            // }

            // Globals::currentScriptLine = fullLine;

            std::ostringstream ss;
            ss << stream.rdbuf();
            std::string code = std::move(ss).str();

            Lexer lexer(code);
            // fullLine.clear();

            auto tokens = lexer.tokenize();
            if (tokens.size() == 0) return nullptr;

            Parser parser(env,tokens, &stream);
            // parser.lineNumber = logicalLineStart;
            // auto statements = parser.parse();
            return  parser.parse();

            // for (auto& ast : statements) {
            //     if (!ast) return nullptr;
            //
            //     // --- fn ---
            //     if (auto* startNode = dynamic_cast<FunctionDefineStartNode*>(ast.get())) {
            //         blockStack.push_back({BlockType::Function, startNode->mFnNameSymbolId, nullptr, parser.lineNumber});
            //         mainProgram->mBody.push_back(std::make_shared<FunctionDefineStartNode>(startNode->mFnNameSymbolId)) ;
            //         continue;
            //     }
            //
            //
            //     bool isIf    = ast->mNodeType == NodeType::IfStatement;
            //     bool isFor   = ast->mNodeType == NodeType::ForStatement
            //                 || ast->mNodeType == NodeType::RangeStatement
            //                 || ast->mNodeType == NodeType::ForEachStatement;
            //     bool isWhile = ast->mNodeType == NodeType::WhileStatement;
            //
            //
            //     if (isFor || isWhile || isIf ) {
            //         std::shared_ptr<ASTNode> sharedBase = std::move(ast);
            //         std::shared_ptr<BlockStatement> sharedLoop = std::static_pointer_cast<BlockStatement>(sharedBase);
            //
            //         BlockType bType = isIf ? BlockType::IfBlock : (isFor ? BlockType::ForLoop : BlockType::WhileLoop);
            //         BlockStatement* blockPtr = sharedLoop.get();
            //
            //         auto& outerBlock = blockStack.back();
            //         if (outerBlock.mType == BlockType::Function) {
            //             FunctionMap::RegisteredScriptFunctions[outerBlock.mFuncNameSymbolId].body.push_back(sharedLoop);
            //
            //         } else
            //         // ifBlock
            //         if (outerBlock.mType == BlockType::IfBlock) {
            //             auto* parentIf = dynamic_cast<IfStatement*>(outerBlock.mBlockNodePointer);
            //             if (parentIf && parentIf->mElseBranch != nullptr) {
            //                 parentIf->mElseBranch->mBody.push_back(sharedLoop);
            //             } else {
            //                 outerBlock.mBlockNodePointer->mBody.push_back(sharedLoop);
            //             }
            //         } else
            //         // ForLoop / WhileLoop
            //         if (outerBlock.mType == BlockType::ForLoop
            //             || outerBlock.mType == BlockType::WhileLoop
            //         ) {
            //             outerBlock.mBlockNodePointer->mBody.push_back(sharedLoop);
            //         }
            //
            //         blockStack.push_back({bType, 0, blockPtr, parser.lineNumber});
            //         continue;
            //     }
            //     // ---- end -----
            //     else if (ast->mNodeType == NodeType::EndNode) {
            //         if (blockStack.size() <= 1) {
            //             parser.PrintParseError("Syntax-Error: 'end' without starting statement.");
            //             return nullptr;
            //         }
            //
            //         blockStack.pop_back();
            //
            //         continue;
            //     }
            //     else if (ast->mNodeType == NodeType::ElseMarkerNode) {
            //         if (blockStack.empty() || blockStack.back().mType != BlockType::IfBlock) {
            //             parser.PrintParseError("Syntax-Error: 'else' without matching 'if'.");
            //             return nullptr;
            //         }
            //         auto* actualIf = dynamic_cast<IfStatement*>(blockStack.back().mBlockNodePointer);
            //         if (actualIf) {
            //             auto elseBlock = std::make_shared<BlockStatement>();
            //             actualIf->mElseBranch = elseBlock;
            //         }
            //         continue;
            //     }
            //     else if (ast->mNodeType == NodeType::ElIfStatement) {
            //         if (blockStack.empty() || blockStack.back().mType != BlockType::IfBlock) {
            //             parser.PrintParseError("Syntax-Error: 'elif' without matching 'if'.");
            //             return nullptr;
            //         }
            //
            //         auto* parentIf = dynamic_cast<IfStatement*>(blockStack.back().mBlockNodePointer);
            //
            //
            //         std::shared_ptr<ElIfStatement> elifNode = std::shared_ptr<ElIfStatement>(static_cast<ElIfStatement*>(ast.release()));
            //         parentIf->mElseBranch = elifNode;
            //         blockStack.back().mBlockNodePointer = elifNode.get();
            //         continue;
            //     }
            //
            //     // ----------- CURRENT BLOCK -------------
            //     auto& currentBlock = blockStack.back();
            //     std::shared_ptr<ASTNode> sharedAst = std::move(ast);
            //
            //     if (currentBlock.mType == BlockType::Function) {
            //         FunctionMap::RegisteredScriptFunctions[currentBlock.mFuncNameSymbolId].body.push_back(sharedAst);
            //     } else {
            //         auto* actualIf = dynamic_cast<IfStatement*>(currentBlock.mBlockNodePointer);
            //         if (currentBlock.mType == BlockType::IfBlock && actualIf && actualIf->mElseBranch != nullptr) {
            //             actualIf->mElseBranch->mBody.push_back(sharedAst);
            //         } else {
            //             currentBlock.mBlockNodePointer->mBody.push_back(sharedAst);
            //         }
            //     }
            // } // for statements
        // } // while lines ...



        // if (blockStack.size() > 1) {
        //     Tools::errorf("Syntax-Error: One or more END statements are missing!\n");
        //     Tools::printf(" │─────────────────────────────────────────────────────────────────────\n");
        //     Tools::printf(" │ %d open nodes \n", (int)blockStack.size() - 1);
        //     Tools::printf(" │─────────────────────────────────────────────────────────────────────\n");
        //     for (size_t i = 1; i < blockStack.size() ; i++) {
        //         auto& block = blockStack[i];
        //         Tools::printf(" │ %04d ├─ %s\n", block.mLineNumber,  parser.getLine().c_str());
        //     }
        //     Tools::printf(" │\n");
        //     return nullptr;
        // }

        // return mainProgram;
    }

    // --------------------------------------------------------------------------------
    bool RunScriptStream(std::istream& stream, Environment& env) {

        auto mainProgram = ParseScriptToAST(env,stream);
        if (!mainProgram) return false;

        env.execute(mainProgram.get());

        return true;
    }

    #ifdef DREIZEHN_DEBUG
    bool PrintScriptFileAST(const std::string& filename, Environment& env) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            Tools::errorf("Script file cant be opened: %s\n", filename.c_str());
            return false;
        }
        Tools::printSeparator(80);
        Tools::printf(" --- %s --- \n",filename.c_str());
        Tools::printSeparator(80);
        auto mainProgram = ParseScriptToAST(env,file);
        if (!mainProgram) return false;
        mainProgram->print();
        Tools::printSeparator(80);
        return true;
    }
    #endif

    // --------------------------------------------------------------------------------
    Value RunScriptStreamByteCode(std::istream& stream, Environment& env, bool resetByteCode  ) {

        auto mainProgram = ParseScriptToAST(env,stream);
        if (!mainProgram) return false;

        if (resetByteCode) env.getVM().resetChunk();
        return env.getVM().compileExec(env, mainProgram.get());

    }

    // --------------------------------------------------------------------------------
    std::shared_ptr<BlockStatement> IncludeScriptFile(Environment& env,const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            Tools::errorf("Script file cant be opened: %s\n", filename.c_str());
            return nullptr;
        }

        return ParseScriptToAST(env,file);
    }
    // --------------------------------------------------------------------------------
    bool RunScriptFile(const std::string& filename, Environment& env, bool useByteCode) {
        std::ifstream file(filename);
        env.CurrentFilename = filename;
        if (!file.is_open()) {
            Tools::errorf("Script file cant be opened: %s\n", filename.c_str());
            return false;
        }
        if (useByteCode) {
            RunScriptStreamByteCode(file,env, false);
            return true ; //??
        }
        return RunScriptStream(file, env);
    }
}
