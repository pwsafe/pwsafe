# Contributing to PasswordSafe

Thank you for your interest in contributing to PasswordSafe. This document covers everything
you need to know before opening an issue or submitting a pull request.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Ways to Contribute](#ways-to-contribute)
- [Reporting Bugs](#reporting-bugs)
- [Reporting Security Vulnerabilities](#reporting-security-vulnerabilities)
- [Requesting Features](#requesting-features)
- [Setting Up Your Development Environment](#setting-up-your-development-environment)
- [Making Changes](#making-changes)
- [Pull Request Guidelines](#pull-request-guidelines)
- [Coding Standards](#coding-standards)
- [Commit Messages](#commit-messages)
- [License](#license)
- [AI Assistance](#ai-assistance)

---

## Code of Conduct

We expect all contributors to engage respectfully with one another. Be constructive in code
review, assume good faith, and keep discussions focused on technical merit. Maintainers reserve
the right to close issues or PRs where conduct falls short of this standard.

---

## Ways to Contribute

You do not need to write code to contribute. Useful contributions include:

- Reporting bugs with clear reproduction steps
- Improving documentation or translations
- Writing or improving tests
- Reviewing open pull requests
- Helping triage issues

---

## Reporting Bugs

Before filing a bug report, please search the [existing issues](../../issues) to avoid
duplicates.

When opening a bug report, include:

- A clear, descriptive title
- Steps to reproduce the problem
- Expected behaviour vs. actual behaviour
- PasswordSafe version, operating system, and OS version
- Any relevant logs, screenshots, or `.psafe3` database details (never include actual passwords
  or sensitive data)

---

## Reporting Security Vulnerabilities

**Do not report security vulnerabilities through public GitHub issues.**

If you believe you have found a security vulnerability, please contact the maintainers directly
at [INSERT SECURITY CONTACT EMAIL]. Provide a description of the issue, steps to reproduce, and
the potential impact. We will acknowledge receipt within 48 hours and aim to issue a fix or
mitigation within a reasonable timeframe.

PasswordSafe handles users' credentials. Security bugs are treated with the highest priority.

---

## Requesting Features

Feature requests are welcome. Open an issue using the Feature Request template and describe:

- The problem you are trying to solve (not just the solution you have in mind)
- How the feature would benefit users
- Any alternatives you have considered

Please discuss significant features in an issue before investing time in implementation. A PR
that has not been discussed may not align with the project's direction and may be declined.

---

## Setting Up Your Development Environment

### Prerequisites

- Windows: Visual Studio 2019 or later (Community edition is fine)
- Linux: GCC or Clang, wxWidgets, CMake
- Git

### Building

Refer to the platform-specific build instructions in [`docs/build.md`](docs/build.md) (Windows)
and [`docs/build-linux.md`](docs/build-linux.md) (Linux). If you encounter build issues, check
the [existing issues](../../issues) first, then open a new one if needed.

---

## Making Changes

1. **Fork** the repository and create a branch from `main` (or the current development branch).
   Use a descriptive branch name: `fix/crash-on-empty-group`, `feature/yubikey-support`.
2. **Keep changes focused.** One logical change per PR — bug fixes and feature additions should
   not be mixed.
3. **Write or update tests** for any behaviour you add or change.
4. **Verify your build** compiles cleanly with no new warnings on at least one supported
   platform before opening the PR.
5. **Run the test suite** and confirm all existing tests pass.

---

## Pull Request Guidelines

- Target the correct base branch (typically `main`; check open PRs to confirm the current
  convention).
- Fill out the PR description template. Explain *what* changed and *why*, not just *how*.
- Keep PRs small and reviewable. Large PRs take longer to review and are more likely to be
  asked to be split.
- Link any related issues with `Fixes #NNN` or `Closes #NNN` in the PR description so they
  close automatically on merge.
- Be responsive to review feedback. PRs with no activity for 30 days may be closed; they can
  be reopened when you are ready to continue.
- Do not force-push to a PR branch after review has started unless asked to do so by a
  maintainer.

### Signed-off-by

Each commit must include a `Signed-off-by:` line certifying that you have the right to submit
the contribution under the project's license, in accordance with the
[Developer Certificate of Origin](https://developercertificate.org/):

```
Signed-off-by: Your Name <your.email@example.com>
```

Git can add this automatically with `git commit -s`.

---

## Coding Standards

PasswordSafe is a C++ project. Follow the conventions you observe in the existing code:

- Use the same indentation style (tabs, not spaces) and brace placement as the surrounding code.
- Prefer RAII and standard library facilities over manual resource management.
- Avoid raw `new`/`delete` in new code; prefer smart pointers.
- All code handling passwords, keys, or other sensitive data must zero memory explicitly after
  use (`SecureZeroMemory` on Windows; equivalent on other platforms). Do not rely on destructors
  alone.
- Avoid platform-specific APIs unless inside a clearly delineated platform layer.
- New files must include the project's standard copyright and license header.

When in doubt, match the style of the file you are editing rather than introducing a new
convention.

---

## Commit Messages

Follow the conventional structure:

```
Short summary in imperative mood, 72 characters or less

Optional longer explanation of what changed and why. Wrap at 72 characters.
Reference any related issues here.

Signed-off-by: Your Name <your.email@example.com>
```

Good examples:
- `Fix crash when group name contains a colon`
- `Add Argon2id as an alternative KDF option`
- `Docs: update build instructions for VS 2022`

Avoid vague messages like `fix`, `update`, or `WIP`.

---

## License

By contributing to PasswordSafe you agree that your contributions will be licensed under the
same [Artistic License 2.0](LICENSE) that covers the project. You must have the right to submit
the code under this license (i.e. it is your own work, or you have permission from the
copyright holder).

---

## AI Assistance

Contributors may use AI coding assistants, subject to the guidelines in
[README.AI.md](README.AI.md). The short version: you are responsible for every line you submit,
regardless of how it was produced, and any material AI assistance must be disclosed with an
`Assisted-by:` tag in the PR description.
