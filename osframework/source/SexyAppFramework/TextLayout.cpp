#include "TextLayout.h"
#include "Font.h"

#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32)
#define random(x) rand(x)
#endif

using namespace Sexy;

TextLayout::TextLayout()
{
	Init();
}

void TextLayout::Init()
{
	mCachePolicy = AUTO_CACHE;
	mCacheUpdated = false;
	mFont = 0;
	mWidth = 0;
	mHeight = 0;
	mNumGlyphs = 0;
	mLineSpacing = -1;
	mJustification = -1;
	mSingleLine = true;
	mWrap = false;
	mRich = false;
	mDirty = true;
	mRect = Rect(0, 0, 0, 0);
	mCanCached = false;
	mSameColorCnt = 0;
	mCacheCnt = 0;
}

TextLayout::TextLayout(const std::string& text, Font* font,
		       bool rich, bool singleline)
{
	Init();
	mFont = font;
	mRich = rich;
	mSingleLine = singleline;
	SetText(text);
}

TextLayout::TextLayout(const std::wstring& text, Font* font,
		       bool rich, bool singleline)
{
	Init();
	mFont = font;
	mRich = rich;
	mSingleLine = singleline;
	SetText(text);
}

TextLayout::~TextLayout()
{
}

size_t TextLayout::GetNumGlyphs()
{
	Update();
	return mNumGlyphs;
}

void TextLayout::SetText(const std::string &text, bool rich)
{
	SetText(Graphics::WStringFromString(text), rich);
}

void TextLayout::SetText(const std::wstring &text, bool rich)
{
	if (mText == text && mRich == rich)
		return;

	mText = text;
	mRich = rich;
	mDirty = true;
}

int TextLayout::GetWidth()
{
	Update();
	return mWidth;
}

int TextLayout::GetHeight()
{
	Update();
	return mHeight;
}

void TextLayout::SetFont(Font *font)
{
	if (font == mFont)
		return;
	mFont = font;
	mDirty = true;
}

Font* TextLayout::GetFont()
{
	return mFont;
}

Rect TextLayout::GetRect() const
{
	return mRect;
}

void TextLayout::SetWidth(int width)
{
	SetRect(Rect(mRect.mX, mRect.mY, width, mHeight));
}

void TextLayout::SetHeight(int height)
{
	SetRect(Rect(mRect.mX, mRect.mY, mWidth, height));
}

void TextLayout::SetRect(const Rect &rect)
{
	if (mRect == rect)
		return;

	mRect = rect;
	if (mRect.mX < 0)
		mRect.mX = 0;
	if (mRect.mWidth && mRect.mX > mRect.mWidth - 1)
		mRect.mX = mRect.mWidth - 1;
	if (mRect.mY < 0)
		mRect.mY = 0;
	if (mRect.mHeight && mRect.mX > mRect.mHeight - 1)
		mRect.mY = mRect.mHeight - 1;
	mDirty = true;
}

void TextLayout::SetCachePolicy(TextCachePolicy policy)
{
	mCachePolicy = policy;
}

TextCachePolicy TextLayout::GetCachePolicy() const
{
	return mCachePolicy;
}

void TextLayout::FastDrawLine(Graphics *g, TextLine& line,
			      int xoffset, int yoffset,
			      const Color &color, int justification,
			      const Rect& rect)
{
	switch (justification)
	{
	case 0:
		xoffset += (rect.mWidth - line.mExtents.mWidth) / 2;
		break;
	case 1:
		xoffset += (rect.mWidth - line.mExtents.mWidth);
		break;
	}

	bool colorizeImages = g->GetColorizeImages();

	g->SetColor(color);
	if (!mFont->IsComposited())
		g->SetColorizeImages(true);
	g->DrawImage(&mCacheImage, xoffset,
		     yoffset - mFont->GetAscent() + mFont->GetAscentPadding());

	g->SetColorizeImages(colorizeImages);
}

