#ifndef __TEXT_LAYOUT_H__
#define __TEXT_LAYOUT_H__

#include "Graphics.h"
#include "Font.h"

namespace Sexy {

	struct TextExtents
	{
		int mWidth;
		int mHeight;
	};

	struct TextRun
	{
		int         mWidth;
		bool        mHasColor;
		Color       mColor;
		GlyphVector mGlyphs;
	};
	typedef std::vector<TextRun> TextRunVector;

	struct TextLine
	{
		TextExtents   mExtents;
		size_t        mNumGlyphs;
		TextRunVector mRuns;

		TextLine()
		{
			mExtents.mWidth = 0;
			mExtents.mHeight = 0;
			mNumGlyphs = 0;
		}
	};
	typedef std::vector<TextLine> TextLineVector;

	class TextLayout {

	public:
		TextLayout();
		~TextLayout();

	public:
		void                      SetText(const std::string &text, bool rich = false);
		void                      SetText(const std::wstring &text, bool rich = false);
		int                       GetWidth();
		int                       GetHeight();
		void                      SetFont(Font *font);
		Font*                     GetFont();
		void                      SetRect(const Rect &rect);
		void                      SetLineSpacing(int linespacing);
		int                       GetLineSpacing();
		void                      SetJustification(int justification);
		int                       GetJustification();
		void                      SetSingleLine(bool singleline);
		bool                      GetSingleLine();
		void                      SetWrap(bool wrap);
		bool                      GetWrap();

		size_t                    GetNumGlyphs();
		const TextLineVector&     GetLines();
		const TextLine*           GetLine(size_t line);
		TextExtents               GetLineExtents(size_t line);

		void                      Draw(Graphics *g,
					       int x, int y,
					       const Color &color);

		void                      DrawGlyphs(Graphics *g,
						     size_t from, size_t length,
						     int x, int y,
						     const Color &color);

		void                      DrawLine(Graphics *g,
						   size_t line,
						   int x, int y,
						   const Color &color);

		void                      DrawLines(Graphics *g,
						    size_t from, size_t length,
						    int x, int y,
						    const Color &color);

	private:
		void Update();
		int  GetGlyphsWidth(const GlyphVector &glyphs);
		int  BuildLine(std::wstring text,
			       int offset, int length,
			       bool rich, TextLine &line);
		void BuildLines();
		void DrawLine(Graphics *g,
			      TextLine& line,
			      size_t from, size_t length,
			      int xoffset, int yoffset,
			      const Color &color,
			      int justification,
			      const Rect& rect);

	private:
		bool           mDirty;
		Font          *mFont;
		std::wstring   mText;
		TextLineVector mLines;
		size_t         mNumGlyphs;
		Rect           mRect;
		int            mWidth;
		int            mHeight;
		int            mLineSpacing;
		int            mJustification;
		bool           mSingleLine;
		bool           mWrap;
		bool           mRich;
	};

}

#endif
