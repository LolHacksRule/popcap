#ifndef __FREETYPE_FONT_H__
#define __FREETYPE_FONT_H__

#include "Font.h"
#include "FreeTypeBaseFont.h"

namespace Sexy
{

	class ImageFont;
	class SexyAppBase;

	class FreeTypeFont : public Font
	{
	public:
		SexyAppBase*			mApp;

		void Init(SexyAppBase* theApp, const std::string& theFace,
			  int thePointSize, bool bold, bool italics,
			  bool underline);

	public:
		FreeTypeFont(const std::string& theFace, int thePointSize,
			     bool bold = false, bool italics = false,
			     bool underline = false);

		FreeTypeFont(SexyAppBase* theApp, const std::string& theFace,
			     int thePointSize, bool bold = false, bool italics = false,
			     bool underline = false);
		FreeTypeFont(const FreeTypeFont& theFreeTypeFont);

		virtual ~FreeTypeFont();

		virtual int			StringWidth(const SexyString& theString);
		virtual void			DrawString(Graphics* g, int theX, int theY,
							   const SexyString& theString, const Color& theColor,
							   const Rect& theClipRect);

		virtual Font*			Duplicate();

	private:
		FreeTypeBaseFont              * mBaseFont;
		FT_Matrix                       mMatrix;
		float                           mSize;

		FT_Face                         mFace;

		int                             mDescent;

		void                            LockFace();
		void                            UnlockFace();
	};

}

#endif //__SYSFONT_H__
