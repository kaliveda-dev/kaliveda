\defgroup Calibration Detector Calibration
\brief Detector signal and calibration handling

# Detector signals and calibrations #

Each detector has an associated list of signals which may be DAQ parameters,
the result of some calibration procedure, or derived from a mathematical
combination of existing parameters:

~~~~~~~~~~{.cpp}
gMultiDetArray->GetDetector("SI_0801")->GetListOfDetectorSignals().ls()

OBJ: KVUniqueNameList	KVSeqCollection_6276	Optimised list in which objects with the same name can only be placed once : 0
 KVACQParamSignal 		 PG 		 Signal PG of detector SI_0801 		 [150.368108]
 KVCalibratedSignal 		 Volts 	 Signal Volts calculated from signal PG of detector SI_0801 		 [0.019724]
 KVCalibratedSignal 		 Energy 	 Signal Energy calculated from signal Volts of detector SI_0801 		 [8.311131]

gMultiDetArray->GetDetector("CSI-1111")->GetListOfDetectorSignals().ls()

OBJ: KVUniqueNameList	KVSeqCollection_33634	Optimised list in which objects with the same name can only be placed once : 0
 KVDetectorSignal 		 Q3.FastFPGAEnergy 		 Signal Q3.FastFPGAEnergy of detector CSI-1111 		 [0.000000]
 KVDetectorSignal 		 Q3.FPGAEnergy 		 Signal Q3.FPGAEnergy of detector CSI-1111 		 [0.000000]
 KVDetectorSignalExpression 		 ID_CSI-1111-VARX 		 Signal calculated as Q3.FastFPGAEnergy/Q3.FPGAEnergy for detector CSI-1111 		 [0.000000]
~~~~~~~~~~

The value of any signal can be retrieved using its name:

~~~~~~~~~~{.cpp}
gMultiDetArray->GetDetector("SI_0801")->GetDetectorSignalValue("PG")

(double) 234.
~~~~~~~~~~

If the signal in question is not defined, this method returns 0. You can test if
a given signal is defined for a detector using:

~~~~~~~~~~{.cpp}
gMultiDetArray->GetDetector("SI1-111")->HasDetectorSignal("Q3.FPGAEnergy")

(bool) false
~~~~~~~~~~

For more information on these methods, see the KVDetector class reference.

The base class which handles detector signals is KVDetectorSignal. As shown below, there is a family of related
classes which have very similar behaviour which handle specific types of signals:

