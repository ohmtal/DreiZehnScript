#pragma once

namespace DreiZehn {

    Value compileExec(Environment& env, ASTNode* node);

    Value runDirectThreadedVM(BytecodeChunk& chunk, U32 numLocals);


}
