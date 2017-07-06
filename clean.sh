#!/bin/bash

make config=release -j4 ${PLATFORM}.${STRATEGY} ${PACKET} clean
