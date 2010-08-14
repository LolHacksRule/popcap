#ifndef __FREETYPE_FONT_H__
#define __FREETYPE_FONT_H__

#include "Font.h"
#include "FreeTypeScaledFont.h"

namespace Sexy
{
	class Image;
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

		virtual int			StringWidth(const std::string& theString);
		virtual void			DrawString(Graphics* g, int theX, int theY,
							   const std::string& theString,
							   const Color& theColor,
							   const Rect& theClipRect);

		virtual int			StringWidth(const std::wstring& theString);
		virtual void			DrawString(Graphics* g, int theX, int theY,
							   const std::wstring& theString,
							   const Color& theColor,
							   const Rect& theClipRect);

		virtual int			CharWidth(int theChar);
		virtual int			CharWidthKern(int theChar, int thePrevChar);
		virtual bool                    StringToGlyphs(const std::wstring &theString,
							       GlyphVector &theGlyphs);
		virtual void                    DrawGlyphs(Graphics *g, int theX, int theY,
							   GlyphVector& theGlyphs, const Color &theColor,
							   const Rect& theClipRect);

		virtual Font*			Duplicate();

	private:
		FreeTypeScaledFont	      * mScaledFont;
	};

}

#endif
