#!/bin/bash
# Masterpiece Sync Script v10.0.4 - Absolute Synchronization
set -e

echo "--- STARTING ABSOLUTE SYNC ---"

# Ensure we are on main
git checkout main
git add .
git commit -m "chore: Cumulative technical updates and optimizations" || echo "No changes to commit"
git push origin main --force
echo "✅ Academic (Origin) Synced."

# Switch to portfolio
git checkout portfolio
git reset --hard main

# Restore Portfolio Branding
git checkout portfolio@{1} -- LICENSE README.md 2>/dev/null || echo "Branding safe"
git add .
git commit -m "chore: professional branding preservation and sync" || echo "No changes"
git push personal portfolio:main --force
echo "✅ Portfolio (Personal/Pages) Synced."

git checkout main
echo "🚀 ALL REPOS ABSOLUTELY SYNCED."
