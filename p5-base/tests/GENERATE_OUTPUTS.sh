#!/bin/bash
# Script to generate expected output files for tests 15-20
# Run this inside your Docker container from the solution/ directory

echo "Building test executables..."
cd "$(dirname "$0")/../solution" || exit 1
make test_bb_sequences test_bb_stress > /dev/null 2>&1

if [ ! -f ./test_bb_sequences ]; then
    echo "ERROR: test_bb_sequences not built"
    exit 1
fi

if [ ! -f ./test_bb_stress ]; then
    echo "ERROR: test_bb_stress not built"
    exit 1
fi

echo "Generating expected output files..."

# Test 15: P1->C1
echo "Generating tests/15.out..."
./test_bb_sequences 2>&1 | sed -E 's/\[[0-9]+ ms\] //g' | \
sed -n '/=== Test: P1->C1 ===/,/=== P1->C1:/p' > ../tests/tests/15.out

# Test 16: P1->P2->C1->C2
echo "Generating tests/16.out..."
./test_bb_sequences 2>&1 | sed -E 's/\[[0-9]+ ms\] //g' | \
sed -n '/=== Test: P1->P2->C1->C2 ===/,/=== P1->P2->C1->C2:/p' > ../tests/tests/16.out

# Test 17: P1->C1->P2->C2->P3->C3
echo "Generating tests/17.out..."
./test_bb_sequences 2>&1 | sed -E 's/\[[0-9]+ ms\] //g' | \
sed -n '/=== Test: P1->C1->P2->C2->P3->C3 ===/,/=== P1->C1->P2->C2->P3->C3:/p' > ../tests/tests/17.out

# Test 18: P1->P2->P3->C1->C2->C3
echo "Generating tests/18.out..."
./test_bb_sequences 2>&1 | sed -E 's/\[[0-9]+ ms\] //g' | \
sed -n '/=== Test: P1->P2->P3->C1->C2->C3 ===/,/=== P1->P2->P3->C1->C2->C3:/p' > ../tests/tests/18.out

# Test 19: FIFO-P1->P2->P3->P4->C1->C2->C3->C4
echo "Generating tests/19.out..."
./test_bb_sequences 2>&1 | sed -E 's/\[[0-9]+ ms\] //g' | \
sed -n '/=== Test: FIFO-P1->P2->P3->P4->C1->C2->C3->C4 ===/,/=== FIFO-P1->P2->P3->P4->C1->C2->C3->C4:/p' > ../tests/tests/19.out

# Test 20: Stress test
echo "Generating tests/20.out..."
./test_bb_stress 2>&1 | sed -E 's/\[[0-9]+ ms\] //g' | \
grep -E '(^===|^PASS|^FAIL|Configuration:|Producers:|Consumers:|Buffer|Total|checksum)' > ../tests/tests/20.out

echo ""
echo "Done! Generated output files for tests 15-20."
echo "Now run: cd ../tests && ./run_tests.sh"

