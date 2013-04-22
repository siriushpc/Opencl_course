DEPTH = ../../../../..

include $(DEPTH)/make/openclsdkdefs.mk 

####
#
#  Targets
#
####

OPENCL			= 1
SAMPLE_EXE		= 1
EXE_TARGET 		= convolucionPrueba
EXE_TARGET_INSTALL   	= convolucionPrueba

####
#
#  C/CPP files
#
####

FILES 	= convolucionPrueba
CLFILES = convolucionPrueba_kernels.cl 

LLIBS  	+= SDKUtil

include $(DEPTH)/make/openclsdkrules.mk 

