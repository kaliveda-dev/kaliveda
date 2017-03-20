//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#ifndef __KVZGOUBICOMB_H
#define __KVZGOUBICOMB_H

#include "KVBase.h"
#include <vector>

class KVZGOUBIComb : public KVBase {
protected:
   std::vector<std::vector<int>> N1;
   std::vector<std::vector<int>> N2;
   std::vector<std::vector<int>> N3;
   std::vector<std:: vector<int>> N4;
   std::vector<std:: vector<int>> N5;
   std::vector<std:: vector<int>> N6;
   std::vector<std::vector<int>> N7;
   std::vector<std::vector<int>> N8;
   std::vector<std::vector<int>> N9;
   std::vector<std::vector<int>> N10;

public:
   KVZGOUBIComb();
   KVZGOUBIComb(const KVZGOUBIComb&);
   KVZGOUBIComb(const Char_t* name, const Char_t* title = "");

   virtual ~KVZGOUBIComb();
   virtual void Copy(TObject&) const;
   std::vector<std::vector<int>> comb(int N, int K);
   std::vector<std::vector<int>> allcomb(int N);
   std::vector<std::vector<int>> GetComb(int N);

   ClassDef(KVZGOUBIComb, 1) //Class used to access one ZGOUBI Trajectory
};

#endif
