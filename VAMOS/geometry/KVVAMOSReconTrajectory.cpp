//Created by KVClassFactory on Thu Mar 28 14:12:49 2013
//Author: Guilain ADEMARD

#include "KVVAMOSReconTrajectory.h"

ClassImp(KVVAMOSReconTrajectory)

////////////////////////////////////////////////////////////////////////////////
/* BEGIN_HTML
<center><h2>KVVAMOSReconTrajectory</h2></center>
<center><h4>Handle data used to reconstruct the trajectory of a nucleus in VAMOS</h4></center>

This class allows to handle data or coordinates used in the reconstruction of
the trajectory in the VAMOS spectrometer. This class is used both for the
reconstruction from the focal plane to the target point and for the inverse
reconstruction by the friend classes <a href="./KVVAMOSReconNuc.html">KVVAMOSReconNuc</a> and <a href="./KVVAMOSTransferMatrix.html">KVVAMOSTransferMatrix</a>.
<figure><a name="FIG1"></a>
<img src="http://indra.in2p3.fr/KaliVedaDoc/images/kvvamosrecontrajectory_refframes.png"><br>
<figcaption>Fig1. - Representation of the trajectory at
the <a href="#FP">focal plane</a> and at the target point (<a href="#LAB">lab frame</a>)
<br> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
within different coordinate systems. The angle alpha  is the rotation angle of VAMOS
<br> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
around the Y-axis in the VAMOS coordinate system.
</figcaption>
</figure>


The nucleus trajectory to be reconstructed is represented in <a href="#FIG1">Fig1.</a> by the velocity vector:
END_HTML
Begin_Latex( fontsize=20 )
#vec{v} #(){v_{x},v_{y},v_{z} }
End_Latex
BEGIN_HTML



<a name="CoordSys"></a><h2>Coordinate systems</h2>

<a name="LAB"></a><h3>1) In laboratory frame of reference (lab-frame)</h3>

<p>By convention, the direct coordinate system in KaliVeda
(called INDRA coord. syst. in <a href="#FIG1">Fig1.</a>) has the target position for its
center. The beam direction defines the positive (+ve) Z-axis, with respect to which
polar angles are measured. The +ve X-axis is taken to be vertical, and
azimuth angles increase when going clockwise from the +ve X-axis, looking
towards the +ve z direction. The angles in the INDRA spherical coord. syst. are named:
END_HTML
Begin_Latex( fontsize=20 )
#theta     (polar angle)
#phi     (azimuth angle)
End_Latex
BEGIN_HTML
theta varies between 0 and 180 degrees while phi varies between 0 and 359.999... degrees.

<p>In the VAMOS community, the convention is a little bit different. The direct
coord. syst. (calle VAMOS coord. syst. in <a href="#FIG1">Fig1.</a>) is
also centered at the target point but the +ve X-axis is taken to be horizontal
and the +ve Y-axis is taken to be vertical. The +ve Z-axis, polar angles and
azimuth angles are defined as previously. The angles in the VAMOS spherical coord. system are
named:
END_HTML
Begin_Latex( fontsize=20 )
#theta_{l}     (polar angle)
#phi_{l}     (azimuth angle)
End_Latex
BEGIN_HTML
the polar angle varies between 0 to 180 degrees and the azimuth angle between -179.999... to 180 degrees. They are accessible by
calling the methods <a href="#KVVAMOSReconTrajectory:GetThetaL">GetThetaL</a>
and <a href="#KVVAMOSReconTrajectory:GetPhiL">GetPhiL</a>

<p>For the trajectory reconstruction (see <a href="./KVVAMOSTransferMatrix.html">KVVAMOSTransferMatrix</a>)
, the algorithm does not used the polar coordinates but other coordinates
of the standard ion optics formalism. In this formalism two angles are defined:
<ul>
<li> theta_v: angle between the Z-axis and the projection of the velocity vector of  the trajectory
on the XZ plane (symmetry plane) in the lab-frame of reference.
It is returned by the method
   <a href="#KVVAMOSReconTrajectory:GetThetaV">GetThetaV</a>;</li>
<li> phi_v: angle between the velocity vector of the trajectory and
its projection on the XZ plane (symmetry plane) in the lab-frame.
It is returned by the method
   <a href="#KVVAMOSReconTrajectory:GetPhiV">GetPhiV</a>.</li>
</ul>
Both angles vary between -90 to 90 degrees.
The relations between the cartesian coordinates of the velocity and these angles in the lab-frame are:
END_HTML
Begin_Latex( fontsize=20 )
v_{x} = v sin(#theta_{v}) cos(#phi_{v})
v_{y} = v sin(#phi_{v})
v_{z} = v cos(#theta_{v}) cos(#phi_{v})
End_Latex
Begin_Latex( fontsize=20 )
#theta_{v} = arctan( v_{x}/v_{z} )
#phi_{v} = arcsin( v_{y}/v )
End_Latex
BEGIN_HTML

<a name="FP"></a><h3>2) In Focal-Plane frame of reference (FP-frame)</h3>

 The origin of the direct coord. syst. is the center of the focal plane,
 the +ve X-axis is taken to be horizontal and the +ve Y-axis is taken to be
 vertical as for the VAMOS coord. syst. in the lab-frame. The +ve Z-axis
 follows the reference trajectory, polar angles and azimuth angles are defined as previously.

The relations between the cartesian coordinates of the velocity and the different angles used in the FP-frame of reference are:
END_HTML
Begin_Latex( fontsize=20 )
v_{x} = v sin(#theta_{f}) cos(#phi_{f})
v_{y} = v sin(#phi_{f})
v_{z} = v cos(#theta_{f}) cos(#phi_{f})
End_Latex
Begin_Latex( fontsize=20 )
#theta_{f} = arctan( v_{x}/v_{z} )
#phi_{f} = arcsin( v_{y}/v )
End_Latex
BEGIN_HTML
These two angles vary between -90 to 90 degrees and are accessible by
calling the methods <a href="#KVVAMOSReconTrajectory:GetThetaF">GetThetaF</a>
and <a href="#KVVAMOSReconTrajectory:GetPhiF">GetPhiF</a>.

<p> The intersection point of the trajectory and the focal plane is located by
the cartesian coordinates Xf and Yf (see <a href="#FIG1">Fig1.</a>) in cm.
END_HTML
*/

