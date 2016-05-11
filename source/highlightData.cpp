/*******************************************************************************
*                                                                              *
* highlightData.c -- Maintain, and allow user to edit, highlight pattern list  *
*                    used for syntax highlighting                              *
*                                                                              *
* Copyright (C) 1999 Mark Edel                                                 *
*                                                                              *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version. In addition, you may distribute version of this program linked to   *
* Motif or Open Motif. See README for details.                                 *
*                                                                              *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA                                 *
*                                                                              *
* Nirvana Text Editor                                                          *
* April, 1997                                                                  *
*                                                                              *
* Written by Mark Edel                                                         *
*                                                                              *
*******************************************************************************/

#include <QMessageBox>
#include <QPushButton>
#include "ui/DialogLanguageModes.h"
#include "ui/DialogDrawingStyles.h"
#include "ui/DialogSyntaxPatterns.h"

#include "highlightData.h"
#include "TextBuffer.h"
#include "nedit.h"
#include "highlight.h"
#include "regularExp.h"
#include "preferences.h"
#include "help.h"
#include "window.h"
#include "Document.h"
#include "regexConvert.h"
#include "MotifHelper.h"
#include "PatternSet.h"
#include "HighlightPattern.h"
#include "HighlightStyle.h"
#include "misc.h"
#include "managedList.h"
#include "memory.h"

#include <cstdio>
#include <cstring>
#include <climits>
#include <sys/param.h>
#include <memory>
#include <algorithm>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>

namespace {

/* Maximum number of patterns allowed in a pattern set (regular expression
   limitations are probably much more restrictive).  */
const int MAX_PATTERNS = 127;

// Names for the fonts that can be used for syntax highlighting 
const int N_FONT_TYPES = 4;

static const char *FontTypeNames[N_FONT_TYPES] = {
	"Plain", 
	"Italic", 
	"Bold", 
	"Bold Italic"
};


DialogDrawingStyles *DrawingStyles = nullptr;
DialogSyntaxPatterns *SyntaxPatterns = nullptr;

}

// list of available highlight styles 
QList<HighlightStyle *> HighlightStyles;

static bool isDefaultPatternSet(PatternSet *patSet);
static bool styleError(const char *stringStart, const char *stoppedAt, const char *message);
static HighlightPattern *readHighlightPatterns(const char **inPtr, int withBraces, const char **errMsg, int *nPatterns);
static bool checkHighlightDialogData(void);
static int lookupNamedStyle(view::string_view styleName);
static int readHighlightPattern(const char **inPtr, const char **errMsg, HighlightPattern *pattern);
static int updatePatternSet(void);
static PatternSet *getDialogPatternSet(void);
static PatternSet *highlightError(const char *stringStart, const char *stoppedAt, const char *message);
static PatternSet *readPatternSet(const char **inPtr, int convertOld);
static std::string createPatternsString(PatternSet *patSet, const char *indentStr);
static void convertOldPatternSet(PatternSet *patSet);
static void langModeCB(Widget w, XtPointer clientData, XtPointer callData);
static void setStyleMenu(view::string_view styleName);
static Widget createHighlightStylesMenu(Widget parent);
static QString convertPatternExprEx(const QString &patternRE, const char *patSetName, const char *patName, bool isSubsExpr);

// Highlight dialog information 
static struct {
	Widget shell;
	Widget lmOptMenu;
	Widget lmPulldown;
	Widget styleOptMenu;
	Widget stylePulldown;
	Widget nameW;
	Widget topLevelW;
	Widget deferredW;
	Widget subPatW;
	Widget colorPatW;
	Widget simpleW;
	Widget rangeW;
	Widget parentW;
	Widget startW;
	Widget endW;
	Widget errorW;
	Widget lineContextW;
	Widget charContextW;
	Widget managedListW;
	Widget parentLbl;
	Widget startLbl;
	Widget endLbl;
	Widget errorLbl;
	Widget matchLbl;
	QString langModeName;
	int nPatterns;
	HighlightPattern **patterns;
} HighlightDialog = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,   nullptr, nullptr, nullptr,
                     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, QString(), 0,       nullptr};

// Pattern sources loaded from the .nedit file or set by the user 
int NPatternSets = 0;
PatternSet *PatternSets[MAX_LANGUAGE_MODES];

static const char *DefaultPatternSets[] = {
	#include "DefaultPatternSet00.inc"
	,
	#include "DefaultPatternSet01.inc"
	,
	#include "DefaultPatternSet02.inc"
	,
	#include "DefaultPatternSet03.inc"
	,
	#include "DefaultPatternSet04.inc"
	,
	#include "DefaultPatternSet05.inc"
	,
	#include "DefaultPatternSet06.inc"
	,
	#include "DefaultPatternSet07.inc"
	,
	#include "DefaultPatternSet08.inc"
	,
	#include "DefaultPatternSet09.inc"
	,
	#include "DefaultPatternSet10.inc"
	,
	#include "DefaultPatternSet11.inc"
	,
	#include "DefaultPatternSet12.inc"
	,
	#include "DefaultPatternSet13.inc"
	,
	#include "DefaultPatternSet14.inc"
	,
	#include "DefaultPatternSet15.inc"
	,
	#include "DefaultPatternSet16.inc"
	,
	#include "DefaultPatternSet17.inc"
	,
	#include "DefaultPatternSet18.inc"
	,
	#include "DefaultPatternSet19.inc"
	,
	#include "DefaultPatternSet20.inc"
	,
	#include "DefaultPatternSet21.inc"
	,
	#include "DefaultPatternSet22.inc"
	,
	#include "DefaultPatternSet23.inc"
	,
	#include "DefaultPatternSet24.inc"
	,
	#include "DefaultPatternSet25.inc"
	,
	#include "DefaultPatternSet26.inc"
	,
	#include "DefaultPatternSet27.inc"
};