void TextLayout::DrawLine(Graphics *g, TextLine& line,
			  size_t from, size_t length,
			  int xoffset, int yoffset,
			  const Color &color, int justification,
			  const Rect& rect)
{
	switch (justification)
	{
	case 0:
		xoffset += (rect.mWidth - line.mExtents.mWidth) / 2;
		break;
	case 1:
		xoffset += (rect.mWidth - line.mExtents.mWidth);
		break;
	}

	size_t numglyphs = 0;
	size_t to = from + length;
	for (size_t i = 0; i < line.mRuns.size(); i++)
	{
		TextRun &run = line.mRuns[i];
		GlyphVector &glyphs = run.mGlyphs;
		Color curcolor;

		if (run.mHasColor)
			curcolor = Color(run.mColor.mRed,
					 run.mColor.mGreen,
					 run.mColor.mBlue,
					 color.mAlpha);
		else
			curcolor = color;

		size_t glyphslength = 0;
		if (from >= numglyphs)
		{
			size_t glyphsstart = from - numglyphs;

			if (to < numglyphs + glyphs.size())
				glyphslength = to - from;
			else
				glyphslength = glyphs.size() - (from - numglyphs);
			mFont->DrawGlyphs(g, xoffset, yoffset,
					  glyphs, glyphsstart, glyphslength,
					  curcolor, g->mClipRect);
		}
		numglyphs += run.mGlyphs.size();
		from += glyphslength;
		if (from == to)
			break;
		xoffset += run.mWidth;
	}
}

void TextLayout::Draw(Graphics *g, int x, int y, const Color &color)
{
	if (!mFont)
		return;
	Update();

	if (!mLines.size())
		return;

	Color oldcolor = g->GetColor();

	if (mCanCached)
	{
		if (mLastColor == color)
		{
			mSameColorCnt++;
			if (!mCacheUpdated && mSameColorCnt > mCacheCnt)
				UpdateCache(color, true);
		}
		else
		{
			mSameColorCnt = 0;
			mCacheUpdated = false;
			mLastColor = color;
		}
	}

	if (mSingleLine)
	{
		TextLine &line = mLines[0];
		int xoffset = x + mRect.mX;
		int yoffset = y + mRect.mY;
		Rect rect = mRect;

		if (rect.mWidth == 0)
			rect.mWidth = line.mExtents.mWidth;

		if (!rect.mHeight ||
		    mRect.mY + line.mExtents.mHeight <= mRect.mHeight)
		{
			if (mCacheUpdated)
				FastDrawLine(g, line, xoffset, yoffset,
					     color, mJustification, rect);
			else
				DrawLine(g, line, 0, line.mNumGlyphs, xoffset, yoffset,
					 color, mJustification, rect);
		}
	}
	else
	{
		int xbase = mRect.mX;
		int ybase =  mRect.mY + mFont->GetAscent() - mFont->GetAscentPadding();
		int xoffset = 0;
		int yoffset = 0;

		if (mLines.size() == 1 && mCacheUpdated)
		{
			TextLine &line = mLines[0];
			Rect rect = mRect;

			if (rect.mWidth == 0)
				rect.mWidth = line.mExtents.mWidth;

			FastDrawLine(g, line,
				     x + xbase + xoffset,
				     y + ybase + yoffset,
				     color, mJustification, rect);
		}
		else
		{
			for (size_t i = 0; i < mLines.size(); i++)
			{
				TextLine &line = mLines[i];
				Rect rect = mRect;

				if (rect.mWidth == 0)
					rect.mWidth = line.mExtents.mWidth;

				if (!rect.mHeight ||
				    (yoffset + mRect.mY >= 0 &&
				     yoffset + mRect.mY + line.mExtents.mHeight <= rect.mHeight))
					DrawLine(g, line, 0, line.mNumGlyphs,
						 x + xbase + xoffset, y + ybase + yoffset,
						 color, mJustification, rect);
				xbase = 0;
				yoffset += line.mExtents.mHeight;
			}
		}
	}

	g->SetColor(oldcolor);
}

