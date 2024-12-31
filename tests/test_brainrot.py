import subprocess
import json
import os
import pytest

# Load expected results from JSON file
with open("expected_results.json", "r") as file:
    expected_results = json.load(file)

@pytest.mark.parametrize("example,expected_output", expected_results.items())
def test_brainrot_examples(example, expected_output):
    # Define the command to execute
    command = f".././brainrot < ../examples/{example}"

    # Run the command and capture the output
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True)

    # Combine stdout and stderr intelligently
    actual_output = result.stdout.strip() if result.stdout.strip() else result.stderr.strip()

    # Assert that the combined output matches the expected output
    assert actual_output == expected_output.strip(), (
        f"Output for {example} did not match.\n"
        f"Expected:\n{expected_output}\n"
        f"Actual:\n{actual_output}\n"
        f"Stderr:\n{result.stderr}"
    )

    # Ensure the command completed successfully
    assert result.returncode == 0, (
        f"Command for {example} failed with return code {result.returncode}\n"
        f"Stderr:\n{result.stderr}"
    )


if __name__ == "__main__":
    pytest.main(["-v", os.path.basename(__file__)])