/*
** Read a string (from the  value of the styles resource) containing highlight
** styles information, parse it, and load it into the stored highlight style
** list (HighlightStyles) for this NEdit session.
*/
bool LoadStylesStringEx(const std::string &string) {

	// TODO(eteran): implement this using better algorithms

	const char *inString = &string[0];
    const char *errMsg;
	QString fontStr;
    const char *inPtr = &string[0];
    int i;

	for (;;) {

		// skip over blank space 
		inPtr += strspn(inPtr, " \t");

		// Allocate a language mode structure in which to store the info. 
		auto hs = new HighlightStyle;

		// read style name 
		QString name = ReadSymbolicFieldEx(&inPtr);
		if (name.isNull()) {
			return styleError(inString, inPtr, "style name required");
		}		
		hs->name = name;
		
		if (!SkipDelimiter(&inPtr, &errMsg)) {
			delete hs;
			return styleError(inString,inPtr, errMsg);
		}

		// read color 
		QString color = ReadSymbolicFieldEx(&inPtr);
		if (color.isNull()) {
			delete hs;
			return styleError(inString,inPtr, "color name required");
		}
		
		hs->color   = color;
		hs->bgColor = QString();
		
		if (SkipOptSeparator('/', &inPtr)) {
			// read bgColor
			QString s = ReadSymbolicFieldEx(&inPtr); // no error if fails 
			if(!s.isNull()) {
				hs->bgColor = s;
			}
	
		}
		
		if (!SkipDelimiter(&inPtr, &errMsg)) {
			delete hs;
			return styleError(inString,inPtr, errMsg);
		}

		// read the font type 
		// TODO(eteran): Note, assumes success!
		fontStr = ReadSymbolicFieldEx(&inPtr);
		
		for (i = 0; i < N_FONT_TYPES; i++) {
			if (FontTypeNames[i] == fontStr.toStdString()) {
				hs->font = i;
				break;
			}
		}
	
		if (i == N_FONT_TYPES) {
			delete hs;
			return styleError(inString, inPtr, "unrecognized font type");
		}

		/* pattern set was read correctly, add/change it in the list */\
		for (i = 0; i < HighlightStyles.size(); i++) {
			if (HighlightStyles[i]->name == hs->name) {			
				delete HighlightStyles[i];
				HighlightStyles[i] = hs;
				break;
			}
		}
		
	
		if (i == HighlightStyles.size()) {
			HighlightStyles.push_back(hs);
		}

		// if the string ends here, we're done 
		inPtr += strspn(inPtr, " \t\n");
		if (*inPtr == '\0') {
			return true;
		}
	}
}

/*
** Create a string in the correct format for the styles resource, containing
** all of the highlight styles information from the stored highlight style
** list (HighlightStyles) for this NEdit session.
*/
QString WriteStylesStringEx(void) {
	int i;
	HighlightStyle *style;

	auto outBuf = memory::make_unique<TextBuffer>();

	for (i = 0; i < HighlightStyles.size(); i++) {
		style = HighlightStyles[i];
		outBuf->BufAppendEx("\t");
		outBuf->BufAppendEx(style->name.toStdString());
		outBuf->BufAppendEx(":");
		outBuf->BufAppendEx(style->color.toStdString());
		if (!style->bgColor.isNull()) {
			outBuf->BufAppendEx("/");
			outBuf->BufAppendEx(style->bgColor.toStdString());
		}
		outBuf->BufAppendEx(":");
		outBuf->BufAppendEx(FontTypeNames[style->font]);
		outBuf->BufAppendEx("\\n\\\n");
	}

	// Get the output, and lop off the trailing newlines 
	return QString::fromStdString(outBuf->BufGetRangeEx(0, outBuf->BufGetLength() - (i == 1 ? 0 : 4)));
}

/*
** Read a string representing highlight pattern sets and add them
** to the PatternSets list of loaded highlight patterns.  Note that the
** patterns themselves are not parsed until they are actually used.
**
** The argument convertOld, reads patterns in pre 5.1 format (which means
** that they may contain regular expressions are of the older syntax where
** braces were not quoted, and \0 was a legal substitution character).
*/

