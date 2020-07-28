/*******************************************************************************
$Id: KVNumberList.h,v 1.20 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.20 $
$Date: 2009/03/03 14:27:15 $
$Author: franklan $
*******************************************************************************/

#ifndef __KVNUMBERLIST_H
#define __KVNUMBERLIST_H

#include <vector>
#include <TString.h>
#include <TArrayI.h>
#include <TObject.h>
#include <TList.h>
#include "KVConfig.h"

typedef std::vector<Int_t> IntArray;
typedef std::vector<Int_t>::iterator IntArrayIter;
typedef std::vector<Int_t>::const_iterator IntArrayCIter;

/**
\class KVNumberList
\brief Strings used to represent a set of ranges of values

Handles lists of positive numbers such as `"15-17 1-12 14"` or `"345,356-390"`.
Strings with this format can be handled and analysed by this class.
They can also be iterated over. Note that the numbers in the initializing string
do not have to be in ascending order, or indeed unique. The resulting KVNumberList
will contain only the unique values in ascending order.

### Examples

Create a new list with one of the constructors:
~~~~{.cpp}
    KVNumberList a("1-20, 51, 52-56");
    KVNumberList i{1,5,11,3,2}; // C++11 initializer list
~~~~
Change an existing list:
~~~~{.cpp}
    a.SetList("1001 1003-1005,1010")
~~~~
Get first and last values of ranges in list (i.e. largest and smallest included values)
~~~~{.cpp}
    a.First()   (would give 1001 for previous example)
    a.Last()   (would give 1010 for previous example)
~~~~
Get a vector filled with all values corresponding to ranges defined in list:
~~~~{.cpp}
    std::vector<int> val = a.GetArray():
~~~~
`val[0]` is same value as `a.First()`. `val[n-1]` is same value as `a.Last()`.

Use AsString() or GetList() method to obtain list in its most compact form, using the "1-20, 51, 52-56" format.

IsEmpty() returns kTRUE if the list is empty ;-p

#### Iterating over all numbers in the list

Initialise first by calling Begin(), then loop until End() returns kTRUE:

~~~~~~~~~~{.cpp}
    KVNumberList r("1-10");
    r.Begin();
    while( !r.End() ){
       Int_t next_val = r.Next();
       ...
    }
~~~~~~~~~~

If list is empty, End() always returns kTRUE and Next() returns -1.

Methods begin() and end() return a std:iterator for the underlying std::vector<int>.
This means (from C++11 onwards), range-based for loops can be used:

~~~~~~~~~~{.cpp}
   KVNumberList pl("1-3,6");
   for(auto i : pl) cout << i << " ";
~~~~~~~~~~

*/

class KVNumberList : public TObject {

   mutable TString fString;
   TArrayI* fLowerBounds; //->
   TArrayI* fUpperBounds; //->
   mutable Int_t fNLimits;              //number of limits in arrays
   mutable Int_t fMaxNLimits;           //size of arrays
   mutable Int_t fFirstValue;           //smallest value included in list
   mutable Int_t fLastValue;            //largest value included in list
   mutable Int_t fNValues;              //total number of values included in ranges

   mutable TString fTMPSTR;//! dummy string to compute AsString (non static)

   mutable IntArrayIter fIterIndex;//! used by Next() to iterate over list
   mutable IntArrayIter fEndList;//! used by Next() & End() to iterate over list
   mutable IntArray fValues;//! used by Next() to iterate over list
   TString   fName;//name of the list

   mutable Bool_t fIsParsed;//!

   void init_numberlist();
   void clear() const;
   void ParseList() const;
   void AddLimits(Int_t min, Int_t max) const;
   void AddLimits(TString& string) const;
   void ParseAndFindLimits(const TString& string, const Char_t delim) const;

public:

   KVNumberList();
   KVNumberList(const KVNumberList&);
   KVNumberList(const Char_t*);
   KVNumberList(Int_t);
   KVNumberList(Int_t deb, Int_t fin, Int_t pas);
#ifdef WITH_CPP11
   KVNumberList(std::initializer_list<int> L);
#endif
   virtual ~ KVNumberList()
   {
      delete fUpperBounds;
      delete fLowerBounds;
   }

   virtual void     SetName(const char* name);
   virtual const char* GetName() const
   {
      return fName.Data();
   }

   /* LIST MODIFIERS */
   /* Set the number list */
   void Set(const TString& l)
   {
      SetList(l);
   }
   void Set(Int_t min, Int_t max, Int_t pas = 1)
   {
      SetMinMax(min, max, pas);
   }
   void SetList(const TString&);
   void SetMinMax(Int_t min, Int_t max, Int_t pas = 1);
   KVNumberList& operator=(const KVNumberList&);
   void Copy(TObject&) const;

   /* Add numbers/lists to the list */
   void Add(Int_t);
   void Add(const KVNumberList&);
   void Add(Int_t, Int_t*);
   void Add(const IntArray&);

   /* Remove numbers/lists from the list */
   void Remove(Int_t);
   void Remove(const Char_t*);
   void Remove(const KVNumberList&);
   void Remove(Int_t n, Int_t* arr);

   /* Clear the list */
   void Clear(Option_t* = "");

   /* Intersection of two sets of numbers i.e. those which are in both lists */
   void Inter(const KVNumberList& list);

   /* LIST PROPERTIES */
   Bool_t Contains(Int_t val) const;
   Int_t First() const;
   Int_t Last() const;
   Int_t GetNValues() const;
   Int_t GetEntries() const
   {
      return GetNValues();
   }
   Bool_t IsEmpty() const
   {
      if (!fIsParsed) ParseList();
      return (fNValues == 0);
   }
   Bool_t IsFull(Int_t vinf = -1, Int_t vsup = -1) const;

   /* LIST MEMBER ACCESS */
   Int_t At(Int_t index) const;
   Int_t operator[](Int_t index) const;
   IntArray GetArray() const;
   const Char_t* GetList() const;
   const Char_t* GetExpandedList() const;
   const Char_t* AsString(Int_t maxchars = 0) const;
   const Char_t* AsHumanReadableString() const;

   Int_t GetRandom() const;

   /* LIST ITERATORS */
   Int_t Next(void) const;
   void Begin(void) const;
   Bool_t End(void) const
   {
      return (fIterIndex == fEndList);
   }
   IntArrayIter begin();
   IntArrayIter end();

   /* LIST ARITHMETIC OPERATIONS */
   KVNumberList operator+(const KVNumberList&);
   KVNumberList operator-(const KVNumberList&);
   KVNumberList GetComplementaryList() const;
   KVNumberList GetSubList(Int_t vinf, Int_t vsup) const;
   TList* CutInSubList(Int_t number);
   /* MISCELLANEOUS */
   /* Generate TTree::Draw selection string */
   TString GetLogical(const Char_t* observable) const;
   TString GetSQL(const Char_t* column) const;
   /* Convert to c-string */
   operator const char* () const
   {
      return GetList();
   }
   /* Print list properties */
   void Print(Option_t* = "") const;
   void PrintLimits() const;
   /* tests for equality */
   bool operator==(const KVNumberList&) const;
   bool operator!=(const KVNumberList&) const;

   ClassDef(KVNumberList, 3)    //Strings used to represent a set of ranges of values
};

#endif
