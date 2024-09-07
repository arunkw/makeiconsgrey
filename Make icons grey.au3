#cs ----------------------------------------------------------------------------

 AutoIt Version: 3.3.16.1
 Author:         myName

 Script Function:
	Template AutoIt script.

#ce ----------------------------------------------------------------------------

; Script Start - Add your code below here
#cs ----------------------------------------------------------------------------

 AutoIt Version: 3.3.16.1
 Author:         myName

 Script Function:
	Template AutoIt script.

#ce ----------------------------------------------------------------------------

; Declare and initialize the $apps array
Global $apps = ["vlc.exe"] ; Add the list of application executables you want to monitor

; Script Start - Add your code below here
Func ChangeIcon($appName, $iconPath)
    ; Assuming you have the original and monochrome icons in specific folders
    ; Local $originalIcon = "C:\Icons\" & $appName & "_color.ico"
    Local $originalIcon = "C:\Users\ADMIN\Desktop\Icons\vlc16x16.png"
    ; Local $monochromeIcon = "C:\Icons\" & $appName & "_bw.ico"
    Local $monochromeIcon = "C:\Users\ADMIN\Desktop\Icons\vlc16x16bw.png"

    If $iconPath = "original" Then
        ; Code to set the original icon
        RegWrite("HKEY_CURRENT_USER\Software\Classes\Applications\" & $appName, "Icon", "REG_SZ", $originalIcon)
    Else
        ; Code to set the monochrome icon
        RegWrite("HKEY_CURRENT_USER\Software\Classes\Applications\" & $appName, "Icon", "REG_SZ", $monochromeIcon)
    EndIf
EndFunc

While 1
    For $i = 0 To UBound($apps) - 1
        If ProcessExists($apps[$i]) Then
            ChangeIcon($apps[$i], "original")
        Else
            ChangeIcon($apps[$i], "monochrome")
        EndIf
    Next
    Sleep(5000)
WEnd
