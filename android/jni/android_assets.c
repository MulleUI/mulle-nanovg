#include "android_assets.h"

#include <stdlib.h>
#include <string.h>


static AAssetManager  *g_assetManager;


void androidAssetsInit( AAssetManager *mgr)
{
   g_assetManager = mgr;
}


unsigned char *androidAssetLoad( const char *path, int *outSize)
{
   AAsset        *asset;
   unsigned char *buf;
   off_t          size;

   if( outSize)
      *outSize = 0;
   if( g_assetManager == NULL || path == NULL)
      return( NULL);

   asset = AAssetManager_open( g_assetManager, path, AASSET_MODE_BUFFER);
   if( asset == NULL)
      return( NULL);

   size = AAsset_getLength( asset);
   buf  = malloc( (size_t) size);
   if( buf == NULL)
   {
      AAsset_close( asset);
      return( NULL);
   }
   if( AAsset_read( asset, buf, (size_t) size) != size)
   {
      free( buf);
      AAsset_close( asset);
      return( NULL);
   }
   AAsset_close( asset);

   if( outSize)
      *outSize = (int) size;
   return( buf);
}