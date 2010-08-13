#include "TextLayout.h"

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
	mRect = Rect(0, 0, 0, 0);
}

TextLayout::~TextLayout()
{
}

void TextLayout::SetText(const std::string &text, bool rich)
{
	SetText(WStringFromString(text), rich);
}

void TextLayout::SetText(const std::wstring &text, bool rich)
{
	if (mText == text && mRich = rich)
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

void TextLayout::Draw(Graphics *g, int x, int y, const Color &color)
{
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

void TextLayout::Update()
{
	if (mFont)
	{
		mWidth = 0;
		mHeight = 0;
	}
	else
	{
		mWidth = 0;
		mHeight = 0;
	}
	mDirty = false;
}
