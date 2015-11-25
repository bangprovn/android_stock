#!/bin/bash
###############################################################################
#
#                           Kernel Build Script 
#
###############################################################################
# 2011-10-24 effectivesky : modified
# 2010-12-29 allydrop     : created
###############################################################################
##############################################################################
# set toolchain
##############################################################################
# export PATH=$(pwd)/$(your tool chain path)/bin:$PATH
# export CROSS_COMPILE=$(your compiler prefix)
#export PATH=$(pwd)/../../toolchain_arm-eabi-4.6/arm-eabi-4.6/bin:$PATH

export PATH=$(pwd)/../../../arm-eabi-4.6/bin:$PATH
export CROSS_COMPILE=arm-eabi-
make ARCH=arm msm8974_ef60s_tp20_user_defconfig
make -j8 ARCH=arm


