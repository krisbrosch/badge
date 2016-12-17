#!/bin/bash
# Requires links or copies of interface config and openocd binary in openocd directory

IMAGE=""
if [[ $# -eq 1 ]] ; then
  IMAGE=`readlink -f "${1}"`
  if [[ ! -f $IMAGE ]] ; then
    echo "Couldn't find firmware hex file ${IMAGE}"
    exit -1
  fi
fi

cd `dirname $0`/../openocd/

if [[ $IMAGE == "" ]] ; then
  if [[ -f ../badge/build/badge.hex ]] ; then
    IMAGE=../badge/build/badge.hex
    echo "Using badge.hex from bulid"
  elif [[ -f ../badge/Debug/badge.hex ]] ; then
    IMAGE=../badge/Debug/badge.hex
    echo "Using badge.hex from Debug (CCS)"
  else
    echo "Couldn't find any compiled firmware hex files."
    exit -1
  fi
fi

./openocd -f ./interface.cfg -f ./badge.cfg \
        -c "msp432p4 init 0" \
        -c "msp432p4 mass_erase 0" \
        -c "msp432p4 init 0" \
        -c "flash write_image \"${IMAGE}\" 0 ihex" \
        -c "reset run" \
        -c "shutdown"

