#$Id: Makefile,v 1.25 2009/03/03 14:27:15 franklan Exp $
#$Revision: 1.25 $
#$Date: 2009/03/03 14:27:15 $
#$Author: franklan $
#
#Makefile for KaliVeda web site/documentation

###########################################
#                                                                        #
#   DO NOT CHANGE ANY OF THE FOLLOWING   #
#                                                                        #
###########################################

include $(ROOT_MAKEFILE_PATH)/Makefile.arch

CXXFLAGS += -I$(KVINSTALLDIR)/include 

ifeq ($(debug),yes)
CXXFLAGS += -g
endif

KV_INC =
#-----------Handle backwards compatibility issues depending on ROOT version
include $(KVPROJ_ROOT_ABS)/Makefile.compat
#-------------------------------------------------------------------------------------------
CXXFLAGS += $(KV_INC)

ifeq ($(ARCH),win32)
LINK_LIBS = '$(KVROOT)\lib\libKVIndraFNL.lib' '$(KVROOT)\lib\libKVIndra5.lib' '$(KVROOT)\lib\libKVIndra.lib' '$(KVROOT)\lib\libKVMultiDet.lib'
else
LINK_LIBS = -L$(KVROOT)/lib -lKVIndraFNL -lKVIndra5 -lKVIndra -lKVMultiDet
endif

#ganil libraries for reading raw data only build on linux systems
ifeq ($(PLATFORM),linux)
export INDRADLT = yes
ifneq ($(WITH_VAMOS),no)
LINK_LIBS +=  -lVAMOS
endif
LINK_LIBS +=  -lROOTGanilTape -lgan_tape
CXXFLAGS += -I$(GANILTAPE_INC)
else
export INDRADLT = no
endif

ifeq ($(MAKE_GANTAPE_XRD),yes)
LINK_LIBS += -L$(ROOTSYS)/lib -lXrdPosix
endif

ifeq ($(ARCH),macosx)
LIBS += -L$(ROOTSYS)/lib -lPhysics -lGUI
endif

ifeq ($(ARCH),win32)
LIBS += '$(ROOTSYS)lib\libHtml.lib' '$(ROOTSYS)lib\libThread.lib'
else
LIBS += -lHtml -lThread
endif

ifeq ($(ARCH),win32)
MAKEHTMLS = make_html_doc.cxx
$(MAKEHTMLS): make_html_doc.C
	-cp make_html_doc.C $(MAKEHTMLS)
else
MAKEHTMLS = make_html_doc.C
endif
MAKEHTMLO = make_html_doc.$(ObjSuf)
MAKEHTML  = make_html_doc$(ExeSuf)
SCANCLASSESS = ScanClasses.cpp
SCANCLASSESO = ScanClasses.$(ObjSuf)

.PHONY : clean logs

clean:
	-rm -f $(MAKEHTMLO) $(SCANCLASSESO)  $(MAKEHTML)
        
$(MAKEHTML): $(SCANCLASSESO) $(MAKEHTMLO)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(LINK_LIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(MAKEHTMLO): $(MAKEHTMLS)
	$(CXX) $(CXXFLAGS) -c $<

$(SCANCLASSESO): $(SCANCLASSESS)
	$(CXX) $(CXXFLAGS) -c $<

install_html: $(MAKEHTML)
	-mkdir -p $(KVROOT)/KaliVedaDoc
	-mkdir -p $(KVROOT)/KaliVedaDoc/images
	-mkdir -p $(KVROOT)/KaliVedaDoc/css
	-mkdir -p $(KVROOT)/KaliVedaDoc/js
	-mkdir -p $(KVROOT)/KaliVedaDoc/tools
	-mkdir -p $(KVROOT)/KaliVedaDoc/KaliVedaGUI_fichiers
	./$(MAKEHTML)
	-cp ClassCategories.html $(KVROOT)/KaliVedaDoc/
	-cp examples/Examples.html $(KVROOT)/KaliVedaDoc/
#replace date and version in download.html	
	cat download.html | sed "s/KV_VERSION/${VERSION_NUMBER}-${KV_BUILD_DATE}/g" > tmp.html
	cat tmp.html | sed "s/KV_DATE/${KV_BUILD_DATE}/g" > Download.html
	-cp *.html $(KVROOT)/KaliVedaDoc/
	-cp Download.html $(KVROOT)/KaliVedaDoc/download.html
	-cp css/* $(KVROOT)/KaliVedaDoc/css/
	-cp js/* $(KVROOT)/KaliVedaDoc/js/
	-cp images/* $(KVROOT)/KaliVedaDoc/images/
	-cp tools/* $(KVROOT)/KaliVedaDoc/tools/
	-cp KaliVedaGUI_fichiers/* $(KVROOT)/KaliVedaDoc/KaliVedaGUI_fichiers/
	-rm -f $(KVROOT)/KaliVedaDoc/index.html
	-ln -s $(KVROOT)/KaliVedaDoc/about.html $(KVROOT)/KaliVedaDoc/index.html
	
logs :
	$(CVS2CL)/cvs2cl.pl --xml --xml-encoding iso-8859-15 --noxmlns -f ChangeLog.xml
	xsltproc -o ChangeLog.html $(CVS2CL)/cl2html-ciaglia.xslt ChangeLog.xml
