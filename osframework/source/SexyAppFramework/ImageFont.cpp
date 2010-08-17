#include "Common.h"
#include "ImageFont.h"
#include "Graphics.h"
#include "Image.h"
#include "SexyAppBase.h"
#include "MemoryImage.h"
#include "AutoCrit.h"

using namespace Sexy;

CharData::CharData()
{
	mWidth = 0;
	mOrder = 0;
}

FontLayer::FontLayer(FontData* theFontData)
{
	mFontData = theFontData;
	mDrawMode = -1;
	mSpacing = 0;
	mPointSize = 0;
	mAscent = 0;
	mAscentPadding = 0;
	mMinPointSize = -1;
	mMaxPointSize = -1;
	mHeight = 0;
	mDefaultHeight = 0;
	mColorMult = Color::White;
	mColorAdd = Color(0, 0, 0, 0);
	mLineSpacingOffset = 0;
	mBaseOrder = 0;
}

FontLayer::FontLayer(const FontLayer& theFontLayer) :
	mFontData(theFontLayer.mFontData),
	mRequiredTags(theFontLayer.mRequiredTags),
	mExcludedTags(theFontLayer.mExcludedTags),
	mCharDataMap(theFontLayer.mCharDataMap),
	mColorMult(theFontLayer.mColorMult),
	mColorAdd(theFontLayer.mColorAdd),
	mImage(theFontLayer.mImage),
	mDrawMode(theFontLayer.mDrawMode),
	mOffset(theFontLayer.mOffset),
	mSpacing(theFontLayer.mSpacing),
	mMinPointSize(theFontLayer.mMinPointSize),
	mMaxPointSize(theFontLayer.mMaxPointSize),
	mPointSize(theFontLayer.mPointSize),
	mAscent(theFontLayer.mAscent),
	mAscentPadding(theFontLayer.mAscentPadding),
	mHeight(theFontLayer.mHeight),
	mDefaultHeight(theFontLayer.mDefaultHeight),
	mLineSpacingOffset(theFontLayer.mLineSpacingOffset),
	mBaseOrder(theFontLayer.mBaseOrder)
{
}

CharData* FontLayer::SetCharData(int theChar)
{
	CharDataMap::iterator anItr = mCharDataMap.find(theChar);
	if (anItr != mCharDataMap.end())
		return &anItr->second;
	return &mCharDataMap.insert(CharDataMap::value_type(theChar, CharData())).first->second;
}

CharData* FontLayer::GetCharData(int theChar)
{
	CharDataMap::iterator anItr = mCharDataMap.find(theChar);
	if (anItr != mCharDataMap.end())
		return &anItr->second;

	anItr = mCharDataMap.find('?');
	if (anItr != mCharDataMap.end())
		return &anItr->second;

	return &mCharDataMap.insert(CharDataMap::value_type('?',
							    CharData())).first->second;
}

bool FontLayer::isGlyph(int theChar)
{
	CharDataMap::iterator anItr = mCharDataMap.find(theChar);
	if (anItr != mCharDataMap.end())
		return true;

	return false;
}

FontData::FontData()
{
	mInitialized = false;

	mApp = NULL;
	mRefCount = 0;
	mDefaultPointSize = 0;
}

FontData::~FontData()
{
	DataElementMap::iterator anItr = mDefineMap.begin();
	while (anItr != mDefineMap.end())
	{
		std::wstring aDefineName = anItr->first;
		DataElement* aDataElement = anItr->second;

		delete aDataElement;
		++anItr;
	}
}

void FontData::Ref()
{
	mRefCount++;
}

void FontData::DeRef()
{
	if (--mRefCount == 0)
	{
		delete this;
	}
}

bool FontData::Error(const std::string& theError)
{
	if (mApp != NULL)
	{
		std::string anErrorString = mFontErrorHeader + theError;

		if (mCurrentLine.length() > 0)
		{
			anErrorString += " on Line " + StrFormat("%d:\r\n\r\n", mCurrentLineNum) + WStringToString(mCurrentLine);
		}

		mApp->Popup(anErrorString);
	}

	return false;
}

bool FontData::DataToLayer(DataElement* theSource, FontLayer** theFontLayer)
{
	*theFontLayer = NULL;

	if (theSource->mIsList)
		return false;

	std::wstring aLayerName = StringToUpper(((SingleDataElement*) theSource)->mString);

	FontLayerMap::iterator anItr = mFontLayerMap.find(aLayerName);
	if (anItr == mFontLayerMap.end())
	{
		Error("Undefined Layer");
		return false;
	}

	*theFontLayer = anItr->second;

	return true;
}

bool FontData::GetColorFromDataElement(DataElement *theElement, Color &theColor)
{
	if (theElement->mIsList)
	{
		DoubleVector aFactorVector;
		if (!DataToDoubleVector(theElement, &aFactorVector) && (aFactorVector.size() == 4))
			return false;

		theColor = Color(
			(int) (aFactorVector[0] * 255),
			(int) (aFactorVector[1] * 255),
			(int) (aFactorVector[2] * 255),
			(int) (aFactorVector[3] * 255));

		return true;
	}

	int aColor = 0;
	if (!StringToInt(((SingleDataElement*) theElement)->mString, &aColor))
		return false;

	theColor = aColor;
	return true;
}


