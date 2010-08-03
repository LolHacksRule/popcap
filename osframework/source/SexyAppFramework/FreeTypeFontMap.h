#ifndef FREETYPEFONTMAP_H
#define FREETYPEFONTMAP_H

#include "AutoCrit.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Sexy
{
	class FreeTypeBaseFont;
	class FreeTypeScaledFont;
	class SexyAppBase;

	class FreeTypeFontMap
	{
		friend class FreeTypeBaseFont;
	public:
		CritSect mCritSect;
		FT_Library mLibrary;

	public:
		static FreeTypeFontMap*    GetFreeTypeFontMap();

		int                        GetMaxFaces();
		int                        GetFaces();
		FT_Library                 GetFreeTypeLibrary();

		// unscaled font managment
		FreeTypeBaseFont*          CreateBaseFont(const char *path,
							  int index);
		void                       RemoveBaseFont(FreeTypeBaseFont* font);

		// scaled font managment
		FreeTypeScaledFont*        CreateScaledFont(SexyAppBase* theApp,
							    const std::string& theFace,
							    int thePointSize,
							    bool bold = false,
							    bool italics = false,
							    bool underline = false);
		FreeTypeScaledFont*        CreateScaledFont(const std::string& theFace,
							    int thePointSize,
							    bool bold = false,
							    bool italics = false,
							    bool underline = false);
		void                       FreeScaledFont(FreeTypeScaledFont *theFont);

		void                       Lock();
		void                       Unlock();

		void                       ReserveFace(FreeTypeBaseFont* font);
		void                       ReleaseFace(FreeTypeBaseFont* font);

	private:
		FreeTypeFontMap();
		~FreeTypeFontMap();

	private:
		int  mMaxFaces;
		int  mFaces;

		typedef std::pair<std::string, int> FontKey;
		typedef std::map<FontKey, FreeTypeBaseFont*> FontMap;

		struct ScaledFontKey {
			SexyAppBase* mApp;
			const std::string& mFace;
			int mPointSize;
			bool mBold;
			bool mItalics;
			bool mUnderline;

			ScaledFontKey(SexyAppBase* theApp,
				      const std::string& theFace,
				      int thePointSize,
				      bool bold = false,
				      bool italics = false,
				      bool underline = false)
			: mApp(theApp), mFace(theFace), mPointSize(thePointSize),
			  mBold(bold), mItalics(italics), mUnderline(underline)
			{
			}

			bool operator < (const ScaledFontKey &rhs) const
			{
				if (mApp < rhs.mApp)
					return true;
				if (mFace < rhs.mFace)
					return true;
				if (mPointSize < rhs.mPointSize)
					return true;
				if (mBold < rhs.mBold)
					return true;
				if (mUnderline < rhs.mUnderline)
					return true;
				return false;
			}

			bool operator == (const ScaledFontKey &rhs) const
			{
				if (mApp != rhs.mApp)
					return false;
				if (mFace != rhs.mFace)
					return true;
				if (mPointSize != rhs.mPointSize)
					return false;
				if (mBold != rhs.mBold)
					return false;
				if (mUnderline != rhs.mUnderline)
					return false;
				return true;
			}

		        bool operator != (const ScaledFontKey &rhs) const
			{
				if (mApp != rhs.mApp)
					return true;
				if (mFace != rhs.mFace)
					return true;
				if (mPointSize != rhs.mPointSize)
					return true;
				if (mBold != rhs.mBold)
					return true;
				if (mUnderline != rhs.mUnderline)
					return true;
				return false;
			}
		};
		typedef std::map<ScaledFontKey, FreeTypeScaledFont*> ScaledFontMap;

		ScaledFontMap                   mScaledFontMap;
		CritSect                        mScaledFontCritSect;
		FontMap                         mFontMap;
	};

}

#endif
