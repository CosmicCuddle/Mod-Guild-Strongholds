# Source inventory for later complete staging build

**Not an instruction to touch your live realm now.** Current Strongholds development is still not deployable.

Your Naxxramas AzerothCore server uses several customised modules, including Individual Progression, Playerbots and Naxxramas Core. We must validate the **actual deployed source and local patches**, not just public module pages.

The repository's read-only `scripts/check-module-inventory.sh` accepts **one argument**: the root path of a source checkout containing `modules/`. When the server operator eventually has a separate backed-up staging source tree, its invocation is:

```bash
bash scripts/check-module-inventory.sh /path/to/STAGING/azerothcore
```

The script reports:
- AzerothCore core Git SHA and whether tracked source differs from that commit;
- a directory name for every module under `modules/`;
- each module's **independent** Git SHA and tracked-file dirty state where available;
- `NO_INDEPENDENT_GIT` for modules not independently versioned (even if located inside the core Git tree);
- explicit `REVIEW_REQUIRED` for unknown/untracked/configuration/build state.

The script does not open application configuration, SQL, SSH credentials, player records or character files, and does not echo Git remotes, modified paths or patch content. It only calls read-only Git revision/diff commands and never edits source.

It does **not** enumerate untracked files or uncommitted files excluded by Git metadata; these require a separate secured source review without posting credentials. It also cannot prove which modules were compiled, enabled or hot-patched in the worldserver binary. A clean Git tree does not imply real runtime compatibility.

`tests/module_inventory_tests.sh` builds disposable fake Git repositories within a temporary directory to validate clean/dirty/unversioned and missing-directory behavior. GitHub Actions runs those fixtures without any access to a server.

**Release gate:** do not modify AzerothCore core, installed IP/Playerbots modules or apply guild housing SQL until a complete staging backup and source inventory have been reviewed and the combined server compiled and tested.
