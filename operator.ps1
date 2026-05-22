# Run the executable
& operatorwizard.exe

$TempFile = "$env:TEMP\temp_cd_path.txt"

# Check if the file exists
if (Test-Path $TempFile) {
	# Read the path and trim any hidden spaces/newlines
	$NewDir = (Get-Content $TempFile -Raw).Trim()
	
	# Clean up the temp file
	Remove-Item $TempFile
	
	# Change the directory in PowerShell
	if ($NewDir) {
		Set-Location $NewDir
	}
}