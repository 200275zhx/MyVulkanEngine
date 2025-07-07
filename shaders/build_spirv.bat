@echo off
REM ------------------------------------------------------------
REM build_spirv.bat - compile all .vert and .frag in glsl -> spir-v
REM ------------------------------------------------------------

pushd "%~dp0"

if not exist "spir-v" mkdir "spir-v"

for %%f in ("glsl\*.vert" "glsl\*.frag") do (
    echo Compiling %%~nxf -> spir-v\%%~nxf.spv
REM enable this for release:
REM glslangValidator -V "%%f" -o "spir-v\%%~nxf.spv"
    
REM enable this for debug:
    glslangValidator -V -g -Od "%%f" -o "spir-v\%%~nxf.spv"
)

echo.
echo All shaders compiled!

popd
pause