bool LoadHighlightStringEx(const std::string &string, int convertOld) {

	// TODO(eteran): rework this to actually use a modern approach
	const char *inString = &string[0];
	const char *inPtr = inString;
	int i;

	for (;;) {

		// Read each pattern set, abort on error 
		PatternSet *patSet = readPatternSet(&inPtr, convertOld);
		if(!patSet) {
			return false;
		}

		// Add/change the pattern set in the list 
		for (i = 0; i < NPatternSets; i++) {
			if (PatternSets[i]->languageMode == patSet->languageMode) {
				delete PatternSets[i];
				PatternSets[i] = patSet;
				break;
			}
		}
		if (i == NPatternSets) {
			PatternSets[NPatternSets++] = patSet;
			if (NPatternSets > MAX_LANGUAGE_MODES) {
				return false;
			}
		}

		// if the string ends here, we're done 
		inPtr += strspn(inPtr, " \t\n");
		if (*inPtr == '\0') {
			return true;
		}
	}
}

/*
** Create a string in the correct format for the highlightPatterns resource,
** containing all of the highlight pattern information from the stored
** highlight pattern list (PatternSets) for this NEdit session.
*/
QString WriteHighlightStringEx(void) {

	bool written = false;
	auto outBuf = memory::make_unique<TextBuffer>();

	for (int psn = 0; psn < NPatternSets; psn++) {
		PatternSet *patSet = PatternSets[psn];
		if (patSet->nPatterns == 0) {
			continue;
		}
		
		written = true;		
		outBuf->BufAppendEx(patSet->languageMode.toStdString());
		outBuf->BufAppendEx(":");
		if (isDefaultPatternSet(patSet))
			outBuf->BufAppendEx("Default\n\t");
		else {
			outBuf->BufAppendEx(std::to_string(patSet->lineContext));
			outBuf->BufAppendEx(":");
			outBuf->BufAppendEx(std::to_string(patSet->charContext));
			outBuf->BufAppendEx("{\n");
			outBuf->BufAppendEx(createPatternsString(patSet, "\t\t"));
			outBuf->BufAppendEx("\t}\n\t");
		}
	}

	// Get the output string, and lop off the trailing newline and tab 
	std::string outStr = outBuf->BufGetRangeEx(0, outBuf->BufGetLength() - (written ? 2 : 0));

	/* Protect newlines and backslashes from translation by the resource
	   reader */
	return QString::fromStdString(EscapeSensitiveCharsEx(outStr));
}

/*
** Update regular expressions in stored pattern sets to version 5.1 regular
** expression syntax, in which braces and \0 have different meanings
*/
static void convertOldPatternSet(PatternSet *patSet) {

	for (int p = 0; p < patSet->nPatterns; p++) {
		HighlightPattern *pattern = &patSet->patterns[p];
		pattern->startRE = convertPatternExprEx(pattern->startRE, patSet->languageMode.toLatin1().data(), pattern->name.c_str(), pattern->flags & COLOR_ONLY);
		pattern->endRE   = convertPatternExprEx(pattern->endRE,   patSet->languageMode.toLatin1().data(), pattern->name.c_str(), pattern->flags & COLOR_ONLY);
		pattern->errorRE = convertPatternExprEx(pattern->errorRE, patSet->languageMode.toLatin1().data(), pattern->name.c_str(), pattern->flags & COLOR_ONLY);
	}
}

/*
** Convert a single regular expression, patternRE, to version 5.1 regular
** expression syntax.  It will convert either a match expression or a
** substitution expression, which must be specified by the setting of
** isSubsExpr.  Error messages are directed to stderr, and include the
** pattern set name and pattern name as passed in patSetName and patName.
*/
static QString convertPatternExprEx(const QString &patternRE, const char *patSetName, const char *patName, bool isSubsExpr) {

	if (patternRE.isNull()) {
		return QString();
	}
	
	if (isSubsExpr) {
		// TODO(eteran): the + 5000 seems a bit wasteful
		const int bufsize = patternRE.size() + 5000;		
		char *newRE = XtMalloc(bufsize);
		ConvertSubstituteRE(patternRE.toLatin1().data(), newRE, bufsize);
		QString s = QLatin1String(newRE);
		XtFree(newRE);
		return s;
	} else {
		try {
			char *newRE = ConvertRE(patternRE.toLatin1().data());
			QString s = QLatin1String(newRE);
			XtFree(newRE);
			return s;
		} catch(const regex_error &e) {
			fprintf(stderr, "NEdit error converting old format regular expression in pattern set %s, pattern %s: %s\n", patSetName, patName, e.what());
		}
	}
	
	return QString();
}

/*
** Find the font (font struct) associated with a named style.
** This routine must only be called with a valid styleName (call
** NamedStyleExists to find out whether styleName is valid).
*/
XFontStruct *FontOfNamedStyle(Document *window, view::string_view styleName) {
	int styleNo = lookupNamedStyle(styleName), fontNum;
	XFontStruct *font;

	if (styleNo < 0)
		return GetDefaultFontStruct(window->fontList_);
	fontNum = HighlightStyles[styleNo]->font;
	if (fontNum == BOLD_FONT)
		font = window->boldFontStruct_;
	else if (fontNum == ITALIC_FONT)
		font = window->italicFontStruct_;
	else if (fontNum == BOLD_ITALIC_FONT)
		font = window->boldItalicFontStruct_;
	else // fontNum == PLAIN_FONT 
		font = GetDefaultFontStruct(window->fontList_);

	// If font isn't loaded, silently substitute primary font 
	return font == nullptr ? GetDefaultFontStruct(window->fontList_) : font;
}