void KVVAMOSReconTrajectory::init()
{
   // initializes data members
   pointFP[0] = pointFP[1] = -666;
   dirFP.SetXYZ(0, 0, -1);
   dirLab.SetXYZ(0, 0, -1);
   Brho = path = -666;
}
//________________________________________________________________

KVVAMOSReconTrajectory::KVVAMOSReconTrajectory()
{
   // Default constructor
   init();
}
//________________________________________________________________

KVVAMOSReconTrajectory::KVVAMOSReconTrajectory(const KVVAMOSReconTrajectory& obj)  : KVBase()
{
   // Copy constructor.
   // This constructor is used to make a copy of an existing KVVAMOSReconTrajectory
   // object.
   init();
   obj.Copy(*this);
}
//________________________________________________________________

KVVAMOSReconTrajectory::~KVVAMOSReconTrajectory()
{
   // Destructor
}
//________________________________________________________________

void KVVAMOSReconTrajectory::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   KVBase::Copy(obj);
   KVVAMOSReconTrajectory& CastedObj = (KVVAMOSReconTrajectory&)obj;
   CastedObj.pointFP[0] = pointFP[0];
   CastedObj.pointFP[1] = pointFP[1];
   CastedObj.dirFP  = dirFP;
   CastedObj.dirLab = dirLab;
   CastedObj.Brho = Brho;
   CastedObj.path = path;
}
//________________________________________________________________

void KVVAMOSReconTrajectory::Reset()
{
   //Reset to initial values
   init();
   ResetBit(kFPisOK);
   ResetBit(kLabIsOK);
   ResetBit(kFPtoLabAttempted);
   ResetBit(kLabToFPattempted);
}
