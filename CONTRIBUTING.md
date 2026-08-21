# Contributing to AzerothUniverseCore / UniverseEmu

Thanks for your interest in contributing! This project is a TrinityCore-based fork (3.3.5a) powering the Azeroth Universe server, and we welcome contributions from anyone: bug fixes, new systems, SQL content, addon/UI work, or documentation.

This guide covers how to get your changes merged smoothly.

## Getting started

1. Fork the repository.
2. Clone your fork locally and add the upstream repo as a remote so you can keep your fork up to date:
   ```
   git remote add upstream https://github.com/AzerothUniverseCore/UniverseEmu.git
   ```
3. Create a feature branch off `main` for your work. Avoid committing directly to `main` on your fork if you plan to keep contributing over time it makes rebasing easier.
4. Make your changes, commit, push to your fork, and open a pull request against `main`.

If you don't have write access to the main repo, this fork-and-PR workflow is the way to go. If you're contributing regularly, feel free to ask about becoming a collaborator.

## Before you start a big system

If you're planning to bring in a large feature (a new class, a full gameplay system, a big UI overhaul), open an issue or ping us on Discord first. It's a lot easier to align on design and avoid duplicated work before you've written 2000 lines than after.

## Fork-specific gotchas

This is a TrinityCore-classic fork, not modern AzerothCore, and a few APIs commonly used in newer AzerothCore modules don't exist here. These are the two most common ones people trip over when porting code in:

- **Config values**: use `sConfigMgr->GetBoolDefault(key, default)` / `GetIntDefault(key, default)`. The modern templated `sConfigMgr->GetOption<T>(key, default)` does not exist on this fork and will fail to compile.
- **Chat messages**: `ChatHandler::PSendSysMessage` uses classic printf-style formatting (`%s`, `%u`, etc.), not `fmt`-style `{}` placeholders. Code written against modern AzerothCore/fmt conventions needs to be adapted before it'll work correctly here.

If your code compiles against modern AzerothCore, expect to do a small adaptation pass for these two points specifically.

## Coding conventions

- Match the existing style in the file you're editing (indentation, brace placement, naming) rather than introducing a new style in the same file.
- Preserve existing line endings. Most C++ sources in this repo use CRLF if your editor normalizes to LF on save, double-check the diff before committing so you're not rewriting every line in the file over a one-line change.
- Keep changes scoped. A PR that fixes one bug or adds one system is much easier to review than a PR that touches unrelated files "while I was in there."

## SQL changes

- World database changes (creature templates, item templates, spells, quests, etc.) go under the appropriate `sql/` world folder.
- Character database changes (new tables tied to player data) go under the `sql/` characters folder.
- Include both the SQL and the matching C++/Lua that depends on it in the same PR a schema change without the code that uses it (or vice versa) is hard to review in isolation.

## Client-side addons (Lua/XML)

- Test in-game with Lua errors enabled (`/console scriptErrors 1` or the equivalent interface option) before submitting. A silent failure (no error, but nothing shows) is just as important to catch as a crash.
- If your addon talks to the server via addon messages, document the message format in a comment near the handler (prefix, command names, argument order) it saves a lot of time for whoever touches it next.
- Watch for load-order issues between XML and Lua files declared in the `.toc`: a script referenced from XML (`OnClick`, `OnValueChanged`, etc.) must already exist as a global by the time the event can fire, not just by the time the addon finishes loading.

## Crediting original work

If you're porting or adapting a system from another project or author, credit them in the PR description, in a header comment in the relevant file, and in the changelog entry. If you're not sure whether a license permits redistribution, ask before merging rather than after. This matters both legally and for keeping good relationships with the wider WoW modding community.

## Commit messages

Keep them short and descriptive, prefixed with the area you touched:

```
Core/Pet: fix wild battle cooldown desync
Web/Panel: fix last_ip not tracking real client IP
UI: fix Lua errors and battle frame not showing on login
```

One line is usually enough. Add a short body only if the "why" isn't obvious from the diff.

## Pull requests

- Describe what changed and why, not just what files were touched.
- Mention how you tested it (compiled locally, tested in-game, which scenario).
- Expect review comments they're about the code, not about you. We'll work through them together.

## Questions

If anything here is unclear, ask on Discord rather than guessing. We'd rather answer a question upfront than review a PR that went in a direction that won't get merged.
