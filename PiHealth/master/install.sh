#!/bin/bash

source /etc/pihealth.conf

for (( i = start; i <= finish; i++ )); do
	mkdir -p ${datadir}/${prename}$i
done
