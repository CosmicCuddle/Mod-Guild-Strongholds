#!/usr/bin/env python3
"""Negative-fixture tests for read-only AzerothCore module integration check."""
from __future__ import annotations

import importlib.util
from pathlib import Path

CHECK = Path(__file__).resolve().parents[1] / "scripts/check-azerothcore-module-loader.py"
spec = importlib.util.spec_from_file_location("module_loader_contract", CHECK)
assert spec and spec.loader
contract = importlib.util.module_from_spec(spec)
spec.loader.exec_module(contract)


def fixtures() -> dict[str, str]:
    return {
        "cmake": '''
string(REGEX REPLACE - "_" LOCALE_SCRIPT_MODULE ${LOCALE_SCRIPT_MODULE})
set(LOADER_FUNCTION "Add${LOCALE_SCRIPT_MODULE}Scripts()")
ConfigureScriptLoader(static a b)
CollectSourceFiles(${MODULE_SOURCE_PATH} PRIVATE_SOURCES_MODULES)
CollectSourceFiles(${MODULE_SOURCE_PATH} MODULE_SOURCE_PRIVATE_SOURCES)
''',
        "configure": 'set(MODULE_SOURCE_PATH "${MODULE_BASE_PATH}/${module}/src")',
        "auto": 'function(CollectSourceFiles current_dir variable)\n  file(GLOB X ${current_dir}/*.cpp)\nendfunction()',
        "generated": '@ACORE_SCRIPTS_INVOKE@',
        "loader": '''
void Addmod_guild_strongholdsScripts()
{
    NaxxGuildStrongholds::AddBootstrapScripts();
#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)
    NaxxGuildStrongholds::AddStagingDiagnosticsScripts();
#endif
}
void AddMod_Guild_StrongholdsScripts()
{
    Addmod_guild_strongholdsScripts();
}
#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)
void AddStagingDiagnosticsScripts();
#endif
''',
        "bootstrap": '''sConfigMgr->GetOption<bool>(
"NaxxGuildStrongholds.Enabled", false);''',
        "staging": '#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)\n#endif',
        "worldscript_header": '''
WORLDHOOK_ON_BEFORE_CONFIG_LOAD,
WORLDHOOK_ON_STARTUP,
WorldScript(char const* name, std::vector<uint16> enabledHooks);
virtual void OnBeforeConfigLoad(bool reload) {}
virtual void OnStartup() {}
''',
        "command_header": '''
virtual std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const = 0;
'''
    }


def main() -> None:
    n = 0

    def assert_result(test_case: dict[str, str], expected: bool, label: str):
        nonlocal n
        n += 1
        errors = contract.validate(**test_case)
        if (len(errors) == 0) != expected:
            raise AssertionError(f"{label}: {errors}")

    f = fixtures()
    assert_result(f, True, "Matching source shape")
    for folder, expected in (
        ("mod-guild-strongholds", "Addmod_guild_strongholdsScripts"),
        ("Mod-Guild-Strongholds", "AddMod_Guild_StrongholdsScripts"),
    ):
        n += 1
        assert contract.loader_symbol(folder) == expected

    changes = [
        ("cmake", 'string(REGEX REPLACE - "_"', 'string(REGEX REPLACE "." "_"' ),
        ("cmake", 'CollectSourceFiles(${MODULE_SOURCE_PATH} PRIVATE_SOURCES_MODULES)', ''),
        ("configure", '/${module}/src', '/${module}/tests'),
        ("generated", '@ACORE_SCRIPTS_INVOKE@', ''),
        ("loader", 'void AddMod_Guild_StrongholdsScripts()', 'void AddUnknownScripts()'),
        ("loader", '    Addmod_guild_strongholdsScripts();', '    AddUnknownScripts();'),
        ("loader", '#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)', '// removed',),
        ("loader", 'NaxxGuildStrongholds::AddBootstrapScripts();', ''),
        ("bootstrap", '"NaxxGuildStrongholds.Enabled", false',
                      '"NaxxGuildStrongholds.Enabled", true'),
        ("bootstrap", 'GetOption<bool>(', 'TeleportTo(1); GetOption<bool>('),
        ("worldscript_header", 'WORLDHOOK_ON_STARTUP', 'REMOVED_STARTUP'),
        ("command_header", 'ChatCommandBuilder', 'UnknownCmdType')
    ]
    for field, old, new in changes:
        fixture = fixtures()
        fixture[field] = fixture[field].replace(old, new)
        assert fixture[field] != f[field], "Broken test mutation"
        assert_result(fixture, False, f"Reject {field} change")

    print(f"PASS: {n} upstream loader contract and negative regression checks")


if __name__ == "__main__":
    main()
