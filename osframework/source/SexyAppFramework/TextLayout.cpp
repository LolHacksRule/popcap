#include "TextLayout.h"
#include "Font.h"

using namespace Sexy;

TextLayout::TextLayout()
{
	mFont = 0;
	mWidth = 0;
	mHeight = 0;
	mLineSpacing = -1;
	mJustification = -1;
	mWrap = false;
	mRich = false;
	mDirty = true;
	mRect = Rect(0, 0, 0, 0);
}

TextLayout::~TextLayout()
{
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

void TextLayout::SetRect(const Rect &rect)
{
	if (mRect == rect)
		return;

	mRect = rect;
	mDirty = true;
}

void TextLayout::DrawLine(Graphics *g, TextLine& line, int xoffset, int yoffset,
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

	for (size_t i = 0; i < line.mRuns.size(); i++)
	{
		TextRun &run = line.mRuns[i];
		Color curcolor;

		if (run.mHasColor)
			curcolor = Color(run.mColor.mRed,
					 run.mColor.mGreen,
					 run.mColor.mBlue,
					 color.mAlpha);
		else
			curcolor = color;

		mFont->DrawGlyphs(g, xoffset, yoffset, run.mGlyphs, curcolor, g->mClipRect);
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

	if (!mWrap)
	{
		TextLine &line = mLines[0];
		int xoffset = x;
		int yoffset = y;
		Rect rect = mRect;

		if (rect.mWidth == 0)
			rect.mWidth = line.mExtents.mWidth;

		DrawLine(g, line, xoffset, yoffset, color, mJustification,
			 rect);
	}
	else
	{
		int xoffset = x;
		int yoffset = y + mFont->GetAscent() - mFont->GetAscentPadding();

		for (size_t i = 0; i < mLines.size(); i++)
		{
			TextLine &line = mLines[i];
			Rect rect = mRect;

			if (rect.mWidth == 0)
				rect.mWidth = line.mExtents.mWidth;

			if (!rect.mHeight ||
			    (yoffset - y >= rect.mY &&
			     yoffset - y + line.mExtents.mHeight <= rect.mY + rect.mHeight))
				DrawLine(g, line, xoffset, yoffset, color,
					 mJustification, rect);
			yoffset += line.mExtents.mHeight;
		}
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

int TextLayout::GetLineSpacing()
{
	return mLineSpacing;
}

void TextLayout::SetJustification(int justification)
{
	if (mJustification == justification)
		return;

	mJustification = justification;
	mDirty = true;
}

int TextLayout::GetJustification()
{
	return mJustification;
}

void TextLayout::SetWrap(bool wrap)
{
	if (mWrap == wrap)
		return;

	mWrap = wrap;
	mDirty = true;
}

bool TextLayout::GetWrap()
{
	return mWrap;
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
		x += run.mWidth;
	}

	return x;
}

void TextLayout::BuildLines()
{
	mLines.clear();

	int linespacing = mLineSpacing <= 0 ? mFont->GetLineSpacing() : mLineSpacing;
	if (!mWrap)
	{
		mLines.push_back(TextLine());

		TextLine &line = mLines.back();
		int width = BuildLine(mText, 0, mText.length(), mRich, line);
		line.mExtents.mWidth = width;
		line.mExtents.mHeight = linespacing;
		mWidth = width;
		mHeight = linespacing;
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
		int indentx = 0;

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
				curwidth = mRect.mWidth + 1; // force word wrap
				spacepos = curpos;
				curpos += 1; // skip enter on next go round
			}

			curwidth += mFont->CharWidthKern(curchar, prevchar);
			prevchar = curchar;

			if (curwidth > mRect.mWidth) // need to wrap
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
				}

				if (writtenwidth > maxwidth)
					maxwidth = writtenwidth;
				height += line.mExtents.mHeight;

				linestartpos = curpos;
				spacepos = -1;
				curwidth = 0;
				prevchar = 0;
				indentx = 0;
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
			height += line.mExtents.mHeight;
		}
		else if (curchar == '\n')
		{
			mLines.push_back(TextLine());

			TextLine& line = mLines.back();
			line.mExtents.mWidth = 0;
			line.mExtents.mHeight = linespacing;
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
}
