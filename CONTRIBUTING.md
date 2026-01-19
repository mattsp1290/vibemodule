# Contributing to Vibemodule

Thank you for your interest in contributing to Vibemodule! This document provides guidelines and information for contributors.

## Code of Conduct

Be respectful and constructive in all interactions. We welcome contributions from everyone.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/vibemodule.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Run tests: `ctest --preset tests`
6. Commit your changes with a clear message
7. Push to your fork and create a Pull Request

## Development Setup

### Prerequisites

- CMake 3.21 or later
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- For JUCE builds: platform-specific dependencies (see README.md)

### Building

```bash
# Configure and build tests
cmake --preset tests
cmake --build --preset tests

# Run tests
ctest --preset tests
```

## Code Style

We use clang-format and clang-tidy for consistent code style. Configuration files are included in the repository.

### General Guidelines

- Use **C++17** features where appropriate
- Follow the existing code style in the codebase
- Keep functions focused and reasonably sized
- Prefer clarity over cleverness

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes | CamelCase | `CloudsReverb` |
| Functions | CamelCase | `SetAmount()` |
| Variables | lower_case | `sample_rate` |
| Constants | kCamelCase | `kBufferSize` |
| Macros | UPPER_CASE | `DISALLOW_COPY_AND_ASSIGN` |
| Template parameters | CamelCase | `Format` |

### Formatting

- Indentation: 2 spaces (no tabs)
- Column limit: 100 characters
- Braces: Attach style (opening brace on same line)
- Pointers/references: Left-aligned (`float* ptr`, not `float *ptr`)

### Example Code Style

```cpp
namespace clouds {

class ExampleClass {
 public:
  ExampleClass() : member_variable_(0.0f) {}
  ~ExampleClass() = default;

  void ProcessSample(float input) {
    if (input > 0.0f) {
      member_variable_ = input * kGainFactor;
    } else {
      member_variable_ = 0.0f;
    }
  }

  float GetValue() const { return member_variable_; }

 private:
  static constexpr float kGainFactor = 0.5f;
  float member_variable_;
};

}  // namespace clouds
```

### Running Formatters

```bash
# Format a file
clang-format -i path/to/file.cpp

# Check formatting (dry run)
clang-format --dry-run path/to/file.cpp

# Run clang-tidy
clang-tidy path/to/file.cpp -- -I libs/clouds-dsp/include
```

## Pull Request Process

### Before Submitting

1. **Run tests**: Ensure all tests pass with `ctest --preset tests`
2. **Format code**: Run clang-format on modified files
3. **Update documentation**: If your change affects the API or behavior
4. **Add tests**: For new functionality or bug fixes

### PR Title Format

Use a clear, descriptive title:
- `Add: <feature description>`
- `Fix: <bug description>`
- `Update: <what was updated>`
- `Refactor: <what was refactored>`

### PR Description

Include:
- **Summary**: What does this PR do?
- **Motivation**: Why is this change needed?
- **Testing**: How was this tested?
- **Breaking changes**: If any

### Review Process

1. A maintainer will review your PR
2. Address any feedback or requested changes
3. Once approved, a maintainer will merge the PR

## Testing

### Writing Tests

- Use Catch2 for unit tests
- Place tests in `tests/` directory
- Name test files `test_<component>.cpp`
- Use descriptive test case names

### Test Structure

```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE("ComponentName functionality", "[component]") {
    // Setup
    Component component;

    SECTION("specific behavior") {
        // Test
        CHECK(component.method() == expected_value);
    }

    SECTION("another behavior") {
        // Another test
        CHECK(component.otherMethod() == Approx(expected).margin(0.001f));
    }
}
```

## Architecture Notes

### Core DSP Library

The `libs/clouds-dsp/` directory contains the platform-independent DSP code:
- `include/clouds/` - Main reverb implementation
- `include/stmlib/` - Ported utility functions from stmlib

### Platform Implementations

Each platform has its own directory under `platforms/`:
- `juce/` - VST3/AU plugin for desktop DAWs
- `ssp/` - Percussa SSP module (ARM Linux)
- `daisy/` - Daisy Patch firmware (Cortex-M7)

### Adding New Features

1. Implement core DSP in `libs/clouds-dsp/`
2. Add appropriate tests
3. Update platform implementations as needed
4. Update documentation

## Reporting Issues

When reporting bugs, please include:
- Operating system and version
- Compiler and version
- Steps to reproduce
- Expected vs actual behavior
- Relevant logs or error messages

## Feature Requests

Feature requests are welcome! Please:
- Check existing issues first
- Describe the use case
- Explain expected behavior
- Consider implementation complexity

## Questions?

Open an issue with the "question" label or start a discussion.
