#!/bin/sh

set -e

# This script is executed from the top-level project root
DEPLOY_ROOT="$(pwd)"

# This is the CMake out-of-tree build dir
BUILD_DIR="/tmp/build"
# This is an esp-idf component dir (added to project EXTRA_COMPONENT_DIRS)
MODULE_DIR="/tmp"

# Remove first arg, treat the rest as SRCS
MOD_NAME="$1"
shift
SRCS="$@"

export IDF_PROJECT_PATH="${DEPLOY_ROOT}/firmware"

export PATH="${IDF_PROJECT_PATH}/xtensa-esp32-elf/bin:${PATH}"

export IDF_PATH="${IDF_PROJECT_PATH}/esp-idf"

cat << EOF > ${MODULE_DIR}/CMakeLists.txt
set(COMPONENT_REQUIRES actor_model)
set(COMPONENT_SRCS ${SRCS})
register_component()
MODULE_GENERATE_ELF(module_elf ${MOD_NAME} ${SRCS})
EOF

cat ${MODULE_DIR}/CMakeLists.txt

# Navigate to the firmware build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Copy config settings to build dir
cp -r "${IDF_PROJECT_PATH}/build/sdkconfig" "${BUILD_DIR}"

# Generate cmake build files
${IDF_PROJECT_PATH}/build/bin/cmake \
  -DSDKCONFIG="${BUILD_DIR}/sdkconfig" \
  -DPYTHON_DEPS_CHECKED:BOOL=ON \
  -DGIT_FOUND:BOOL=OFF \
  \
  "${IDF_PROJECT_PATH}"

# Build the elf
make "module_elf_TARGET"
