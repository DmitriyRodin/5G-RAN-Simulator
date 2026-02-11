 #!/bin/bash

files=$(git diff-tree --no-commit-id --name-only -r HEAD | grep -E '\.(cpp|h|hpp|cc)$')

if [ -z "$files" ]; then
    echo "Oops. No modified C++ files found in the current commit"
    exit 0
fi

echo "Start formatting modified files"
for file in $files; do
    if [ -f "$file" ]; then
        echo "Processing: $file"
        clang-format -i -style=file "$file"
    fi
done

echo "Done!"
