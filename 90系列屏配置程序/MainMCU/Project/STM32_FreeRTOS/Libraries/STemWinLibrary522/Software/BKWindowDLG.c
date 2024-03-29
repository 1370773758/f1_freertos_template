
#define ID_WINDOW_0     (GUI_ID_USER + 0x80)


static const GUI_WIDGET_CREATE_INFO _aBKWINDialogCreate[] = {
  { WINDOW_CreateIndirect, "BKWindow", ID_WINDOW_0, 0, 0, 320, 240, 0, 0x0, 0 },
};

static void _cbBKWINDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:

    hItem = pMsg->hWin;
    WINDOW_SetBkColor(hItem, 0x00000000);

    break;

  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

WM_HWIN CreateBKWindow(void) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_aBKWINDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbBKWINDialog, WM_HBKWIN, 0, 0);
  return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
