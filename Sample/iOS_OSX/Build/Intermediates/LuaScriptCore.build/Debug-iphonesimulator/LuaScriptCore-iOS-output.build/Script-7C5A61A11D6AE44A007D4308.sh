#!/bin/sh
# Sets the target folders and the final framework product.
TARGET_NAME=${PROJECT_NAME}-iOS
FMK_NAME=lib${TARGET_NAME}
DEST_LIB_NAME=lib${PROJECT_NAME}

# Install dir will be the final output to the framework.
# The following line create it in the root folder of the current project.
INSTALL_BASE_DIR=${SRCROOT}/../../Release/iOS/
INSTALL_DIR=${INSTALL_BASE_DIR}${DEST_LIB_NAME}.a

# Working dir will be deleted after the framework creation.
WRK_DIR=build
DEVICE_DIR=${WRK_DIR}/Release-iphoneos/${FMK_NAME}.a
SIMULATOR_DIR=${WRK_DIR}/Release-iphonesimulator/${FMK_NAME}.a

# -configuration ${CONFIGURATION}
# Clean and Building both architectures.
xcodebuild OTHER_CFLAGS="-fembed-bitcode" -arch "armv7s" -arch "armv7" -arch "arm64" -configuration "Release" -target "${TARGET_NAME}" -sdk iphoneos clean build
xcodebuild OTHER_CFLAGS="-fembed-bitcode" -arch "i386" -arch "x86_64" -configuration "Release" -target "${TARGET_NAME}" -sdk iphonesimulator clean build

# Cleaning the oldest.
if [ -d "${INSTALL_BASE_DIR}" ]
then
rm -rf "${INSTALL_BASE_DIR}"
fi

mkdir -p "${INSTALL_BASE_DIR}include/"

cp -r "${WRK_DIR}/Release-iphoneos/include/LuaScriptCore-iOS/" "${INSTALL_BASE_DIR}include/"

# Uses the Lipo Tool to merge both binary files (i386 + armv6/armv7) into one Universal final product.
lipo -create "${DEVICE_DIR}" "${SIMULATOR_DIR}" -output "${INSTALL_DIR}"

rm -r "${WRK_DIR}"

