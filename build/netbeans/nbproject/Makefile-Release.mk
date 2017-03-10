#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/56252444/cdc512.o \
	${OBJECTDIR}/_ext/56252444/indexer.o \
	${OBJECTDIR}/_ext/56252444/main.o \
	${OBJECTDIR}/_ext/61759c81/cdc512.o \
	${OBJECTDIR}/_ext/61759c81/indexer.o \
	${OBJECTDIR}/_ext/61759c81/locale_traits.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-std=c++17 -Wall -Wextra
CXXFLAGS=-std=c++17 -Wall -Wextra

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lsqlite3

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/netbeans

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/netbeans: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/netbeans ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/56252444/cdc512.o: ../../src/cdc512.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/56252444
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Werror -s -I. -Iinclude -I../../include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56252444/cdc512.o ../../src/cdc512.cpp

${OBJECTDIR}/_ext/56252444/indexer.o: ../../src/indexer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/56252444
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Werror -s -I. -Iinclude -I../../include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56252444/indexer.o ../../src/indexer.cpp

${OBJECTDIR}/_ext/56252444/main.o: ../../src/main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/56252444
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Werror -s -I. -Iinclude -I../../include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56252444/main.o ../../src/main.cpp

${OBJECTDIR}/_ext/61759c81/cdc512.o: ../../tests/cdc512.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/61759c81
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Werror -s -I. -Iinclude -I../../include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/61759c81/cdc512.o ../../tests/cdc512.cpp

${OBJECTDIR}/_ext/61759c81/indexer.o: ../../tests/indexer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/61759c81
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Werror -s -I. -Iinclude -I../../include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/61759c81/indexer.o ../../tests/indexer.cpp

${OBJECTDIR}/_ext/61759c81/locale_traits.o: ../../tests/locale_traits.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/61759c81
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Werror -s -I. -Iinclude -I../../include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/61759c81/locale_traits.o ../../tests/locale_traits.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
