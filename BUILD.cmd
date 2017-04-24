@echo off

title Packing Assets
echo ^>^>^>^>^>^> Packing Assets ^<^<^<^<^<^<
pushd Workspace
call AssetManager.exe.lnk --update --pack
if errorlevel 1 (
   title Assets Failure
   echo ^>^>^>^>^>^> Assets Failure ^<^<^<^<^<^<
   popd
   pause
   goto :eof
)
popd

call :BuildConfiguration Debug || goto :eof
call :BuildConfiguration Release || goto :eof

title Done!
echo Done!
goto :eof

:BuildConfiguration
title Building %1
echo ^>^>^>^>^>^> Building %1 ^<^<^<^<^<^<
"C:\Program Files (x86)\MSBuild\14.0\Bin\MSbuild.exe" /t:Build /p:Configuration=%1 /property:Platform=Win32 "Jewel3D_Samples.sln"
if errorlevel 1 (
   title Build Failure %1
   echo ^>^>^>^>^>^> Build Failure %1 ^<^<^<^<^<^<
   pause
)
goto :eof
