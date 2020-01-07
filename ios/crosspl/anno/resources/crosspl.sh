#!/bin/bash

set -o nounset
set -o errexit

echo "Current Workspace: $(pwd)";
echo "ProductName: $PRODUCT_NAME"


PROJECT_DIR="$PWD";
CROSSPL_MOD_DIR="$PROJECT_DIR/$ANNO_PATH/";
CROSSPL_OUT_DIR="$PROJECT_DIR/build/Release/";
set +o nounset
if [ -z "$CROSSPL_TMP_DIR" ]; then
  CROSSPL_TMP_DIR="$PROJECT_DIR/$TARGET_PATH/autogen-proxy/";
fi
set -o nounset
mkdir -p "$CROSSPL_TMP_DIR";
echo "Make dir: $CROSSPL_TMP_DIR"

if [ ! -f "$CROSSPL_OUT_DIR"/anno ]; then
  xcodebuild -target "anno";
fi
"$CROSSPL_OUT_DIR"/anno "$CROSSPL_MOD_DIR/resources" "$PRODUCT_NAME" "$PRODUCT_BUNDLE_IDENTIFIER" "$PROJECT_DIR/$TARGET_PATH/" "$CROSSPL_TMP_DIR/";
