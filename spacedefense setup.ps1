# Define the repository URL and the directory name
$repoUrl = "https://github.com/ravegoth/spacedefense/archive/refs/heads/master.zip"
$zipFile = "spacedefense.zip"
$extractPath = "spacedefense-master"

# Download the repository zip file
Invoke-WebRequest -Uri $repoUrl -OutFile $zipFile

# Extract the zip file
Expand-Archive -Path $zipFile -DestinationPath . -Force

# Remove the zip file
Remove-Item $zipFile

# Change to the extracted directory
Set-Location $extractPath

# Run the batch file
Start-Process "run-game.bat"
