#ifndef ANDROID_ASSETS_H
#define ANDROID_ASSETS_H

#include <android/asset_manager.h>

// Must be called once at startup with the app's asset manager
// (app->activity->assetManager).
void androidAssetsInit( AAssetManager *mgr);

// Load an asset (relative to the APK's assets/ root) into a malloc'd buffer.
// Returns NULL if the asset does not exist. Caller owns the buffer
// (free() it), except when handed to nvgCreateFontMem with freeData=1,
// which takes ownership.
unsigned char *androidAssetLoad( const char *path, int *outSize);

#endif