void TextLayout::DrawGlyphs(Graphics *g,
			    size_t from, size_t length,
			    int x, int y,
			    const Color &color)
{
	Update();
	if (!mNumGlyphs || from > mNumGlyphs - 1)
		return;

	size_t numglyphs = 0;
	size_t i;
	for (i = 0; i < mLines.size(); i++)
	{
		if (from >= numglyphs && from < numglyphs + mLines[i].mNumGlyphs)
			break;
		numglyphs += mLines[i].mNumGlyphs;
	}

	size_t to = from + length;
	if (to > mNumGlyphs)
		to = mNumGlyphs;

	Rect rect;
	int xbase = mRect.mX;
	int ybase =  mRect.mY;
	int xoffset = 0;
	int yoffset = 0;

	if (!mSingleLine)
		ybase += mFont->GetAscent() - mFont->GetAscentPadding();

	for (; i < mLines.size(); i++)
	{
		TextLine& line = mLines[i];

		rect = mRect;
		if (rect.mWidth == 0)
			rect.mWidth = line.mExtents.mWidth;

		size_t start = from - numglyphs;
		size_t length;

		if (to - from < line.mNumGlyphs)
			length = to - from;
		else
			length = line.mNumGlyphs - start;

		if (!rect.mHeight ||
		    (yoffset + mRect.mY >= 0 &&
		     yoffset + mRect.mY + line.mExtents.mHeight <= rect.mHeight))
			DrawLine(g, line, start, length,
				 x + xbase + xoffset,
				 y + ybase + yoffset,
				 color, mJustification, rect);
		xbase = 0;
		from += length;
		numglyphs += line.mNumGlyphs;
		if (from == to)
			break;
		yoffset += line.mExtents.mHeight;
	}
}

void TextLayout::DrawLine(Graphics *g,
			  size_t line,
			  int x, int y,
			  const Color &color)
{
	DrawLines(g, line, 1, x, y, color);

}

void TextLayout::DrawLines(Graphics *g,
			   size_t from, size_t length,
			   int x, int y,
			   const Color &color)
{
	Update();

	if (mSingleLine)
	{
		if (from > 0 || length != 1)
			return;

		Draw(g, x, y, color);
		return;
	}

	Color oldcolor = g->GetColor();

	size_t to = mLines.size();
	if (from + length < to)
		to = from + length;

	int xbase = mRect.mX;
	int ybase = mRect.mY;
	int xoffset = 0;
	int yoffset = 0;

	if (!mSingleLine)
		ybase += mFont->GetAscent() - mFont->GetAscentPadding();

	for (size_t i = 0; i < to; i++)
	{
		TextLine &line = mLines[i];
		Rect rect = mRect;

		if (rect.mWidth == 0)
			rect.mWidth = line.mExtents.mWidth;

		if (i >= from && (!rect.mHeight ||
				  (yoffset + mRect.mY >= 0 &&
				   yoffset + mRect.mY + line.mExtents.mHeight <= rect.mHeight)))
			DrawLine(g, line, 0, line.mNumGlyphs,
				 x + xbase + xoffset,
				 y + ybase + yoffset,
				 color, mJustification, rect);
		xbase = 0;
		yoffset += line.mExtents.mHeight;
	}

	g->SetColor(oldcolor);
}

void TextLayout::SetLineSpacing(int linespacing)
{
	if (mLineSpacing == linespacing)
		return;

	mLineSpacing = linespacing;
	mDirty = true;
}

int TextLayout::GetLineSpacing() const
{
	return mLineSpacing;
}

void TextLayout::SetJustification(int justification)
{
	if (justification != -1 && justification != 0 &&
	    justification != 1)
		return;
	if (mJustification == justification)
		return;

	mJustification = justification;
}

int TextLayout::GetJustification() const
{
	return mJustification;
}

void TextLayout::SetWrap(bool wrap)
{
	if (mWrap == wrap)
		return;

	mWrap = wrap;

	// single line mode don't do wrapping
	if (!mSingleLine)
		mDirty = true;
}

bool TextLayout::GetWrap() const
{
	return mWrap;
}

void TextLayout::SetSingleLine(bool singleline)
{
	if (mSingleLine == singleline)
		return;

	mSingleLine = singleline;
	mDirty = true;
}

bool TextLayout::GetSingleLine() const
{
	return mSingleLine;
}

const TextLineVector& TextLayout::GetLines()
{
	Update();
	return mLines;
}

const TextLine* TextLayout::GetLine(size_t line)
{
	Update();

	if (line < mLines.size())
		return &mLines[line];
	return 0;
}

TextExtents TextLayout::GetLineExtents(size_t index)
{
	const TextLine* line = GetLine(index);

	if (!line)
	{
		TextExtents extents;

		extents.mWidth = 0;
		extents.mHeight = 0;
		return extents;
	}

	return line->mExtents;
}

int TextLayout::GetGlyphsWidth(const GlyphVector &glyphs)
{
	int width = 0;

	for (size_t i = 0; i < glyphs.size(); i++)
		width += glyphs[i].mAdvanceX;

	return width;
}

