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

echo "Step 3: Running all tests"

ANY_FAILURE=0

echo "---------------------------------------"
echo "Let's start gNB tests:"
if [ -f "build_test/gnb/tests/gnb_tests" ]; then
    ./build_test/gnb/tests/gnb_tests
    if [ $? -ne 0 ]; then ANY_FAILURE=1; fi
else
    echo "Oops: received an error: gnb_tests binary not found!"
    ANY_FAILURE=1
fi

echo ""
echo "---------------------------------------"
echo "And now let's start UE tests:"
if [ -f "build_test/ue/tests/ue_tests" ]; then
    ./build_test/ue/tests/ue_tests
    if [ $? -ne 0 ]; then ANY_FAILURE=1; fi
else
    echo "Oops: received an error: ue_tests binary not found!"
    ANY_FAILURE=1
fi

echo "---------------------------------------"

if [ $ANY_FAILURE -ne 0 ]; then
    echo "❌ SOME TESTS FAILED. Check the logs above."
    exit 1
else
    echo "✅ ALL TESTS PASSED! Cheking is finish. Fin! Go and push it!"
    exit 0
fi
