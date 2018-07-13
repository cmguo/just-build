#!/bin/bash

if [ -z ${STRATEGY} ]
then
make config=release ${PLATFORM} ${PACKET} info
else
make config=release ${PLATFORM}.${STRATEGY} ${PACKET} info
fi
