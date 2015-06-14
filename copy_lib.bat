:: %1 = path to compiled files (eg: \src\game\client\Debug_ges)
:: %2 = project (server or client)
:: %3 = lib folder

echo ------------------------------------------------
echo Compile Path is %1
echo Current Project is %2
echo Lib folder is %3
echo ------------------------------------------------

if exist "%1\%2.lib" copy "%1\%2.lib" "%3\%2.lib"
if exist "%1\%2.pdb" copy "%1\%2.pdb" "%3\%2.pdb"