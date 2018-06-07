Dim fileSystemObject, objectShell, objectSystemEnvironment, folderToAdd, pathToAdd, variablePath

Set fileSystemObject = CreateObject("Scripting.FileSystemObject")

Set objectShell = CreateObject("Wscript.Shell")

Set objectSystemEnvironment = objectShell.Environment("SYSTEM")

Set folderToAdd = fileSystemObject.GetFolder("C:\Program Files (x86)\ssh")

pathToAdd = folderToAdd.path

variablePath = objectShell.ExpandEnvironmentStrings("%Path%")

If InStr(variablePath, pathToAdd) = 0 Then
    objectSystemEnvironment("Path") = variablePath & ";" & pathToAdd

    'Wscript.Echo "Added to PATH"
Else
    'Wscript.Echo "Already in PATH"
End If
