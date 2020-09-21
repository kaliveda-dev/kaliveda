//Created by KVClassFactory on Thu Jan 19 17:29:54 2017
//Author: John Frankland,,,

#include "KVKinematicalFrame.h"
#include "TClass.h"

ClassImp(KVKinematicalFrame)

KVKinematicalFrame::KVKinematicalFrame(const Char_t* name, const KVParticle* original, const KVFrameTransform& trans)
   : TNamed(name, "Kinematical frame"), fTransform(trans), fParticle((KVParticle*)original->IsA()->New())
{
   // Create representation of original particle in transformed frame
   // This frame has a name which can be used to retrieve it from a list

   ReapplyTransform(original);
}

KVKinematicalFrame::KVKinematicalFrame(const KVFrameTransform& trans, const KVParticle* original)
   : TNamed(), fTransform(trans), fParticle((KVParticle*)original->IsA()->New())
{
   // Create representation of original particle in transformed frame

   ReapplyTransform(original);
}

KVKinematicalFrame::KVKinematicalFrame(KVParticle* p, const KVFrameTransform& t)
   : TNamed(), fTransform(t), fParticle(nullptr)
{
   // Modify the kinematics of the particle according to the given transformation
   // Recursively update the kinematics in all frames defined for this particle
   p->Transform(fTransform.Inverse());
   p->UpdateAllFrames();
}

KVKinematicalFrame::KVKinematicalFrame(const KVKinematicalFrame& o)
   : TNamed((const TNamed&)o), fTransform(o.fTransform),
     fParticle(o.GetParticle() ? (KVParticle*)o.GetParticle()->IsA()->New() : nullptr)
{
   // Copy constructor required for rootcint (not rootcling)
   if (GetParticle()) o.GetParticle()->Copy(*GetParticle());
}

KVKinematicalFrame& KVKinematicalFrame::operator=(const KVKinematicalFrame& o)
{
   // Assignment operator required for rootcint (not rootcling)

   if (&o == this) return (*this);
   fTransform = o.fTransform;
   fParticle.reset(o.GetParticle() ? (KVParticle*)o.GetParticle()->IsA()->New() : nullptr);
   if (GetParticle()) o.GetParticle()->Copy(*GetParticle());
   return *this;
}

void KVKinematicalFrame::ReapplyTransform(const KVParticle* original)
{
   // Apply stored kinematical transformation to the particle

   original->Copy(*(fParticle.get()));   //copy all information on particle
   fParticle->Transform(fTransform.Inverse());
}

void KVKinematicalFrame::ApplyTransform(const KVParticle* original, const KVFrameTransform& trans)
{
   // Apply new kinematical transformation to the particle and update all subframes

   fTransform = trans;
   ReapplyTransform(original);
   // recursively update all subframes
   fParticle->UpdateAllFrames();
}
