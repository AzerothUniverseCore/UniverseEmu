$DataRoot = "C:\Servers\GameServers\UniverseEmu\World\data\cdn\Data"

$Source = "http://localhost:1119/"

$BuildNum = 12340 

$csharpCrc32 = @"
using System;
using System.IO;

public static class Crc32Util
{
    private static readonly uint[] Table = BuildTable();

    private static uint[] BuildTable()
    {
        uint[] table = new uint[256];
        for (uint i = 0; i < 256; i++)
        {
            uint c = i;
            for (int k = 0; k < 8; k++)
                c = ((c & 1) != 0) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
            table[i] = c;
        }
        return table;
    }

    public static uint Compute(string path)
    {
        uint crc = 0xFFFFFFFF;
        byte[] buffer = new byte[4 * 1024 * 1024];
        using (FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read, buffer.Length))
        {
            int read;
            while ((read = fs.Read(buffer, 0, buffer.Length)) > 0)
            {
                for (int i = 0; i < read; i++)
                    crc = Table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
            }
        }
        return crc ^ 0xFFFFFFFF;
    }
}
"@
Add-Type -TypeDefinition $csharpCrc32 -Language CSharp

function Get-Crc32([string]$Path)
{
    return [Crc32Util]::Compute($Path)
}

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("version=1")
$lines.Add("isTrial=0")
$lines.Add("source=$Source")
$lines.Add("Data;0;$BuildNum;0")
$lines.Add("Data/enUS;0;$BuildNum;0")
$lines.Add("Data/frFR;0;$BuildNum;0")

function Add-FolderFiles([string]$RelPrefix, [string]$FullPath)
{
    if (-not (Test-Path $FullPath))
    {
        Write-Warning "Dossier introuvable, ignore : $FullPath"
        return
    }

    Get-ChildItem -Path $FullPath -File | Where-Object {
        $_.Extension -ieq ".MPQ" -or $_.Name -ieq "realmlist.wtf"
    } | Sort-Object Name | ForEach-Object {
        $sw = [System.Diagnostics.Stopwatch]::StartNew()
        $crc = Get-Crc32 $_.FullName
        $sw.Stop()
        Write-Host ("{0}/{1,-28} CRC32={2,-12} ({3:N1} Mo, {4:N1}s)" -f $RelPrefix, $_.Name, $crc, ($_.Length / 1MB), $sw.Elapsed.TotalSeconds)
        $lines.Add("$RelPrefix/$($_.Name);$crc;$BuildNum;0")
    }
}

Write-Host "Calcul des CRC32 en cours (peut prendre plusieurs minutes selon la taille des patchs)..."
Write-Host ""

Add-FolderFiles "Data" $DataRoot
Add-FolderFiles "Data/enUS" (Join-Path $DataRoot "enUS")
Add-FolderFiles "Data/frFR" (Join-Path $DataRoot "frFR")

$OutFile = Join-Path (Split-Path $DataRoot -Parent) "WoW.mfil"
$lines | Set-Content -Path $OutFile -Encoding ASCII

Write-Host ""
Write-Host "Termine ! WoW.mfil genere ici : $OutFile"
Write-Host "Verifie la ligne 'source=' avant de le distribuer a tes joueurs."
