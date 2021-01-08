#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
; #Warn  ; Enable warnings to assist with detecting common errors.
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.
#^!F1::
TrayTip, Zoom Controller, Mac Mode, 20, 17
return

#^!F2::
TrayTip, Zoom Controller, Windows Mode, 20, 17
return

#^!F3::
TrayTip, Zoom Controller, Controller has been idle for awhile and will soon turn itself off.`n`nPress any button or rotate knob to keep awake., 20, 17
return

#^!F4::
TrayTip, Zoom Controller, Zoom Mode, 20, 17
return

#^!F5::
TrayTip, Zoom Controller, YouTube Mode, 20, 17
return