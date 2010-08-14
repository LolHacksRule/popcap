#ifndef __TEXT_LAYOUT_H__
#define __TEXT_LAYOUT_H__

#include "Graphics.h"

namespace Sexy {

	class TextLayout {

	public:
		TextLayout();
		~TextLayout();

	public:
		void SetText(const std::string &text, bool rich = false);
		void SetText(const std::wstring &text, bool rich = false);
		int GetWidth();
		int GetHeight();
		void SetFont(Font *font);
		Font* GetFont();
		void SetRect(const Rect &rect);
		void SetLineSpacing(int linespacing);
		int  GetLineSpacing();
		void SetJustification(int justification);
		int GetJustification();
		void SetWrap(bool wrap);
		bool GetWrap();
		void Draw(Graphics *g, int x, int y, const Color &color);

	private:
		void Update();

	private:
		bool          mDirty;
		Font         *mFont;
		std::wstring  mText;
		Rect          mRect;
		int           mWidth;
		int           mHeight;
		int           mLineSpacing;
		int           mJustification;
		bool          mWrap;
		bool          mRich;
	};

}

#endif
