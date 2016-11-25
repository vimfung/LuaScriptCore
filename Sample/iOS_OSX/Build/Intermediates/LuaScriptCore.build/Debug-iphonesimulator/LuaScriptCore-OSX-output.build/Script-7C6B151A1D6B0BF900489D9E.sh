#!/bin/sh
# Sets the target folders and the final framework product.
TARGET_NAME=${PROJECT_NAME}-OSX
FMK_NAME=lib${TARGET_NAME}
DEST_LIB_NAME=lib${PROJECT_NAME}

# Install dir will be the final output to the framework.
# The following line create it in the root folder of the current project.
INSTALL_BASE_DIR=${SRCROOT}/../../Release/OSX/
INSTALL_DIR=${INSTALL_BASE_DIR}${DEST_LIB_NAME}.a

# Working dir will be deleted after the framework creation.
WRK_DIR=build
DEVICE_DIR=${WRK_DIR}/Release/${FMK_NAME}.a

# -configuration ${CONFIGURATION}
# Clean and Building both architectures.
xcodebuild -arch "i386" -arch "x86_64" -configuration "Release" -target "${TARGET_NAME}" -sdk macosx clean build

# Cleaning the oldest.
if [ -d "${INSTALL_BASE_DIR}" ]
then
rm -rf "${INSTALL_BASE_DIR}"
fi

mkdir -p "${INSTALL_BASE_DIR}include/"

cp -r "${WRK_DIR}/Release/include/LuaScriptCore-OSX/" "${INSTALL_BASE_DIR}include/"
cp "${DEVICE_DIR}" "${INSTALL_DIR}"

rm -r "${WRK_DIR}"

