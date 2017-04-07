rem copy command usage 
copy %cd%\files\Update.exe %cd%\files\Update.exe.txt
copy %cd%\files\config.xml %cd%\files\config.xml.txt
copy %cd%\files\srv.bat %cd%\files\srv.bat.txt
copy %cd%\files\srvuninstall.bat %cd%\files\srvuninstall.bat.txt

rem variable assignment
set date0=%date:~0,10%
set time0=%time:~1,8%
set dttm=%date0:/=%%time0::=%

rem external command execute
makensis /V1 /DTIMESTAMP=%dttm:.=% setup.nsi