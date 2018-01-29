﻿// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2016-2017 NVIDIA Corporation. All rights reserved.


#ifndef NVBLASTPREPROCESSORINTERNAL_H
#define NVBLASTPREPROCESSORINTERNAL_H


#include "NvPreprocessor.h"


/**
Macros for more convenient logging
*/
#define NVBLASTLL_LOG_ERROR(_logFn, _msg)		if (_logFn != nullptr) { _logFn(NvBlastMessage::Error, _msg, __FILE__, __LINE__); } ((void)0)
#define NVBLASTLL_LOG_WARNING(_logFn, _msg)		if (_logFn != nullptr) { _logFn(NvBlastMessage::Warning, _msg, __FILE__, __LINE__); } ((void)0)
#define NVBLASTLL_LOG_INFO(_logFn, _msg)		if (_logFn != nullptr) { _logFn(NvBlastMessage::Info, _msg, __FILE__, __LINE__); } ((void)0)
#define NVBLASTLL_LOG_DEBUG(_logFn, _msg)		if (_logFn != nullptr) { _logFn(NvBlastMessage::Debug, _msg, __FILE__, __LINE__); } ((void)0)


/** Blast will check function parameters for debug and checked builds. */
#define NVBLASTLL_CHECK_PARAMS (NV_DEBUG || NV_CHECKED)


#if NVBLASTLL_CHECK_PARAMS
#define NVBLASTLL_CHECK(_expr, _logFn, _msg, _onFail)																		\
	{																													\
		if(!(_expr))																									\
		{																												\
			if (_logFn) { _logFn(NvBlastMessage::Error, _msg, __FILE__, __LINE__); }									\
			{ _onFail; };																								\
		}																												\
	}																													
#else
#define NVBLASTLL_CHECK(_expr, _logFn, _msg, _onFail) NV_UNUSED(_logFn)
#endif


#endif // ifndef NVBLASTPREPROCESSORINTERNAL_H