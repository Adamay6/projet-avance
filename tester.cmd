@echo off
setlocal
chcp 65001 >nul 2>&1
set "PYTHONUTF8=1"
set "PYTHONIOENCODING=utf-8"
cd /d "%~dp0"
set "OUTILS_PROJET="
if exist "%~dp0outils-locaux.txt" set /p "OUTILS_PROJET=" < "%~dp0outils-locaux.txt"
if defined OUTILS_PROJET set "PATH=%OUTILS_PROJET%;%PATH%"
if exist "%~dp0w64devkit\bin\gcc.exe" set "PATH=%~dp0w64devkit\bin;%PATH%"
py -3 -c "import sys; sys.exit(sys.version_info < (3, 9))" >nul 2>&1
if not errorlevel 1 goto python_py
python -c "import sys; sys.exit(sys.version_info < (3, 9))" >nul 2>&1
if not errorlevel 1 goto python_simple
python3 -c "import sys; sys.exit(sys.version_info < (3, 9))" >nul 2>&1
if not errorlevel 1 goto python_trois
where gcc >nul 2>&1
if errorlevel 1 goto outils_absents
where make >nul 2>&1
if errorlevel 1 goto outils_absents
where sh >nul 2>&1
if errorlevel 1 goto outils_absents
echo Python absent ; utilisation du Makefile avec les outils disponibles.
if "%~1"=="" (
    make test
) else (
    make %*
)
exit /b %errorlevel%

:outils_absents
echo Python 3.9 minimum est introuvable. Voir README.md pour l'installer.
exit /b 1

:python_py
py -3 "%~dp0tester.py" %*
exit /b %errorlevel%

:python_simple
python "%~dp0tester.py" %*
exit /b %errorlevel%

:python_trois
python3 "%~dp0tester.py" %*
exit /b %errorlevel%
