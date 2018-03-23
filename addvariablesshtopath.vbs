Dim FSO, OFProgramFiles, fullpath, objshell, objSysEnv

Set FSO = CreateObject("Scripting.FileSystemObject")

Set objshell= CreateObject("Wscript.Shell")

Set OFProgramFiles= FSO. GetFolder("C:\Program Files (x86)\ssh")

fullpath= OFProgramFiles.path
Set objSysEnv= objshell.Environment("SYSTEM")
objSysEnv("Path")=objSysEnv("PATH") & ";" & fullpath