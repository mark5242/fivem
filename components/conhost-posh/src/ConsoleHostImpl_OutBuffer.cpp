#include "StdInc.h"
#include "ConsoleHostImpl.h"
#include "FontRenderer.h"
#include "DrawCommands.h"
#include "Screen.h"

struct ConsoleCharacter
{
	wchar_t character;
	uint16_t flags;
};

static int g_bufferWidth;
static int g_bufferHeight;
static ConsoleCharacter* g_consoleBuffer;

void ConHost_NewBuffer(int width, int height)
{
	if (g_consoleBuffer != nullptr)
	{
		delete[] g_consoleBuffer;
		g_consoleBuffer = nullptr;
	}

	width += 1;

	g_consoleBuffer = new ConsoleCharacter[width * height];
	g_bufferWidth = width;
	g_bufferHeight = height;

	memset(g_consoleBuffer, 0, width * height * sizeof(ConsoleCharacter));
}

void* ConHost_GetBuffer()
{
	return g_consoleBuffer;
}

extern bool g_consoleFlag;

static InitFunction initFunction([] ()
{
	OnPostFrontendRender.Connect([] ()
	{
		if (!g_consoleFlag)
		{
			return;
		}

		static bool stringRectSet = false;
		static CRect stringRect;

		if (!stringRectSet)
		{
			TheFonts->GetStringMetrics(L"a", 16.0f, 1.0f, "Lucida Console", stringRect);
		}

		CRect backRect(0.0f, 0.0f, GetScreenResolutionX(), (16.0f * 25) + 16.0f);
		CRGBA backColor(0, 0x2b, 0x36, 220);
		CRGBA frontColor(0xee, 0xe8, 0xd5);

		TheFonts->DrawRectangle(backRect, backColor);

		backRect.SetRect(backRect.fX1, backRect.fY2, backRect.fX2, backRect.fY2 + 5.0f);
		backColor.red *= 0.8;
		backColor.green *= 0.8;
		backColor.blue *= 0.8;
		backColor.alpha = 250;

		TheFonts->DrawRectangle(backRect, backColor);

		for (int i = 0; i < g_bufferHeight; i++)
		{
			wchar_t str[2048];
			int strIdx = 0;
			int strStart = 0;

			float y = 16.0f * i;

			for (int j = 0; j < g_bufferWidth; j++)
			{
				ConsoleCharacter character = g_consoleBuffer[(j * g_bufferHeight) + i];

				if (character.character == '\0')
				{
					if (strIdx > 0)
					{
						str[strIdx] = ' ';
						strIdx++;
					}
				}
				else
				{
					if (strIdx == 0)
					{
						strStart = j;
					}

					str[strIdx] = character.character;
					strIdx++;
				}
			}

			if (strIdx > 0)
			{
				// reset
				str[strIdx] = L'\0';

				strIdx = 0;

				// write!
				CRect frontRect((strStart * 16.0f) + 8.0f, y + 8.0f, GetScreenResolutionX() - 8.0f, y + 8.0f);

				TheFonts->DrawText(str, frontRect, frontColor, 16.0f, 1.0f, "Lucida Console");
			}
		}

		int cx, cy;
		ConHost_GetCursorPos(cx, cy);

		CRect cursorRect(cx * stringRect.Width() + 8.0f, cy * 16.0f + 14.0f + 8.0f, cx * stringRect.Width() + stringRect.Width() + 8.0f, cy * 16.0f + 16.0f + 8.0f);

		TheFonts->DrawRectangle(cursorRect, CRGBA(255, 255, 255));
	}, 90);
});