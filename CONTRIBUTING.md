# Contributing to PasswordSafe

Thank you for your interest in contributing to PasswordSafe. This document describes what
you need to know before opening an issue or submitting a pull request.

---

## Code of Conduct

Be constructive in code review, assume good faith, and keep discussions focused on technical merit.
Issues or PRs will be closed where conduct falls short of this standard.

---

## Ways to Contribute

You do not need to write code to contribute. Useful contributions include:

- Reporting bugs with clear reproduction steps
- Improving documentation or translations
- Writing or improving tests
- Reviewing open pull requests

---

## Reporting Bugs

Before filing a bug report, please search the existing issues to avoid duplicates.
New issues should be submitted on GitHub at https://github.com/pwsafe/pwsafe/issues, although SourceForge is also supported: https://sourceforge.net/p/passwordsafe/_list/tickets

When opening a bug report, include:

- A clear, descriptive title
- Steps to reproduce the problem
- Expected vs. actual behavior
- PasswordSafe version, operating system, and OS version
- A sample database, if relevant (never include actual passwords!)

---

## Reporting Security Vulnerabilities

See [Security.md](SECURITY.md).

---

## Requesting Features

Feature requests are welcome. However, before proposing a feature, please search the existing issues to avoid duplicates.
New requests should be submitted on GitHub at https://github.com/pwsafe/pwsafe/issues,
although SourceForge is also supported: https://sourceforge.net/p/passwordsafe/feature-requests/

When submitting a feature request, please describe:
- The problem you are trying to solve (not just the solution you have in mind)
- How the feature would benefit users
- Any alternatives you have considered

Please discuss significant features in an issue before investing time in implementation. A PR
that has not been discussed may not align with the project's direction and may be declined.

---

## Setting Up Your Development Environment

See the relevant README files under the docs directory.

For specific platforms:
- [README.WINDOWS.DEVELOPERS.md](docs/README.WINDOWS.DEVELOPERS.md)
- [README.LINUX.DEVELOPERS.md](docs/README.LINUX.DEVELOPERS.md)
- [README.MAC.DEVELOPERS.md](docs/README.MAC.DEVELOPERS.md)
- [README.FREEBSD.md](docs/README.FREEBSD.md)
- [README.OPENBSD.md](docs/README.OPENBSD.md)

General information:
- [README.CMAKE.md](docs/README.CMAKE.md)
- [README.XML.md](docs/README.XML.md)

## Making Changes

1. **Fork** the repository and create a branch from `master`.
   Use a descriptive branch name: `fix/crash-on-empty-group`, `feature/yubikey-support`.
2. **Keep changes focused.** One logical change per PR — bug fixes and feature additions should
   not be mixed.
3. **Write or update tests** for any core behavior you add or change.
4. **Verify your build** compiles cleanly with no new warnings on at least one supported
   platform before opening the PR. Once it is opened on GitHub, make sure its build workflows succeed on all platforms and try to fix any build issues that arise (if you don't have access to a platform, you can still update your PR and wait for the build results to see if the fix worked).
5. **Run the test suite** and confirm all existing tests pass.

---

## Pull Request Guidelines

- Target the correct base branch (typically `master`).
- Fill out the PR description template. Explain *what* changed and *why*, not just *how*.
- Keep PRs small and reviewable. Large PRs take longer to review and are more likely to be
  asked to be split.
- Link any related issues with `Fixes #NNN` or `Resolves #NNN` in the PR description so they
  close automatically on merge.
- Be responsive to review feedback. PRs with no activity for 30 days may be closed; they can
  be reopened when you are ready to continue.

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

- Use the same indentation style and brace placement as the surrounding code.
- Prefer RAII and standard library facilities over manual resource management.
- Avoid raw `new`/`delete` in new code; prefer smart pointers.
- `stringT` is the typedef for `std::wstring`. It should be used for non-sensitive strings.
- Use the `StringX` class for handling passwords, keys, or other sensitive textual data.
 This is like `std::wstring` in all respects, except that memory is scrubbed before being returned to system.
- Likewise, use the `VectorX` class instead of `std::vector` for sensitive binary data, e.g., passkey credentials.
- Keep the UI-related functionality in a folder specific to the platform, i.e., src/ui/Windows or src/ui/wxWidgets,
  but try to extract any common functionality and/or business logic into src/core so it can be reused across platforms.
- Keep all platform-specific APIs under src/os, defining a common header file and platform-specific implementations.
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
the code under this license (i.e., it is your own work, or you have permission from the
copyright holder).

---

## AI Assistance

Contributors may use AI coding assistants, **however** _you_ (the submitter) are responsible for every line you submit,
regardless of how it was produced. **The contributor is always the author. AI is a tool, not a co-author.**


The following obligations apply to all PRs, and are **heightened** when AI was involved, because AI
tools can be confidently wrong in ways that are not obvious on casual inspection.

**Correctness.** You have read and understood every line. AI tools routinely produce code that
compiles and runs but contains subtle logic errors, edge-case failures, hallucinated API calls,
or resource leaks. You are responsible for catching all of these before submission.

**Security.** PasswordSafe is a security-critical application. AI tools are particularly
unreliable for security-sensitive code and may suggest patterns that are functionally plausible
but cryptographically weak, or introduce subtle vulnerabilities (buffer overruns, use-after-free,
timing side-channels, incorrect crypto API usage). Any AI-assisted code touching cryptography,
authentication, key derivation, or sensitive memory handling must be reviewed with exceptional
care. When in doubt, open a discussion before submitting.

**License and provenance.** AI models may reproduce portions of copyrighted code without
attribution, and some AI tool EULAs affect ownership of generated output. Check your tool's
terms. If generated code closely resembles a known third-party implementation with an incompatible license,
then it needs to be rewritten.
If the license is compatible, then add a reference to the original code and its license in a comment.
In any case, your`Signed-off-by:` certifies that you have the right to submit the code under this project's
license — AI cannot sign off on your behalf.

**Codebase consistency.** AI tools generate code against the broad open-source corpus, not this
codebase. Ensure contributions match PasswordSafe's style and conventions as
described above.

**Tests.** AI-generated tests often pass vacuously or miss the cases that matter. Do not submit
tests you cannot explain and defend.

---

## What Is Not Permitted

- Submitting AI-generated code you have not read and understood in full.
- Having an AI agent add a `Signed-off-by:` tag. Only a human may certify the
  [Developer Certificate of Origin](https://developercertificate.org/).
- Bulk AI-generated PRs with minimal human review. If reviewing your PR would take maintainers
  substantially longer than it took you to verify it, it will be closed.

---

Finally, consider the following:
- Is AI solving the correct problem?
- Are alternative approaches?
- AI tends to be overly verbose in its comments and documentation, focusing on the context of the requested
  fix rather than how the added/modified code works with the rest of the system. 
  In the long run, this makes the code less maintainable. Comments in the code shouldn't reflect the prompt
  used to generate the code. This information belongs in the commit message.