int FontOfNamedStyleIsBold(view::string_view styleName) {
	int styleNo = lookupNamedStyle(styleName), fontNum;

	if (styleNo < 0)
		return 0;
	fontNum = HighlightStyles[styleNo]->font;
	return (fontNum == BOLD_FONT || fontNum == BOLD_ITALIC_FONT);
}

int FontOfNamedStyleIsItalic(view::string_view styleName) {
	int styleNo = lookupNamedStyle(styleName), fontNum;

	if (styleNo < 0)
		return 0;
	fontNum = HighlightStyles[styleNo]->font;
	return (fontNum == ITALIC_FONT || fontNum == BOLD_ITALIC_FONT);
}

/*
** Find the color associated with a named style.  This routine must only be
** called with a valid styleName (call NamedStyleExists to find out whether
** styleName is valid).
*/
QString ColorOfNamedStyleEx(view::string_view styleName) {
	int styleNo = lookupNamedStyle(styleName);

	if (styleNo < 0) {
		return QLatin1String("black");
	}
		
	return HighlightStyles[styleNo]->color;
}

/*
** Find the background color associated with a named style.
*/
QString BgColorOfNamedStyleEx(view::string_view styleName) {
	int styleNo = lookupNamedStyle(styleName);

	if (styleNo < 0) {
		return QLatin1String("");
	}
	
	return HighlightStyles[styleNo]->bgColor;	
	
}

/*
** Determine whether a named style exists
*/
bool NamedStyleExists(view::string_view styleName) {
	return lookupNamedStyle(styleName) != -1;
}

/*
** Look through the list of pattern sets, and find the one for a particular
** language.  Returns nullptr if not found.
*/
PatternSet *FindPatternSet(const QString &langModeName) {

	for (int i = 0; i < NPatternSets; i++) {
		if (langModeName == PatternSets[i]->languageMode) {
			return PatternSets[i];
		}
	}
	
	return nullptr;
}

/*
** Returns True if there are highlight patterns, or potential patterns
** not yet committed in the syntax highlighting dialog for a language mode,
*/
bool LMHasHighlightPatterns(const QString &languageMode) {
	if (FindPatternSet(languageMode) != nullptr)
		return true;
		
	return HighlightDialog.shell != nullptr && languageMode == HighlightDialog.langModeName && HighlightDialog.nPatterns != 0;
}

/*
** Change the language mode name of pattern sets for language "oldName" to
** "newName" in both the stored patterns, and the pattern set currently being
** edited in the dialog.
*/
void RenameHighlightPattern(view::string_view oldName, view::string_view newName) {

	for (int i = 0; i < NPatternSets; i++) {
	
		if (PatternSets[i]->languageMode.toStdString() == oldName) {
			PatternSets[i]->languageMode = QString::fromStdString(newName.to_string());
		}
	}
	
	if (HighlightDialog.shell) {
		if (HighlightDialog.langModeName.toStdString() == oldName) {
			HighlightDialog.langModeName = QString::fromStdString(newName.to_string());
		}
	}
}

/*
** Create a pulldown menu pane with the names of the current highlight styles.
** XmNuserData for each item contains a pointer to the name.
*/
static Widget createHighlightStylesMenu(Widget parent) {

	Widget menu = CreatePulldownMenu(parent, "highlightStyles", nullptr, 0);
	
	for(HighlightStyle *style : HighlightStyles) {
	
		XmString s1 = XmStringCreateSimpleEx(style->name);
	
		XtVaCreateManagedWidget(
			"highlightStyles", 
			xmPushButtonWidgetClass, 
			menu, 
			XmNlabelString, 
			s1,
			XmNuserData, 
			style->name.data(), // NOTE(eteran): is this safe? will it be invalidated at some point?
			nullptr);
			
		XmStringFree(s1);
	}
	return menu;
}

static std::string createPatternsString(PatternSet *patSet, const char *indentStr) {

	auto outBuf = std::unique_ptr<TextBuffer>(new TextBuffer);

	for (int pn = 0; pn < patSet->nPatterns; pn++) {
		HighlightPattern *pat = &patSet->patterns[pn];
		outBuf->BufAppendEx(indentStr);
		outBuf->BufAppendEx(pat->name);
		outBuf->BufAppendEx(":");
		if (!pat->startRE.isNull()) {
			std::string str = MakeQuotedStringEx(pat->startRE.toStdString());
			outBuf->BufAppendEx(str);
		}
		outBuf->BufAppendEx(":");
		if (!pat->endRE.isNull()) {
			std::string str = MakeQuotedStringEx(pat->endRE.toStdString());
			outBuf->BufAppendEx(str);
		}
		outBuf->BufAppendEx(":");
		if (!pat->errorRE.isNull()) {
			std::string str = MakeQuotedStringEx(pat->errorRE.toStdString());
			outBuf->BufAppendEx(str);
		}
		outBuf->BufAppendEx(":");
		outBuf->BufAppendEx(pat->style.toStdString());
		outBuf->BufAppendEx(":");
		if (!pat->subPatternOf.isNull())
			outBuf->BufAppendEx(pat->subPatternOf.toStdString());
		outBuf->BufAppendEx(":");
		if (pat->flags & DEFER_PARSING)
			outBuf->BufAppendEx("D");
		if (pat->flags & PARSE_SUBPATS_FROM_START)
			outBuf->BufAppendEx("R");
		if (pat->flags & COLOR_ONLY)
			outBuf->BufAppendEx("C");
		outBuf->BufAppendEx("\n");
	}
	return outBuf->BufGetAllEx();
}

