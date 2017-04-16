# How to use this script
# launch powershell with admin
# powershell -ExecutionPolicy ByPass -File [script.ps1]
# or run the PowerShell script from cmd.exe like this:
# powershell -noexit "& ""C:\my_path\yada_yada\run_import_script.ps1""" (enter)
 
Function createNewFiles($directory)
{ 
  $path =  $directory
  Write-Output "parameter is:" + $path.toString();
  
  get-childitem $path.toString() -recurse | where {$_.psIsContainer -eq $true} | % {
      $newfile = $_.FullName + "/ReadMe.md"
      New-Item $newfile.toString() -ItemType file
  }
    
   
   
  
} 
# RUN SCRIPT 
createNewFiles "D:/MathxH/Adrasteia"
"SCRIPT FINISHED"