using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnitTests;

namespace UnitTester {
    class Program {
        static void Main(string[] args) {
            Console.WriteLine("Beginning Tests\n===============\n");

            // Run all the tests
            var tests = Tests.getTests();
            foreach(String test in tests) {
                // Print friendly name
                String[] batches = Tests.getBatches(test);
                String friendly = String.Join("::", batches);
                Console.WriteLine("Running: {0}", friendly);

                // Run test
                bool result = Tests.runTest(test);
                Console.WriteLine("Result:  {0}\n", result ? "Succeeded" : "Failed");
            }

            Console.WriteLine("\n===========\nTests Ended");
            Console.ReadLine();
        }
    }
}