/*
** Read in a pattern set character string, and advance *inPtr beyond it.
** Returns nullptr and outputs an error to stderr on failure.
*/
static PatternSet *readPatternSet(const char **inPtr, int convertOld) {
	const char *errMsg;
	const char *stringStart = *inPtr;
	PatternSet patSet;

	// remove leading whitespace 
	*inPtr += strspn(*inPtr, " \t\n");

	// read language mode field 
	if(const char *s = ReadSymbolicField(inPtr)) {
		patSet.languageMode = QLatin1String(s);
	}
	
	if (patSet.languageMode.isNull()) {
		return highlightError(stringStart, *inPtr, "language mode must be specified");
	}

	if (!SkipDelimiter(inPtr, &errMsg)) {
		return highlightError(stringStart, *inPtr, errMsg);
	}

	/* look for "Default" keyword, and if it's there, return the default
	   pattern set */
	if (!strncmp(*inPtr, "Default", 7)) {
		*inPtr += 7;
		PatternSet *retPatSet = readDefaultPatternSet(patSet.languageMode.toLatin1().data());
		if(!retPatSet)
			return highlightError(stringStart, *inPtr, "No default pattern set");
		return retPatSet;
	}

	// read line context field 
	if (!ReadNumericField(inPtr, &patSet.lineContext))
		return highlightError(stringStart, *inPtr, "unreadable line context field");
	if (!SkipDelimiter(inPtr, &errMsg))
		return highlightError(stringStart, *inPtr, errMsg);

	// read character context field 
	if (!ReadNumericField(inPtr, &patSet.charContext))
		return highlightError(stringStart, *inPtr, "unreadable character context field");

	// read pattern list 
	patSet.patterns = readHighlightPatterns(inPtr, True, &errMsg, &patSet.nPatterns);
	if (!patSet.patterns)
		return highlightError(stringStart, *inPtr, errMsg);

	// pattern set was read correctly, make an allocated copy to return 
	auto retPatSet = new PatternSet(patSet);

	/* Convert pre-5.1 pattern sets which use old regular expression
	   syntax to quote braces and use & rather than \0 */
	if (convertOld) {
		convertOldPatternSet(retPatSet);
	}

	return retPatSet;
}

/*
** Parse a set of highlight patterns into an array of HighlightPattern
** structures, and a language mode name.  If unsuccessful, returns nullptr with
** (statically allocated) message in "errMsg".
*/
static HighlightPattern *readHighlightPatterns(const char **inPtr, int withBraces, const char **errMsg, int *nPatterns) {
	HighlightPattern *pat, *returnedList, patternList[MAX_PATTERNS];

	// skip over blank space 
	*inPtr += strspn(*inPtr, " \t\n");

	// look for initial brace 
	if (withBraces) {
		if (**inPtr != '{') {
			*errMsg = "pattern list must begin with \"{\"";
			return False;
		}
		(*inPtr)++;
	}

	/*
	** parse each pattern in the list
	*/
	pat = patternList;
	while (true) {
		*inPtr += strspn(*inPtr, " \t\n");
		if (**inPtr == '\0') {
			if (withBraces) {
				*errMsg = "end of pattern list not found";
				return nullptr;
			} else
				break;
		} else if (**inPtr == '}') {
			(*inPtr)++;
			break;
		}
		if (pat - patternList >= MAX_PATTERNS) {
			*errMsg = "max number of patterns exceeded\n";
			return nullptr;
		}
		if (!readHighlightPattern(inPtr, errMsg, pat++))
			return nullptr;
	}

	// allocate a more appropriately sized list to return patterns 
	*nPatterns = pat - patternList;
	returnedList = new HighlightPattern[*nPatterns];
	std::copy_n(patternList, *nPatterns, returnedList);
	return returnedList;
}