int TextLayout::BuildLine(std::wstring text, int offset, int length,
			   bool rich, TextLine &line)
{
	std::wstring str;
	bool hasColor = false;
	DWORD aColor = 0;
	int x = 0;

	for (int i = offset; i < offset + length; i++)
	{
		if (text[i] == L'^' && rich)
		{
			if (i + 1 < length && text[i + 1] == L'^') // literal '^'
			{
				str += '^';
				i++;
			}
			else if (i > length - 8) // badly formatted color specification
			{
				break;
			}
			else // change color instruction
			{
				aColor = 0;

		 		if (text[i + 1] == L'o')
				{
					if (text.substr(i + 1, 6) == L"oldclr", 6)
						hasColor = false;
				}
				else
				{
					for (int aDigitNum = 0; aDigitNum < 6; aDigitNum++)
					{
						int aChar = text[i + aDigitNum + 1];
						int aVal = 0;

						if (aChar >= L'0' && aChar <= L'9')
							aVal = aChar - L'0';
						else if (aChar >= L'A' && aChar <= L'F')
							aVal = (aChar - L'A') + 10;
						else if (aChar >= L'a' && aChar <= L'f')
							aVal = (aChar - L'a') + 10;

						aColor += (aVal << ((5 - aDigitNum) * 4));
					}
					hasColor = true;
				}

				line.mRuns.push_back(TextRun());

				TextRun& run = line.mRuns.back();
				run.mHasColor = hasColor;
				if (hasColor)
					run.mColor = Color((aColor >> 16) & 0xFF,
							   (aColor >> 8) & 0xFF,
							   (aColor) & 0xFF,
							   0xFF);
				else
					run.mColor = Color::White;

#ifdef DEBUG_TEXT_LAYOUT
				printf("New text run: %S\n", str.c_str());
#endif

				mFont->StringToGlyphs(str, run.mGlyphs);
				i += 7;

				run.mWidth = GetGlyphsWidth(run.mGlyphs);
				line.mNumGlyphs += run.mGlyphs.size();
				x += run.mWidth;

				str = L"";
			}
		}
		else
		{
			str += text[i];
		}
	}

	if (!str.empty())
	{
		line.mRuns.push_back(TextRun());

		TextRun& run = line.mRuns.back();
		run.mHasColor = hasColor;
		if (hasColor)
			run.mColor = Color((aColor >> 16) & 0xFF,
					   (aColor >> 8) & 0xFF,
					   (aColor) & 0xFF,
					   0xFF);
		else
			run.mColor = Color::White;

#ifdef DEBUG_TEXT_LAYOUT
		printf("New text run: %S\n", str.c_str());
#endif

		mFont->StringToGlyphs(str, run.mGlyphs);

		run.mWidth = GetGlyphsWidth(run.mGlyphs);
		line.mNumGlyphs += run.mGlyphs.size();
		x += run.mWidth;
	}

	return x;
}

