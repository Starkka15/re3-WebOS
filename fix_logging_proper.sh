#!/bin/bash
# Proper fix for FD leaks
# Pattern: fflush(log); with NO fclose on same or next line -> add fclose

cd "$(dirname "$0")/src"

# Find all .cpp files and fix the pattern where fflush is NOT followed by fclose
find . -name "*.cpp" -type f | while read file; do
    # Use awk to add fclose after fflush when it's missing
    awk '
    /fflush\(log\);/ {
        print $0
        # Read next line
        getline nextline
        # Check if next line contains fclose
        if (nextline !~ /fclose\(log\)/) {
            # Insert fclose before the next line
            print "\t\t\tfclose(log);"
        }
        print nextline
        next
    }
    { print }
    ' "$file" > "$file.tmp" && mv "$file.tmp" "$file"
done

echo "Fix applied!"
