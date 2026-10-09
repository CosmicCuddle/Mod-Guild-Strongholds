-- Guild Strongholds v0.1.0 - DRAFT manual character database schema
-- DO NOT execute on a live database yet.
-- Explicitly select and back up the character database before an approved install.
-- All tables are module-owned; no changes to AzerothCore's existing tables.
-- This location is intentionally outside the automatic AzerothCore SQL updater paths.

CREATE TABLE IF NOT EXISTS `naxx_gs_settlement` (
  `guild_id` INT UNSIGNED NOT NULL,
  `guild_created_at` BIGINT UNSIGNED NOT NULL,
  `lifecycle_state` VARCHAR(16) NOT NULL DEFAULT 'active',
  `lifecycle_version` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `theme_key` VARCHAR(32) NOT NULL,
  `development_level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `development_xp` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `guild_supplies` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`guild_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `naxx_gs_building` (
  `guild_id` INT UNSIGNED NOT NULL,
  `plot_index` SMALLINT UNSIGNED NOT NULL,
  `building_key` VARCHAR(48) NOT NULL,
  `building_stage` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `construction_progress` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`guild_id`, `plot_index`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Pending construction projects; single active project per guild/plot.
-- Do not apply until staging-tested inventory and receipt transactions exist.
CREATE TABLE IF NOT EXISTS `naxx_gs_project` (
  `guild_id` INT UNSIGNED NOT NULL,
  `plot_key` VARCHAR(32) NOT NULL,
  `project_key` VARCHAR(48) NOT NULL,
  `supplies_contributed` INT UNSIGNED NOT NULL DEFAULT 0,
  `timber_contributed` INT UNSIGNED NOT NULL DEFAULT 0,
  `iron_contributed` INT UNSIGNED NOT NULL DEFAULT 0,
  `version` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`guild_id`, `plot_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `naxx_gs_unlock` (
  `guild_id` INT UNSIGNED NOT NULL,
  `unlock_key` VARCHAR(80) NOT NULL,
  `unlocked_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`guild_id`, `unlock_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `naxx_gs_decoration` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `guild_id` INT UNSIGNED NOT NULL,
  `slot_key` VARCHAR(64) NOT NULL,
  `object_entry` INT UNSIGNED NOT NULL,
  `rotation` FLOAT NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_guild_slot` (`guild_id`, `slot_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `naxx_gs_contribution` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `guild_id` INT UNSIGNED NOT NULL,
  `player_guid` INT UNSIGNED NOT NULL,
  `activity_key` VARCHAR(80) NOT NULL,
  `receipt_key` VARCHAR(64) NOT NULL,
  `units` INT UNSIGNED NOT NULL,
  `contributed_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_guild_receipt` (`guild_id`, `receipt_key`),
  KEY `idx_guild_time` (`guild_id`, `contributed_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Draft ONLY: persisted safe-return ticket for crash/disband recovery.
-- Must be committed BEFORE teleport. One outstanding visit per character.
-- Never permit live entry until property isolation and safe-exit hooks work.
CREATE TABLE IF NOT EXISTS `naxx_gs_visit` (
  `player_guid` INT UNSIGNED NOT NULL,
  `guild_id` INT UNSIGNED NOT NULL,
  `guild_created_at` BIGINT UNSIGNED NOT NULL,
  `visit_nonce` VARCHAR(64) NOT NULL,
  `property_key` VARCHAR(32) NOT NULL,
  `visit_state` VARCHAR(16) NOT NULL DEFAULT 'prepared',
  `return_map` INT UNSIGNED NOT NULL,
  `return_instance` INT UNSIGNED NOT NULL DEFAULT 0,
  `return_x` FLOAT NOT NULL,
  `return_y` FLOAT NOT NULL,
  `return_z` FLOAT NOT NULL,
  `return_o` FLOAT NOT NULL,
  `version` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`player_guid`),
  UNIQUE KEY `uk_visit_nonce` (`visit_nonce`),
  KEY `idx_guild_visit` (`guild_id`, `guild_created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Draft-only phase reservation ledger. NOT game phase changes.
-- Historical/retired bits remain reserved; never auto-reuse them.
CREATE TABLE IF NOT EXISTS `naxx_gs_isolation_slot` (
  `guild_id` INT UNSIGNED NOT NULL,
  `guild_created_at` BIGINT UNSIGNED NOT NULL,
  `phase_bit` INT UNSIGNED NOT NULL,
  `lease_state` VARCHAR(16) NOT NULL DEFAULT 'held',
  `version` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `reserved_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`guild_id`),
  UNIQUE KEY `uk_isolation_phase_bit` (`phase_bit`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `naxx_gs_ledger` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `guild_id` INT UNSIGNED NOT NULL,
  `event_key` VARCHAR(80) NOT NULL,
  `actor_guid` INT UNSIGNED DEFAULT NULL,
  `details` VARCHAR(255) DEFAULT NULL,
  `event_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_guild_event_time` (`guild_id`, `event_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


-- DRAFT ONLY: dedicated raid trophy history and receipts. NOT a migration
-- from legacy naxx_gs_unlock: the old table is deliberately left unchanged.
-- These tables grant NO in-game rewards. Must NOT be applied to production.
-- History is keyed to original guild generation, not guild numeric ID alone.
CREATE TABLE IF NOT EXISTS `naxx_gs_trophy_receipt` (
  `guild_id` INT UNSIGNED NOT NULL,
  `guild_created_at` BIGINT UNSIGNED NOT NULL,
  `event_receipt` VARCHAR(80) NOT NULL,
  `trophy_key` VARCHAR(80) NOT NULL,
  `raid_map_id` INT UNSIGNED NOT NULL,
  `raid_instance_id` INT UNSIGNED NOT NULL,
  `verified_human_count` TINYINT UNSIGNED NOT NULL,
  `recorded_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`guild_id`, `guild_created_at`, `event_receipt`),
  KEY `idx_trophy_receipt_key` (`guild_id`, `guild_created_at`, `trophy_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Only a receipt + unlock + existing audit-LEDGER row in ONE transaction
-- can represent an earned trophy. No real adapter executes this contract.
CREATE TABLE IF NOT EXISTS `naxx_gs_trophy_unlock` (
  `guild_id` INT UNSIGNED NOT NULL,
  `guild_created_at` BIGINT UNSIGNED NOT NULL,
  `trophy_key` VARCHAR(80) NOT NULL,
  `first_event_receipt` VARCHAR(80) NOT NULL,
  `unlocked_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`guild_id`, `guild_created_at`, `trophy_key`),
  UNIQUE KEY `uk_trophy_first_receipt` (`guild_id`, `guild_created_at`, `first_event_receipt`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