void TextLayout::BuildLines()
{
	mLines.clear();
	mNumGlyphs = 0;

	int linespacing = mLineSpacing <= 0 ? mFont->GetLineSpacing() : mLineSpacing;
	if (mSingleLine)
	{
		mLines.push_back(TextLine());

		TextLine &line = mLines.back();
		int width = BuildLine(mText, 0, mText.length(), mRich, line);
		line.mExtents.mWidth = width;
		line.mExtents.mHeight = linespacing;
		mWidth = width;
		mHeight = linespacing;
		mNumGlyphs = line.mNumGlyphs;
	}
	else
	{
		int length = mText.length();
		int curpos = 0;
		int linestartpos = 0;
		int curwidth = 0;
		int curchar = 0;
		int nextchar = 0;
		int prevchar = 0;
		int spacepos = -1;
		int maxwidth = 0;
		int height = 0;
		int indentx = mRect.mX;
		bool needwrap = false;

		while (curpos < length)
		{
			curchar = mText[curpos];
			nextchar = mText[curpos + 1];
			if (curchar == L'^' && mRich) // Handle special color modifier
			{
				if(curpos + 1 < length)
				{
					if (nextchar == L'^')
					{
                                                // literal '^' -> just skip the extra '^'
						curpos += 1;
					}
					else
					{
						// skip the color specifier when calculating the width
						curpos += 8;
						continue;
					}
				}
			}
			else if (curchar == L' ')
			{
				spacepos = curpos;
			}
			else if (curchar == L'\n')
			{
				// force wrap
				needwrap = true;
				spacepos = curpos;
				curpos += 1; // skip enter on next go round
			}

			curwidth += mFont->CharWidthKern(curchar, prevchar);
			prevchar = curchar;

			if (needwrap || (mWrap && curwidth > mRect.mWidth)) // need to wrap
			{
				mLines.push_back(TextLine());

				TextLine& line = mLines.back();

				int writtenwidth;
				if (spacepos != -1)
				{
					BuildLine(mText, linestartpos,
						  spacepos - linestartpos,
						  mRich, line);

#ifdef DEBUG_TEXT_LAYOUT
					printf("New Line: %S\n",
					       mText.substr(linestartpos,
							    spacepos - linestartpos).c_str());
#endif

					writtenwidth = curwidth + indentx;
					line.mExtents.mWidth = writtenwidth;
					line.mExtents.mHeight = linespacing;
					mNumGlyphs += line.mNumGlyphs;

					// skip spaces
					curpos = spacepos + 1;
					if (curchar != L'\n' && curchar == L' ')
					{
						while (curpos < length)
						{
							if (mText[curpos] == L' ')
								break;
							curpos++;
						}
					}
					linestartpos = curpos;
				}
				else
				{
                                       // ensure at least one character gets written
					if (curpos < linestartpos + 1)
						curpos += 1;

					writtenwidth = BuildLine(mText, linestartpos,
								 curpos - linestartpos,
								 mRich, line);

#ifdef DEBUG_TEXT_LAYOUT
					printf("New Line: %S\n",
					       mText.substr(linestartpos,
							    curpos - linestartpos).c_str());
#endif

					line.mExtents.mWidth = writtenwidth;
					line.mExtents.mHeight = linespacing;
					mNumGlyphs += line.mNumGlyphs;
				}

				if (writtenwidth > maxwidth)
					maxwidth = writtenwidth;
				height += line.mExtents.mHeight;

				linestartpos = curpos;
				spacepos = -1;
				curwidth = 0;
				prevchar = 0;
				indentx = 0;
				needwrap = false;
			}
			else
			{
				curpos += 1;
			}
		}

		if (linestartpos < length) // write the last piece
		{
			int writtenwidth;

			mLines.push_back(TextLine());

			TextLine& line = mLines.back();

			writtenwidth = BuildLine(mText, linestartpos,
						 length - linestartpos,
						 mRich, line);

#ifdef DEBUG_TEXT_LAYOUT
			printf("New Line: %S\n",
			       mText.substr(linestartpos,
					    length - linestartpos).c_str());
#endif

			if (writtenwidth > maxwidth)
				maxwidth = writtenwidth;

			line.mExtents.mWidth = writtenwidth;
			line.mExtents.mHeight = linespacing;
			mNumGlyphs += line.mNumGlyphs;
			height += line.mExtents.mHeight;
		}
		else if (curchar == '\n')
		{
			mLines.push_back(TextLine());

			TextLine& line = mLines.back();
			line.mExtents.mWidth = 0;
			line.mExtents.mHeight = linespacing;
			line.mNumGlyphs = 0;
			height += line.mExtents.mHeight;
		}

		mWidth = maxwidth;
		mHeight = height;
	}
}

void TextLayout::Update()
{
	if (!mDirty)
		return;

	if (mFont)
	{
		BuildLines();
	}
	else
	{
		mLines.clear();
		mWidth = 0;
		mHeight = 0;
	}
	mDirty = false;
	mCacheUpdated = false;
	mCacheImage.Create(0, 0);
	UpdateCache(Color::White);
}

void TextLayout::UpdateCache(const Color& color, bool force)
{
	if (!mWidth || !mHeight || mCachePolicy == NO_CACHE || mRich)
		return;

	if (mLines.size() > 1)
		return;

	if (mWidth * mHeight * 4 > 32 * 1024)
		return;

	mCanCached = true;

	if (!force && mFont->IsComposited())
		return;

	int offset = mFont->GetAscent() - mFont->GetAscentPadding();

	mCacheImage.Create(mWidth, mHeight);

	Graphics g(&mCacheImage);
	mCanCached = false;
	Draw(&g, 0, mSingleLine ? offset : 0, color);

	mCanCached = true;
	mSameColorCnt = 0;
	mCacheColor = color;
	mCacheUpdated = true;
	mCacheCnt = 12 + random() % 60;
}
