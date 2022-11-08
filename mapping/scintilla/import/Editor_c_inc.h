

//=================inc-new-flag=================
#include <Scintilla.h>
void Editor::NotifyLexerChanged(Document*, void*) {
}
void Editor::SetRedraw() {
	SetScrollBars();
	Redraw();
}
void Editor::SetSearchStringLight(size_t i, bool isClear) {
	if (SearchStringLight == -1 || i >= SearchStringRanges.size()) {
		return;
	}
	int PrevCurrentIndicator = pdoc->decorations->GetCurrentIndicator();
	int PrevCurrentValue = pdoc->decorations->GetCurrentValue();
	int CurrentValue = ((isClear) ? 0 : SearchStringLightValue);
	pdoc->DecorationSetCurrentIndicator(static_cast<int>(SearchStringLight));
	pdoc->decorations->SetCurrentValue(static_cast<int>(CurrentValue));

	pdoc->DecorationFillRange(static_cast<Sci::Position>(SearchStringRanges[i].start), CurrentValue, SearchStringRanges[i].end - SearchStringRanges[i].start);

	pdoc->DecorationSetCurrentIndicator(static_cast<int>(PrevCurrentIndicator));
	pdoc->decorations->SetCurrentValue(static_cast<int>(PrevCurrentValue));
}
void Editor::SetSearchStringLight(bool isClear) {
	if (SearchStringLight == -1) {
		return;
	}
	int PrevCurrentIndicator = pdoc->decorations->GetCurrentIndicator();
	int PrevCurrentValue = pdoc->decorations->GetCurrentValue();
	int CurrentValue = ((isClear) ? 0 : SearchStringLightValue);
	pdoc->DecorationSetCurrentIndicator(static_cast<int>(SearchStringLight));
	pdoc->decorations->SetCurrentValue(static_cast<int>(CurrentValue));

	//end=length;
	for (std::vector<Range>::const_iterator it = SearchStringRanges.begin(); it != SearchStringRanges.end(); ++it) {
		pdoc->DecorationFillRange(it->start, CurrentValue, it->end - it->start);
	}
	pdoc->DecorationSetCurrentIndicator(static_cast<int>(PrevCurrentIndicator));
	pdoc->decorations->SetCurrentValue(static_cast<int>(PrevCurrentValue));
}
void Editor::FoldLineNotUpdate(Sci::Line line, FoldAction action) {
	if (line >= 0) {
		if (action == FoldAction::Toggle) {
			if ((pdoc->GetLevel(line) & SC_FOLDLEVELHEADERFLAG) == 0) {
			line = pdoc->GetFoldParent(line);
			if (line < 0)
				return;
			}
			action = (pcs->GetExpanded(line)) ? FoldAction::Contract : FoldAction::Expand;
		}

		if (action == FoldAction::Contract) {
			const Sci::Line lineMaxSubord = pdoc->GetLastChild(line);
			if (lineMaxSubord > line) {
				pcs->SetExpanded(line, false);
				pcs->SetVisible(line + 1, lineMaxSubord, false);

				const Sci::Line lineCurrent =
					pdoc->SciLineFromPosition(sel.MainCaret());
				if (lineCurrent > line && lineCurrent <= lineMaxSubord) {
					// This does not re-expand the fold
					EnsureCaretVisible();
				}
			}

		}
		else {
			pcs->SetExpanded(line, true);
			ExpandLine(line);
		}
	}
}
/*
void Editor::FoldLine(Sci::Line line, int action) {
	if (line >= 0) {
		if (action == SC_FOLDACTION_TOGGLE) {
			if ((pdoc->GetLevel(line) & SC_FOLDLEVELHEADERFLAG) == 0) {
				line = pdoc->GetFoldParent(line);
				if (line < 0)
					return;
			}
			action = (pcs->GetExpanded(line)) ? SC_FOLDACTION_CONTRACT : SC_FOLDACTION_EXPAND;
		}

		if (action == SC_FOLDACTION_CONTRACT) {
			const Sci::Line lineMaxSubord = pdoc->GetLastChild(line);
			if (lineMaxSubord > line) {
				pcs->SetExpanded(line, false);
				pcs->SetVisible(line + 1, lineMaxSubord, false);

				const Sci::Line lineCurrent =
					pdoc->SciLineFromPosition(sel.MainCaret());
				if (lineCurrent > line && lineCurrent <= lineMaxSubord) {
					// This does not re-expand the fold
					EnsureCaretVisible();
				}
			}

		}
		else {
			if (!(pcs->GetVisible(line))) {
				//重置且跳转选择,可能不必要,效率较低。
				EnsureLineVisible(line, false);
				GoToLine(line);
			}
			pcs->SetExpanded(line, true);
			ExpandLine(line);
		}
		SetScrollBars();
		Redraw();
	}
}
*/
/*
void Editor::FoldAll(int action) {
	pdoc->EnsureStyledTo(pdoc->Length());
	const Sci::Line maxLine = pdoc->LinesTotal();
	bool expanding = action == SC_FOLDACTION_EXPAND;
	if (action == SC_FOLDACTION_TOGGLE) {
		// Discover current state
		for (int lineSeek = 0; lineSeek < maxLine; lineSeek++) {
			if (pdoc->GetLevel(lineSeek) & SC_FOLDLEVELHEADERFLAG) {
				expanding = !pcs->GetExpanded(lineSeek);
				break;
			}
		}
	}
	if (expanding) {
		pcs->SetVisible(0, maxLine - 1, true);
		for (int line = 0; line < maxLine; line++) {
			const int levelLine = pdoc->GetLevel(line);
			if (levelLine & SC_FOLDLEVELHEADERFLAG) {
				SetFoldExpanded(line, true);
			}
		}
	}
	else {
		for (Sci::Line line = 0; line < maxLine; line++) {
			const int level = pdoc->GetLevel(line);
			if ((level & SC_FOLDLEVELHEADERFLAG) &&
				(SC_FOLDLEVELBASE == LevelNumber(static_cast<FoldLevel>(level)))) {
				SetFoldExpanded(line, false);
				const Sci::Line lineMaxSubord = pdoc->GetLastChild(line, (FoldLevel)0);
				if (lineMaxSubord > line) {
					pcs->SetVisible(line + 1, lineMaxSubord, false);
				}
			}
		}
	}
	SetScrollBars();
	Redraw();
}
*/
void Editor::SetPosFoldExpandVisible(Sci::Position pos) {
	if (pos<0 || pos > pdoc->Length() - 1) {
		return;
	}
	Sci::Position line = pdoc->LineFromPosition(static_cast<Sci::Position>(pos));
	do {
		FoldLineNotUpdate(line, FoldAction::Expand);
	} while ((line = pdoc->GetFoldParent(line)) >= 0);
	SetRedraw();
}


