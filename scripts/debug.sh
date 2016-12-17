#!/bin/bash

cd `dirname $0`/../badge/build/
exec arm-none-eabi-gdb -q -ex "target remote localhost:3333" badge.out

