#!/bin/bash

set -e
set -o pipefail

cd "$(dirname "$0")/.."

TARGET="${1:-RealmJS}"
CONFIG="${2:-Release}"

DEVICE_FRAMEWORK="build/$CONFIG-iphoneos/$TARGET.framework"
SIMULATOR_FRAMEWORK="build/$CONFIG-iphonesimulator/$TARGET.framework"
UNIVERSAL_FRAMEWORK="build/$CONFIG-universal/$TARGET.framework"

build() {
	xcodebuild -target "$TARGET" -configuration "$CONFIG" "$@"
}

build -sdk iphoneos
build -sdk iphonesimulator -arch i386 -arch x86_64 ONLY_ACTIVE_ARCH=NO

mkdir -p "$UNIVERSAL_FRAMEWORK"
rm -rf "$UNIVERSAL_FRAMEWORK"
cp -R "$DEVICE_FRAMEWORK" "$UNIVERSAL_FRAMEWORK"

xcrun lipo -create "$DEVICE_FRAMEWORK/$TARGET" "$SIMULATOR_FRAMEWORK/$TARGET" -output "$UNIVERSAL_FRAMEWORK/$TARGET"
