#!/bin/sh

cd `dirname $0`/../openocd/
exec ./openocd -f ./interface.cfg -f ./badge.cfg

