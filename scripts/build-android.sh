#!/bin/bash - 

set -o errexit
set -o nounset

SCRIPT_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd);
PROJECT_DIR=$(dirname "${SCRIPT_DIR}");
BUILD_DIR="$PROJECT_DIR/build";
PACKAGE_DIR="$BUILD_DIR/package";

PROJECT_NAME="Elastos.SDK.Contact";
PROJECT_BUILDTYPE="Release";
PROJECT_REVISION="$(git rev-list --count HEAD)";
PROJECT_VERSION="v0.1.$PROJECT_REVISION";

"$SCRIPT_DIR/build.sh" --platform Android --arch armeabi-v7a;
"$SCRIPT_DIR/build.sh" --platform Android --arch arm64-v8a;
"$SCRIPT_DIR/build.sh" --platform Android --arch x86_64;

mkdir -p "$PACKAGE_DIR";
CROSSPL_PATH="$PACKAGE_DIR/crosspl-lib-${PROJECT_VERSION}.aar";
TARGET_PATH="$PACKAGE_DIR/${PROJECT_NAME}-${PROJECT_VERSION}.aar";

ANDROID_DIR="$PROJECT_DIR/android";
cd "$ANDROID_DIR";
./gradlew :crosspl:lib:assembleRelease :sdk:assembleRelease -P versionCode=${PROJECT_REVISION} -P versionName=${PROJECT_VERSION/v/}
rm -rf "$TARGET_PATH";
cp "$ANDROID_DIR/crosspl/lib/build/outputs/aar/lib-release.aar" "$CROSSPL_PATH";
cp "$ANDROID_DIR/sdk/build/outputs/aar/sdk-release.aar" "$TARGET_PATH";
git tag --force ${PROJECT_VERSION}


TARBALL_PATH="$PACKAGE_DIR/${PROJECT_NAME}-android-${PROJECT_VERSION}.zip";
cd "$PACKAGE_DIR";
zip -r "$TARBALL_PATH" . -i $(basename "$TARGET_PATH") $(basename "$CROSSPL_PATH");

echo "Done!!!";

