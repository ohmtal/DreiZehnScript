//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Loader
//-----------------------------------------------------------------------------
#include <fstream>
#include <istream>
#include <string>
#include <vector>

#include "Environment.h"
#include "toolbox/Tools.h"
#include "core/Parser.h"

namespace DreiZehn {


    std::shared_ptr<BlockStatement> ParseScriptToAST(std::istream& stream) {
        std::string line;
        int lineCount = 0;

        auto mainProgram = std::make_shared<BlockStatement>();

        std::vector<OpenBlock> blockStack;
        blockStack.push_back({BlockType::IfBlock, 0, mainProgram.get()});

        // concat Backslash lines !!!
        std::string fullLine = "";
        int logicalLineStart = 0;

        while (std::getline(stream, line)) {
            lineCount++;

            if (fullLine.empty()) {
                logicalLineStart = lineCount;
            }

            size_t lastRealChar = line.find_last_not_of(" \t\r\n");
            if (lastRealChar != std::string::npos && line[lastRealChar] == '\\') {
                fullLine += line.substr(0, lastRealChar);
                continue;
            } else {
                fullLine += line;
            }

            Globals::currentScriptLineNumber = logicalLineStart;

            size_t firstRealChar = fullLine.find_first_not_of(" \t\r\n");
            if (firstRealChar == std::string::npos) {
                fullLine.clear();
                continue;
            }

            // handled by lexer/parser
            // // Shell Script Style
            // if (fullLine[firstRealChar] == '#') {
            //     fullLine.clear();
            //     continue;
            // }
            // Lua Style
            if (fullLine[firstRealChar] == '-' &&
                firstRealChar + 1 < fullLine.length() &&
                fullLine[firstRealChar + 1] == '-') {
                fullLine.clear();
                continue;
            }

            Globals::currentScriptLine = fullLine;

            Lexer lexer(fullLine);
            fullLine.clear();

            auto tokens = lexer.tokenize();
            if (tokens.size() == 0) continue;

            Parser parser(tokens);
            auto statements = parser.parseStatements();

            for (auto& ast : statements) {
                if (!ast) continue;

                // --- fn ---
                if (auto* startNode = dynamic_cast<FunctionDefineStartNode*>(ast.get())) {
                    blockStack.push_back({BlockType::Function, startNode->mFnNameSymbolId, nullptr});
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
                        // pre elif:
                        // if (parentIf && parentIf->mIsInElseBranch) {
                        //     parentIf->mElseBody.push_back(sharedLoop);
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

                    blockStack.push_back({bType, 0, blockPtr});
                    continue;
                }
                else
                // ---- end -----
                // if (dynamic_cast<FunctionDefineEndNode*>(ast.get())) {
                if (ast->mNodeType == NodeType::EndNode) {
                    if (blockStack.size() <= 1) {
                        Tools::PrintParseError("Syntax-Error: 'end' without starting statement.");
                        return nullptr;
                    }

                    //NOTE nested if with less end :
                    // 0.6c removed again !
                    // while (blockStack.size() > 1 && blockStack.back().mIsImplicit) {
                    //     blockStack.pop_back();
                    // }

                    blockStack.pop_back();

                    continue;
                }
                else
                // --- else ---
                // pre elif:
                // if (ast->mNodeType == NodeType::ElseMarkerNode) {
                //     if (blockStack.empty() || blockStack.back().mType != BlockType::IfBlock) {
                //         Tools::PrintParseError("Syntax-Error: 'else' without matching 'if'.");
                //         return nullptr;
                //     }
                //     auto* actualIf = dynamic_cast<IfStatement*>(blockStack.back().mBlockNodePointer);
                //     if (actualIf) actualIf->mIsInElseBranch = true;
                //     continue;
                // }
                if (ast->mNodeType == NodeType::ElseMarkerNode) {
                    if (blockStack.empty() || blockStack.back().mType != BlockType::IfBlock) {
                        Tools::PrintParseError("Syntax-Error: 'else' without matching 'if'.");
                        return nullptr;
                    }
                    auto* actualIf = dynamic_cast<IfStatement*>(blockStack.back().mBlockNodePointer);
                    if (actualIf) {
                        auto elseBlock = std::make_shared<BlockStatement>();
                        actualIf->mElseBranch = elseBlock;
                    }
                    continue;
                }
                else
                if (ast->mNodeType == NodeType::ElIfStatement) {
                    if (blockStack.empty() || blockStack.back().mType != BlockType::IfBlock) {
                        Tools::PrintParseError("Syntax-Error: 'elif' without matching 'if'.");
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
                    // if (currentBlock.mType == BlockType::IfBlock && actualIf && actualIf->mIsInElseBranch) {
                    //     actualIf->mElseBody.push_back(sharedAst);
                    if (currentBlock.mType == BlockType::IfBlock && actualIf && actualIf->mElseBranch != nullptr) {
                        actualIf->mElseBranch->mBody.push_back(sharedAst);
                    } else {
                        currentBlock.mBlockNodePointer->mBody.push_back(sharedAst);
                    }
                }
            }
        }

        if (blockStack.size() > 1) {
            Tools::PrintParseError("Syntax-Error: missing end");
            return nullptr;
        }

        return mainProgram;
    }

    bool RunScriptStream(std::istream& stream, Environment& env) {

        auto mainProgram = ParseScriptToAST(stream);
        if (!mainProgram) return false;

        env.execute(mainProgram.get());

        return true;
    }


    // --------------------------------------------------------------------------------
    bool RunScriptFile(const std::string& filename, Environment& env) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            Tools::errorf("Script file cant be opened: %s\n", filename.c_str());
            return false;
        }
        return RunScriptStream(file, env);
    }
}
