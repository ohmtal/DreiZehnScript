//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// The power machine :)
//  1. Build AST
//  2. chunk.rawInstructions
//  3. threadChunk(chunk)
//  4. runDirectThreadedVM(chunk)

//-----------------------------------------------------------------------------
#pragma once


namespace DreiZehn {




Value runDirectThreadedVM(BytecodeChunk& chunk, size_t numLocals = 64);

//-----------------------------------------------------------------------------
} // namespace DreiZehn
