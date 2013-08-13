include $(top_srcdir)/common.am
noinst_LIBRARIES = libter.a
noinst_HEADERS = alignmentStruct.h bestShiftStruct.h documentStructure.h hashMap.h hashMapInfos.h hashMapStringInfos.h infosHasher.h multiEvaluation.h multiTxtDocument.h segmentStructure.h sgmlDocument.h stringHasher.h stringInfosHasher.h terAlignment.h tercalc.h terEvaluation.h terShift.h tinystr.h tools.h xmlStructure.h tinyxml.h

libter_a_SOURCES = alignmentStruct.cpp documentStructure.cpp hashMap.cpp hashMapInfos.cpp hashMapStringInfos.cpp infosHasher.cpp multiEvaluation.cpp multiTxtDocument.cpp segmentStructure.cpp sgmlDocument.cpp stringHasher.cpp stringInfosHasher.cpp terAlignment.cpp tercalc.cpp terShift.cpp tinystr.cpp tools.cpp
# tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp  xmlStructure.cpp

AM_CXXFLAGS += -W -Wall -Wno-sign-compare $(GTEST_CPPFLAGS) -I$(srcdir)/.. 
