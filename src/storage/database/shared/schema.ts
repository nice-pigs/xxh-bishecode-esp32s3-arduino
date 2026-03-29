import { pgTable, serial, timestamp, varchar, text, integer, doublePrecision, index } from "drizzle-orm/pg-core"
import { sql } from "drizzle-orm"

export const healthCheck = pgTable("health_check", {
	id: serial().notNull(),
	updatedAt: timestamp("updated_at", { withTimezone: true, mode: 'string' }).defaultNow(),
});

// 景点识别记录表
export const spotRecords = pgTable("spot_records", {
	id: varchar("id", { length: 100 }).primaryKey(),
	image_url: text("image_url").notNull(),
	spot_name: varchar("spot_name", { length: 200 }).notNull(),
	description: text("description").notNull(),
	latitude: doublePrecision("latitude").default(0).notNull(),
	longitude: doublePrecision("longitude").default(0).notNull(),
	audio_url: text("audio_url"),
	audio_url_mp3: text("audio_url_mp3"),
	created_at: timestamp("created_at", { withTimezone: true }).defaultNow().notNull(),
}, (table) => [
	index("spot_records_created_at_idx").on(table.created_at),
	index("spot_records_spot_name_idx").on(table.spot_name),
]);

// 设备状态表
export const deviceStatus = pgTable("device_status", {
	id: serial("id").primaryKey(),
	device_id: varchar("device_id", { length: 100 }).notNull().unique(),
	latitude: doublePrecision("latitude").default(0).notNull(),
	longitude: doublePrecision("longitude").default(0).notNull(),
	altitude: doublePrecision("altitude").default(0).notNull(),
	satellites: integer("satellites").default(0).notNull(),
	speed: doublePrecision("speed").default(0).notNull(),
	battery_level: integer("battery_level"),
	last_update: timestamp("last_update", { withTimezone: true }).defaultNow().notNull(),
}, (table) => [
	index("device_status_device_id_idx").on(table.device_id),
	index("device_status_last_update_idx").on(table.last_update),
]);
