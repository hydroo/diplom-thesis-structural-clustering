#! /usr/bin/env bash

for a in $(ls | grep "test-"); do
	echo running $a.
	./$a/$a
done
