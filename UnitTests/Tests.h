#pragma once

#include "macros.h"

using namespace System;
using namespace System::Collections::Generic;

namespace UnitTests {
    /// Implementation hiding
    namespace Adder {
        private ref class TestsAdder {
        public:
            static bool addTest(String^ name, IntPtr f);
        };
    }

    /// Unit test manager
    public ref class Tests {
    public:
        typedef System::Collections::Generic::Dictionary<String^, IntPtr> TestLibrary; ///< Container type for tests

        /// Runs a unit test
        /// @param name: Name of test to run
        /// @return Success of the test
        static bool runTest(String^ name);
        /// @return List of all unit test names
        static TestLibrary::KeyCollection^ getTests();

        /// Convert a unit test name into a friendly representation
        /// @return Array of batch strings (l[l.Length - 1] = friendly name of test)
        static array<String^>^ getBatches(String^ name);

        static TestLibrary^ m_tests = gcnew TestLibrary(); ///< Member variable that stores tests
    };
}
