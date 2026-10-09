#!/usr/bin/env python3
"""Read-only check of real AzerothCore module loader conventions.

Checks upstream CMake SOURCE COLLECTION and folder-based loader symbol
generation against this repository's two supported folder names.
It is not an AzerothCore build or installed-fork compatibility approval.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

PINNED_UPSTREAM = "7b2cecef92b271a468e39d89831b520b20ae06a8"
ALLOWED_FOLDERS = ("mod-guild-strongholds", "Mod-Guild-Strongholds")


def loader_symbol(folder: str) -> str:
    return "Add" + folder.replace("-", "_") + "Scripts"


def validate(cmake: str, configure: str, auto: str, generated: str,
             loader: str, bootstrap: str, staging: str,
             worldscript_header: str, command_header: str) -> list[str]:
    problems: list[str] = []

    def need(condition: bool, reason: str) -> None:
        if not condition:
            problems.append(reason)

    need('string(REGEX REPLACE - "_" LOCALE_SCRIPT_MODULE' in cmake,
         "Core no longer replaces '-' in module folder loader")
    need('"Add${LOCALE_SCRIPT_MODULE}Scripts()"' in cmake,
         "Core loader function naming contract changed")
    need("ConfigureScriptLoader(" in cmake,
         "Core loader generator missing")
    need("CollectSourceFiles(${MODULE_SOURCE_PATH} PRIVATE_SOURCES_MODULES)" in cmake,
         "Static modules no longer collect src/ in expected way")
    need("CollectSourceFiles(${MODULE_SOURCE_PATH} MODULE_SOURCE_PRIVATE_SOURCES)" in cmake,
         "Dynamic modules no longer collect src/ in expected way")
    need('${MODULE_BASE_PATH}/${module}/src' in configure,
         "Core module source directory contract changed")
    need("function(CollectSourceFiles current_dir variable)" in auto,
         "Expected source collector function missing")
    need("${current_dir}/*.cpp" in auto,
         "Expected recursive C++ src collector changed")
    need("@ACORE_SCRIPTS_INVOKE@" in generated,
         "Generated loader no longer invokes module entry points")

    for folder in ALLOWED_FOLDERS:
        symbol = loader_symbol(folder)
        need(re.search(r"\bvoid\s+" + re.escape(symbol) +
                       r"\s*\(\s*\)\s*\{", loader) is not None,
             f"Expected module entry point not exported: {symbol}")

    # Only the canonical registration path gets to call gameplay hooks
    # when an opt-in is eventually reviewed.
    need(re.search(r"void\s+AddMod_Guild_StrongholdsScripts\s*\(\)\s*\{\s*"
                   r"Addmod_guild_strongholdsScripts\s*\(\s*\)\s*;\s*\}",
                   loader) is not None,
         "Uppercase clone alias does not delegate to canonical entry point")
    need("NaxxGuildStrongholds::AddBootstrapScripts();" in loader,
         "Passive bootstrap was removed")

    # Compile-time condition must gate BOTH declaration and invocation,
    # with normal builds keeping the staging command disabled.
    need(loader.count("#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)") == 2,
         "Staging-only diagnostic declaration/call compile guards changed")
    need(loader.count("NaxxGuildStrongholds::AddStagingDiagnosticsScripts();") == 1,
         "Staging diagnostics not registered exactly once under guard")
    need("#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)" in staging,
         "Staging implementation no longer compile-gated")

    need('GetOption<bool>(' in bootstrap and
         '"NaxxGuildStrongholds.Enabled", false' in bootstrap,
         "Bootstrap no longer disabled by default")
    for danger in ("TeleportTo(", "SetPhaseMask(", "CharacterDatabase.",
                   "WorldDatabase.", "new Creature", "new GameObject"):
        need(danger not in bootstrap, f"Bootstrap contains gameplay operation: {danger}")

    need(re.search(r"WorldScript\s*\(\s*char\s+const\s*\*\s*name",
                   worldscript_header) is not None,
         "WorldScript constructor signature changed")
    for hook in ("WORLDHOOK_ON_BEFORE_CONFIG_LOAD", "WORLDHOOK_ON_STARTUP",
                 "OnBeforeConfigLoad(bool", "OnStartup()"):
        need(hook in worldscript_header, f"Required WorldScript hook changed: {hook}")
    need("GetCommands() const" in command_header and
         "Acore::ChatCommands::ChatCommandBuilder" in command_header,
         "CommandScript signature changed; diagnostics require review")

    return problems


FILES = {
    "cmake": "modules/CMakeLists.txt",
    "configure": "src/cmake/macros/ConfigureModules.cmake",
    "auto": "src/cmake/macros/AutoCollect.cmake",
    "generated": "modules/ModulesLoader.cpp.in.cmake",
    "worldscript_header": "src/server/game/Scripting/ScriptDefines/WorldScript.h",
    "command_header": "src/server/game/Scripting/ScriptDefines/CommandScript.h",
}


def inspect(core: Path, module: Path) -> list[str]:
    sources = {}
    for key, path in FILES.items():
        sources[key] = (core / path).read_text(encoding="utf-8")
    return validate(
        sources["cmake"], sources["configure"], sources["auto"],
        sources["generated"],
        (module / "src/loader.cpp").read_text(encoding="utf-8"),
        (module / "src/StrongholdBootstrap.cpp").read_text(encoding="utf-8"),
        (module / "src/StrongholdStagingDiagnostics.cpp").read_text(encoding="utf-8"),
        sources["worldscript_header"], sources["command_header"])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("core_source", type=Path)
    parser.add_argument("--module-source", type=Path, default=Path("."))
    options = parser.parse_args()

    try:
        problems = inspect(options.core_source, options.module_source)
    except OSError as exc:
        print(f"BLOCKED: Source checkout incomplete: {exc}", file=sys.stderr)
        return 1

    if problems:
        print("BLOCKED: Upstream AzerothCore module contract changed")
        for issue in problems:
            print(" -", issue)
        return 1

    print("PASS: Pinned upstream folder loader/static+dynamic source contracts")
    print("Supported module folders:", ", ".join(ALLOWED_FOLDERS))
    print("Normal registration: one passive, disabled-by-default WorldScript")
    print("Opt-in GM staging diagnostics: compile-time guarded")
    print("User's deployed fork compile/runtime: NOT TESTED")
    print("No source files or world/character data changed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
