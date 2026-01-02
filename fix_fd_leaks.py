#!/usr/bin/env python3
"""
Fix file descriptor leaks in webOS logging code.
Adds fclose(log); after fflush(log); where missing.
"""

import re
import os
import glob

def fix_file(filepath):
    """Fix FD leaks in a single file"""
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    original = content

    # Pattern: fflush(log); followed by newline/whitespace/comment but NOT fclose
    # We need to add fclose(log); after these
    pattern = r'(fflush\(log\);)(\s*)((?!fclose\(log\)))'

    def replacement(match):
        fflush_stmt = match.group(1)  # "fflush(log);"
        whitespace = match.group(2)   # whitespace after
        next_content = match.group(3) # what comes after

        # Check if the next line already has fclose or a closing brace
        # Skip if we're at end of if block
        if 'fclose' in next_content[:50]:
            return match.group(0)  # Already has fclose, don't modify

        if '}' in next_content[:10]:
            # Closing brace right after, insert before it
            return f"{fflush_stmt} fclose(log);{whitespace}{next_content}"

        # Insert fclose after the whitespace
        return f"{fflush_stmt} fclose(log);{whitespace}{next_content}"

    content = re.sub(pattern, replacement, content)

    # If we made changes, write back
    if content != original:
        with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
            f.write(content)
        return True
    return False

def main():
    # Find all .cpp and .h files in src/
    src_dir = os.path.join(os.path.dirname(__file__), 'src')

    files_to_check = []
    for root, dirs, files in os.walk(src_dir):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                files_to_check.append(os.path.join(root, file))

    fixed_count = 0
    for filepath in files_to_check:
        if fix_file(filepath):
            print(f"Fixed: {filepath}")
            fixed_count += 1

    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == '__main__':
    main()