static int readHighlightPattern(const char **inPtr, const char **errMsg, HighlightPattern *pattern) {

	// read the name field 
	QString name = ReadSymbolicFieldEx(inPtr);
	if (name.isNull()) {
		*errMsg = "pattern name is required";
		return False;
	}
	pattern->name = name.toStdString();
	
	if (!SkipDelimiter(inPtr, errMsg))
		return False;

	// read the start pattern 
	if (!ReadQuotedStringEx(inPtr, errMsg, &pattern->startRE))
		return False;
	if (!SkipDelimiter(inPtr, errMsg))
		return False;

	// read the end pattern 
	
	if (**inPtr == ':') {
		pattern->endRE = QString();
	} else if (!ReadQuotedStringEx(inPtr, errMsg, &pattern->endRE)) {
		return False;
	}

	if (!SkipDelimiter(inPtr, errMsg))
		return False;

	// read the error pattern 
	if (**inPtr == ':') {
		pattern->errorRE = QString();
	} else if (!ReadQuotedStringEx(inPtr, errMsg, &pattern->errorRE)) {
		return False;
	}
	
	if (!SkipDelimiter(inPtr, errMsg))
		return False;

	// read the style field 
	if(const char *s = ReadSymbolicField(inPtr)) {
		pattern->style = QLatin1String(s);
	}
	
	if (pattern->style.isNull()) {
		*errMsg = "style field required in pattern";
		return False;
	}
	
	
	
	if (!SkipDelimiter(inPtr, errMsg))
		return False;

	// read the sub-pattern-of field 
	if(const char *s = ReadSymbolicField(inPtr)) {	
		pattern->subPatternOf = QLatin1String(s);
	}
	
	if (!SkipDelimiter(inPtr, errMsg))
		return False;

	// read flags field 
	pattern->flags = 0;
	for (; **inPtr != '\n' && **inPtr != '}'; (*inPtr)++) {
		if (**inPtr == 'D')
			pattern->flags |= DEFER_PARSING;
		else if (**inPtr == 'R')
			pattern->flags |= PARSE_SUBPATS_FROM_START;
		else if (**inPtr == 'C')
			pattern->flags |= COLOR_ONLY;
		else if (**inPtr != ' ' && **inPtr != '\t') {
			*errMsg = "unreadable flag field";
			return False;
		}
	}
	return True;
}

/*
** Given a language mode name, determine if there is a default (built-in)
** pattern set available for that language mode, and if so, read it and
** return a new allocated copy of it.  The returned pattern set should be
** freed by the caller with delete
*/
PatternSet *readDefaultPatternSet(const char *langModeName) {

	size_t modeNameLen = strlen(langModeName);
	for (int i = 0; i < (int)XtNumber(DefaultPatternSets); i++) {
		if (!strncmp(langModeName, DefaultPatternSets[i], modeNameLen) && DefaultPatternSets[i][modeNameLen] == ':') {
			const char *strPtr = DefaultPatternSets[i];
			return readPatternSet(&strPtr, False);
		}
	}
	return nullptr;
}

/*
** Return True if patSet exactly matches one of the default pattern sets
*/
static bool isDefaultPatternSet(PatternSet *patSet) {

	PatternSet *defaultPatSet = readDefaultPatternSet(patSet->languageMode.toLatin1().data());
	if(!defaultPatSet) {
		return False;
	}
	
	bool retVal = *patSet == *defaultPatSet;
	delete defaultPatSet;
	return retVal;
}

/*
** Short-hand functions for formating and outputing errors for
*/
static PatternSet *highlightError(const char *stringStart, const char *stoppedAt, const char *message) {
	ParseError(nullptr, stringStart, stoppedAt, "highlight pattern", message);
	return nullptr;
}

static bool styleError(const char *stringStart, const char *stoppedAt, const char *message) {
	ParseError(nullptr, stringStart, stoppedAt, "style specification", message);
	return false;
}

/*
** Present a dialog for editing highlight style information
*/
void EditHighlightStyles(const char *initialStyle) {

	if(!DrawingStyles) {
		DrawingStyles = new DialogDrawingStyles();
	}
	
	DrawingStyles->setStyleByName(QLatin1String(initialStyle));
	DrawingStyles->show();
	DrawingStyles->raise();
}

/*
** Present a dialog for editing highlight pattern information
*/
void EditHighlightPatterns(Document *window) {


	if(SyntaxPatterns) {
		SyntaxPatterns->show();
		SyntaxPatterns->raise();
		return;	
	}
	
	if (LanguageModeName(0).isNull()) {
	
		QMessageBox::warning(nullptr /* window->shell_ */, QLatin1String("No Language Modes"), 
			QLatin1String("No Language Modes available for syntax highlighting\n"
			              "Add language modes under Preferenses->Language Modes"));
		return;
	}	
	
	QString languageName = LanguageModeName(window->languageMode_ == PLAIN_LANGUAGE_MODE ? 0 : window->languageMode_);
	SyntaxPatterns = new DialogSyntaxPatterns();
	SyntaxPatterns->setLanguageName(languageName);
	SyntaxPatterns->show();
	SyntaxPatterns->raise();	
}

/*
** If a syntax highlighting dialog is up, ask to have the option menu for
** chosing highlight styles updated (via a call to createHighlightStylesMenu)
*/
void updateHighlightStyleMenu(void) {
	Widget oldMenu;
	int patIndex;

	if (HighlightDialog.shell == nullptr)
		return;

	oldMenu = HighlightDialog.stylePulldown;
	HighlightDialog.stylePulldown = createHighlightStylesMenu(XtParent(XtParent(oldMenu)));
	XtVaSetValues(XmOptionButtonGadget(HighlightDialog.styleOptMenu), XmNsubMenuId, HighlightDialog.stylePulldown, nullptr);
	patIndex = ManagedListSelectedIndex(HighlightDialog.managedListW);
	if (patIndex == -1) {
		setStyleMenu("Plain");
	} else {
		setStyleMenu(HighlightDialog.patterns[patIndex]->style.toStdString());
	}

	XtDestroyWidget(oldMenu);
}

