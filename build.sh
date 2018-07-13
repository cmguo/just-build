#!/bin/bash

if [ -z ${STRATEGY} ]
then
make config=release -j4 ${PLATFORM} ${PACKET}
else
make config=release -j4 ${PLATFORM}.${STRATEGY} ${PACKET}
fi
