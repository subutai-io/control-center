Dim oShell
Set oShell = WScript.CreateObject ("WScript.Shell")
oShell.Run "taskkill /F /fi ""IMAGENAME eq SubutaiControlCenter.exe""", , True