DEPTH = ../../../../..

include $(DEPTH)/make/openclsdkdefs.mk 

####
#
#  Targets
#
####

OPENCL			= 1
SAMPLE_EXE		= 1
EXE_TARGET 		= simpleImageRotation
EXE_TARGET_INSTALL   	= simpleImageRotation

####
#
#  C/CPP files
#
####

FILES 	= simpleImageRotation
CLFILES = simpleImageRotation_kernels.cl 

LLIBS  	+= SDKUtil

include $(DEPTH)/make/openclsdkrules.mk 

