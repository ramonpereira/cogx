#!/bin/bash

# sanity check
if [ $# -ne 1 ] ; then
  echo "Usage: `basename $0` <cast install prefix, e.g. /usr/local>"
  exit $E_BADARGS
fi

# startup the CAST servers. 

# where things are for runtime
CAST_DIR=$1
CAST_INSTALL_ROOT=${CAST_DIR}

CAST_BIN_PREFIX=bin
CAST_BIN_DIR=${CAST_INSTALL_ROOT}/${CAST_BIN_PREFIX}

CAST_LIB_PREFIX=lib/cast
CAST_LIB_DIR=${CAST_INSTALL_ROOT}/${CAST_LIB_PREFIX}

CAST_JAR=${CAST_INSTALL_ROOT}/share/java/cast.jar

WE_SET_ICE_CONFIG=1
CAST_CONFIG_PATH=share/cast/config/cast_ice_config
CAST_ICE_CONFIG=${CAST_INSTALL_ROOT}/${CAST_CONFIG_PATH}


# check for ice config info
if [[ "$ICE_CONFIG" ]] ; then
    echo "--------------------------------------------------------------------------"
    echo "ICE_CONFIG is already set.";
    echo "You should also include the contents of ${CAST_ICE_CONFIG} in your config." 
    echo "--------------------------------------------------------------------------"
    WE_SET_ICE_CONFIG=0
else
    export ICE_CONFIG=${CAST_ICE_CONFIG}
fi 

java -ea -classpath ${CAST_JAR}:$CLASSPATH cast.server.ComponentServer &
JAVA_SERVER_JOB=$!


SAVED_DYLIB_PATH=${DYLD_LIBRARY_PATH}
SAVED_LIB_PATH=${LD_LIBRARY_PATH}
export DYLD_LIBRARY_PATH=${CAST_LIB_DIR}:${DYLD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${CAST_LIB_DIR}:${LD_LIBRARY_PATH}
${CAST_BIN_DIR}/cast-server-c++ &
CPP_SERVER_JOB=$!

echo Java server: ${JAVA_SERVER_JOB}
echo CPP server: ${CPP_SERVER_JOB}


# if we're killed, take down everyone else with us
trap "kill  ${CPP_SERVER_JOB}; kill  ${JAVA_SERVER_JOB}" INT TERM EXIT

wait

# just in case
# kill ${CPP_SERVER_JOB}
# kill ${JAVA_SERVER_JOB}

# reset evn vars
export LD_LIBRARY_PATH=${SAVED_LIB_PATH}
export DYLD_LIBRARY_PATH=${SAVED_DYLIB_PATH}

if (( $WE_SET_ICE_CONFIG )) ; then
    unset ICE_CONFIG
fi
