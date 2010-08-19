#ifndef __TEXT_LAYOUT_H__
#define __TEXT_LAYOUT_H__

#include "Graphics.h"
#include "Font.h"
#include "MemoryImage.h"

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

	enum TextCachePolicy
	{
		AUTO_CACHE = 0,
		FORCE_CACHE,
		NO_CACHE
	};
	class TextLayout {

	public:
		TextLayout();
		TextLayout(const std::string& text, Font* font = 0,
			   bool rich = false, bool singleline = false);
		TextLayout(const std::wstring& text, Font* font = 0,
			   bool rich = false, bool singleline = false);
		~TextLayout();

	public:
		void                      SetText(const std::string &text, bool rich = false);
		void                      SetText(const std::wstring &text, bool rich = false);
		int                       GetWidth();
		int                       GetHeight();
		void                      SetFont(Font *font);
		Font*                     GetFont();
		void                      SetWidth(int width);
		void                      SetHeight(int height);
		void                      SetRect(const Rect &rect);
		Rect                      GetRect() const;
		void                      SetLineSpacing(int linespacing);
		int                       GetLineSpacing() const;
		void                      SetJustification(int justification);
		int                       GetJustification() const;
		void                      SetSingleLine(bool singleline);
		bool                      GetSingleLine() const;
		void                      SetWrap(bool wrap);
		bool                      GetWrap() const;
		void                      SetCachePolicy(TextCachePolicy policy);
		TextCachePolicy           GetCachePolicy() const;

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
		void Init();
		void Update();
		void UpdateCache(const Color& color, bool force = false);
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
		void FastDrawLine(Graphics *g,
				  TextLine& line,
				  int xoffset, int yoffset,
				  const Color &color,
				  int justification,
				  const Rect& rect);

	private:
		bool             mDirty;
		TextCachePolicy  mCachePolicy;
		Font            *mFont;
		std::wstring     mText;
		TextLineVector   mLines;
		size_t           mNumGlyphs;
		Rect             mRect;
		int              mWidth;
		int              mHeight;
		int              mLineSpacing;
		int              mJustification;
		bool             mSingleLine;
		bool             mWrap;
		bool             mRich;
		MemoryImage      mCacheImage;
		Color            mCacheColor;
		bool             mCacheUpdated;
		Color            mLastColor;
		int              mSameColorCnt;
		bool             mCanCached;
		int              mCacheCnt;
	};

}

#endif