/*
** If a syntax highlighting dialog is up, ask to have the option menu for
** chosing language mode updated (via a call to CreateLanguageModeMenu)
*/
void UpdateLanguageModeMenu(void) {
	Widget oldMenu;

	if (HighlightDialog.shell == nullptr)
		return;

	oldMenu = HighlightDialog.lmPulldown;
	HighlightDialog.lmPulldown = CreateLanguageModeMenu(XtParent(XtParent(oldMenu)), langModeCB, nullptr);
	XtVaSetValues(XmOptionButtonGadget(HighlightDialog.lmOptMenu), XmNsubMenuId, HighlightDialog.lmPulldown, nullptr);
	SetLangModeMenu(HighlightDialog.lmOptMenu, HighlightDialog.langModeName.toLatin1().data());

	XtDestroyWidget(oldMenu);
}

static void langModeCB(Widget w, XtPointer clientData, XtPointer callData) {

	Q_UNUSED(w);
	Q_UNUSED(clientData);
	Q_UNUSED(callData);

	char *modeName;
	PatternSet emptyPatSet;
	int i;

	// Get the newly selected mode name.  If it's the same, do nothing 
	XtVaGetValues(w, XmNuserData, &modeName, nullptr);
	if ((modeName == HighlightDialog.langModeName.toStdString()))
		return;

	// Look up the original version of the patterns being edited 
	PatternSet *oldPatSet = FindPatternSet(HighlightDialog.langModeName);
	if(!oldPatSet) {
		oldPatSet = &emptyPatSet;
	}

	/* Get the current information displayed by the dialog.  If it's bad,
	   give the user the chance to throw it out or go back and fix it.  If
	   it has changed, give the user the chance to apply discard or cancel. */
	PatternSet *newPatSet = getDialogPatternSet();

	if(!newPatSet) {
		QMessageBox messageBox(nullptr /*HighlightDialog.shell*/);
		messageBox.setWindowTitle(QLatin1String("Incomplete Language Mode"));
		messageBox.setIcon(QMessageBox::Warning);
		messageBox.setText(QLatin1String("Discard incomplete entry\nfor current language mode?"));
		QPushButton *buttonKeep    = messageBox.addButton(QLatin1String("Keep"), QMessageBox::RejectRole);
		QPushButton *buttonDiscard = messageBox.addButton(QLatin1String("Discard"), QMessageBox::AcceptRole);
		Q_UNUSED(buttonDiscard);
		
		messageBox.exec();
		if (messageBox.clickedButton() == buttonKeep) {
			SetLangModeMenu(HighlightDialog.lmOptMenu, HighlightDialog.langModeName.toLatin1().data());
			return;
		}
	} else if (*oldPatSet != *newPatSet) {
		QMessageBox messageBox(nullptr /*HighlightDialog.shell*/);
		messageBox.setWindowTitle(QLatin1String("Language Mode"));
		messageBox.setIcon(QMessageBox::Warning);
		messageBox.setText(QString(QLatin1String("Apply changes for language mode %1?")).arg(HighlightDialog.langModeName));
		QPushButton *buttonApply   = messageBox.addButton(QLatin1String("Apply Changes"), QMessageBox::AcceptRole);
		QPushButton *buttonDiscard = messageBox.addButton(QLatin1String("Discard Changes"), QMessageBox::RejectRole);
		QPushButton *buttonCancel  = messageBox.addButton(QMessageBox::Cancel);
		Q_UNUSED(buttonDiscard);

		
		messageBox.exec();
		if (messageBox.clickedButton() == buttonCancel) {
			SetLangModeMenu(HighlightDialog.lmOptMenu, HighlightDialog.langModeName.toLatin1().data());
			return;
		} else if (messageBox.clickedButton() == buttonApply) {
			updatePatternSet();
		}
	}

	if(newPatSet)
		delete newPatSet;

	// Free the old dialog information 
	for (int i = 0; i < HighlightDialog.nPatterns; i++) {
		delete HighlightDialog.patterns[i];
	}

	// Fill the dialog with the new language mode information 
	HighlightDialog.langModeName = QLatin1String(modeName);
	newPatSet = FindPatternSet(QLatin1String(modeName));
	if(!newPatSet) {
		HighlightDialog.nPatterns = 0;
		SetIntText(HighlightDialog.lineContextW, 1);
		SetIntText(HighlightDialog.charContextW, 0);
	} else {
		for (i = 0; i < newPatSet->nPatterns; i++) {
			HighlightDialog.patterns[i] = new HighlightPattern(newPatSet->patterns[i]);
		}
		HighlightDialog.nPatterns = newPatSet->nPatterns;
		SetIntText(HighlightDialog.lineContextW, newPatSet->lineContext);
		SetIntText(HighlightDialog.charContextW, newPatSet->charContext);
	}
	ChangeManagedListData(HighlightDialog.managedListW);
}


/*
** Do a test compile of the patterns currently displayed in the highlight
** patterns dialog, and display warning dialogs if there are problems
*/
static bool checkHighlightDialogData(void) {

	// Get the pattern information from the dialog 
	PatternSet *patSet = getDialogPatternSet();
	if(!patSet) {
		return false;
	}

	// Compile the patterns  
	bool result = (patSet->nPatterns == 0) ? true : TestHighlightPatterns(patSet);
	delete patSet;
	return result;
}

