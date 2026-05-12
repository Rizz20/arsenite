import subprocess
import os
import sys

def run_test(name, path, expected_output=None, expected_exit_code=0):
    print(f"Running test: {name}...", end="", flush=True)
    
    # Run compiler
    try:
        # Redirect stderr to devnull to avoid cluttering test output with IR prints
        with open(os.devnull, 'w') as devnull:
            res = subprocess.run(["./compiler", path], capture_output=True, text=True)
    except Exception as e:
        print(f" FAIL (Compiler error: {e})")
        return False

    if res.returncode != 0:
        print(f" FAIL (Compiler failed with code {res.returncode})")
        print(res.stderr)
        return False

    # Run executable
    try:
        res = subprocess.run(["./main_exec"], capture_output=True, text=True)
    except Exception as e:
        print(f" FAIL (Execution error: {e})")
        return False

    success = True
    if res.returncode != expected_exit_code:
        print(f" FAIL (Exit code {res.returncode}, expected {expected_exit_code})")
        success = False
    elif expected_output and expected_output not in res.stdout:
        print(f" FAIL (Output mismatch)")
        print(f"  Expected to find: {expected_output}")
        print(f"  Got: {res.stdout}")
        success = False
    else:
        print(" PASS")
    
    return success

def main():
    if not os.path.exists("./compiler"):
        print("Error: compiler not found. Run 'make' first.")
        sys.exit(1)

    tests = [
        ("Arithmetic", "tests/arithmetic.at", "Add: 30, Sub: 10, Mul: 200, Div: 2"),
        ("Return Value", "tests/return_val.at", None, 42),
        ("Strings", "tests/strings.at", "Hello, Arsenite!\nMultiple and strings: 1, 2"),
    ]

    passed = 0
    for test in tests:
        if run_test(*test):
            passed += 1

    print(f"\nSummary: {passed}/{len(tests)} tests passed.")
    if passed < len(tests):
        sys.exit(1)

if __name__ == "__main__":
    main()
