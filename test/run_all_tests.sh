#!/bin/bash
set -e

echo "=== 1. Building and Running Host C++ Unit Tests ==="
g++ -std=c++17 -Wall -Wextra test/test_vocab_store.cpp -o test/test_vocab_store
./test/test_vocab_store
echo "✓ Host C++ unit tests passed."

echo ""
echo "=== 2. Running Patch and Installer Tests ==="
python3 test/test_patch.py
echo "✓ Patcher and installer tests passed."

echo ""
echo "=== 3. Running xteink-plugins Integration Tests ==="
python3 test/test_xteink_integration.py
echo "✓ xteink-plugins integration tests passed."

echo ""
echo "=== ALL TESTS PASSED! ==="