/*
** Set the styles menu in the currently displayed highlight dialog to show
** a particular style
*/
static void setStyleMenu(view::string_view styleName) {
	int i;
	Cardinal nItems;
	WidgetList items;
	Widget selectedItem;
	char *itemStyle;

	XtVaGetValues(HighlightDialog.stylePulldown, XmNchildren, &items, XmNnumChildren, &nItems, nullptr);
	if (nItems == 0)
		return;
	selectedItem = items[0];
	for (i = 0; i < (int)nItems; i++) {
		XtVaGetValues(items[i], XmNuserData, &itemStyle, nullptr);
		if (styleName == itemStyle) {
			selectedItem = items[i];
			break;
		}
	}
	XtVaSetValues(HighlightDialog.styleOptMenu, XmNmenuHistory, selectedItem, nullptr);
}

/*
** Update the pattern set being edited in the Syntax Highlighting dialog
** with the information that the dialog is currently displaying, and
** apply changes to any window which is currently using the patterns.
*/
static int updatePatternSet(void) {
	PatternSet *patSet;
	int psn, oldNum = -1;

	// Make sure the patterns are valid and compile 
	if (!checkHighlightDialogData())
		return False;

	// Get the current data 
	patSet = getDialogPatternSet();
	if(!patSet)
		return False;

	// Find the pattern being modified 
	for (psn = 0; psn < NPatternSets; psn++)
		if (HighlightDialog.langModeName == PatternSets[psn]->languageMode)
			break;

	/* If it's a new pattern, add it at the end, otherwise free the
	   existing pattern set and replace it */
	if (psn == NPatternSets) {
		PatternSets[NPatternSets++] = patSet;
		oldNum = 0;
	} else {
		oldNum = PatternSets[psn]->nPatterns;
		delete PatternSets[psn];
		PatternSets[psn] = patSet;
	}

	/* Find windows that are currently using this pattern set and
	   re-do the highlighting */
	for(Document *window: WindowList) {
		if (patSet->nPatterns > 0) {
			if (window->languageMode_ != PLAIN_LANGUAGE_MODE && (LanguageModeName(window->languageMode_) == patSet->languageMode)) {
				/*  The user worked on the current document's language mode, so
				    we have to make some changes immediately. For inactive
				    modes, the changes will be activated on activation.  */
				if (oldNum == 0) {
					/*  Highlighting (including menu entry) was deactivated in
					    this function or in preferences.c::reapplyLanguageMode()
					    if the old set had no patterns, so reactivate menu entry. */
					if (window->IsTopDocument()) {
						XtSetSensitive(window->highlightItem_, True);
					}

					//  Reactivate highlighting if it's default  
					window->highlightSyntax_ = GetPrefHighlightSyntax();
				}

				if (window->highlightSyntax_) {
					StopHighlighting(window);
					if (window->IsTopDocument()) {
						XtSetSensitive(window->highlightItem_, True);
						window->SetToggleButtonState(window->highlightItem_, True, False);
					}
					StartHighlighting(window, True);
				}
			}
		} else {
			/*  No pattern in pattern set. This will probably not happen much,
			    but you never know.  */
			StopHighlighting(window);
			window->highlightSyntax_ = False;

			if (window->IsTopDocument()) {
				XtSetSensitive(window->highlightItem_, False);
				window->SetToggleButtonState(window->highlightItem_, False, False);
			}
		}
	}

	// Note that preferences have been changed 
	MarkPrefsChanged();

	return True;
}

/*
** Get the current information that the user has entered in the syntax
** highlighting dialog.  Return nullptr if the data is currently invalid
*/
static PatternSet *getDialogPatternSet(void) {
	int lineContext, charContext;

	// Get the current contents of the "patterns" dialog fields 
	if (!UpdateManagedList(HighlightDialog.managedListW, True))
		return nullptr;

	// Get the line and character context values 
	if (GetIntTextWarn(HighlightDialog.lineContextW, &lineContext, "context lines", True) != TEXT_READ_OK)
		return nullptr;
	if (GetIntTextWarn(HighlightDialog.charContextW, &charContext, "context lines", True) != TEXT_READ_OK)
		return nullptr;

	/* Allocate a new pattern set structure and copy the fields read from the
	   dialog, including the modified pattern list into it */
	auto patSet = new PatternSet(HighlightDialog.nPatterns);
	patSet->languageMode = HighlightDialog.langModeName;
	patSet->lineContext  = lineContext;
	patSet->charContext  = charContext;
	
	for (int i = 0; i < HighlightDialog.nPatterns; i++) {		
		patSet->patterns[i] = *(HighlightDialog.patterns[i]);
	}
	
	return patSet;
}

/*
** Find the index into the HighlightStyles array corresponding to "styleName".
** If styleName is not found, return -1.
*/
static int lookupNamedStyle(view::string_view styleName) {

	for (int i = 0; i < HighlightStyles.size(); i++) {
		if (HighlightStyles[i]->name.toStdString() == styleName) {
			return i;
		}
	}
	
	return -1;
}

/*
** Returns a unique number of a given style name
*/
int IndexOfNamedStyle(view::string_view styleName) {
	return lookupNamedStyle(styleName);
}
