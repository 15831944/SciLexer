

//=================inc-new-flag=================

bool ScintillaBase::AutoCompleteActive() {
	return ac.Active();
}
void ScintillaBase::AutoCompleteShow(Sci::Position lenEntered, const char* list) {
	listType = 0;
	AutoCompleteStart(lenEntered, list);
}
void ScintillaBase::AutoCompleteUpdate() {
	if (!ac.Active()) {
		return;
	}
	Sci::Position lenEntered = ac.startLen;

	/*
	//Platform::DebugPrintf("AutoComplete %s\n", list);
	ct.CallTipCancel();

	if (ac.chooseSingle && (listType == 0)) {
		if (list && !strchr(list, ac.GetSeparator())) {
			const char *typeSep = strchr(list, ac.GetTypesep());
			const Sci::Position lenInsert = typeSep ?
				(typeSep-list) : strlen(list);
			if (ac.ignoreCase) {
				// May need to convert the case before invocation, so remove lenEntered characters
				AutoCompleteInsert(sel.MainCaret() - lenEntered, lenEntered, list, lenInsert);
			} else {
				AutoCompleteInsert(sel.MainCaret(), 0, list + lenEntered, lenInsert - lenEntered);
			}
			ac.Cancel();
			return;
		}
	}

	ListOptions options{
		vs.ElementColour(Element::List),
		vs.ElementColour(Element::ListBack),
		vs.ElementColour(Element::ListSelected),
		vs.ElementColour(Element::ListSelectedBack),
		ac.options,
	};

	ac.Start(wMain, idAutoComplete, sel.MainCaret(), PointMainCaret(),
				lenEntered, vs.lineHeight, IsUnicodeMode(), technology, options);
	*/
	const PRectangle rcClient = GetClientRectangle();
	Point pt = LocationFromPosition(sel.MainCaret() - lenEntered);
	PRectangle rcPopupBounds = wMain.GetMonitorRect(pt);
	if (rcPopupBounds.Height() == 0)
		rcPopupBounds = rcClient;

	int heightLB = ac.heightLBDefault;
	int widthLB = ac.widthLBDefault;
	if (pt.x >= rcClient.right - widthLB) {
		HorizontalScrollTo(static_cast<int>(xOffset + pt.x - rcClient.right + widthLB));
		Redraw();
		pt = PointMainCaret();
	}
	if (wMargin.Created()) {
		pt = pt + GetVisibleOriginInMain();
	}
	/*
	PRectangle rcac;
	rcac.left = pt.x - ac.lb->CaretFromEdge();
	if (pt.y >= rcPopupBounds.bottom - heightLB &&  // Won't fit below.
			pt.y >= (rcPopupBounds.bottom + rcPopupBounds.top) / 2) { // and there is more room above.
		rcac.top = pt.y - heightLB;
		if (rcac.top < rcPopupBounds.top) {
			heightLB -= static_cast<int>(rcPopupBounds.top - rcac.top);
			rcac.top = rcPopupBounds.top;
		}
	} else {
		rcac.top = pt.y + vs.lineHeight;
	}
	rcac.right = rcac.left + widthLB;
	rcac.bottom = static_cast<XYPOSITION>(std::min(static_cast<int>(rcac.top) + heightLB, static_cast<int>(rcPopupBounds.bottom)));
	ac.lb->SetPositionRelative(rcac, &wMain);
	ac.lb->SetFont(vs.styles[StyleDefault].font.get());
	*/
	const int aveCharWidth = static_cast<int>(vs.styles[StyleDefault].aveCharWidth);

	//ac.lb->SetAverageCharWidth(aveCharWidth);
	//ac.lb->SetDelegate(this);

	//ac.SetList(list ? list : "");

	// Fiddle the position of the list so it is right next to the target and wide enough for all its strings
	PRectangle rcList = ac.lb->GetDesiredRect();
	const int heightAlloced = static_cast<int>(rcList.bottom - rcList.top);
	widthLB = std::max(widthLB, static_cast<int>(rcList.right - rcList.left));
	if (maxListWidth != 0)
		widthLB = std::min(widthLB, aveCharWidth * maxListWidth);
	// Make an allowance for large strings in list
	rcList.left = pt.x - ac.lb->CaretFromEdge();
	rcList.right = rcList.left + widthLB;
	if (((pt.y + vs.lineHeight) >= (rcPopupBounds.bottom - heightAlloced)) &&  // Won't fit below.
		((pt.y + vs.lineHeight / 2) >= (rcPopupBounds.bottom + rcPopupBounds.top) / 2)) { // and there is more room above.
		rcList.top = pt.y - heightAlloced;
	}
	else {
		rcList.top = pt.y + vs.lineHeight;
	}
	rcList.bottom = rcList.top + heightAlloced;
	ac.lb->SetPositionRelative(rcList, &wMain);
	ac.Show(true);
	//if (lenEntered != 0) {
	//	AutoCompleteMoveToCurrentWord();
	//}

}
void ScintillaBase::MsgCallTipShow(Sci::Position pos, const char* defn) {
	//=================inc-new-flag=================
	ct.posWordStart=pos;
	//=================inc-new-end=================
	CallTipShow(LocationFromPosition(pos),
		defn);

}
void ScintillaBase::MsgCallTipUpdate() {
	if (!ct.inCallTipMode) {
		return;
	}
	Point pt = LocationFromPosition(ct.posWordStart);
	/*
	ac.Cancel();
	// If container knows about StyleCallTip then use it in place of the
	// StyleDefault for the face name, size and character set. Also use it
	// for the foreground and background colour.
	const int ctStyle = ct.UseStyleCallTip() ? StyleCallTip : StyleDefault;
	const Style& style = vs.styles[ctStyle];
	if (ct.UseStyleCallTip()) {
		ct.SetForeBack(style.fore, style.back);
	}
	if (wMargin.Created()) {
		pt = pt + GetVisibleOriginInMain();
	}
	AutoSurface surfaceMeasure(this);
	PRectangle rc = ct.CallTipStart(sel.MainCaret(), pt,
		vs.lineHeight,
		defn,
		CodePage(),
		surfaceMeasure,
		style.font);
	// If the call-tip window would be out of the client
	// space


	const PRectangle rcClient = GetClientRectangle();
	const int offset = vs.lineHeight + static_cast<int>(rc.Height());
	// adjust so it displays above the text.
	if (rc.bottom > rcClient.bottom && rc.Height() < rcClient.Height()) {
		rc.top -= offset;
		rc.bottom -= offset;
	}
	// adjust so it displays below the text.
	if (rc.top < rcClient.top && rc.Height() < rcClient.Height()) {
		rc.top += offset;
		rc.bottom += offset;
	}
	// Now display the window.
	CreateCallTipWindow(rc);
	ct.wCallTip.SetPositionRelative(rc, &wMain);
	ct.wCallTip.Show();
	*/
}
//=================inc-new-end=================
