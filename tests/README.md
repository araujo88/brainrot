# Brainrot Test Suite

This repository contains a test suite for validating the outputs of `brainrot` examples using `pytest`.

## Setup Instructions

Follow these steps to set up the environment and run the tests:

### 1. Create a Virtual Environment

Use Python's `venv` module to create a virtual environment:

```bash
python -m venv .env
```

Activate the virtual environment:

- On Linux/Mac:
  ```bash
  source .env/bin/activate
  ```
- On Windows:
  ```bash
  .env\Scripts\activate
  ```

### 2. Install Dependencies

Install the required Python packages using `pip`:

```bash
pip install -r requirements.txt
```

### 3. Add `brainrot` Executable

Ensure the `brainrot` executable is available in the root directory or accessible via the system path.

### 4. Structure Your Files

Ensure the following structure is maintained:

```
.
├── examples/
│   ├── boolean.brainrot
│   ├── fizz_buzz.brainrot
│   ├── hello_world.brainrot
│   ├── sizeof.brainrot
│   └── ...
├── tests/
│   ├── expected_results.json
│   ├── requirements.txt
│   ├── test_brainrot.py
│   ├── README.md
│   └── ...
└── ...
```

### 5. Run Tests

Execute the test suite using `pytest`:

```bash
pytest -v
```

### 6. Deactivate the Virtual Environment

Once done, deactivate the virtual environment:

```bash
deactivate
```

## Updating Test Cases

1. Add or update example files in the `examples/` directory.
2. Update the `expected_results.json` file with the corresponding expected outputs.

## Troubleshooting

- Ensure `brainrot` has the necessary execution permissions:
  ```bash
  chmod +x ./brainrot
  ```
- Verify paths in `expected_results.json` and `examples/`.
- If you encounter any issues, check the error messages in the `pytest` output.
