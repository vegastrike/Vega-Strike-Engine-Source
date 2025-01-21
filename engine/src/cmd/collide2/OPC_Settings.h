///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * OPC_Settings.h
 *
 * Copyright (C) 2001 Pierre Terdiman
 * Copyright (C) 2022, 2025 Stephen G. Tuggy
 * Copyright (C) 2023 Benjamen R. Meyer
 *
 * This file is part of OPCODE - Optimized Collision Detection
 * (http://www.codercorner.com/Opcode.htm) and has been
 * incorporated into Vega Strike
 * (https://github.com/vegastrike/Vega-Strike-Engine-Source).
 *
 * Public Domain
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Contains compilation flags.
 *  \file		OPC_Settings.h
 *  \author		Pierre Terdiman
 *  \date		May, 12, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_SETTINGS_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_SETTINGS_H

//! Use CPU comparisons (comment that line to use standard FPU compares)
#if defined(CS_PROCESSOR_X86)
// Defining this causes aliasing bugs. It is not legal C++ to do so.
// i.e. the AIR() and IR() defines are very buggy and actually cause wrong
// code in optimize.
//#define OPC_CPU_COMPARE
#endif

//! Use FCOMI / FCMOV on Pentium-Pro based processors (comment that line to use plain C++)
//	#define OPC_USE_FCOMI

//! Use epsilon value in tri-tri overlap test
#define OPC_TRITRI_EPSILON_TEST

//! Use tree-coherence or not [not implemented yet]
//	#define OPC_USE_TREE_COHERENCE

//! Use callbacks or direct pointers. Using callbacks might be a bit slower (but probably not much)
#define OPC_USE_CALLBACKS

//! Support triangle and vertex strides or not. Using strides might be a bit slower (but probably not much)
//	#define OPC_USE_STRIDE

//! Discard negative pointer in vanilla trees
#define OPC_NO_NEG_VANILLA_TREE

//! Use a callback in the ray collider
#define OPC_RAYHIT_CALLBACK

// NB: no compilation flag to enable/disable stats since they're actually needed in the box/box overlap test

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_SETTINGS_H
