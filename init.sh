#!/bin/bash

if [ -z ${STRATEGY} ] 
then
OUT_DIR=output/${PLATFORM}/${PACKET}/release
else
OUT_DIR=output/${PLATFORM}.${STRATEGY}/${PACKET}/release
fi
