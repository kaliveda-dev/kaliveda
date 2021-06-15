\defgroup Stopping Energy Loss & Range Calculations
\brief Range tables and energy loss calculations for charged particles

Energy losses and ranges of charged particles in matter can be calculated for
a wide range of materials and ions, principally through the classes
KVMaterial, KVDetector and KVTarget:

~~~~{.cpp}
KVMaterial mat("CsI", 10*KVUnits::cm);

mat.GetRange(2,4,20)      // range of alpha with KE 20 MeV
9.83723725162513885e-02   // in g/cm**2

mat.GetLinearRange(2,4,20)
2.18120548833410678e-02   // in cm

KVNucleus a("4He", 5.0);                // alpha particle with KE 5 MeV/u
KVDetector det("Si",150*KVUnits::um);   // silicon detector 150um thick

det.GetELostByParticle(&a)    // dE of 20-MeV alpha in 150um of silicon
1.02854420719389914e+01   // in MeV
~~~~

In actual fact, these classes do not contain the code required to calculate
the range or energy loss of particles in matter. They are just an interface to
a class derived from KVIonRangeTable, of which two exist in KaliVeda:
KVedaLoss (`VEDALOSS`) and KVRangeYanez (`RANGE`). By default the range table is `VEDALOSS`.

  + `VEDALOSS` uses a 6-parameter fit to the range for each \f$Z\f$ from 1 to 100 in any of the predefined
    materials for which fits are available.
    The fitted ranges were calculated using Northcliffe-Shilling stopping powers (electronic + nuclear)
    up to 2.5 MeV/nucleon, and Hubert-Bimbot-Gauvin tables from 2.5 MeV/nucleon upwards.
    
  + `RANGE` can calculate ranges for any ion/material using either the Northcliffe-Schilling or the
    Hubert-Bimbot-Gauvin tables, or a mix of the two. By default the switch from NS to HBG occurs at
    2.5 MeV/nucleon without interpolation. Only 9 predefined compounds/mixtures are available by default.

## Changing the default range table

The default range table used can be defined/changed in your `.kvrootrc` file:

~~~~~~~~~~~~~~~
# Ion range table used by KVMaterial. You can change this to use a different plugin defined as above.
KVMaterial.IonRangeTable:   VEDALOSS
~~~~~~~~~~~~~~~

To know which range table is the current default:

~~~~~~~~~~~~~~{.cpp}
KVMaterial::GetRangeTable()->GetName()
"RANGE"          // KVRangeYanez table is default
~~~~~~~~~~~~~~

To change the default range table at any time:

~~~~~~~~~~~~~~{.cpp}
KVMaterial::ChangeRangeTable("RANGE")

Info in <KVRangeYanez::ReadPredefinedMaterials>: Reading materials in file : rangeyanez_compounds.data
Material : Mylar (Myl)   State : solid
Material : Kapton (Kap)   State : solid
Material : Octofluoropropane (C3F8)   State : gas
Material : Cesium Iodide (CsI)   State : solid
Material : Potassium Chloride (KCl)   State : solid
Material : Tetrafluoromethane (CF4)   State : gas
Material : Isobutane  (C4H10)   State : gas
Material : NE102/NE110 Scintillator (NE102)   State : solid
Material : Air (Air)   State : gas
~~~~~~~~~~~~~~
 
## Adding new materials to the range tables
New materials can be added to either of the range tables by the user at any time. These new materials will be
automatically saved and reloaded the next time that the range table is used. New materials can either be
'elemental' (i.e. composed of a single atomic element), 'compound' or a 'mixture'. An 'elemental' material
can either be isotopically pure or composed of a mixture of stable isotopes according to natural abundance.
The following 3 methods are defined for all range tables (see KVIonRangeTable class reference for more details):

~~~~{.cpp}
//
// methods of KVIonRangeTable and derived classes
//

// add isotopically-pure or mixture of naturally-occurring isotopes of one element
AddElementalMaterial(Int_t Z, Int_t A = 0)

// add compound composed of nelem different atoms. give number of atoms of each Z & A in formula, plus density if solid.
AddCompoundMaterial(const Char_t *name, const Char_t *symbol, Int_t nelem, Int_t* z, Int_t* a, Int_t* natoms, Double_t density = -1.0)

// add mixture composed of nelem different atoms. give number of atoms of each Z & A and proportions of mixture, plus density if solid.
AddMixedMaterial(const Char_t *name, const Char_t *symbol, Int_t nelem, Int_t* z, Int_t* a, Int_t* natoms, Double_t* proportion, Double_t density = -1.0)
~~~~

Adding a new material to the `VEDALOSS` range tables means generating the range of ions \f$Z=1-100\f$ in the
material and fitting them with the `VEDALOSS` parameterisation. This is handled by the KVedaLossRangeFitter class.
We use the `RANGE` tables to generate any new materials, and in addition you can directly add a material which
has already been defined for `RANGE` to `VEDALOSS` with method

~~~~{.cpp}
// import RANGE material in to VEDALOSS
KVedaLoss::AddRANGEMaterial(const Char_t* name)
~~~~

Each new material's definitions will be written in a file `[material_name].dat` which will be stored in one of two subdirectories
(`VEDALOSS` or `RANGE`) in your KaliVeda working directory (path returned by KVBase::GetWORKDIRFilePath()), in other words:

~~~~{.bash}
# for a 'standard' installation:
${KVROOT}/VEDALOSS
${KVROOT}/RANGE

# GNU-style install (option -Dgnuinstall=yes given to cmake)
${HOME}/.kaliveda/VEDALOSS
${HOME}/.kaliveda/RANGE
~~~~

Adding such a file to one of these directories from elsewhere will also add the new material to the relevant range table
the next time that it is initialised.
