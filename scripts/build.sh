#!/bin/bash

cd `dirname $0`/../badge/build/
exec make "$@"
