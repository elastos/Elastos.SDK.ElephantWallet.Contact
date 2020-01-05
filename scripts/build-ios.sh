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

"$SCRIPT_DIR/build.sh" --platform iOS --arch arm64;
"$SCRIPT_DIR/build.sh" --platform iOS --arch x86_64;

IOS_DIR="$PROJECT_DIR/ios";
cd "$IOS_DIR";
export CURRENT_PROJECT_VERSION=${PROJECT_REVISION};
export CURRENT_PROJECT_VERSIONNAME=${PROJECT_VERSION/v/};
echo "Building anno ...";
xcodebuild -target "anno"
echo "Building lib ...";
xcodebuild -target "lib" -configuration "${PROJECT_BUILDTYPE}" -arch arm64  -sdk "iphoneos" \
    CURRENT_PROJECT_VERSION=${PROJECT_REVISION} CURRENT_PROJECT_VERSIONNAME=${PROJECT_VERSION/v/};
xcodebuild -target "lib" -configuration "${PROJECT_BUILDTYPE}" -arch x86_64 -sdk "iphonesimulator" \
    CURRENT_PROJECT_VERSION=${PROJECT_REVISION} CURRENT_PROJECT_VERSIONNAME=${PROJECT_VERSION/v/};
echo "Building sdk ...";
xcodebuild -target "sdk" -configuration "${PROJECT_BUILDTYPE}" -arch arm64  -sdk "iphoneos" \
    CURRENT_PROJECT_VERSION=${PROJECT_REVISION} CURRENT_PROJECT_VERSIONNAME=${PROJECT_VERSION/v/};
xcodebuild -target "sdk" -configuration "${PROJECT_BUILDTYPE}" -arch x86_64 -sdk "iphonesimulator" \
    CURRENT_PROJECT_VERSION=${PROJECT_REVISION} CURRENT_PROJECT_VERSIONNAME=${PROJECT_VERSION/v/};

echo "Lipo lib ...";
TARGET_CROSSPL="$PACKAGE_DIR/CrossPL.framework";
rm -rf "$TARGET_CROSSPL" && mkdir -p "$TARGET_CROSSPL";
cp -r "$IOS_DIR/build/${PROJECT_BUILDTYPE}-iphonesimulator/CrossPL.framework/"* "$TARGET_CROSSPL/";
cp -r "$IOS_DIR/build/${PROJECT_BUILDTYPE}-iphoneos/CrossPL.framework/"* "$TARGET_CROSSPL/";
rm "$TARGET_CROSSPL/CrossPL";
rm -rf "$TARGET_CROSSPL/_CodeSignature";
lipo -create -output "$TARGET_CROSSPL/CrossPL" \
	"$IOS_DIR/build/${PROJECT_BUILDTYPE}-iphoneos/CrossPL.framework/CrossPL" \
	"$IOS_DIR/build/${PROJECT_BUILDTYPE}-iphonesimulator/CrossPL.framework/CrossPL";

echo "Lipo sdk ...";
TARGET_SDK="$PACKAGE_DIR/ContactSDK.framework";
rm -rf "$TARGET_SDK" && mkdir -p "$TARGET_SDK";
cp -r "$IOS_DIR/build/${PROJECT_BUILDTYPE}-iphonesimulator/ContactSDK.framework/"* "$TARGET_SDK/";
cp -r "$IOS_DIR/build/${PROJECT_BUILDTYPE}-iphoneos/ContactSDK.framework/"* "$TARGET_SDK/";
rm "$TARGET_SDK/ContactSDK";
rm -rf "$TARGET_SDK/_CodeSignature";
lipo -create -output "$TARGET_SDK/ContactSDK" \
	"$IOS_DIR/build/${PROJECT_BUILDTYPE}-iphoneos/ContactSDK.framework/ContactSDK" \
	"$IOS_DIR/build/${PROJECT_BUILDTYPE}-iphonesimulator/ContactSDK.framework/ContactSDK";

git tag --force ${PROJECT_VERSION}

TARBALL_PATH="$PACKAGE_DIR/${PROJECT_NAME}-ios-${PROJECT_VERSION}.zip";
cd $PACKAGE_DIR;
rm -rf "$TARBALL_PATH";
zip -r "$TARBALL_PATH" $(basename "$TARGET_CROSSPL") $(basename "$TARGET_SDK");

echo "Done!!!";

