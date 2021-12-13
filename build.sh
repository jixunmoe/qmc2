#!/bin/sh

set -e

pushd "$(realpath "$(dirname "$0")")"

REPO_ROOT="$PWD"

build() {
  mkdir -p "build/bin"

  case "$1" in
  win_i686) build_win i686 ;;
  win_x86_64) build_win x86_64 ;;

  linux_x64) build_linux_x64 ;;
  # armhf) build_armhf ;;
  # arm64) build_arm64 ;;
  *)
    echo unknown arch "'$1'"
    return 1
    ;;
  esac
}

build_win() {
  ARCH_NAME="$1" # e.g. x86_64
  mkdir -p "build/win_${ARCH_NAME}"
  pushd "build/win_${ARCH_NAME}"
  cmake ../.. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="${REPO_ROOT}/cmake/mingw-${ARCH_NAME}-linux.cmake"
  make
  cp "QMC2-decoder/QMC2-decoder.exe" \
    "../bin/QMC2-decoder.${ARCH_NAME}.exe"
  (
    cd ../bin
    zip -9 "QMC2-decoder.win.${ARCH_NAME}.zip" \
      "QMC2-decoder.${ARCH_NAME}.exe"
  )
  popd # build/win_???
}

build_linux_x64() {
  mkdir -p "build/linux_x86_64"
  pushd "build/linux_x86_64"
  cmake ../.. -DCMAKE_BUILD_TYPE=Release
  make
  cp "QMC2-decoder/QMC2-decoder" \
    "../bin/QMC2-decoder.linux.x86_64"
  (
    cd ../bin
    tar zcvf "QMC2-decoder.linux.x86_64.tar.gz" \
      "QMC2-decoder.linux.x86_64"
  )
  popd # build/linux_x86_64
}

if [ $# -eq 0 ]; then
  build win_i686
  build win_x86_64
  build linux_x64
  # build armhf
  # build arm64
else
  while [ $# -gt 0 ]; do
    build "$1"
    shift
  done
fi
