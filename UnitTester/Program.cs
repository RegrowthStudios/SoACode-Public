using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using UnitTests;

namespace UnitTester {
    class Program {
        static void Main(string[] args) {
            Console.WriteLine("Tests Start\n===========\n");

            // Run all the tests
            Stopwatch timer = new Stopwatch();
            var tests = Tests.getTests();
            foreach(String test in tests) {
                // Print friendly name
                String[] batches = Tests.getBatches(test);
                String friendly = String.Join("::", batches);
                Console.WriteLine("Running: {0}", friendly);

                // Run test
                timer.Reset();
                timer.Start();
                bool result = Tests.runTest(test);
                timer.Stop();
                Console.WriteLine("Result:  {0}", result ? "Pass" : "Fail");
                Console.WriteLine("Time:    {0} ms\n", timer.Elapsed.Milliseconds);
            }

            Console.WriteLine("\n===========\nTests Ended");
            Console.ReadLine();
        }
    }
}