size_t Editor::SetSearchStringRanges(const char* str, Sci::Position start
	, Sci::Position end, int flags, int indic, int indicvalue) {
	if (SearchStringRanges.size() > 0) {
		SetsSearchStringsClear();
	}
	if (!pdoc || !str) {
		return 0;
	}
	if (start < 0 || start >= pdoc->Length()) {
		return 0;
	}
	if (end < start || end >= pdoc->Length()) {
		end = pdoc->Length();
	}
	int length = strlen(str);
	if (length == 0 || pdoc->Length() <= 0) {
		return 0;
	}
	SelectSearchRange.start = start;
	SelectSearchRange.end = end;
	CurSearchString = str;
	searchFlags = static_cast<Scintilla::FindOption>(flags);

	if (!pdoc->HasCaseFolder())
		pdoc->SetCaseFolder(CaseFolderForEncoding());

	Sci::Position searchStart = start;
	const Sci::Position searchEnd = end;
	for (;;) {
		Sci::Position lengthFound = length;
		try {
			const Sci::Position pos = pdoc->FindText(searchStart, searchEnd,
				str, searchFlags, &lengthFound);
			if (pos >= 0 && lengthFound > 0) {
				SearchStringRanges.push_back(Range(pos, pos + lengthFound));
				searchStart = pos + lengthFound;
			}
			else {
				break;
			}
		}
		catch (RegexError&) {
			SearchStringRanges.clear();
			SearchStringLight = -1;
			SearchStringLightValue = 0;
			CurSearchString = "";

			errorStatus = Scintilla::Status::RegEx;

			return 0;
		}
	}
	if (indic >= 0) {
		SearchStringLight = indic;
		SearchStringLightValue = indicvalue;
		SetSearchStringLight(false);
	}
	return SearchStringRanges.size();
}
size_t Editor::UpdateSearchStringRanges(Sci::Position start, Sci::Position end) {
	if (CurSearchString.length() == 0) {
		return 0;
	}
	size_t size = SearchStringRanges.size();
	Sci::Position searchStart = start;
	const Sci::Position searchEnd = end;
	for (;;) {
		Sci::Position lengthFound = CurSearchString.length();
		try {
			const Sci::Position pos = pdoc->FindText(searchStart, searchEnd,
				CurSearchString.data(), searchFlags, &lengthFound);
			if (pos >= 0 && lengthFound > 0) {
				SearchStringRanges.push_back(Range(pos, pos + lengthFound));
				searchStart = pos + lengthFound;
			}
			else {
				break;
			}
		}
		catch (RegexError&) {
			errorStatus = Scintilla::Status::RegEx;
			return 0;
		}
	}
	size_t count = SearchStringRanges.size();
	if (SearchStringLight >= 0) {
		int PrevCurrentIndicator = pdoc->decorations->GetCurrentIndicator();
		int PrevCurrentValue = pdoc->decorations->GetCurrentValue();
		int CurrentValue = SearchStringLightValue;
		pdoc->DecorationSetCurrentIndicator(static_cast<int>(SearchStringLight));
		pdoc->decorations->SetCurrentValue(static_cast<int>(CurrentValue));

		for (size_t i = size; i < count; i++) {
			pdoc->DecorationFillRange(static_cast<Sci::Position>(SearchStringRanges.at(i).start),
				CurrentValue, SearchStringRanges.at(i).end - SearchStringRanges.at(i).start);
		}
		pdoc->DecorationSetCurrentIndicator(static_cast<int>(PrevCurrentIndicator));
		pdoc->decorations->SetCurrentValue(static_cast<int>(PrevCurrentValue));
	}
	return (count < size) ? 0 : count - size;
}
int Editor::SelectSearchString(size_t i) {
	if (i < SearchStringRanges.size()) {
		Sci::Position nStart = static_cast<Sci::Position>(SearchStringRanges[i].start);
		Sci::Position nEnd = SearchStringRanges[i].end;
		if (nEnd < 0)
			nEnd = pdoc->Length();
		if (nStart < 0)
			nStart = nEnd; 	// Remove selection
		InvalidateSelection(SelectionRange(nStart, nEnd));
		sel.Clear();
		sel.selType = Selection::SelTypes::stream;
		SetSelection(nEnd, nStart);
		Sci::Position anchor = sel.IsRectangular() ? sel.Rectangular().anchor.Position() : sel.MainAnchor();
		SetPosFoldExpandVisible(anchor);
		EnsureCaretVisible();
		return nStart;
	}
	else {
		if (sel.LimitsForRectangularElseMain().start.Position() < sel.LimitsForRectangularElseMain().end.Position()) {
			sel.Clear();
			ContainerNeedsUpdate(Update::Selection);
			Redraw();
		}
	}
	return -1;
}
//更改位置删除
//std::swap(CurMatchStringRanges.at(i), CurMatchStringRanges.back());
//CurMatchStringRanges.pop_back();
int Editor::ReplaceSearchString(size_t i, const char* cstr, bool select) {
	std::string newstr("");
	if (cstr) {
		newstr.assign(cstr);
	}
	if (i < SearchStringRanges.size()) {
		int start = SearchStringRanges[i].start;
		int end = SearchStringRanges[i].end;
		targetRange.start.SetPosition(static_cast<Sci::Position>(start));
		targetRange.end.SetPosition(end);
		//删除
		{
			int PrevCurrentIndicator = pdoc->decorations->GetCurrentIndicator();
			int PrevCurrentValue = pdoc->decorations->GetCurrentValue();
			pdoc->DecorationSetCurrentIndicator(static_cast<int>(SearchStringLight));
			pdoc->decorations->SetCurrentValue(static_cast<int>(0));
			for (; SearchStringRanges.size() > i;) {
				pdoc->DecorationFillRange(static_cast<Sci::Position>(SearchStringRanges.back().start),
					0, SearchStringRanges.back().end - SearchStringRanges.back().start);
				SearchStringRanges.pop_back();
			}
			pdoc->DecorationSetCurrentIndicator(static_cast<int>(PrevCurrentIndicator));
			pdoc->decorations->SetCurrentValue(static_cast<int>(PrevCurrentValue));
		}

		//替换后的高亮不自动更新,不然追加到后面,下次搜索或替换时又会被搜索到。
		int flag = ((int)searchFlags & (int)FindOption::RegExp || (int)searchFlags & (int)FindOption::Cxx11RegEx);
		int len = ReplaceTarget(flag,
			newstr.data(), static_cast<Sci::Position>(newstr.length()));
		if (select) {
			Sci::Position nStart = static_cast<Sci::Position>(start);
			Sci::Position nEnd = nStart + len;
			if (nEnd < 0)
				nEnd = pdoc->Length();
			if (nStart < 0)
				nStart = nEnd; 	// Remove selection
			InvalidateSelection(SelectionRange(nStart, nEnd));
			sel.Clear();
			sel.selType = Selection::SelTypes::stream;
			SetSelection(nEnd, nStart);
			Sci::Position anchor = sel.IsRectangular() ? sel.Rectangular().anchor.Position() : sel.MainAnchor();
			SetPosFoldExpandVisible(anchor);
			EnsureCaretVisible();
		}
		UpdateSearchStringRanges(start + len, SelectSearchRange.end);
		return start + len;
	}
	else if (select) {
		if (sel.LimitsForRectangularElseMain().start.Position() < sel.LimitsForRectangularElseMain().end.Position()) {
			sel.Clear();
			ContainerNeedsUpdate(Update::Selection);
			Redraw();
		}
	}
	return 0;
}
size_t Editor::ReplaceAllSearchString(const char* cstr, bool select) {
	std::string newstr("");
	if (cstr) {
		newstr.assign(cstr);
	}
	SetSearchStringLight(true);
	size_t nCount = 0;
	Sci::Position FirstPos = (SearchStringRanges.size()) ? SearchStringRanges.back().start : 0;
	Sci::Position FirstLen = 0;
	Sci::Position nStart = 0;
	Sci::Position nEnd = 0;
	int len = 0;
	for (; SearchStringRanges.size();) {
		nStart = SearchStringRanges.back().start;
		nEnd = SearchStringRanges.back().end;
		targetRange.start.SetPosition(static_cast<Sci::Position>(nStart));
		targetRange.end.SetPosition(nEnd);
		int flag = ((int)searchFlags & (int)FindOption::RegExp || (int)searchFlags & (int)FindOption::Cxx11RegEx);
		len = ReplaceTarget(flag, newstr.data(), static_cast<Sci::Position>(newstr.length()));

		(nCount) ? (FirstPos += (len - (nEnd - nStart))) : FirstLen = len;
		nCount++;
		SearchStringRanges.pop_back();
	}
	if (nCount && select) {
		nStart = FirstPos;
		nEnd = nStart + FirstLen;
		if (nEnd < 0)
			nEnd = pdoc->Length();
		if (nStart < 0)
			nStart = nEnd; 	// Remove selection
		InvalidateSelection(SelectionRange(nStart, nEnd));
		sel.Clear();
		sel.selType = Selection::SelTypes::stream;
		SetSelection(nEnd, nStart);
		Sci::Position anchor = sel.IsRectangular() ? sel.Rectangular().anchor.Position() : sel.MainAnchor();
		SetPosFoldExpandVisible(anchor);
		EnsureCaretVisible();
	}
	else if (select) {
		if (sel.LimitsForRectangularElseMain().start.Position() < sel.LimitsForRectangularElseMain().end.Position()) {
			sel.Clear();
			ContainerNeedsUpdate(Update::Selection);
			Redraw();
		}
	}
	SearchStringRanges.clear();
	SearchStringLight = -1;
	SearchStringLightValue = 0;
	CurSearchString = "";
	return nCount;
}
void Editor::SetsSearchStringsClear() {
	SetSearchStringLight(true);
	SearchStringRanges.clear();
	SearchStringLight = -1;
	SearchStringLightValue = 0;
	CurSearchString = "";
}
int Editor::GetSearchStringPosStart(size_t i) {
	if (i < SearchStringRanges.size()) {
		return SearchStringRanges.at(i).start;
	}
	return -1;
}
size_t Editor::GetSearchStringsCount() {
	return SearchStringRanges.size();
}
void Editor::EnsureStyledAll() {
	if (!pdoc) {
		return;
	}
	pdoc->EnsureStyledTo(pdoc->Length());
}

//=================inc-new-end=================
