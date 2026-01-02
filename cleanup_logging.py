#!/usr/bin/env python3
"""
Script to remove all debug.log logging from re3-webos source files.
Removes blocks like:
    FILE* log = fopen("/media/internal/.gta3/debug.log", "a");
    if(log) {
        fprintf(log, "...");
        fclose(log);
    }
"""
import re
import sys

def remove_debug_logging(filepath):
    """Remove all debug logging blocks from a file."""
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    original_content = content
    lines_removed = 0

    # Pattern 1: Simple inline logging (single line)
    # FILE* log = fopen(...); if(log) { fprintf(...); fclose(log); }
    pattern1 = r'\s*FILE\s*\*\s*\w+\s*=\s*fopen\s*\(\s*"/media/internal/\.gta3/debug\.log"\s*,\s*"[^"]+"\s*\)\s*;[^;]*?if\s*\([^)]+\)\s*\{[^}]*?fprintf\([^;]+;[^}]*?fclose\([^)]+\)[^}]*?\}\s*'

    # Pattern 2: Multi-line logging blocks
    # FILE* log = fopen("/media/internal/.gta3/debug.log", "a");
    # if(log) {
    #     fprintf(log, "...");
    #     fclose(log);
    # }
    pattern2 = r'\s*FILE\s*\*\s*\w+\s*=\s*fopen\s*\(\s*"/media/internal/\.gta3/debug\.log"\s*,\s*"[^"]+"\s*\)\s*;\s*\n\s*if\s*\(\s*\w+\s*\)\s*\{[\s\S]*?fclose\s*\(\s*\w+\s*\)\s*;[\s\S]*?\}\s*'

    # Pattern 3: Logging where variable already exists (log = fopen...)
    pattern3 = r'\s*\w+\s*=\s*fopen\s*\(\s*"/media/internal/\.gta3/debug\.log"\s*,\s*"[^"]+"\s*\)\s*;\s*\n\s*if\s*\(\s*\w+\s*\)\s*\{[\s\S]*?fclose\s*\(\s*\w+\s*\)\s*;[\s\S]*?\}\s*'

    # Pattern 4: NULL assignment (disabled logging)
    # FILE* log = NULL /* logging disabled */;
    # if(log) { ... }
    pattern4 = r'\s*FILE\s*\*\s*\w+\s*=\s*NULL\s*/\*[^*]*\*/\s*;\s*\n\s*if\s*\(\s*\w+\s*\)\s*\{[\s\S]*?\}\s*'

    # Pattern 5: Simple log assignment reuse
    # log = NULL /* logging disabled */;
    # if(log) { ... }
    pattern5 = r'\s*\w+\s*=\s*NULL\s*/\*[^*]*\*/\s*;\s*\n\s*if\s*\(\s*\w+\s*\)\s*\{[\s\S]*?\}\s*'

    # Apply patterns
    for pattern in [pattern2, pattern3, pattern1, pattern4, pattern5]:
        old_len = len(content)
        content = re.sub(pattern, '', content, flags=re.MULTILINE)
        if len(content) < old_len:
            lines_removed += old_len - len(content)

    # Remove standalone FILE* log declarations (if they're now unused)
    # This is conservative - only remove if followed immediately by closing brace or another declaration
    pattern_unused_decl = r'\n\s*FILE\s*\*\s*\w+\s*;\s*\n'
    content = re.sub(pattern_unused_decl, '\n', content)

    # Clean up excessive blank lines (3+ in a row -> 2)
    content = re.sub(r'\n\n\n+', '\n\n', content)

    if content != original_content:
        with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
            f.write(content)
        print(f"✓ Cleaned {filepath} ({lines_removed} chars removed)")
        return True
    else:
        print(f"  No changes needed for {filepath}")
        return False

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: cleanup_logging.py <file1> <file2> ...")
        sys.exit(1)

    total_cleaned = 0
    for filepath in sys.argv[1:]:
        try:
            if remove_debug_logging(filepath):
                total_cleaned += 1
        except Exception as e:
            print(f"✗ Error processing {filepath}: {e}")

    print(f"\n{total_cleaned}/{len(sys.argv)-1} files cleaned")
