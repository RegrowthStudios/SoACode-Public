#pragma once

#ifndef macros_h__
#define macros_h__

#include "Tests.h"

using namespace System;

typedef bool(*TestFunc)();

#define TEST(NAME) \
namespace UnitTestsFuncs { \
    bool UT_##NAME(); \
    bool UT_##NAME##_init = UnitTests::Adder::TestsAdder::addTest(gcnew String(#NAME), IntPtr(UT_##NAME)); \
} \
bool UnitTestsFuncs::UT_##NAME() \

#endif // macros_h__
