#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Find and remove all .git directories starting from the script's location
find "$SCRIPT_DIR" -type d -name '.git*' -exec rm -rf {} +

echo "All .git directories have been removed from $SCRIPT_DIR and its subdirectories."
