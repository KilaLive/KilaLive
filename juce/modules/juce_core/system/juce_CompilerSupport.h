/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

/*
   This file provides flags for compiler features that aren't supported on all platforms.
*/

//==============================================================================
// GCC
#if JUCE_GCC

#if (__GNUC__ * 100 + __GNUC_MINOR__) < 407
#error "JUCE requires GCC 4.7 or later"
#endif

#if !(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
#error "JUCE requires that GCC has C++11 compatibility enabled"
#endif

#define JUCE_COMPILER_SUPPORTS_NOEXCEPT 1
#define JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS 1
#define JUCE_STDLIB_HAS_STD_FUNCTION_SUPPORT 1

#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 500
#define JUCE_HAS_CONSTEXPR 1
#endif

#ifndef JUCE_EXCEPTIONS_DISABLED
#if !__EXCEPTIONS
#define JUCE_EXCEPTIONS_DISABLED 1
#endif
#endif
#endif

//==============================================================================
// Clang
#if JUCE_CLANG

#if (__clang_major__ < 3) || (__clang_major__ == 3 && __clang_minor__ < 3)
#error "JUCE requires Clang 3.3 or later"
#endif

#define JUCE_COMPILER_SUPPORTS_NOEXCEPT 1
#define JUCE_HAS_CONSTEXPR 1

#if defined(_LIBCPP_VERSION) || !(JUCE_MAC || JUCE_IOS)
#define JUCE_STDLIB_HAS_STD_FUNCTION_SUPPORT 1
#define JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS 1
#endif

#ifndef JUCE_COMPILER_SUPPORTS_ARC
#define JUCE_COMPILER_SUPPORTS_ARC 1
#endif

#ifndef JUCE_EXCEPTIONS_DISABLED
#if !__has_feature(cxx_exceptions)
#define JUCE_EXCEPTIONS_DISABLED 1
#endif
#endif

#endif

//==============================================================================
// MSVC
#if JUCE_MSVC

#if _MSC_VER < 1800  // VS2013
#error "JUCE requires Visual Studio 2013 or later"
#endif

#define JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS 1
#define JUCE_STDLIB_HAS_STD_FUNCTION_SUPPORT 1

#if _MSC_VER >= 1900  // VS2015
#define JUCE_COMPILER_SUPPORTS_NOEXCEPT 1
#define JUCE_HAS_CONSTEXPR 1
#else
#define _ALLOW_KEYWORD_MACROS 1  // prevent a warning
#undef noexcept
#define noexcept throw()
#endif

#ifndef JUCE_EXCEPTIONS_DISABLED
#if !_CPPUNWIND
#define JUCE_EXCEPTIONS_DISABLED 1
#endif
#endif
#endif

//==============================================================================
#if JUCE_HAS_CONSTEXPR
#define JUCE_CONSTEXPR constexpr
#else
#define JUCE_CONSTEXPR
#endif

#if !DOXYGEN
// These are old flags that are now supported on all compatible build targets
#define JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL 1
#define JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES 1
#define JUCE_DELETED_FUNCTION = delete
#endif

//==============================================================================
#if JUCE_ANDROID
#define JUCE_ATOMIC_AVAILABLE 1
#elif defined(_LIBCPP_VERSION)
#define JUCE_ATOMIC_AVAILABLE (_LIBCPP_VERSION >= 3700)
#elif defined(__GLIBCXX__)
#define JUCE_ATOMIC_AVAILABLE (__GLIBCXX__ >= 20130322)  // GCC versions 4.8 and later
#elif defined(_MSC_VER)
#define JUCE_ATOMIC_AVAILABLE 1  // Visual Studio 2013 and later
#else
#define JUCE_ATOMIC_AVAILABLE 0
#endif
