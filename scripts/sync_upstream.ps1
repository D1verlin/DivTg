<#
.SYNOPSIS
    DivTG Upstream Sync Tool
    Synchronizes your DivTG mod with official Telegram Desktop releases.

.DESCRIPTION
    Fetches official upstream tags, creates a dedicated sync branch,
    and cleanly rebases our isolated DivTG changes on top of the new tag.

.PARAMETER UpstreamTag
    The official tdesktop tag to sync with (e.g. "v7.3.3", "v7.4.0").
    If omitted, lists the latest available official tags.
#>

[CmdletBinding()]
param(
    [string]$UpstreamTag
)

$ErrorActionPreference = "Stop"

Write-Host "=== DivTG Upstream Synchronizer ===" -ForegroundColor Cyan

# 1. Verify remote 'upstream' exists
$remotes = git remote
if ($remotes -notcontains "upstream") {
    Write-Host "[!] Upstream remote not found. Adding https://github.com/telegramdesktop/tdesktop.git..." -ForegroundColor Yellow
    git remote add upstream https://github.com/telegramdesktop/tdesktop.git
    git config remote.upstream.fetch "+refs/heads/*:refs/remotes/upstream/*"
    git config --add remote.upstream.fetch "+refs/tags/*:refs/tags/*"
}

# 2. Fetch official tags
Write-Host "[+] Fetching official upstream tags..." -ForegroundColor Green
git fetch upstream --tags --quiet

if (-not $UpstreamTag) {
    Write-Host "`nLatest 10 official Telegram Desktop tags:" -ForegroundColor Yellow
    git tag -l "v*" | Sort-Object -Descending | Select-Object -First 10 | ForEach-Object {
        Write-Host "   $_" -ForegroundColor White
    }
    Write-Host "`nUsage: .\scripts\sync_upstream.ps1 -UpstreamTag <tag_name>" -ForegroundColor Cyan
    Write-Host "Example: .\scripts\sync_upstream.ps1 -UpstreamTag v7.4.0`n" -ForegroundColor Cyan
    return
}

# 3. Check if target tag exists
$tagExists = git rev-parse --verify --quiet "refs/tags/$UpstreamTag"
if (-not $tagExists) {
    Write-Error "Tag '$UpstreamTag' does not exist in upstream repository."
}

Write-Host "[+] Target tag verified: $UpstreamTag" -ForegroundColor Green

# 4. Check for unstaged changes
$status = git status --porcelain
if ($status) {
    Write-Error "Working directory has uncommitted changes. Please commit or stash them first."
}

# 5. Create backup branch and perform rebase
$currentBranch = (git branch --show-current).Trim()
$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$backupBranch = "backup-$currentBranch-$timestamp"

Write-Host "[+] Creating safety backup branch: $backupBranch" -ForegroundColor Yellow
git branch $backupBranch

Write-Host "[+] Rebasing $currentBranch onto $UpstreamTag..." -ForegroundColor Green
try {
    git rebase $UpstreamTag
    Write-Host "`n[SUCCESS] Successfully rebased DivTG onto $UpstreamTag!" -ForegroundColor Green
    Write-Host "Safety backup preserved in: $backupBranch" -ForegroundColor DarkGray
} catch {
    Write-Host "`n[!] Conflict encountered during rebase." -ForegroundColor Red
    Write-Host "Resolve any conflicts with 'git status' and 'git rebase --continue'" -ForegroundColor Yellow
    Write-Host "Or abort with 'git rebase --abort'" -ForegroundColor Yellow
}