bool FontData::HandleCommand(const ListDataElement& theParams)
{
	std::wstring aCmd = ((SingleDataElement*) theParams.mElementVector[0])->mString;

	bool invalidNumParams = false;
	bool invalidParamFormat = false;
	bool literalError = false;
	bool sizeMismatch = false;

#define LWS(x) StringToLower(x)
	aCmd = StringToLower(aCmd);
	if (aCmd == LWS(L"Define"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			if (!theParams.mElementVector[1]->mIsList)
			{
				std::wstring aDefineName = StringToUpper(((SingleDataElement*) theParams.mElementVector[1])->mString);

				if (!IsImmediate(aDefineName))
				{
					DataElementMap::iterator anItr = mDefineMap.find(aDefineName);
					if (anItr != mDefineMap.end())
					{
						delete anItr->second;
						mDefineMap.erase(anItr);
					}

					if (theParams.mElementVector[2]->mIsList)
					{
						ListDataElement* aValues = new ListDataElement();
						if (!GetValues(((ListDataElement*) theParams.mElementVector[2]), aValues))
						{
							delete aValues;
							return false;
						}

						mDefineMap.insert(DataElementMap::value_type(aDefineName, aValues));
					}
					else
					{
						SingleDataElement* aDefParam = (SingleDataElement*) theParams.mElementVector[2];

						DataElement* aDerefVal = Dereference(aDefParam->mString);

						if (aDerefVal)
							mDefineMap.insert(DataElementMap::value_type(aDefineName, aDerefVal->Duplicate()));
						else
							mDefineMap.insert(DataElementMap::value_type(aDefineName, aDefParam->Duplicate()));
					}
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"CreateHorzSpanRectList"))
	{
		if (theParams.mElementVector.size() == 4)
		{
			IntVector aRectIntVector;
			IntVector aWidthsVector;

			if ((!theParams.mElementVector[1]->mIsList) &&
				(DataToIntVector(theParams.mElementVector[2], &aRectIntVector)) &&
				(aRectIntVector.size() == 4) &&
				(DataToIntVector(theParams.mElementVector[3], &aWidthsVector)))
			{
				std::wstring aDefineName = StringToUpper(((SingleDataElement*) theParams.mElementVector[1])->mString);

				int aXPos = 0;

				ListDataElement* aRectList = new ListDataElement();

				for (uint32 aWidthNum = 0; aWidthNum < aWidthsVector.size(); aWidthNum++)
				{
					ListDataElement* aRectElement = new ListDataElement();
					aRectList->mElementVector.push_back(aRectElement);

					char aStr[256];
					
					sprintf(aStr, "%d", aRectIntVector[0] + aXPos);
					aRectElement->mElementVector.push_back(new SingleDataElement(StringToWString(aStr)));

					sprintf(aStr, "%d", aRectIntVector[1]);
					aRectElement->mElementVector.push_back(new SingleDataElement(StringToWString(aStr)));

					sprintf(aStr, "%d", aWidthsVector[aWidthNum]);
					aRectElement->mElementVector.push_back(new SingleDataElement(StringToWString(aStr)));

					sprintf(aStr, "%d", aRectIntVector[3]);
					aRectElement->mElementVector.push_back(new SingleDataElement(StringToWString(aStr)));

					aXPos += aWidthsVector[aWidthNum];
				}

				DataElementMap::iterator anItr = mDefineMap.find(aDefineName);
				if (anItr != mDefineMap.end())
				{
					delete anItr->second;
					mDefineMap.erase(anItr);
				}

				mDefineMap.insert(DataElementMap::value_type(aDefineName, aRectList));
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"SetDefaultPointSize"))
	{
		if (theParams.mElementVector.size() == 2)
		{
			int aPointSize;

			if ((!theParams.mElementVector[1]->mIsList) &&
				(StringToInt(((SingleDataElement*) theParams.mElementVector[1])->mString, &aPointSize)))
			{
				mDefaultPointSize = aPointSize;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"SetCharMap"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			WStringVector aFromVector;
			WStringVector aToVector;

			if ((DataToStringVector(theParams.mElementVector[1], &aFromVector)) &&
				(DataToStringVector(theParams.mElementVector[2], &aToVector)))
			{
				if (aFromVector.size() == aToVector.size())
				{
					for (uint32 aMapIdx = 0; aMapIdx < aFromVector.size(); aMapIdx++)
					{
						if ((aFromVector[aMapIdx].length() == 1) && (aToVector[aMapIdx].length() == 1))
						{
							mCharMap.insert
								(SexyCharToSexyCharMap::value_type
								 (aFromVector[aMapIdx][0],
								  aToVector[aMapIdx][0]));
						}
						else
							invalidParamFormat = true;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd ==  LWS(L"CreateLayer"))
	{
		if (theParams.mElementVector.size() == 2)
		{
			if (!theParams.mElementVector[1]->mIsList)
			{
				std::wstring aLayerName = StringToUpper(((SingleDataElement*) theParams.mElementVector[1])->mString);

				mFontLayerList.push_back(FontLayer(this));
				FontLayer* aFontLayer = &mFontLayerList.back();

				if (!mFontLayerMap.insert(FontLayerMap::value_type(aLayerName, aFontLayer)).second)
				{
					Error("Layer Already Exists");
				}
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"CreateLayerFrom"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aSourceLayer;

			if ((!theParams.mElementVector[1]->mIsList) && (DataToLayer(theParams.mElementVector[2], &aSourceLayer)))
			{
				std::wstring aLayerName = StringToUpper(((SingleDataElement*) theParams.mElementVector[1])->mString);

				mFontLayerList.push_back(FontLayer(*aSourceLayer));
				FontLayer* aFontLayer = &mFontLayerList.back();

				if (!mFontLayerMap.insert(FontLayerMap::value_type(aLayerName, aFontLayer)).second)
				{
					Error("Layer Already Exists");
				}
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd.c_str() == LWS(L"LayerRequireTags"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			WStringVector aStringVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aStringVector)))
			{
				for (uint32 i = 0; i < aStringVector.size(); i++)
					aLayer->mRequiredTags.push_back(StringToUpper(aStringVector[i]));
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerExcludeTags"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			WStringVector aStringVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aStringVector)))
			{
				for (uint32 i = 0; i < aStringVector.size(); i++)
					aLayer->mExcludedTags.push_back(StringToUpper(aStringVector[i]));
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerPointRange"))
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList) &&
				(!theParams.mElementVector[3]->mIsList))
			{
				int aMinPointSize;
				int aMaxPointSize;

				if ((StringToInt(((SingleDataElement*) theParams.mElementVector[2])->mString, &aMinPointSize)) &&
					(StringToInt(((SingleDataElement*) theParams.mElementVector[3])->mString, &aMaxPointSize)))
				{
					aLayer->mMinPointSize = aMinPointSize;
					aLayer->mMaxPointSize = aMaxPointSize;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetPointSize"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int aPointSize;
				if (StringToInt(((SingleDataElement*) theParams.mElementVector[2])->mString, &aPointSize))
				{
					aLayer->mPointSize = aPointSize;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetHeight"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int aHeight;
				if (StringToInt(((SingleDataElement*) theParams.mElementVector[2])->mString, &aHeight))
				{
					aLayer->mHeight = aHeight;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetImage"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			std::wstring aFileNameString;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToString(theParams.mElementVector[2], &aFileNameString)))
			{
				std::string aFileName = GetPathFrom(WStringToString(aFileNameString),
								    GetFileDir(mSourceFile));

				bool isNew;
				SharedImageRef anImage = mApp->GetSharedImage(aFileName, "", &isNew);

				if ((Image*) anImage != NULL)
				{
					if (isNew)
						anImage->Palletize();
					aLayer->mImage = anImage;
				}
				else
				{
					Error("Failed to Load Image");
					return false;
				}
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetDrawMode"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) && (!theParams.mElementVector[2]->mIsList))
			{
				int anDrawMode;
				if ((StringToInt(((SingleDataElement*) theParams.mElementVector[2])->mString, &anDrawMode)) &&
					(anDrawMode >= 0) && (anDrawMode <= 1))
				{
					aLayer->mDrawMode = anDrawMode;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetColorMult"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if (DataToLayer(theParams.mElementVector[1], &aLayer))
			{
				if (!GetColorFromDataElement(theParams.mElementVector[2],aLayer->mColorMult))
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetColorAdd"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if (DataToLayer(theParams.mElementVector[1], &aLayer))
			{
				if (!GetColorFromDataElement(theParams.mElementVector[2],aLayer->mColorAdd))
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetAscent"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int anAscent;
				if (StringToInt(((SingleDataElement*) theParams.mElementVector[2])->mString, &anAscent))
				{
					aLayer->mAscent = anAscent;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetAscentPadding"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int anAscent;
				if (StringToInt(((SingleDataElement*) theParams.mElementVector[2])->mString, &anAscent))
				{
					aLayer->mAscentPadding = anAscent;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetLineSpacingOffset"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int anAscent;
				if (StringToInt(((SingleDataElement*) theParams.mElementVector[2])->mString, &anAscent))
				{
					aLayer->mLineSpacingOffset = anAscent;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetOffset"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			IntVector anOffset;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) && (DataToIntVector(theParams.mElementVector[2], &anOffset)) && (anOffset.size() == 2))
			{
				aLayer->mOffset.mX = anOffset[0];
				aLayer->mOffset.mY = anOffset[1];
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetCharWidths"))
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			WStringVector aCharsVector;
			IntVector aCharWidthsVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aCharsVector)) &&
				(DataToIntVector(theParams.mElementVector[3], &aCharWidthsVector)))
			{
				if (aCharsVector.size() == aCharWidthsVector.size())
				{
					for (uint32 i = 0; i < aCharsVector.size(); i++)
					{
						if (aCharsVector[i].length() == 1)
						{

							aLayer->SetCharData(aCharsVector[i][0])->mWidth =
								aCharWidthsVector[i];
						}
						else
							invalidParamFormat = true;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetSpacing"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			IntVector anOffset;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int aSpacing;

				if (StringToInt(((SingleDataElement*) theParams.mElementVector[2])->mString, &aSpacing))
				{
					aLayer->mSpacing = aSpacing;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetImageMap"))
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			WStringVector aCharsVector;
			ListDataElement aRectList;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aCharsVector)) &&
				(DataToList(theParams.mElementVector[3], &aRectList)))
			{
				if (aCharsVector.size() == aRectList.mElementVector.size())
				{
					if ((Image*) aLayer->mImage != NULL)
					{
						int anImageWidth = aLayer->mImage->GetWidth();
						int anImageHeight = aLayer->mImage->GetHeight();

						for (uint32 i = 0; i < aCharsVector.size(); i++)
						{
							IntVector aRectElement;

							if ((aCharsVector[i].length() == 1) &&
								(DataToIntVector(aRectList.mElementVector[i], &aRectElement)) &&
								(aRectElement.size() == 4))

							{
								Rect aRect = Rect(aRectElement[0], aRectElement[1], aRectElement[2], aRectElement[3]);

								if ((aRect.mX < 0) || (aRect.mY < 0) ||
									(aRect.mX + aRect.mWidth > anImageWidth) || (aRect.mY + aRect.mHeight > anImageHeight))
								{
									Error("Image rectangle out of bounds");
									return false;
								}

								aLayer->GetCharData(aCharsVector[i][0])->mImageRect = aRect;
							}
							else
								invalidParamFormat = true;
						}

						aLayer->mDefaultHeight = 0;
						CharDataMap::iterator anItr = aLayer->mCharDataMap.begin();
						while (anItr != aLayer->mCharDataMap.end())
						{
							CharData* aCharData = &anItr->second;
							if (aCharData->mImageRect.mHeight + aCharData->mOffset.mY > aLayer->mDefaultHeight)
								aLayer->mDefaultHeight = aCharData->mImageRect.mHeight + aCharData->mOffset.mY;
							++anItr;
						}
					}
					else
					{
						Error("Layer image not set");
						return false;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetCharOffsets"))
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			WStringVector aCharsVector;
			ListDataElement aRectList;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aCharsVector)) &&
				(DataToList(theParams.mElementVector[3], &aRectList)))
			{
				if (aCharsVector.size() == aRectList.mElementVector.size())
				{
					for (uint32 i = 0; i < aCharsVector.size(); i++)
					{
						IntVector aRectElement;

						if ((aCharsVector[i].length() == 1) &&
							(DataToIntVector(aRectList.mElementVector[i], &aRectElement)) &&
							(aRectElement.size() == 2))
						{
							aLayer->GetCharData(aCharsVector[i][0])->mOffset =
								Point(aRectElement[0], aRectElement[1]);
						}
						else
							invalidParamFormat = true;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetKerningPairs"))
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			WStringVector aPairsVector;
			IntVector anOffsetsVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aPairsVector)) &&
				(DataToIntVector(theParams.mElementVector[3], &anOffsetsVector)))
			{
				if (aPairsVector.size() == anOffsetsVector.size())
				{
					for (uint32 i = 0; i < aPairsVector.size(); i++)
					{
						if (aPairsVector[i].length() == 2)
						{
							CharData* aCharData = aLayer->GetCharData(aPairsVector[i][0]);
							aCharData->mKerningOffsets.insert
								(SexyCharToIntMap::value_type(aPairsVector[i][1], anOffsetsVector[i]));
						}
						else
							invalidParamFormat = true;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetBaseOrder"))
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int aBaseOrder;
				if (StringToInt(((SingleDataElement*) theParams.mElementVector[2])->mString, &aBaseOrder))
				{
					aLayer->mBaseOrder = aBaseOrder;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetCharOrders"))
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			WStringVector aCharsVector;
			IntVector aCharOrdersVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aCharsVector)) &&
				(DataToIntVector(theParams.mElementVector[3], &aCharOrdersVector)))
			{
				if (aCharsVector.size() == aCharOrdersVector.size())
				{
					for (uint32 i = 0; i < aCharsVector.size(); i++)
					{
						if (aCharsVector[i].length() == 1)
						{
							aLayer->GetCharData(aCharsVector[i][0])->mOrder =
								aCharOrdersVector[i];
						}
						else
							invalidParamFormat = true;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (aCmd == LWS(L"LayerSetExInfo"))
 	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			WStringVector aKeys;
			WStringVector aValues;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aKeys)) &&
				(DataToStringVector(theParams.mElementVector[3], &aValues)))
			{
				if (aKeys.size() == aValues.size())
				{
					for (StringVector::size_type i = 0; i < aKeys.size(); ++i)
						aLayer->mExtendedInfo.insert
							(FontLayer::ExInfoMap::value_type(aKeys[i],
											  aValues[i]));
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else
	{
		Error("Unknown Command");
		return false;
	}

	if (invalidNumParams)
	{
		Error("Invalid Number of Parameters");
		return false;
	}

	if (invalidParamFormat)
	{
		Error("Invalid Paramater Type");
		return false;
	}

	if (literalError)
	{
		Error("Undefined Value");
		return false;
	}

	if (sizeMismatch)
	{
		Error("List Size Mismatch");
		return false;
	}

	return true;
}

bool FontData::Load(SexyAppBase* theSexyApp, const std::string& theFontDescFileName)
{
	if (mInitialized)
		return false;

	bool hasErrors = false;

	mApp = theSexyApp;
	mCurrentLine = L"";

	mFontErrorHeader = "Font Descriptor Error in " + theFontDescFileName + "\r\n";

	mSourceFile = theFontDescFileName;

	mInitialized = LoadDescriptor(theFontDescFileName);	;

	return !hasErrors;
}

bool FontData::LoadLegacy(Image* theFontImage, const std::string& theFontDescFileName)
{
	if (mInitialized)
		return false;

	mFontLayerList.push_back(FontLayer(this));
	FontLayer* aFontLayer = &mFontLayerList.back();

	FontLayerMap::iterator anItr = mFontLayerMap.insert(FontLayerMap::value_type(L"", aFontLayer)).first;
	if (anItr == mFontLayerMap.end())
		return false;

	aFontLayer->mImage = (MemoryImage*) theFontImage;
	aFontLayer->mDefaultHeight = aFontLayer->mImage->GetHeight();
	aFontLayer->mAscent = aFontLayer->mImage->GetHeight();

	int aCharPos = 0;
	FILE *aStream = fopen(theFontDescFileName.c_str(), "r");

	if (aStream==NULL)
 		return false;

	mSourceFile = theFontDescFileName;

	int aSpaceWidth = 0, aSpaceChar = ' ';
	fscanf(aStream,"%d%d",&aSpaceWidth,&aFontLayer->mAscent);
	aFontLayer->GetCharData(aSpaceChar)->mWidth = aSpaceWidth;

	while (!feof(aStream))
 	{
		char aBuf[2] = { 0, 0 }; // needed because fscanf will null terminate the string it reads
 		char aChar = 0;
 		int aWidth = 0;

		fscanf(aStream,"%1s%d",aBuf,&aWidth);
		aChar = aBuf[0];


		if (aChar == 0)
			break;

		aFontLayer->GetCharData(aChar)->mImageRect =
			Rect(aCharPos, 0, aWidth, aFontLayer->mImage->GetHeight());
		aFontLayer->GetCharData(aChar)->mWidth = aWidth;

		aCharPos += aWidth;
	}

	int c;

	for (c = 'A'; c <= 'Z'; c++)
		if ((aFontLayer->GetCharData(c)->mWidth == 0) && (aFontLayer->GetCharData(c - 'A' + 'a')->mWidth != 0))
			mCharMap.insert(SexyCharToSexyCharMap::value_type(c, c - 'A' + 'a'));
	for (c = 'a'; c <= 'z'; c++)
		if ((aFontLayer->GetCharData(c)->mWidth == 0) && (aFontLayer->GetCharData(c - 'a' + 'A')->mWidth != 0))
			mCharMap.insert(SexyCharToSexyCharMap::value_type(c, c - 'a' + 'A'));

	mInitialized = true;
	fclose(aStream);

	return true;
}

////

ActiveFontLayer::ActiveFontLayer()
{
	mScaledImage = NULL;
	mOwnsImage = false;
}

ActiveFontLayer::ActiveFontLayer(const ActiveFontLayer& theActiveFontLayer) :
	mBaseFontLayer(theActiveFontLayer.mBaseFontLayer),
	mScaledImage(theActiveFontLayer.mScaledImage),
	mOwnsImage(theActiveFontLayer.mOwnsImage),
	mScaledCharImageRects(theActiveFontLayer.mScaledCharImageRects)
{
	if (mOwnsImage)
		mScaledImage = mBaseFontLayer->mFontData->mApp->CopyImage(mScaledImage);
}

ActiveFontLayer::~ActiveFontLayer()
{
	if (mOwnsImage)
		delete mScaledImage;
}

////

ImageFont::ImageFont(SexyAppBase* theSexyApp, const std::string& theFontDescFileName)
{
	mScale = 1.0;
	mFontData = new FontData();
	mFontData->Ref();
	mFontData->Load(theSexyApp, theFontDescFileName);
	mPointSize = mFontData->mDefaultPointSize;
	GenerateActiveFontLayers();
	mActiveListValid = true;
	mForceScaledImagesWhite = false;
}

ImageFont::ImageFont(Image *theFontImage)
{
	mScale = 1.0;
	mFontData = new FontData();
	mFontData->Ref();
	mFontData->mInitialized = true;
	mPointSize = mFontData->mDefaultPointSize;
	mActiveListValid = false;
	mForceScaledImagesWhite = false;

	mFontData->mFontLayerList.push_back(FontLayer(mFontData));
	FontLayer* aFontLayer = &mFontData->mFontLayerList.back();

	mFontData->mFontLayerMap.insert(FontLayerMap::value_type(L"", aFontLayer)).first;
	aFontLayer->mImage = (MemoryImage*) theFontImage;
	aFontLayer->mDefaultHeight = aFontLayer->mImage->GetHeight();
	aFontLayer->mAscent = aFontLayer->mImage->GetHeight();
}

ImageFont::ImageFont(const ImageFont& theImageFont) :
	Font(theImageFont),
	mFontData(theImageFont.mFontData),
	mPointSize(theImageFont.mPointSize),
	mTagVector(theImageFont.mTagVector),
	mActiveListValid(theImageFont.mActiveListValid),
	mScale(theImageFont.mScale),
	mForceScaledImagesWhite(theImageFont.mForceScaledImagesWhite)
{
	mFontData->Ref();

	if (mActiveListValid)
		mActiveLayerList = theImageFont.mActiveLayerList;
}

ImageFont::ImageFont(Image* theFontImage, const std::string& theFontDescFileName)
{

	mScale = 1.0;
	mFontData = new FontData();
	mFontData->Ref();
	mFontData->LoadLegacy(theFontImage, theFontDescFileName);
	mPointSize = mFontData->mDefaultPointSize;
	GenerateActiveFontLayers();
	mActiveListValid = true;
}

ImageFont::~ImageFont()
{
	mFontData->DeRef();
}

/*ImageFont::ImageFont(const ImageFont& theImageFont, Image* theImage) :
	Font(theImageFont),
	mImage(theImage)
{
	for (int i = 0; i < 256; i++)
	{
		mCharPos[i] = theImageFont.mCharPos[i];
		mCharWidth[i] = theImageFont.mCharWidth[i];
	}
}*/

void ImageFont::GenerateActiveFontLayers()
{
	if (!mFontData->mInitialized)
		return;

	mActiveLayerList.clear();

	uint32 i;

	mAscent = 0;
	mAscentPadding = 0;
	mHeight = 0;
	mLineSpacingOffset = 0;

	FontLayerList::iterator anItr = mFontData->mFontLayerList.begin();

	bool firstLayer = true;

	while (anItr != mFontData->mFontLayerList.end())
	{
		FontLayer* aFontLayer = &*anItr;

		if ((mPointSize >= aFontLayer->mMinPointSize) &&
			((mPointSize <= aFontLayer->mMaxPointSize) || (aFontLayer->mMaxPointSize == -1)))
		{
			bool active = true;

			// Make sure all required tags are included
			for (i = 0; i < aFontLayer->mRequiredTags.size(); i++)
				if (std::find(mTagVector.begin(), mTagVector.end(), aFontLayer->mRequiredTags[i]) == mTagVector.end())
					active = false;

			// Make sure no excluded tags are included
			for (i = 0; i < mTagVector.size(); i++)
				if (std::find(aFontLayer->mExcludedTags.begin(), aFontLayer->mExcludedTags.end(),
					mTagVector[i]) != aFontLayer->mExcludedTags.end())
					active = false;

			if (active)
			{
				mActiveLayerList.push_back(ActiveFontLayer());

				ActiveFontLayer* anActiveFontLayer = &mActiveLayerList.back();

				anActiveFontLayer->mBaseFontLayer = aFontLayer;

				double aLayerPointSize = 1;
				double aPointSize = mScale;

				if ((mScale == 1.0) && ((aFontLayer->mPointSize == 0) || (mPointSize == aFontLayer->mPointSize)))
				{
					anActiveFontLayer->mScaledImage = aFontLayer->mImage;
					anActiveFontLayer->mOwnsImage = false;

					// Use the specified point size

					CharDataMap::iterator anItr = aFontLayer->mCharDataMap.begin();
					while (anItr != aFontLayer->mCharDataMap.end())
					{
						CharData* aCharData = &anItr->second;
						anActiveFontLayer->mScaledCharImageRects.insert(SexyCharToRectMap::value_type(anItr->first, aCharData->mImageRect));
						++anItr;
					}
				}
				else
				{
					if (aFontLayer->mPointSize != 0)
					{
						aLayerPointSize = aFontLayer->mPointSize;
						aPointSize = mPointSize * mScale;
					}

					// Resize font elements
					MemoryImage* aMemoryImage = new MemoryImage(mFontData->mApp);

					int aCurX = 0;
					int aMaxHeight = 0;

					CharDataMap::iterator anItr = aFontLayer->mCharDataMap.begin();
					while (anItr != aFontLayer->mCharDataMap.end())
					{
						CharData* aCharData = &anItr->second;
						Rect* anOrigRect = &aCharData->mImageRect;

						Rect aScaledRect(aCurX, 0,
							(int) ((anOrigRect->mWidth * aPointSize) / aLayerPointSize),
							(int) ((anOrigRect->mHeight * aPointSize) / aLayerPointSize));

						anActiveFontLayer->mScaledCharImageRects.insert
							(SexyCharToRectMap::value_type(anItr->first, aScaledRect));

						if (aScaledRect.mHeight > aMaxHeight)
							aMaxHeight = aScaledRect.mHeight;

						aCurX += aScaledRect.mWidth;
						++anItr;
					}

					anActiveFontLayer->mScaledImage = aMemoryImage;
					anActiveFontLayer->mOwnsImage = true;

					// Create the image now

					aMemoryImage->Create(aCurX, aMaxHeight);

					Graphics g(aMemoryImage);

					anItr = aFontLayer->mCharDataMap.begin();
					while (anItr != aFontLayer->mCharDataMap.end())
 					{
						CharData* aCharData = &anItr->second;
 						if ((Image*) aFontLayer->mImage != NULL)
							g.DrawImage(aFontLayer->mImage,
								    anActiveFontLayer->mScaledCharImageRects[anItr->first],
								    aCharData->mImageRect);
						++anItr;
 					}

					if (mForceScaledImagesWhite)
					{
						int aCount = aMemoryImage->mWidth*aMemoryImage->mHeight;
						uint32* aBits = aMemoryImage->GetBits();

						for (int i = 0; i < aCount; i++, aBits++)
							*(aBits) = *aBits | 0x00FFFFFF;
					}

					aMemoryImage->Palletize();
				}

				int aLayerAscent = int((aFontLayer->mAscent * aPointSize) / aLayerPointSize);
				if (aLayerAscent > mAscent)
					mAscent = aLayerAscent;

				if (aFontLayer->mHeight != 0)
				{
					int aLayerHeight = int((aFontLayer->mHeight * aPointSize) / aLayerPointSize);
					if (aLayerHeight > mHeight)
						mHeight = aLayerHeight;
				}
				else
				{
					int aLayerHeight = int((aFontLayer->mDefaultHeight * aPointSize) / aLayerPointSize);
					if (aLayerHeight > mHeight)
						mHeight = aLayerHeight;
				}

				int anAscentPadding = int((aFontLayer->mAscentPadding * aPointSize) / aLayerPointSize);
				if ((firstLayer) || (anAscentPadding < mAscentPadding))
					mAscentPadding = anAscentPadding;

				int aLineSpacingOffset = int((aFontLayer->mLineSpacingOffset * aPointSize) / aLayerPointSize);
				if ((firstLayer) || (aLineSpacingOffset > mLineSpacingOffset))
					mLineSpacingOffset = aLineSpacingOffset;

				firstLayer = false;
			}
		}

		++anItr;
	}
}

int ImageFont::StringWidth(const std::string& theString)
{
	std::wstring aString;
	int aLen = Graphics::WStringFromString(theString, aString);
	if (aLen > 0)
		return StringWidth(aString);

	int aWidth = 0;
	int aPrevChar = 0;
	for(int i=0; i<(int)theString.length(); i++)
	{
		int aChar = theString[i];
		aWidth += CharWidthKern(aChar,aPrevChar);
		aPrevChar = aChar;
	}

	return aWidth;
}

int ImageFont::StringWidth(const std::wstring& theString)
{
	int aWidth = 0;
	int aPrevChar = 0;
	for(int i=0; i<(int)theString.length(); i++)
	{
		int aChar = theString[i];
		aWidth += CharWidthKern(aChar,aPrevChar);
		aPrevChar = aChar;
	}

	return aWidth;
}

int ImageFont::CharWidthKern(int theChar, int thePrevChar)
{
	Prepare();

	int aMaxXPos = 0;
	double aPointSize = mPointSize * mScale;

	theChar = GetMappedChar(theChar);
 	if (thePrevChar != 0)
		thePrevChar = GetMappedChar(thePrevChar);

	ActiveFontLayerList::iterator anItr = mActiveLayerList.begin();
	while (anItr != mActiveLayerList.end())
	{
		ActiveFontLayer* anActiveFontLayer = &*anItr;

		int aLayerXPos = 0;

		int aCharWidth;
		int aSpacing;

		int aLayerPointSize = anActiveFontLayer->mBaseFontLayer->mPointSize;

		if (aLayerPointSize == 0)
		{
			aCharWidth = int(anActiveFontLayer->mBaseFontLayer->GetCharData(theChar)->mWidth * mScale);

			if (thePrevChar != 0)
			{
				aSpacing = anActiveFontLayer->mBaseFontLayer->mSpacing;
				CharData* aPrevCharData = anActiveFontLayer->mBaseFontLayer->GetCharData(thePrevChar);
				SexyCharToIntMap::iterator aKernItr = aPrevCharData->mKerningOffsets.find(theChar);
				if (aKernItr != aPrevCharData->mKerningOffsets.end())
					aSpacing += aKernItr->second * mScale;
			}
			else
				aSpacing = 0;
		}
		else
		{
			aCharWidth = int(anActiveFontLayer->mBaseFontLayer->GetCharData
					 (theChar)->mWidth * aPointSize / aLayerPointSize);

			if (thePrevChar != 0)
			{

				aSpacing = anActiveFontLayer->mBaseFontLayer->mSpacing;
				CharData* aPrevCharData = anActiveFontLayer->mBaseFontLayer->GetCharData(thePrevChar);
				SexyCharToIntMap::iterator aKernItr = aPrevCharData->mKerningOffsets.find(theChar);
				if (aKernItr != aPrevCharData->mKerningOffsets.end())
					aSpacing += aKernItr->second * aPointSize / aLayerPointSize;
			}
			else
				aSpacing = 0;
		}

		aLayerXPos += aCharWidth + aSpacing;

		if (aLayerXPos > aMaxXPos)
			aMaxXPos = aLayerXPos;

		++anItr;
	}

	return aMaxXPos;
}

int ImageFont::CharWidth(int theChar)
{
	return CharWidthKern(theChar,0);
}

CritSect gRenderCritSec;
static const int POOL_SIZE = 4096;
static RenderCommand gRenderCommandPool[POOL_SIZE];
static RenderCommand* gRenderTail[256];
static RenderCommand* gRenderHead[256];

void ImageFont::DrawStringEx(Graphics* g, int theX, int theY, const SexyString& theString,
			     const Color& theColor, const Rect* theClipRect, RectList* theDrawnAreas,
			     int* theWidth)
{
	AutoCrit anAutoCrit(gRenderCritSec);

	int aPoolIdx;

	for (aPoolIdx = 0; aPoolIdx < 256; aPoolIdx++)
	{
		gRenderHead[aPoolIdx] = NULL;
		gRenderTail[aPoolIdx] = NULL;
	}

	//int aXPos = theX;

	if (theDrawnAreas != NULL)
		theDrawnAreas->clear();


	/*if (theDrawnArea != NULL)
		*theDrawnArea = Rect(0, 0, 0, 0);*/

	if (!mFontData->mInitialized)
	{
		if (theWidth != NULL)
			*theWidth = 0;
		return;
	}

	Prepare();

	bool colorizeImages = g->GetColorizeImages();
	g->SetColorizeImages(true);

	int aCurXPos = theX;
	int aCurPoolIdx = 0;

	for (uint32 aCharNum = 0; aCharNum < theString.length(); aCharNum++)
	{
		int aChar = GetMappedChar(theString[aCharNum]);

		int aNextChar = 0;
		if (aCharNum < theString.length() - 1)
			aNextChar = GetMappedChar(theString[aCharNum + 1]);

		int aMaxXPos = aCurXPos;

		ActiveFontLayerList::iterator anItr = mActiveLayerList.begin();
		while (anItr != mActiveLayerList.end())
		{
			ActiveFontLayer* anActiveFontLayer = &*anItr;
			if (false && !anActiveFontLayer->mBaseFontLayer->isGlyph(aChar))
				aChar = _S('?');
			CharData* aCharData = anActiveFontLayer->mBaseFontLayer->GetCharData(aChar);

			int aLayerXPos = aCurXPos;

			int anImageX;
			int anImageY;
			int aCharWidth;
			int aSpacing;

			int aLayerPointSize = anActiveFontLayer->mBaseFontLayer->mPointSize;

			double aScale = mScale;
			if (aLayerPointSize != 0)
				aScale *= mPointSize / aLayerPointSize;

			if (aScale == 1.0)
			{
				anImageX = aLayerXPos + anActiveFontLayer->mBaseFontLayer->mOffset.mX + aCharData->mOffset.mX;
				anImageY = theY - (anActiveFontLayer->mBaseFontLayer->mAscent - anActiveFontLayer->mBaseFontLayer->mOffset.mY - aCharData->mOffset.mY);
				aCharWidth = aCharData->mWidth;

				if (aNextChar != 0)
				{
					aSpacing = anActiveFontLayer->mBaseFontLayer->mSpacing;
					SexyCharToIntMap::iterator aKernItr = aCharData->mKerningOffsets.find(aNextChar);
					if (aKernItr != aCharData->mKerningOffsets.end())
						aSpacing += aKernItr->second;
				}
				else
					aSpacing = 0;
			}
			else
			{
				anImageX = aLayerXPos + (int) ((anActiveFontLayer->mBaseFontLayer->mOffset.mX + aCharData->mOffset.mX) * aScale);
				anImageY = theY - (int) ((anActiveFontLayer->mBaseFontLayer->mAscent - anActiveFontLayer->mBaseFontLayer->mOffset.mY - aCharData->mOffset.mY) * aScale);
				aCharWidth = (aCharData->mWidth * aScale);

				if (aNextChar != 0)
				{
					aSpacing = anActiveFontLayer->mBaseFontLayer->mSpacing;
					SexyCharToIntMap::iterator aKernItr = aCharData->mKerningOffsets.find(aNextChar);
					if (aKernItr != aCharData->mKerningOffsets.end())
						aSpacing += (int) (aKernItr->second * aScale);
				}
				else
					aSpacing = 0;
			}

			Color aColor;
			aColor.mRed = std::min((theColor.mRed * anActiveFontLayer->mBaseFontLayer->mColorMult.mRed / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mRed, 255);
			aColor.mGreen = std::min((theColor.mGreen * anActiveFontLayer->mBaseFontLayer->mColorMult.mGreen / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mGreen, 255);
			aColor.mBlue = std::min((theColor.mBlue * anActiveFontLayer->mBaseFontLayer->mColorMult.mBlue / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mBlue, 255);
			aColor.mAlpha = std::min((theColor.mAlpha * anActiveFontLayer->mBaseFontLayer->mColorMult.mAlpha / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mAlpha, 255);

			int anOrder = anActiveFontLayer->mBaseFontLayer->mBaseOrder + anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mOrder;

			if (aCurPoolIdx >= POOL_SIZE)
				break;

			RenderCommand* aRenderCommand = &gRenderCommandPool[aCurPoolIdx++];

			aRenderCommand->mImage = anActiveFontLayer->mScaledImage;
			aRenderCommand->mColor = aColor;
			aRenderCommand->mDest[0] = anImageX;
			aRenderCommand->mDest[1] = anImageY;
			aRenderCommand->mSrc[0] = anActiveFontLayer->mScaledCharImageRects[aChar].mX;
			aRenderCommand->mSrc[1] = anActiveFontLayer->mScaledCharImageRects[aChar].mY;
			aRenderCommand->mSrc[2] = anActiveFontLayer->mScaledCharImageRects[aChar].mWidth;
			aRenderCommand->mSrc[3] = anActiveFontLayer->mScaledCharImageRects[aChar].mHeight;
			aRenderCommand->mMode = anActiveFontLayer->mBaseFontLayer->mDrawMode;
			aRenderCommand->mNext = NULL;

			int anOrderIdx = std::min(std::max(anOrder + 128, 0), 255);

			if (gRenderTail[anOrderIdx] == NULL)
			{
				gRenderTail[anOrderIdx] = aRenderCommand;
				gRenderHead[anOrderIdx] = aRenderCommand;
			}
			else
			{
				gRenderTail[anOrderIdx]->mNext = aRenderCommand;
				gRenderTail[anOrderIdx] = aRenderCommand;
			}

			//aRenderCommandMap.insert(RenderCommandMap::value_type(aPriority, aRenderCommand));

			/*int anOldDrawMode = g->GetDrawMode();
			if (anActiveFontLayer->mBaseFontLayer->mDrawMode != -1)
				g->SetDrawMode(anActiveFontLayer->mBaseFontLayer->mDrawMode);
			Color anOrigColor = g->GetColor();
			g->SetColor(aColor);
			if (anActiveFontLayer->mScaledImage != NULL)
				g->DrawImage(anActiveFontLayer->mScaledImage, anImageX, anImageY, anActiveFontLayer->mScaledCharImageRects[aChar]);
			g->SetColor(anOrigColor);
			g->SetDrawMode(anOldDrawMode);*/

			if (theDrawnAreas != NULL)
			{
				Rect aDestRect = Rect(anImageX, anImageY, anActiveFontLayer->mScaledCharImageRects[aChar].mWidth, anActiveFontLayer->mScaledCharImageRects[aChar].mHeight);

				theDrawnAreas->push_back(aDestRect);

				/*if (theDrawnArea->mWidth == 0)
					*theDrawnArea = theDestRect;
				else
				{
					if (theDestRect.mX < theDrawnArea->mX)
					{
						int aDiff = theDestRect.mX - theDrawnArea->mX;
						theDrawnArea->mX += aDiff;
						theDrawnArea->mWidth += aDiff;
					}

					if (theDestRect.mX + theDestRect.mWidth > theDrawnArea->mX + theDrawnArea->mWidth)
						theDrawnArea->mWidth = theDestRect.mX + theDestRect.mWidth - theDrawnArea->mX;

					if (theDestRect.mY < theDrawnArea->mY)
					{
						int aDiff = theDestRect.mY - theDrawnArea->mY;
						theDrawnArea->mY += aDiff;
						theDrawnArea->mHeight += aDiff;
					}

					if (theDestRect.mY + theDestRect.mHeight > theDrawnArea->mY + theDrawnArea->mHeight)
						theDrawnArea->mHeight = theDestRect.mY + theDestRect.mHeight - theDrawnArea->mY;
				}*/
			}

			aLayerXPos += aCharWidth + aSpacing;

			if (aLayerXPos > aMaxXPos)
				aMaxXPos = aLayerXPos;

			++anItr;
		}

		aCurXPos = aMaxXPos;
	}

	if (theWidth != NULL)
		*theWidth = aCurXPos - theX;

	Color anOrigColor = g->GetColor();

	for (aPoolIdx = 0; aPoolIdx < 256; aPoolIdx++)
	{
		RenderCommand* aRenderCommand = gRenderHead[aPoolIdx];

		while (aRenderCommand != NULL)
		{
			int anOldDrawMode = g->GetDrawMode();
			if (aRenderCommand->mMode != -1)
				g->SetDrawMode(aRenderCommand->mMode);
			g->SetColor(Color(aRenderCommand->mColor));
			if (aRenderCommand->mImage != NULL)
				g->DrawImage(aRenderCommand->mImage, aRenderCommand->mDest[0], aRenderCommand->mDest[1], Rect(aRenderCommand->mSrc[0], aRenderCommand->mSrc[1], aRenderCommand->mSrc[2], aRenderCommand->mSrc[3]));
			g->SetDrawMode(anOldDrawMode);

			aRenderCommand = aRenderCommand->mNext;
		}
	}

	g->SetColor(anOrigColor);

	/*RenderCommandMap::iterator anItr = aRenderCommandMap.begin();
	while (anItr != aRenderCommandMap.end())
	{
		RenderCommand* aRenderCommand = &anItr->second;

		int anOldDrawMode = g->GetDrawMode();
		if (aRenderCommand->mMode != -1)
			g->SetDrawMode(aRenderCommand->mMode);
		Color anOrigColor = g->GetColor();
		g->SetColor(aRenderCommand->mColor);
		if (aRenderCommand->mImage != NULL)
			g->DrawImage(aRenderCommand->mImage, aRenderCommand->mDest.mX, aRenderCommand->mDest.mY, aRenderCommand->mSrc);
		g->SetColor(anOrigColor);
		g->SetDrawMode(anOldDrawMode);

		++anItr;
	}*/

	g->SetColorizeImages(colorizeImages);
}

void ImageFont::DrawStringEx(Graphics* g, int theX, int theY, const std::wstring& theString,
			     const Color& theColor, const Rect* theClipRect, RectList* theDrawnAreas,
			     int* theWidth)
{
	AutoCrit anAutoCrit(gRenderCritSec);

	int aPoolIdx;

	for (aPoolIdx = 0; aPoolIdx < 256; aPoolIdx++)
	{
		gRenderHead[aPoolIdx] = NULL;
		gRenderTail[aPoolIdx] = NULL;
	}

	//int aXPos = theX;

	if (theDrawnAreas != NULL)
		theDrawnAreas->clear();


	/*if (theDrawnArea != NULL)
		*theDrawnArea = Rect(0, 0, 0, 0);*/

	if (!mFontData->mInitialized)
	{
		if (theWidth != NULL)
			*theWidth = 0;
		return;
	}

	Prepare();

	bool colorizeImages = g->GetColorizeImages();
	g->SetColorizeImages(true);

	int aCurXPos = theX;
	int aCurPoolIdx = 0;

	for (uint32 aCharNum = 0; aCharNum < theString.length(); aCharNum++)
	{
		int aChar = GetMappedChar(theString[aCharNum]);

		int aNextChar = 0;
		if (aCharNum < theString.length() - 1)
			aNextChar = GetMappedChar(theString[aCharNum + 1]);

		int aMaxXPos = aCurXPos;

		ActiveFontLayerList::iterator anItr = mActiveLayerList.begin();
		while (anItr != mActiveLayerList.end())
		{
			ActiveFontLayer* anActiveFontLayer = &*anItr;
			if (false && !anActiveFontLayer->mBaseFontLayer->isGlyph(aChar))
				aChar = _S('?');
			CharData* aCharData = anActiveFontLayer->mBaseFontLayer->GetCharData(aChar);

			int aLayerXPos = aCurXPos;

			int anImageX;
			int anImageY;
			int aCharWidth;
			int aSpacing;

			int aLayerPointSize = anActiveFontLayer->mBaseFontLayer->mPointSize;

			double aScale = mScale;
			if (aLayerPointSize != 0)
				aScale *= mPointSize / aLayerPointSize;

			if (aScale == 1.0)
			{
				anImageX = aLayerXPos + anActiveFontLayer->mBaseFontLayer->mOffset.mX + aCharData->mOffset.mX;
				anImageY = theY - (anActiveFontLayer->mBaseFontLayer->mAscent - anActiveFontLayer->mBaseFontLayer->mOffset.mY - aCharData->mOffset.mY);
				aCharWidth = aCharData->mWidth;

				if (aNextChar != 0)
				{
					aSpacing = anActiveFontLayer->mBaseFontLayer->mSpacing;
					SexyCharToIntMap::iterator aKernItr = aCharData->mKerningOffsets.find(aNextChar);
					if (aKernItr != aCharData->mKerningOffsets.end())
						aSpacing += aKernItr->second;
				}
				else
					aSpacing = 0;
			}
			else
			{
				anImageX = aLayerXPos + (int) ((anActiveFontLayer->mBaseFontLayer->mOffset.mX + aCharData->mOffset.mX) * aScale);
				anImageY = theY - (int) ((anActiveFontLayer->mBaseFontLayer->mAscent - anActiveFontLayer->mBaseFontLayer->mOffset.mY - aCharData->mOffset.mY) * aScale);
				aCharWidth = (aCharData->mWidth * aScale);

				if (aNextChar != 0)
				{
					aSpacing = anActiveFontLayer->mBaseFontLayer->mSpacing;
					SexyCharToIntMap::iterator aKernItr = aCharData->mKerningOffsets.find(aNextChar);
					if (aKernItr != aCharData->mKerningOffsets.end())
						aSpacing += (int) (aKernItr->second * aScale);
				}
				else
					aSpacing = 0;
			}

			Color aColor;
			aColor.mRed = std::min((theColor.mRed * anActiveFontLayer->mBaseFontLayer->mColorMult.mRed / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mRed, 255);
			aColor.mGreen = std::min((theColor.mGreen * anActiveFontLayer->mBaseFontLayer->mColorMult.mGreen / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mGreen, 255);
			aColor.mBlue = std::min((theColor.mBlue * anActiveFontLayer->mBaseFontLayer->mColorMult.mBlue / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mBlue, 255);
			aColor.mAlpha = std::min((theColor.mAlpha * anActiveFontLayer->mBaseFontLayer->mColorMult.mAlpha / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mAlpha, 255);

			int anOrder = anActiveFontLayer->mBaseFontLayer->mBaseOrder + anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mOrder;

			if (aCurPoolIdx >= POOL_SIZE)
				break;

			RenderCommand* aRenderCommand = &gRenderCommandPool[aCurPoolIdx++];

			aRenderCommand->mImage = anActiveFontLayer->mScaledImage;
			aRenderCommand->mColor = aColor;
			aRenderCommand->mDest[0] = anImageX;
			aRenderCommand->mDest[1] = anImageY;
			aRenderCommand->mSrc[0] = anActiveFontLayer->mScaledCharImageRects[aChar].mX;
			aRenderCommand->mSrc[1] = anActiveFontLayer->mScaledCharImageRects[aChar].mY;
			aRenderCommand->mSrc[2] = anActiveFontLayer->mScaledCharImageRects[aChar].mWidth;
			aRenderCommand->mSrc[3] = anActiveFontLayer->mScaledCharImageRects[aChar].mHeight;
			aRenderCommand->mMode = anActiveFontLayer->mBaseFontLayer->mDrawMode;
			aRenderCommand->mNext = NULL;

			int anOrderIdx = std::min(std::max(anOrder + 128, 0), 255);

			if (gRenderTail[anOrderIdx] == NULL)
			{
				gRenderTail[anOrderIdx] = aRenderCommand;
				gRenderHead[anOrderIdx] = aRenderCommand;
			}
			else
			{
				gRenderTail[anOrderIdx]->mNext = aRenderCommand;
				gRenderTail[anOrderIdx] = aRenderCommand;
			}

			//aRenderCommandMap.insert(RenderCommandMap::value_type(aPriority, aRenderCommand));

			/*int anOldDrawMode = g->GetDrawMode();
			if (anActiveFontLayer->mBaseFontLayer->mDrawMode != -1)
				g->SetDrawMode(anActiveFontLayer->mBaseFontLayer->mDrawMode);
			Color anOrigColor = g->GetColor();
			g->SetColor(aColor);
			if (anActiveFontLayer->mScaledImage != NULL)
				g->DrawImage(anActiveFontLayer->mScaledImage, anImageX, anImageY, anActiveFontLayer->mScaledCharImageRects[aChar]);
			g->SetColor(anOrigColor);
			g->SetDrawMode(anOldDrawMode);*/

			if (theDrawnAreas != NULL)
			{
				Rect aDestRect = Rect(anImageX, anImageY, anActiveFontLayer->mScaledCharImageRects[aChar].mWidth, anActiveFontLayer->mScaledCharImageRects[aChar].mHeight);

				theDrawnAreas->push_back(aDestRect);

				/*if (theDrawnArea->mWidth == 0)
					*theDrawnArea = theDestRect;
				else
				{
					if (theDestRect.mX < theDrawnArea->mX)
					{
						int aDiff = theDestRect.mX - theDrawnArea->mX;
						theDrawnArea->mX += aDiff;
						theDrawnArea->mWidth += aDiff;
					}

					if (theDestRect.mX + theDestRect.mWidth > theDrawnArea->mX + theDrawnArea->mWidth)
						theDrawnArea->mWidth = theDestRect.mX + theDestRect.mWidth - theDrawnArea->mX;

					if (theDestRect.mY < theDrawnArea->mY)
					{
						int aDiff = theDestRect.mY - theDrawnArea->mY;
						theDrawnArea->mY += aDiff;
						theDrawnArea->mHeight += aDiff;
					}

					if (theDestRect.mY + theDestRect.mHeight > theDrawnArea->mY + theDrawnArea->mHeight)
						theDrawnArea->mHeight = theDestRect.mY + theDestRect.mHeight - theDrawnArea->mY;
				}*/
			}

			aLayerXPos += aCharWidth + aSpacing;

			if (aLayerXPos > aMaxXPos)
				aMaxXPos = aLayerXPos;

			++anItr;
		}

		aCurXPos = aMaxXPos;
	}

	if (theWidth != NULL)
		*theWidth = aCurXPos - theX;

	Color anOrigColor = g->GetColor();

	for (aPoolIdx = 0; aPoolIdx < 256; aPoolIdx++)
	{
		RenderCommand* aRenderCommand = gRenderHead[aPoolIdx];

		while (aRenderCommand != NULL)
		{
			int anOldDrawMode = g->GetDrawMode();
			if (aRenderCommand->mMode != -1)
				g->SetDrawMode(aRenderCommand->mMode);
			g->SetColor(Color(aRenderCommand->mColor));
			if (aRenderCommand->mImage != NULL)
				g->DrawImage(aRenderCommand->mImage, aRenderCommand->mDest[0], aRenderCommand->mDest[1], Rect(aRenderCommand->mSrc[0], aRenderCommand->mSrc[1], aRenderCommand->mSrc[2], aRenderCommand->mSrc[3]));
			g->SetDrawMode(anOldDrawMode);

			aRenderCommand = aRenderCommand->mNext;
		}
	}

	g->SetColor(anOrigColor);

	/*RenderCommandMap::iterator anItr = aRenderCommandMap.begin();
	while (anItr != aRenderCommandMap.end())
	{
		RenderCommand* aRenderCommand = &anItr->second;

		int anOldDrawMode = g->GetDrawMode();
		if (aRenderCommand->mMode != -1)
			g->SetDrawMode(aRenderCommand->mMode);
		Color anOrigColor = g->GetColor();
		g->SetColor(aRenderCommand->mColor);
		if (aRenderCommand->mImage != NULL)
			g->DrawImage(aRenderCommand->mImage, aRenderCommand->mDest.mX, aRenderCommand->mDest.mY, aRenderCommand->mSrc);
		g->SetColor(anOrigColor);
		g->SetDrawMode(anOldDrawMode);

		++anItr;
	}*/

	g->SetColorizeImages(colorizeImages);
}

bool ImageFont::StringToGlyphs(const std::wstring& theString, GlyphVector& theGlyphs)
{
	int aPrevChar = 0;
	float x = 0;
	float y = 0;

	Prepare();

	size_t anActiveLayerListSize = mActiveLayerList.size();
	for(size_t i = 0; i < theString.length(); i++)
	{
		int aChar = theString[i];

		theGlyphs.push_back(Glyph());
		Glyph &glyph = theGlyphs.back();

		aChar = glyph.mIndex = GetMappedChar(aChar);

		int aMaxXPos = 0;
		double aPointSize = mPointSize * mScale;
		int aCharMaxWidth = 0;
		int aCharMaxHeight = 0;
		int aSpacing = 0;

		for (size_t j = 0; j < anActiveLayerListSize; j++)
		{
			ActiveFontLayer* anActiveFontLayer = &mActiveLayerList[j];
			FontLayer* aBaseFontLayer = anActiveFontLayer->mBaseFontLayer;
			CharData* aCharData = aBaseFontLayer->GetCharData(aChar);
			CharData* aPrevCharData = 0;

			int aLayerXPos = 0;
			int aLayerPointSize = aBaseFontLayer->mPointSize;
			int aCharWidth = 0;

			if (aPrevChar)
				aPrevCharData = aBaseFontLayer->GetCharData(aPrevChar);
			if (aLayerPointSize == 0)
			{
				aCharWidth = int(aCharData->mWidth * mScale);

				if (aPrevChar != 0)
				{
					aSpacing = aBaseFontLayer->mSpacing;
					SexyCharToIntMap::iterator aKernItr =
						aPrevCharData->mKerningOffsets.find(aChar);

					if (aKernItr != aPrevCharData->mKerningOffsets.end())
						aSpacing += aKernItr->second * mScale;
				}
				else
				{
					aSpacing = 0;
				}
			}
			else
			{
				aCharWidth = int(aCharData->mWidth * aPointSize / aLayerPointSize);

				if (aPrevChar != 0)
				{

					aSpacing = aBaseFontLayer->mSpacing;
					SexyCharToIntMap::iterator aKernItr =
						aPrevCharData->mKerningOffsets.find(aChar);

					if (aKernItr != aPrevCharData->mKerningOffsets.end())
						aSpacing += aKernItr->second * aPointSize / aLayerPointSize;
				}
				else
				{
					aSpacing = 0;
				}
			}

			aLayerXPos += aCharWidth + aSpacing;

			if (aLayerXPos > aMaxXPos)
				aMaxXPos = aLayerXPos;

			int aWidth = anActiveFontLayer->mScaledCharImageRects[aChar].mWidth;
			int aHeight = anActiveFontLayer->mScaledCharImageRects[aChar].mHeight;
			if (aWidth > aCharMaxWidth)
				aCharMaxWidth = aWidth;
			if (aHeight > aCharMaxHeight)
				aCharMaxHeight = aHeight;
			if (j < sizeof(glyph.mNativeData) / sizeof(glyph.mNativeData[0]))
				glyph.mNativeData[j] = aCharData;
		}

		glyph.mX = x;
		glyph.mWidth = aCharMaxWidth;
		glyph.mHeight = aCharMaxHeight;
		glyph.mAdvanceX = aMaxXPos;
		glyph.mAdvanceY = 0;

		x += glyph.mAdvanceX;
		y += glyph.mAdvanceY;

		aPrevChar = aChar;
	}

	return true;
}

static inline Color ColorMulAdd(const Color &c1, const Color &c2, const Color &c3)
{
	Color aColor;

	aColor.mRed = std::min((c1.mRed * c2.mRed / 255) + c3.mRed, 255);
	aColor.mGreen =  std::min((c1.mGreen * c2.mGreen / 255) + c3.mGreen, 255);
	aColor.mBlue = std::min((c1.mBlue * c2.mBlue / 255) + c3.mBlue, 255);
	aColor.mAlpha = std::min((c1.mAlpha * c2.mAlpha / 255) + c3.mAlpha, 255);

	return aColor;
}

void ImageFont::DrawGlyphs(Graphics* g, int theX, int theY,
			   GlyphVector& theGlyphs,
			   size_t from, size_t length,
			   const Color& theColor, const Rect& theClipRect)
{
	if (from > theGlyphs.size() - 1)
		return;

	AutoCrit anAutoCrit(gRenderCritSec);

	int aPoolIdx;

	for (aPoolIdx = 0; aPoolIdx < 256; aPoolIdx++)
	{
		gRenderHead[aPoolIdx] = NULL;
		gRenderTail[aPoolIdx] = NULL;
	}

	Prepare();

	bool colorizeImages = g->GetColorizeImages();
	g->SetColorizeImages(true);

	size_t theGlyphSize = theGlyphs.size();
	size_t aActiveLayerListSize = mActiveLayerList.size();

	std::vector<Color> aColors(aActiveLayerListSize);
	for (size_t i = 0; i < mActiveLayerList.size(); i++)
		aColors[i] = ColorMulAdd(theColor,
					 mActiveLayerList[i].mBaseFontLayer->mColorMult,
					 mActiveLayerList[i].mBaseFontLayer->mColorAdd);

	int aCurPoolIdx = 0;

	if (from + length < theGlyphSize)
		theGlyphSize = from + length;
	for (size_t aCharNum = from; aCharNum < theGlyphSize; aCharNum++)
	{
		Glyph &aGlyph = theGlyphs[aCharNum];
		int aChar = aGlyph.mIndex;

		for (size_t i = 0; i < aActiveLayerListSize; i++)
		{
			ActiveFontLayer* anActiveFontLayer = &mActiveLayerList[i];
			FontLayer* aBaseFontLayer = anActiveFontLayer->mBaseFontLayer;
			CharData* aCharData;

			if (i < sizeof(aGlyph.mNativeData) / sizeof(aGlyph.mNativeData[0]))
				aCharData = (CharData*)aGlyph.mNativeData[i];
			else
				aCharData = aBaseFontLayer->GetCharData(aChar);

			int aLayerXPos = theX + aGlyph.mX;
			int aLayerYPos = theY + aGlyph.mY;

			int anImageX;
			int anImageY;

			int aLayerPointSize = aBaseFontLayer->mPointSize;

			double aScale = mScale;
			if (aLayerPointSize != 0)
				aScale *= mPointSize / aLayerPointSize;

			if (aScale == 1.0)
			{
				anImageX = aLayerXPos + \
					aBaseFontLayer->mOffset.mX + \
					aCharData->mOffset.mX;
				anImageY = aLayerYPos - \
					(aBaseFontLayer->mAscent - \
					 aBaseFontLayer->mOffset.mY - \
					 aCharData->mOffset.mY);
			}
			else
			{
				anImageX =
					aLayerXPos +			\
					(int)((aBaseFontLayer->mOffset.mX + \
					       aCharData->mOffset.mX) * aScale);
				anImageY =
					aLayerYPos - \
					(int)((aBaseFontLayer->mAscent - \
					       aBaseFontLayer->mOffset.mY - \
					       aCharData->mOffset.mY) * aScale);
			}

			int anOrder = aBaseFontLayer->mBaseOrder + aCharData->mOrder;

			if (aCurPoolIdx >= POOL_SIZE)
				break;

			RenderCommand* aRenderCommand = &gRenderCommandPool[aCurPoolIdx++];
			const Rect& rect = anActiveFontLayer->mScaledCharImageRects[aChar];

			aRenderCommand->mImage = anActiveFontLayer->mScaledImage;
			aRenderCommand->mColor = aColors[i];
			aRenderCommand->mDest[0] = anImageX;
			aRenderCommand->mDest[1] = anImageY;
			aRenderCommand->mSrc[0] = rect.mX;
			aRenderCommand->mSrc[1] = rect.mY;
			aRenderCommand->mSrc[2] = rect.mWidth;
			aRenderCommand->mSrc[3] = rect.mHeight;
			aRenderCommand->mMode = aBaseFontLayer->mDrawMode;
			aRenderCommand->mNext = NULL;

			int anOrderIdx = std::min(std::max(anOrder + 128, 0), 255);

			if (gRenderTail[anOrderIdx] == NULL)
			{
				gRenderTail[anOrderIdx] = aRenderCommand;
				gRenderHead[anOrderIdx] = aRenderCommand;
			}
			else
			{
				gRenderTail[anOrderIdx]->mNext = aRenderCommand;
				gRenderTail[anOrderIdx] = aRenderCommand;
			}
		}
	}

	Color anOrigColor = g->GetColor();

	for (aPoolIdx = 0; aPoolIdx < 256; aPoolIdx++)
	{
		RenderCommand* aRenderCommand = gRenderHead[aPoolIdx];

		while (aRenderCommand != NULL)
		{
			int anOldDrawMode = g->GetDrawMode();
			if (aRenderCommand->mMode != -1)
				g->SetDrawMode(aRenderCommand->mMode);
			g->SetColor(Color(aRenderCommand->mColor));
			if (aRenderCommand->mImage != NULL)
				g->DrawImage(aRenderCommand->mImage,
					     aRenderCommand->mDest[0],
					     aRenderCommand->mDest[1],
					     Rect(aRenderCommand->mSrc[0],
						  aRenderCommand->mSrc[1],
						  aRenderCommand->mSrc[2],
						  aRenderCommand->mSrc[3]));
			g->SetDrawMode(anOldDrawMode);

			aRenderCommand = aRenderCommand->mNext;
		}
	}

	g->SetColor(anOrigColor);
	g->SetColorizeImages(colorizeImages);
}

void ImageFont::DrawString(Graphics* g, int theX, int theY, const std::string& theString, const Color& theColor, const Rect& theClipRect)
{
	DrawStringEx(g, theX, theY, theString, theColor, &theClipRect, NULL, NULL);
}

void ImageFont::DrawString(Graphics* g, int theX, int theY, const std::wstring& theString, const Color& theColor, const Rect& theClipRect)
{
	DrawStringEx(g, theX, theY, theString, theColor, &theClipRect, NULL, NULL);
}

Font* ImageFont::Duplicate()
{
	return new ImageFont(*this);
}

void ImageFont::SetPointSize(int thePointSize)
{
	mPointSize = thePointSize;
	mActiveListValid = false;
}

void ImageFont::SetScale(double theScale)
{
	mScale = theScale;
	mActiveListValid = false;
}

int	ImageFont::GetPointSize()
{
	return mPointSize;
}

int	ImageFont::GetDefaultPointSize()
{
	return mFontData->mDefaultPointSize;
}

bool ImageFont::AddTag(const std::wstring& theTagName)
{
	if (HasTag(theTagName))
		return false;

	std::wstring aTagName = StringToUpper(theTagName);
	mTagVector.push_back(aTagName);
	mActiveListValid = false;
	return true;
}

bool ImageFont::RemoveTag(const std::wstring& theTagName)
{
	std::wstring aTagName = StringToUpper(theTagName);

	WStringVector::iterator anItr = std::find(mTagVector.begin(), mTagVector.end(), aTagName);
	if (anItr == mTagVector.end())
		return false;

	mTagVector.erase(anItr);
	mActiveListValid = false;
	return true;
}

bool ImageFont::HasTag(const std::wstring& theTagName)
{
	WStringVector::iterator anItr = std::find(mTagVector.begin(), mTagVector.end(), theTagName);
	return anItr != mTagVector.end();
}

bool ImageFont::AddTag(const std::string& theTagName)
{
	std::wstring aTagName(StringToWString(theTagName));
	return AddTag(aTagName);
}

bool ImageFont::RemoveTag(const std::string& theTagName)
{
	std::wstring aTagName(StringToWString(theTagName));
	return RemoveTag(aTagName);
}

bool ImageFont::HasTag(const std::string& theTagName)
{
	std::wstring aTagName(StringToWString(theTagName));
	return HasTag(aTagName);
}

std::wstring ImageFont::GetDefine(const std::wstring& theName)
{
	DataElement* aDataElement = mFontData->Dereference(theName);

	if (aDataElement == NULL)
		return L"";

	return mFontData->DataElementToString(aDataElement);
}

void ImageFont::Prepare()
{
	if (!mActiveListValid)
	{
		GenerateActiveFontLayers();
		mActiveListValid = true;
	}
}

int ImageFont::GetMappedChar(int theChar)
{
	SexyCharToSexyCharMap::iterator aCharItr = mFontData->mCharMap.find(theChar);
	if (aCharItr != mFontData->mCharMap.end())
		theChar = aCharItr->second;
	return theChar;
}