![](http://indra.in2p3.fr/kaliveda/doc/master/classKVDetectorSignal__inherit__graph.png)


## Signal expressions ##

New signals can be defined and added to detectors using any mathematical expression
involving known signals. For example, in order to define the `ID_CSI-1111-VARX`
signal shown in the previous example above, you would go about it this way:

~~~~~~~~~~{.cpp}
gMultiDetArray->GetDetector("CSI-1111")->AddDetectorSignalExpression("ID_CSI-1111-VARX","Q3.FastFPGAEnergy/Q3.FPGAEnergy");

(bool) true
~~~~~~~~~~

Note that such signal expressions may be added automatically when an identification
telescope is initialised with an identification grid which uses such an
expression as either its `VARX` or `VARY` coordinate
(see <a href="http://indra.in2p3.fr/kaliveda/UsersGuide/identification.html">Particle identification</a> chapter in the User's Guide).

The method KVDetector::AddDetectorSignalExpression() returns `false` if there is a
problem with the expression (no known signals). In this case no new signal is added to the detector.

Detector signal expressions are handled by the KVDetectorSignalExpression class.

## Detector calibration ##

### Adding calibrated signals

Calibrated signals (handled by the KVCalibratedSignal class) are added to detectors when a calibration
is available for a given run of a dataset (experiment). In order to add calibrations to a dataset,
it needs to have a file `[array_name].CalibrationFiles.dat` which contains the names of files containing
different calibrations for different detectors and different runs.

Each of these files should have the same basic structure:

~~~~~~~~~~
# Any useful comments on a line starting with '#'
# Remember that comments are always useful
# You will remember nothing about this calibration in 3 months' time...
SignalIn:                                PG
SignalOut:                               Volts
CalibType:                               ChannelVolt
CalibOptions:                            func=pol3,min=0,max=1,inverse=true
SI_0801:                                 71.9228,3979.44,32.1727,-27.9312
SI_0802:                                 70.9138,3978.88,31.5155,-27.9611
[etc. etc.]
~~~~~~~~~~

This file defines how to transform the `PG` signal of the listed detectors into a new `Volts` signal.
A calibration basically defines how a new calibrated signal is generated from an existing input signal
using a KVCalibrator or derived class:

~~~~~~~~~~~~

   KVDetectorSignal  _______\   KVCalibrator  _______\  KVCalibratedSignal
       'input'              /                        /       'output'
       
~~~~~~~~~~~~

The "transfer function" of KVCalibrator (defined by the `func` parameter in `CalibOptions` above: see below)
can be any function which can be written as a string in the format understood by TF1 or TFormula.
The 'input signal' can of course itself be the result of a calibration procedure, i.e. it can be a
KVCalibratedSignal, or indeed any other signal derived from KVDetectorSignal
(see diagram above).

The file must define _at least_ the input and output signals using their names (`SignalIn` and `SignalOut`
parameters above) and the type of the calibration (used for naming objects). If a specific KVCalibrator-derived class
is to be used in order to perform the calibration i.e. one of the classes which appear in the following diagram:

![](http://indra.in2p3.fr/kaliveda/doc/master/classKVCalibrator__inherit__graph.png)

then a `CalibClass` parameter can be used to specify which one. However, this is not the name of a class: it is a name of one of the
plugins defined in order to extend the KVCalibrator base class. In order to know which plugin name corresponds
to which class, you can use the following two methods:

~~~~~~~~~~{.cpp}
KVBase::GetListOfPlugins("KVCalibrator")

(const Char_t *) "KVLightEnergyCsI KVLightEnergyCsIFull KVPulseHeightDefect KVRecombination"

KVBase::GetListOfPluginURIs("KVCalibrator")

(const Char_t *) "LightEnergyCsI LightEnergyCsIFull PulseHeightDefect Recombination"
~~~~~~~~~~

The order of the class/plugin names in the two lists is the same. In general, the plugin name is just the class name minus the `KV` prefix.

The `CalibOptions` parameter may be used if required to supply any additional information needed to set up the
calibrator object as you wish. If and how to use this parameter depends on which class you are using: look at
the class reference guide to see if the
class has a specialised version of the KVCalibrator::SetOptions() method
(see for example the KVCalibrator::SetOptions() method for the calibrator used in the example above). 

### Obtaining calibrated signal values

Once defined, the calibrated signal values are obtained in the same way as any other,
using the KVDetector::GetDetectorSignalValue() method:

~~~~~~~~~~{.cpp}
gMultiDetArray->GetDetector("SI_0801")->GetDetectorSignalValue("Volts")

(double) 0.040703783

gMultiDetArray->GetDetector("SI_0801")->GetDetectorSignalValue("Energy")

(double) 9.7544453
~~~~~~~~~~

#### Supplying extra parameters

If a calibration requires further parameters in order to be used correctly, they can be passed
to this method as a string containing `name=value` pairs. An example is the Light-Energy calibration for
CsI detectors, which depends on the identification of the particle for which the energy is to be
calculated:

~~~~~~~~~~{.cpp}
// calculate CsI energy from TotLight signal for a Lithium-6

gMultiDetArray->GetDetector("CSI_0801")->GetDetectorSignalValue("Energy", "Z=3,A=6");
~~~~~~~~~~

Any such required extra parameters will be defined in the `Compute` method of the calibrator
(for example, see KVLightEnergyCsI::Compute()).

#### Changing input value

Another use for the parameter list is to change the value of the input signal used by the calibration:
if a parameter `INPUT` is given in the list, its value will be used instead of the current value of the
input signal:

~~~~~~~~~~{.cpp}
// calculate Volts signal for SI_0801 when input signal (PG) = 1024

gMultiDetArray->GetDetector("SI_0801")->GetDetectorSignalValue("Volts", "INPUT=1024");
~~~~~~~~~~

### When is a detector considered 'calibrated' ? ###
By default a detector is considered to be calibrated _i.e._ the
method KVDetector::IsCalibrated() returns `kTRUE`{.cpp} if it has a signal defined called `Energy`.
However this behaviour may be modified in specific daughter classes.

### Inverse calibration ###

Given the situation in the previous examples i.e. where a detector called `SI_0801`
has one or more calibrations available, it is possible to "backtrack" from one
or other final output values and calculate what would be the input, _i.e._ the
calibration can be inverted, either partially or entirely:

~~~~~~~~~~{.cpp}
gMultiDetArray->GetDetector("SI_0801")->GetInverseDetectorSignalValue("Energy", 9.7544453, "Volts")

(double) 0.040703783

gMultiDetArray->GetDetector("SI_0801")->GetInverseDetectorSignalValue("Energy", 9.7544453, "PG")

(double) 234.07877

gMultiDetArray->GetDetector("SI_0801")->GetInverseDetectorSignalValue("Volts", 0.040735, "PG")

(double) 234.07877
~~~~~~~~~~
