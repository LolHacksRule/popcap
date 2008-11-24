{
	if (!LockSurface())
		return;	
	
	int* aMaxRedTable = mDDInterface->mRedAddTable;
	int* aMaxGreenTable = mDDInterface->mGreenAddTable;
	int* aMaxBlueTable = mDDInterface->mBlueAddTable;
	
	int aRedShift = mDDInterface->mRedShift;
	int aGreenShift = mDDInterface->mGreenShift;
	int aBlueShift = mDDInterface->mBlueShift;
	
	int aFPSrcY = theSrcRect.mY * 0x10000;

	int anAddX = (theSrcRect.mWidth * 0x10000) / theDestRect.mWidth;
	int anAddY = (theSrcRect.mHeight * 0x10000) / theDestRect.mHeight;

	if (theColor == Color::White)
	{
		if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
		{
			ushort* aDestPixelsRow = ((ushort*) mLockedSurfaceDesc.lpSurface) + (theDestRect.mY * mLockedSurfaceDesc.lPitch/2) + theDestRect.mX;

			uint32 aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
			uint32 aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
			uint32 aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

			uint32 aRRoundAdd = aRMask >> 1;
			uint32 aGRoundAdd = aGMask >> 1;
			uint32 aBRoundAdd = aBMask >> 1;					
	
			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				ushort* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);
					
					uint32 src = READ_COLOR(aSrcPixels);
					uint32 dest = *aDestPixels;										
					
					if (src != 0)
					{											
						int r = aMaxRedTable[((dest & aRMask) + (src & aRMask)) >> aRedShift];
						int g = aMaxGreenTable[((dest & aGMask) + (src & aGMask)) >> aGreenShift];
						int b = aMaxBlueTable[((dest & aBMask) + (src & aBMask)) >> aBlueShift];

						*(aDestPixels _PLUSPLUS) = 
							(r << aRedShift) |
							(g << aGreenShift) |
							(b << aBlueShift);
					}
					else
						aDestPixels _PLUSPLUS;
						
					aFPSrcX += anAddX;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
				aFPSrcY += anAddY;
			}		
		}
		else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
		{
			uint32* aDestPixelsRow = ((uint32*) mLockedSurfaceDesc.lpSurface) + (theDestRect.mY * mLockedSurfaceDesc.lPitch/4) + theDestRect.mX;

			uint32 aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
			uint32 aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
			uint32 aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

			uint32 aRRoundAdd = aRMask >> 1;
			uint32 aGRoundAdd = aGMask >> 1;
			uint32 aBRoundAdd = aBMask >> 1;					
	
			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				uint32* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);
					
					uint32 src = READ_COLOR(aSrcPixels);
					uint32 dest = *aDestPixels;
															
					if (src != 0)
					{
						int r = aMaxRedTable[((dest & aRMask) + (src & aRMask)) >> aRedShift];
						int g = aMaxGreenTable[((dest & aGMask) + (src & aGMask)) >> aGreenShift];
						int b = aMaxBlueTable[((dest & aBMask) + (src & aBMask)) >> aBlueShift];							

						*(aDestPixels _PLUSPLUS) = 
							(r << aRedShift) |
							(g << aGreenShift) |
							(b << aBlueShift);
					}
					else
						aDestPixels _PLUSPLUS;
						
					aFPSrcX += anAddX;						
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/4;
				aFPSrcY += anAddY;
			}
		}
	}
	else
	{
		int ca = theColor.mAlpha;
		int cr = (theColor.mRed * ca) / 255;
		int cg = (theColor.mGreen * ca) / 255;
		int cb = (theColor.mBlue * ca) / 255;
	
		if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
		{
			ushort* aDestPixelsRow = ((ushort*) mLockedSurfaceDesc.lpSurface) + (theDestRect.mY * mLockedSurfaceDesc.lPitch/2) + theDestRect.mX;

			uint32 aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
			uint32 aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
			uint32 aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

			uint32 aRRoundAdd = aRMask >> 1;
			uint32 aGRoundAdd = aGMask >> 1;
			uint32 aBRoundAdd = aBMask >> 1;					
	
			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				ushort* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);
					
					uint32 src = READ_COLOR(aSrcPixels);
					uint32 dest = *aDestPixels;										
					
					if (src != 0)
					{
						int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + ((((src & aRMask) >> aRedShift) * cr) >> 8)];
						int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + ((((src & aGMask) >> aGreenShift) * cg) >> 8)];
						int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + ((((src & aBMask) >> aBlueShift) * cb) >> 8)];

						*(aDestPixels _PLUSPLUS) = 
							(r << aRedShift) |
							(g << aGreenShift) |
							(b << aBlueShift);
					}
					else
						aDestPixels _PLUSPLUS;

					aFPSrcX += anAddX;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
				aFPSrcY += anAddY;
			}		
		}
		else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
		{
			uint32* aDestPixelsRow = ((uint32*) mLockedSurfaceDesc.lpSurface) + (theDestRect.mY * mLockedSurfaceDesc.lPitch/4) + theDestRect.mX;

			uint32 aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
			uint32 aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
			uint32 aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

			uint32 aRRoundAdd = aRMask >> 1;
			uint32 aGRoundAdd = aGMask >> 1;
			uint32 aBRoundAdd = aBMask >> 1;					
	
			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				uint32* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);
					
					uint32 src = READ_COLOR(aSrcPixels);
					uint32 dest = *aDestPixels;					
					
					if (src != 0)
					{
						int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + ((((src & aRMask) >> aRedShift) * cr) >> 8)];
						int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + ((((src & aGMask) >> aGreenShift) * cg) >> 8)];
						int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + ((((src & aBMask) >> aBlueShift) * cb) >> 8)];

						*(aDestPixels _PLUSPLUS) = 
							(r << aRedShift) |
							(g << aGreenShift) |
							(b << aBlueShift);
					}
					else
						aDestPixels _PLUSPLUS;
						
					aFPSrcX += anAddX;					
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/4;
				aFPSrcY += anAddY;
			}
		}
	}
	
	UnlockSurface();
}