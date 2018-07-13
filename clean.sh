#!/bin/bash

if [ -z ${STRATEGY} ]
then
make config=release -j4 ${PLATFORM} ${PACKET} clean
else
make config=release -j4 ${PLATFORM}.${STRATEGY} ${PACKET} clean
fi
