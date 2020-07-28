#ifndef KV3DGeoTrack_H
#define KV3DGeoTrack_H

#include "TPolyLine3D.h"
#include "TVirtualGeoTrack.h"
#include "KVNucleus.h"

/**
 \class KV3DGeoTrack
 \ingroup Geometry
 \brief Visualise particle trajectories through array geometry
 */

class KV3DGeoTrack : public TPolyLine3D {

protected:

   Int_t Color();
   Int_t          fIndex;
   TVirtualGeoTrack* fTrack;
   KVNucleus fNuc;

public:
   KV3DGeoTrack();
   KV3DGeoTrack(TVirtualGeoTrack*);
   virtual ~KV3DGeoTrack();
   virtual Int_t         DistancetoPrimitive(Int_t px, Int_t py);
   virtual void          ExecuteEvent(Int_t event, Int_t px, Int_t py);
   virtual const Text_t* GetName() const;
   virtual Text_t*       GetObjectInfo(Int_t px, Int_t py) const;
   void Draw(Option_t* option = "");
   Int_t GetZ() const
   {
      return fNuc.GetZ();
   }

   ClassDef(KV3DGeoTrack, 1) //Class to visualize tracks in OpenGL viewer
};
#endif





