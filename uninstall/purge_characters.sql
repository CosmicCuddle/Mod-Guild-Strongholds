-- DANGER: Guild Strongholds module-data purge.
-- This script is OPTIONAL and PERMANENTLY DELETES ALL STRONGHOLD PROGRESS.
-- Stop worldserver first; back up and confirm you selected the CHARACTER database.
-- Ordinary uninstall MUST NOT execute this script.
-- Drops only module-owned naxx_gs_* tables.

DROP TABLE IF EXISTS `naxx_gs_isolation_slot`;
DROP TABLE IF EXISTS `naxx_gs_visit`;
DROP TABLE IF EXISTS `naxx_gs_ledger`;
DROP TABLE IF EXISTS `naxx_gs_contribution`;
DROP TABLE IF EXISTS `naxx_gs_decoration`;
DROP TABLE IF EXISTS `naxx_gs_unlock`;
DROP TABLE IF EXISTS `naxx_gs_project`;
DROP TABLE IF EXISTS `naxx_gs_building`;
DROP TABLE IF EXISTS `naxx_gs_settlement`;
