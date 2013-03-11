DEPTH = ../../../../..

include $(DEPTH)/make/openclsdkdefs.mk 

####
#
#  Targets
#
####

OPENCL			= 1
SAMPLE_EXE		= 1
EXE_TARGET 		= simpleMultiExample
EXE_TARGET_INSTALL   	= simpleMultiExample

####
#
#  C/CPP files
#
####

FILES 	= simpleMultiExample
CLFILES = simpleMultiExample_kernels.cl 

LLIBS  	+= SDKUtil

include $(DEPTH)/make/openclsdkrules.mk 

