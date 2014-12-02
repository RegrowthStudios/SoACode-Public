#include "stdafx.h"
#include "Tests.h"

bool UnitTests::Adder::TestsAdder::addTest(String^ name, IntPtr f) {
    UnitTests::Tests::m_tests->Add(name, f);
    return true;
}

UnitTests::Tests::TestLibrary::KeyCollection^ UnitTests::Tests::getTests() {
    return m_tests->Keys;
}
bool UnitTests::Tests::runTest(String^ name) {
    IntPtr ptrFunc = m_tests[name];
    TestFunc f = (TestFunc)(void*)ptrFunc;
    try {
        return f();
    } catch (...) {
        return false;
    }
}

array<String^>^ UnitTests::Tests::getBatches(String^ name) {
    array<String^>^ a = { "_" };
    return name->Split(a, System::StringSplitOptions::RemoveEmptyEntries);
}

#include "Example.inl"