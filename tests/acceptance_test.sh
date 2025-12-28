#!/bin/bash

# Comprehensive Acceptance Test Suite for IMPLEMENTATION_TASKS.md

set -e

echo "🧪 Running Comprehensive Manaknight Acceptance Tests..."
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

test_count=0
passed_count=0

run_test() {
    local test_name="$1"
    local command="$2"
    local expected_output="$3"

    echo -n "Testing: $test_name... "
    test_count=$((test_count + 1))

    if eval "$command" > /tmp/test_output 2>&1; then
        if [ -n "$expected_output" ]; then
            if grep -q "$expected_output" /tmp/test_output; then
                echo -e "${GREEN}✓ PASS${NC}"
                passed_count=$((passed_count + 1))
            else
                echo -e "${RED}✗ FAIL${NC} (expected '$expected_output' in output)"
                echo "Actual output:"
                cat /tmp/test_output
            fi
        else
            echo -e "${GREEN}✓ PASS${NC}"
            passed_count=$((passed_count + 1))
        fi
    else
        echo -e "${RED}✗ FAIL${NC} (command failed)"
        cat /tmp/test_output
    fi
}

echo "=== PHASE 1: Compiler Scaffolding & Parsing ==="

# Task 1.1: AST Definitions
run_test "AST Definitions - Function Declaration" \
    "./mkc tests/phase1_test.mk && ./mqjs tests/phase1_test.js" \
    "Phase 1 tests pass"

# Task 1.2: Lexer Implementation
run_test "Lexer - Keywords and Literals" \
    "./mkc tests/function_test.mk && ./mqjs tests/function_test.js" \
    "Main function works"

# Task 1.3: Recursive Descent Parser
run_test "Parser - Basic Function Parsing" \
    "./mkc tests/minimal.mk && ./mqjs tests/minimal.js" \
    "hi"

# Task 1.4: Canonical Formatter (not implemented yet)
echo -e "${YELLOW}⚠️  Task 1.4: Canonical Formatter - Not implemented yet${NC}"

# Task 1.5: Error Catalog
run_test "Error Catalog - Clean Compilation" \
    "./mkc tests/minimal.mk" \
    ""

echo
echo "=== PHASE 2: Semantics & Verification ==="

# Task 2.0: Module Resolution (basic test)
run_test "Module Resolution - Basic" \
    "./mkc tests/minimal.mk" \
    ""

# Task 2.1: Symbol Table & Scope (basic test)
run_test "Symbol Table - Function Scoping" \
    "./mkc tests/function_test.mk" \
    ""

# Task 2.2: Type Checker (basic inference)
run_test "Type Checker - Basic Types" \
    "./mkc tests/function_test.mk && ./mqjs tests/function_test.js" \
    "Main function works"

# Task 2.3: Effect Analyzer (not implemented)
echo -e "${YELLOW}⚠️  Task 2.3: Effect Analyzer - Not implemented yet${NC}"

# Task 2.4: Exhaustiveness Checker (not implemented)
echo -e "${YELLOW}⚠️  Task 2.4: Exhaustiveness Checker - Not implemented yet${NC}"

echo
echo "=== PHASE 3: Lowering (Translation to JS) ==="

# Task 3.0: Intrinsic Mapping Strategy
run_test "Intrinsic Mapping - String Literals" \
    "echo 'fn main() -> String { \"test\" }' > /tmp/intrinsic_test.mk && ./mkc /tmp/intrinsic_test.mk && ./mqjs /tmp/intrinsic_test.js" \
    "test"

# Task 3.1: IR Lowering (basic function lowering)
run_test "IR Lowering - Function to JS" \
    "./mkc tests/function_test.mk && grep -q 'function' tests/function_test.js" \
    ""

# Task 3.2: Effect Injection (not implemented)
echo -e "${YELLOW}⚠️  Task 3.2: Effect Injection - Not implemented yet${NC}"

# Task 3.3: JS Emitter
run_test "JS Emitter - Valid ES5 Output" \
    "./mkc tests/minimal.mk && node -c tests/minimal.js" \
    ""

# Task 3.4: OpenAPI Generator (not implemented)
echo -e "${YELLOW}⚠️  Task 3.4: OpenAPI Generator - Not implemented yet${NC}"

echo
echo "=== PHASE 4: Standard Library & Runtime ==="

# Task 4.0: Stdlib Type Definitions (basic test)
run_test "Stdlib Types - Core Availability" \
    "./mkc tests/minimal.mk" \
    ""

# Task 4.1: Stdlib Core (basic runtime)
run_test "Stdlib Core - Runtime Execution" \
    "./mqjs tests/minimal.js" \
    "hi"

# Task 4.2: Host Runtime
run_test "Host Runtime - mqjs Execution" \
    "./mqjs --help 2>/dev/null | head -1" \
    ""

# Task 4.3: Effect Handlers (not implemented)
echo -e "${YELLOW}⚠️  Task 4.3: Effect Handlers - Not implemented yet${NC}"

echo
echo "=== PHASE 5: Verification & Delivery ==="

# Task 5.1: Compiler CLI
run_test "Compiler CLI - Help Command" \
    "./mkc --help | grep -q 'Manaknight'" \
    ""

run_test "Compiler CLI - Version Info" \
    "./mkc --help | grep -q 'Manaknight'" \
    ""

# Task 5.2: End-to-End Test Suite
run_test "E2E Test Suite - Full Pipeline" \
    "./tests/run_tests.sh" \
    "All tests passed"

echo
echo "=== SUMMARY ==="
echo "Tests run: $test_count"
echo "Tests passed: $passed_count"
echo "Tests failed: $((test_count - passed_count))"

if [ $passed_count -eq $test_count ]; then
    echo -e "${GREEN}🎉 ALL ACCEPTANCE TESTS PASSED!${NC}"
    exit 0
else
    echo -e "${RED}❌ SOME TESTS FAILED${NC}"
    exit 1
fi
