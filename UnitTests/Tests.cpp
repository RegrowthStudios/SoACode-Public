#include "stdafx.h"
#include "Tests.h"

typedef bool(*TestFunc)();

bool isInit = false;
TestLibrary* m_tests;
TestLibrary& getLibrary() {
    if (!isInit) {
        m_tests = new TestLibrary();
        isInit = true;
    }
    return *m_tests;
}

bool UnitTests::Adder::TestsAdder::addTest(const nString& name, void* f) {
    getLibrary()[name] = f;
    return true;
}

bool UnitTests::Tests::runTest(const nString& name) {
    void* ptrFunc = getLibrary()[name];
    TestFunc f = (TestFunc)ptrFunc;
    try {
        return f();
    } catch (...) {
        return false;
    }
}

TestLibrary::iterator UnitTests::Tests::begin() {
    return getLibrary().begin();
}
TestLibrary::iterator UnitTests::Tests::end() {
    return getLibrary().end();
}

#ifdef OLD_CLR
UnitTests::Tests::TestLibrary::KeyCollection^ UnitTests::Tests::getTests() {
    return m_tests->Keys;
}
array<String^>^ UnitTests::Tests::getBatches(String^ name) {
    array<String^>^ a = { "_" };
    return name->Split(a, System::StringSplitOptions::RemoveEmptyEntries);
}
#endif // OLD_CLR
