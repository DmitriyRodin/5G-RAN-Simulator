#!/bin/bash

echo "Step 1: Checking code style"
find . -regex '.*\.\(cpp\|hpp\|h\)' -not -path "*/build*/*" | xargs clang-format --dry-run --Werror

if [ $? -ne 0 ]; then
    echo "Oops: Style check FAILED! Please run ./format_commit.sh before pushing."
    exit 1
fi

echo "Step 2: Building project"
cmake -B build_test -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build_test --parallel $(nproc)

if [ $? -ne 0 ]; then
    echo "Oops: Build FAILED!"
    exit 1
fi

echo "Step 3: Running tests"
if [ -f "build_test/ue/tests/ue_tests" ]; then
    cd build_test/ue/tests && ./ue_tests
else
    echo "Oops: received an error: Test binary 'ue_tests' not found in build_test/ue/tests/"
    exit 1
fi

echo "Cheking is finish. Fin! Go and push it!"
