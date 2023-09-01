#!/bin/bash

scp sgx-step.ko MLSIDE:
ssh MLSIDE "sudo -S modprobe -a isgx msr || true; sudo -S insmod sgx-step.ko; sudo dmesg | tail"
