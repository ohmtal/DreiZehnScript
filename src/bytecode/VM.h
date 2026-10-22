#pragma once

namespace DreiZehn {

    Value compileExec(Environment& env, ASTNode* node);

    Value runDirectThreadedVM(BytecodeChunk& chunk, size_t numLocals);
}
