function OnMouse(Code:Integer; wParam:LongInt; lParam: LongInt): longint; stdcall;
var
  Handle: Integer;
begin
  Mouse := MOUSEHOOKSTRUCT(Pointer(lParam)^);
  MyWin := CommonArea^.HMain;
  if (Code < 0) or (Mouse.hwnd = MyWin) or (GetParent(Mouse.hwnd) = MyWin) or
     (GetClass(Mouse.hwnd) = 'TsTrackBar') or (Mouse.hwnd = CommonArea^.HOptions) or 
	 (GetClass(Mouse.hwnd) = 'Shell_TrayWnd') then
  begin
    Result := CallNextHookEx(CommonArea^.HHandleMouse, Code, wParam, lParam);
    Exit;
  end;
  Handle := Mouse.hwnd + Low(Integer);
  if CommonArea^.IsMainWindowVisible then
  begin
    if ((Mouse.wHitTestCode <> HTCAPTION)and (Mouse.wHitTestCode <> HTREDUCE) and
       (Mouse.wHitTestCode <> HTSYSMENU) and (Mouse.wHitTestCode <> HTZOOM) and 
	   (Mouse.wHitTestCode <> HTCLOSE)) or
	   ((Mouse.hwnd<>GetForegroundWindow) and (GetClass(Mouse.hwnd) <> 'NetUIHWND'))
       or (wParam = WM_NCLBUTTONDBLCLK) or (wParam = WM_LBUTTONUP) then
    begin
      PostMessage(MyWin, WM_HOOK, MOUSELEAVEDONCAPTION, Handle);
    end;
  end
  else
    if ((Mouse.wHitTestCode = HTCAPTION) or (Mouse.wHitTestCode = HTREDUCE)or
        (Mouse.wHitTestCode = HTSYSMENU) or (Mouse.wHitTestCode = HTZOOM)  or
        (Mouse.wHitTestCode = HTCLOSE)) and
        ((Mouse.hwnd = GetForegroundWindow) or (GetClass(Mouse.hwnd) = 'NetUIHWND')) then
    begin
      PostMessage(MyWin, WM_HOOK, MOUSEMOVEDONCAPTION, Handle);
    end;

  Result := CallNextHookEx(CommonArea^.HHandleMouse, Code, wParam, lParam);
end; 
