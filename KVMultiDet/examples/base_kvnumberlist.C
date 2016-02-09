//# Examples of use of the KVNumberList class
//
// KVNumberList provides many tools for handling
// discontinuous ranges of integers.
//
// To execute this function, either do
//
// $ kaliveda
// root[0] .L base_kvnumberlist.C+
// root[1] test_kvnumberlist()
//
// or
//
// $ root
// root[0] KVBase::InitEnvironment()
// root[1] .L base_kvnumberlist.C+
// root[2] test_kvnumberlist()

#include "TStopwatch.h"
#include "KVNumberList.h"
#include "Riostream.h"
using namespace std;

void test_kvnumberlist()
{
   // Test constructors
   cout << "TEST CONSTRUCTORS" << endl << endl;
   KVNumberList a("1-10 13 6-7");
   a.SetName("a");
   a.Print();

   KVNumberList b(0, 10, 3);
   b.SetName("b");
   b.Print();

   KVNumberList c(a);
   c.SetName("c");
   c.Print();

   KVNumberList e = "12-15,18,22";
   e.SetName("e");
   e.Print();

   /* Test iteration over list */
   cout << endl << "TEST ITERATION" << endl << endl;
   e.Begin();
   while (! e.End()) {
      cout << "next e value = " << e.Next() << endl;
   }


   // Test modifiers
   cout << endl << "TEST MODIFIERS" << endl;
   /* Setting list */
   cout << "List setters" << endl << endl;
   c.Set("6,1-3,19 12-15");
   c.Print();
   c.Set(1, 12, 2);
   c.Print();
   c = a;
   c.Print();
   b.Copy(c);
   c.Print();

   /* Add values to list */
   cout << endl << "Add values to list" << endl << endl;
   TStopwatch w;
   KVNumberList d;
   for (int i = 0; i < 15000; i++) d.Add(i);
   d.Print();
   w.Print();

   for (int i = 1; i < 10; i += 2) c.Add(i);
   c.Print();

   c.Add("2-4");
   c.Print();

   c.Add(e);
   c.Print();

   int val[] = {11, 8, 10, 21, 16};
   c.Add(5, val);
   c.Print();

   // cumulative additions
   b.Copy(c);
   for (int i = 1; i < 10; i += 2) c.Add(i);
   c.Add("2-4");
   c.Add(e);
   c.Add(5, val);
   c.Print();

   /* Remove values from list */
   cout << endl << "Remove values from list" << endl << endl;
   c.Remove(1);
   c.Print();

   c.Remove("18,21-22");
   c.Print();

   c.Remove(a);
   c.Print();

   /* Intersection of lists */
   cout << endl << "Intersection of lists" << endl << endl;
   KVNumberList A("1-5"), B("3-7");
   A.Inter(B);
   A.Print();

   // List Properties
   cout << endl << "LIST PROPERTIES" << endl;
   cout << A.AsString() << " contains 5 ? : " << A.Contains(5) << endl;
   cout << A.AsString() << " First = " << A.First() << endl;
   cout << A.AsString() << " Last = " << A.Last() << endl;
   cout << A.AsString() << " NValues = " << A.GetNValues() << endl;
   cout << A.AsString() << " IsEmpty ? : " << A.IsEmpty() << endl;
   A.Clear();
   cout << A.AsString() << " IsEmpty ? : " << A.IsEmpty() << endl;

   A = "123-127 129";
   cout << A.AsString() << " IsFull() ? : " << A.IsFull() << endl;
   cout << A.AsString() << " IsFull(123,127) ? : " << A.IsFull(123, 127) << endl;

   // Member access
   cout << endl << "MEMBER ACCESS" << endl;
   KVNumberList ls1("10-15 20-25");
   cout << "\"" << ls1.AsString() << "\"" << ".At(3) = " << ls1.At(3) << endl;
   cout << "\"" << ls1.AsString() << "\"" << ".[5] = " << ls1[5] << endl;
   cout << "\"" << ls1.AsString() << "\"" << ".GetList() = " << ls1.GetList() << endl;
   cout << "\"" << ls1.AsString() << "\"" << ".GetExpandedList() = " << ls1.GetExpandedList() << endl;
   cout << "\"" << ls1.AsString() << "\"" << ".AsString(8) = ";
   cout << ls1.AsString(8) << endl;

   // List arithmetic
   cout << endl << "LIST ARITHMETIC" << endl;
   KVNumberList X("1-5"), Y("6-10");
   KVNumberList Z = X + Y;
   Z.Print();
   KVNumberList W = Z - X;
   W.Print();
   KVNumberList V = "1 3 5 7 9";
   V.GetComplementaryList().Print();
   V.GetSubList(3, 8).Print();

   // Misc
   cout << endl << "MISCELLANEOUS" << endl;
   cout << ls1.GetLogical("x") << endl;
   cout << ls1 << endl;
}
