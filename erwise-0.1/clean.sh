#!/bin/sh
find . -name *.o|xargs rm
find . -name *.a|xargs rm
rm erwise/erwise

