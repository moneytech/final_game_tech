/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Towadev

Description:
	A tower defence clone.
	Levels are loaded from .TMX files (Tiled-Editor).
	All data (Waves, Enemies, Towers) are loaded from xml files.
	Written in C++ (C-Style).

Requirements:
	- C++ Compiler
	- Final XML
	- Final Framework

Author:
	Torsten Spaete

Changelog:
	## 2018-07-06
	- Level size can now be of any size

	## 2018-07-05
	- Corrected for api change in final_game.h
	- Corrected for api change in final_render.h
	- Migrated to new render system and removed all opengl calls

	## 2018-07-03
	- Fixed collision was broken
	- Fixed spawner was active while start-cooldown of wave
	- Fixed enemy prediction was broken

	## 2018-06-25:
	- Introduced Tower Buttons
	- Write selected tower name

	## 2018-06-20:
	- Refactoring
	- WaveData have now a list of SpawnerData, so we can have multiple & different spawners for each wave.
	- Fixed overlay font atlas was too small

	## 2018-06-19:
	- Started a very basic immediate mode UI system
	- Prepare for command buffer rendering
	- Draw tower preview on mouse tile
	- Bugfixes

	## 2018-06-18:
	- Simplified tower rotation
	- Added prediction flags & enemy range test type
	- Changed enemy detection using new flags and range test

	## 2018-06-16:
	- Changed enemy detection and fire on non-targets as well
	- Made tower rotation really bad

	## 2018-06-15:
	- Improved enemy position prediction for towers
	- Improved gun rotation
	- Cooldown only after tower has fired

	## 2018-06-14:
	- Improved HUD rendering
	- Added background for controls/UI rendering
	- Added simple gun rotation

	## 2018-06-11:
	- Heavy refactoring
	- Small bugfixes
	- Render enemy hp as a colored progressbar
	- Render wave cooldown timer
	- Introduced wave state

	## 2018-06-10:
	- Removed entity tile layer from TMX map
	- Waypoints / Goal are now loaded from objects
	- Introduced money and bounty
	- Show current money in HUD

	## 2018-06-09:
	- Tons of bugfixes (Waypoints, Target detection, etc.)
	- Added lots of new properties
	- Added simple HUD showing wave & lifes
	- Improved enemy spawner to support multiple spawners per wave

	## 2018-06-08:
	- Improved enemy target
	- Removed fixed towers
	- Added tower placement using mouse
	- Removed fixed tilemap
	- Added basic TMX parsing support
	- Tiles are now loaded from TMX layers

	## 2018-06-07:
	- Added enemy spawner
	- Added fixed towers with instant lock-on
	- Added bullet shoot from towers with a cooldown

	## 2018-06-06:
	- Improved enemy movement

	## 2018-06-05:
	- Initial creation

Todo:
	- Tower Selecting
		- Single Select
		- Multiple Select (From Mouse-Area)
	- Sell Tower + Button
	- Upgrade Tower + Button
	- Proper Drawing (Layers / Primitives / Barrels)
		- Towers
		- Enemies
	- Move all const arrays out into XML files
	- Manual reload of XMLs and update all data dynamically
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#include <final_platform_layer.h>

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FXML_IMPLEMENTATION
#include <final_xml.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_ASSETS_IMPLEMENTATION
#include <final_assets.h>

#include <final_game.h>

#include "fpl_towadev.h"

constexpr float ShotAngleTolerance = (Pi32 * 0.05f);

static const TowerData TowerDefinitions[] = {
	MakeTowerData(
		/* id: */ "First Tower",
	/* structureRadius: */ MaxTileSize * 0.35f,
	/* detectionRadius: */ MaxTileSize * 2.25f,
	/* unlockRadius: */ MaxTileSize * 2.3f,
	/* gunTubeLength: */ MaxTileSize * 0.55f,
	/* gunCooldown: */ 0.35f,
	/* gunTubeThickness: */ MaxTileSize * 0.2f,
	/* gunRotationSpeed: */ 4.0f,
	/* costs: */ 50,
	/* enemyRangeTestType: */ FireRangeTestType::InSight,
	/* enemyPredictionFlags: */ EnemyPredictionFlags::All,
	/* enemyLockOnMode: */ EnemyLockTargetMode::LockedOn,
	MakeBulletData(
	/* renderRadius: */ MaxTileSize * 0.05f,
	/* collisionRadius: */ MaxTileSize * 0.05f,
	/* speed: */ 2.5f,
	/* damage: */ 15
)
),
MakeTowerData(
	/* id: */ "Second Tower",
	/* structureRadius: */ MaxTileSize * 0.35f,
	/* detectionRadius: */ MaxTileSize * 2.15f,
	/* unlockRadius: */ MaxTileSize * 2.2f,
	/* gunTubeLength: */ MaxTileSize * 0.4f,
	/* gunCooldown: */ 0.2f,
	/* gunTubeThickness: */ MaxTileSize * 0.15f,
	/* gunRotationSpeed: */ 6.0f,
	/* costs: */ 100,
	/* enemyRangeTestType: */ FireRangeTestType::InSight,
	/* enemyPredictionFlags: */ EnemyPredictionFlags::All,
	/* enemyLockOnMode: */ EnemyLockTargetMode::LockedOn,
	MakeBulletData(
	/* renderRadius: */ MaxTileSize * 0.04f,
	/* collisionRadius: */ MaxTileSize * 0.04f,
	/* speed: */ 3.5f,
	/* damage: */ 8
)
),
};

static const CreepData CreepDefinitions[] = {
	MakeCreepData(
		/* id: */ "The Quad",
	/* renderRadius: */ MaxTileSize * 0.25f,
	/* collisionRadius: */ MaxTileSize * 0.2f,
	/* speed: */ 1.0,
	/* hp: */ 100,
	/* bounty: */ 1,
	/* color: */ V4f(1, 1, 1, 1)
),
};

static const WaveData WaveDefinitions[] = {
	MakeWaveData(
		/* level: */ "level1",
	/* startupCooldown: */ 3.0f,
	/* completionBounty: */ 20,
	/* spawners: */
	{
		MakeSpawnData("spawn1", "The Quad", SpawnerStartMode::Fixed, 0, 1.5f, 25),
		MakeSpawnData("spawn1", "The Quad", SpawnerStartMode::AfterTheLast, 0, 1.0f, 10),
	}
),
};

namespace gamelog {
	enum class LogLevel {
		Fatal = 0,
		Error,
		Warning,
		Info,
		Verbose
	};

	static void Write(const LogLevel level, const char *format, va_list argList) {
		char msg[1024];
		fplFormatAnsiStringArgs(msg, FPL_ARRAYCOUNT(msg), format, argList);
		if (level == LogLevel::Fatal)
			fplDebugOut("Fatal: ");
		else if (level == LogLevel::Error)
			fplDebugOut("Error: ");
		else if (level == LogLevel::Warning)
			fplDebugOut("Warning: ");
		fplDebugFormatOut("%s\n", msg);
	}

	static void Info(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Info, format, argList);
		va_end(argList);
	}

	static void Verbose(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Verbose, format, argList);
		va_end(argList);
	}

	static void Warning(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Warning, format, argList);
		va_end(argList);
	}

	static void Error(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Error, format, argList);
		va_end(argList);
	}

	static void Fatal(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Fatal, format, argList);
		va_end(argList);
	}
}

// Forward declarations
namespace level {
	static Vec2i FindTilePosByEntityType(const Level &level, const EntityType type);
	static void LoadWave(GameState &state, const int waveIndex);
}
namespace game {
	static void NewGame(GameState &state);
	static void SetSlowdown(GameState &state, const float duration, const WaveState nextState);
}

namespace ui {
	static void UIBegin(UIContext &ctx, GameState *gameState, RenderState *renderState, const Input &input, const Vec2f &mousePos) {
		ctx.input = {};
		ctx.hot = 0;
		ctx.gameState = gameState;
		ctx.renderState = renderState;
		ctx.input.userPosition = mousePos;
		ctx.input.leftButton = input.mouse.left;
	}

	inline bool UIIsHot(const UIContext &ctx) {
		bool result = ctx.hot > 0;
		return(result);
	}

	inline bool UIIsActive(const UIContext &ctx) {
		bool result = ctx.active > 0;
		return(result);
	}

	inline Vec2f GetUIButtonExt(const Vec2f &radius) {
		Vec2f result = radius;
		return(result);
	}

	inline bool IsInsideButton(UIContext &ctx, const Vec2f &pos, const Vec2f &radius) {
		bool result = Abs(ctx.input.userPosition.x - pos.x) <= radius.w && Abs(ctx.input.userPosition.y - pos.y) <= radius.h;
		return(result);
	}

	enum class UIButtonState {
		None = 0,
		Hover,
		Down,
	};
	typedef void(UIButtonDrawFunction)(GameState &gameState, RenderState &renderState, const Vec2f &pos, const Vec2f &radius, const UIButtonState buttonState, void *userData);

	static bool UIButton(UIContext &ctx, const UIID &id, const Vec2f &pos, const Vec2f &radius, UIButtonDrawFunction *drawFunc, void *userData) {
		bool result = false;
		if (IsInsideButton(ctx, pos, radius)) {
			ctx.hot = id;
		}
		if (ctx.active == id) {
			if (WasPressed(ctx.input.leftButton)) {
				if (ctx.hot == id) {
					result = true;
				}
				ctx.active = 0;
			}
		} else if (ctx.hot == id) {
			if (ctx.input.leftButton.state == fplButtonState_Press) {
				ctx.active = id;
			}
		}

		UIButtonState buttonState = UIButtonState::None;
		if (ctx.hot == id) {
			if (ctx.active == ctx.hot) {
				buttonState = UIButtonState::Down;
			} else {
				buttonState = UIButtonState::Hover;
			}
		}

		drawFunc(*ctx.gameState, *ctx.renderState, pos, radius, buttonState, userData);

		return(result);
	}
}

namespace utils {
	static int StringToInt(const char *str, const int def = 0) {
		int result = def;
		if (str != nullptr) {
			bool isNegative = false;
			const char *p = str;
			if (*p == '-') {
				isNegative = true;
				++p;
			}
			uint32_t value = 0;
			while (isdigit(*p)) {
				short v = *p - '0';
				value = value * 10 + v;
				++p;
			}
			if (isNegative) {
				result = -(int)value;
			} else {
				result = value;
			}
		}
		return(result);
	}

	static float StringToFloat(const char *str, const float def = 0.0f) {
		// @TODO(final): Implement this yourself!
		float result = def;
		if (str != nullptr) {
			result = (float)atof(str);
		}
		return(result);
	}
}

namespace render {
	static void DrawTile(RenderState &renderState, const LevelDimension &dim, const int x, const int y, const bool isFilled, const Vec4f &color) {
		Vec2f pos = TileToWorld(dim, V2i(x, y));
		PushRectangle(renderState, pos, V2f(TileWidth, TileHeight), color, isFilled, 1.0f);
	}

	static void DrawLineStipple(RenderState &renderState, const Vec2f &a, const Vec2f &b, const float stippleWidth, const int modCount, const Vec4f &color, const float lineWidth) {
		assert(stippleWidth > 0);
		Vec2f ab = b - a;
		float d = Vec2Length(ab);
		Vec2f n = ab / d;
		int secCount = (d > stippleWidth) ? (int)(d / stippleWidth) : 1;
		assert(secCount > 0);
		size_t capacity = secCount * 2;
		VertexAllocation vertAlloc = AllocateVertices(renderState, capacity, color, DrawMode::Lines, false, lineWidth);
		Vec2f *p = vertAlloc.verts;
		size_t count = 0;
		for (int sec = 0; sec < secCount; ++sec) {
			float t = sec / (float)secCount;
			Vec2f start = Vec2Lerp(a, t, b);
			Vec2f end = start + n * stippleWidth;
			if (sec % modCount == 0) {
				*p++ = start;
				*p++ = end;
				count += 2;
			}
		}
		assert(count <= capacity);
		*vertAlloc.count = count;
	}

	static void DrawLineLoopStipple(RenderState &renderState, const Vec2f *points, const size_t pointCount, const float stippleWidth, const int modCount, const Vec4f &color, const float lineWidth) {
		assert(pointCount >= 2);
		for (size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
			Vec2f a = points[pointIndex];
			Vec2f b = points[(pointIndex + 1) % pointCount];
			DrawLineStipple(renderState, a, b, stippleWidth, modCount, color, lineWidth);
		}
	}
}

namespace creeps {
	static void SpawnEnemy(Creeps &enemies, const LevelDimension &dim, const Waypoints &waypoints, const Vec2f &spawnPos, const Vec2f &exitPos, const CreepData *data) {
		assert(enemies.count < FPL_ARRAYCOUNT(enemies.list));
		Creep *enemy = &enemies.list[enemies.count++];
		FPL_CLEAR_STRUCT(enemy);
		enemy->id = ++enemies.creepIdCounter;
		enemy->data = data;
		enemy->position = enemy->prevPosition = spawnPos;
		enemy->speed = data->speed;
		enemy->hp = data->hp;
		if (waypoints.first != nullptr) {
			enemy->targetWaypoint = waypoints.first;
			enemy->targetPos = TileToWorld(dim, waypoints.first->tilePos, TileExt);
		} else {
			enemy->targetWaypoint = nullptr;
			enemy->targetPos = exitPos;
		}
		enemy->facingDirection = Vec2Normalize(enemy->targetPos - enemy->position);
		enemy->hasTarget = true;
	}

	static void UpdateSpawner(GameState &state, CreepSpawner &spawner, const float deltaTime) {
		if (spawner.isActive) {
			assert(spawner.remainingCount > 0);
			assert(spawner.cooldown > 0);
			if (spawner.spawnTimer > 0) {
				spawner.spawnTimer -= deltaTime;
			}
			if (spawner.spawnTimer <= 0) {
				SpawnEnemy(state.enemies, state.level.dimension, state.waypoints, spawner.spawnPosition, spawner.exitPosition, spawner.spawnTemplate);
				--spawner.remainingCount;
				if (spawner.remainingCount == 0) {
					spawner.spawnTimer = 0;
					spawner.isActive = false;
				} else {
					spawner.spawnTimer = spawner.cooldown;
				}
			}
		}
	}

	static void AddSpawner(CreepSpawners &spawners, const LevelDimension &dim, const Vec2i &spawnTilePos, const Vec2i &goalTilePos, const float initialCooldown, const float cooldown, const size_t count, const SpawnerStartMode startMode, const CreepData *spawnTemplate) {
		assert(spawners.count < FPL_ARRAYCOUNT(spawners.list));
		CreepSpawner &spawner = spawners.list[spawners.count++];
		spawner = {};
		spawner.spawnPosition = TileToWorld(dim, spawnTilePos, TileExt);
		spawner.exitPosition = TileToWorld(dim, goalTilePos, TileExt);
		spawner.cooldown = cooldown;
		spawner.spawnTimer = initialCooldown;
		spawner.totalCount = count;
		spawner.remainingCount = count;
		spawner.spawnTemplate = spawnTemplate;
		spawner.isActive = false;
		spawner.startMode = startMode;
	}

	static void CreepDead(GameState &state, Creep &enemy) {
		enemy.id = 0;
		enemy.hasTarget = false;
		enemy.targetWaypoint = nullptr;
		enemy.isDead = true;
		enemy.hp = 0;
	}

	static void CreepReachedExit(GameState &state, Creep &enemy) {
		CreepDead(state, enemy);
		state.stats.lifes--;
		if (state.wave.isActive && state.stats.lifes <= 0) {
			state.stats.lifes = 0;
			state.wave.isActive = false;
			game::SetSlowdown(state, 6.0f, WaveState::Lost);
		}
	}

	static void SetCreepNextTarget(GameState &state, Creep &enemy) {
		const LevelDimension &dim = state.level.dimension;
		Vec2i goalTilePos = level::FindTilePosByEntityType(state.level, EntityType::Goal);
		assert(goalTilePos.x > -1 && goalTilePos.y > -1);
		Vec2i creepTilePos = WorldToTile(dim, enemy.position);
		if (enemy.targetWaypoint != nullptr) {
			const Waypoint waypoint = *enemy.targetWaypoint;
			assert(Vec2Length(waypoint.direction) == 1);
			Vec2f creepDir = waypoint.direction;
			if (waypoint.next != nullptr) {
				enemy.targetPos = TileToWorld(dim, waypoint.next->tilePos, TileExt);
				enemy.targetWaypoint = waypoint.next;
			} else {
				enemy.targetWaypoint = nullptr;
				enemy.targetPos = TileToWorld(dim, goalTilePos, TileExt);
			}
			enemy.hasTarget = true;
			enemy.facingDirection = Vec2Normalize(enemy.targetPos - enemy.position);
		} else {
			enemy.hasTarget = false;
			assert(IsVec2Equals(creepTilePos, goalTilePos));
			CreepReachedExit(state, enemy);
		}
	}

	static const CreepData *FindEnemyById(GameState &state, const char *id) {
		for (int i = 0; i < state.assets.creepDefinitionCount; ++i) {
			if (strcmp(state.assets.creepDefinitions[i].id, id) == 0) {
				return &state.assets.creepDefinitions[i];
			}
		}
		return nullptr;
	}

	static void CreepHit(GameState &state, Creep &enemy, const Bullet &bullet) {
		enemy.hp -= bullet.data->damage;
		if (enemy.hp <= 0) {
			CreepDead(state, enemy);
			state.stats.money += enemy.data->bounty;
		}
	}

	static void AllEnemiesKilled(GameState &state) {
		state.stats.money += state.assets.waveDefinitions[state.wave.activeIndex].completionBounty;
		if (state.wave.activeIndex < (state.assets.waveDefinitionCount - 1)) {
			level::LoadWave(state, state.wave.activeIndex + 1);
		} else {
			state.wave.state = WaveState::Won;
			state.wave.isActive = false;
			game::SetSlowdown(state, 6.0f, WaveState::Won);
		}
	}
}

namespace level {
	inline Tile *GetTile(Level &level, const Vec2i &tilePos) {
		if (IsValidTile(level.dimension, tilePos)) {
			int index = tilePos.y * (int)level.dimension.tileCountX + tilePos.x;
			return &level.tiles[index];
		}
		return nullptr;
	}

	static Vec2i FindTilePosByEntityType(const Level &level, const EntityType type) {
		for (size_t y = 0; y < level.dimension.tileCountY; ++y) {
			for (size_t x = 0; x < level.dimension.tileCountX; ++x) {
				size_t index = y * level.dimension.tileCountX + x;
				if (level.tiles[index].entityType == type) {
					return V2i((int)x, (int)y);
				}
			}
		}
		return V2i(-1, -1);
	}

	constexpr float WaypointDirectionWidth = 0.35f;

	static void ClearWaypoints(Waypoints &waypoints) {
		waypoints.first = waypoints.last = nullptr;
		waypoints.used = 0;
	}

	static Waypoint *AddWaypoint(Waypoints &waypoints, const LevelDimension &dim, const Vec2i &tilePos, const Vec2f &dir) {
		assert(waypoints.used < FPL_ARRAYCOUNT(waypoints.freeList));
		Waypoint *waypoint = &waypoints.freeList[waypoints.used++];
		waypoint->tilePos = tilePos;
		waypoint->position = TileToWorld(dim, tilePos, TileExt);
		waypoint->direction = dir;
		if (waypoints.first == nullptr) {
			waypoints.first = waypoints.last = waypoint;
		} else {
			waypoints.last->next = waypoint;
			waypoints.last = waypoint;
		}
		return(waypoint);
	}

	static void ParseLevelLayer(fxmlTag *childTag, LevelLayer *targetLayer) {
		const char *layerName = fxmlGetAttributeValue(childTag, "name");
		fplCopyAnsiString(layerName, targetLayer->name, FPL_ARRAYCOUNT(targetLayer->name));
		targetLayer->mapWidth = utils::StringToInt(fxmlGetAttributeValue(childTag, "width"));
		targetLayer->mapHeight = utils::StringToInt(fxmlGetAttributeValue(childTag, "height"));
		targetLayer->data = (uint32_t *)fplMemoryAllocate(sizeof(uint32_t) * targetLayer->mapWidth * targetLayer->mapHeight);
		targetLayer->opacity = utils::StringToFloat(fxmlGetAttributeValue(childTag, "opacity"), 1.0f);

		fxmlTag *dataTag = fxmlFindTagByName(childTag, "data");
		if (dataTag != nullptr) {
			const char *encodingStr = fxmlGetAttributeValue(dataTag, "encoding");
			if (strcmp(encodingStr, "csv") == 0) {
				const char *p = dataTag->value;
				int index = 0;
				while (*p) {
					if (isdigit(*p)) {
						uint32_t tileValue = 0;
						while (isdigit(*p)) {
							short v = *p - '0';
							tileValue = tileValue * 10 + v;
							++p;
						}
						int row = index / (int)targetLayer->mapWidth;
						int col = index % (int)targetLayer->mapWidth;
						assert(row >= 0 && row < (int)targetLayer->mapHeight);
						assert(col >= 0 && col < (int)targetLayer->mapWidth);
						int tileIndex = row * targetLayer->mapWidth + col;
						targetLayer->data[tileIndex] = tileValue;
					} else if (*p == ',') {
						++p;
						++index;
					} else {
						++p;
					}
				}
			}
		}
	}

	static LevelTileset *FindLevelTileset(LevelData &level, const char *name) {
		for (size_t i = 0; i < level.tilesetCount; ++i) {
			if (fplIsStringEqual(level.tilesets[i].name, name)) {
				return &level.tilesets[i];
			}
		}
		return nullptr;
	}

	static void ParseLevelObjects(fxmlTag *objectGroupTag, LevelData &level) {
		assert(level.tileWidth > 0);
		assert(level.tileHeight > 0);
		LevelTileset *entitiesTileset = FindLevelTileset(level, "entities");
		assert(entitiesTileset != nullptr);
		fxmlTag *childTag = objectGroupTag->firstChild;
		while (childTag != nullptr) {
			if (childTag->type == fxmlTagType_Element) {
				if (strcmp(childTag->name, "object") == 0) {
					int gid = utils::StringToInt(fxmlGetAttributeValue(childTag, "gid"));
					int x = utils::StringToInt(fxmlGetAttributeValue(childTag, "x"), -1);
					int y = utils::StringToInt(fxmlGetAttributeValue(childTag, "y"), -1);
					int w = utils::StringToInt(fxmlGetAttributeValue(childTag, "width"), 0);
					int h = utils::StringToInt(fxmlGetAttributeValue(childTag, "height"), 0);
					float cx = (float)x + (float)w * 0.5f;
					float cy = (float)y - (float)h * 0.5f;
					int tileX = (int)floorf(cx / (float)level.tileWidth);
					int tileY = (int)floorf(cy / (float)level.tileHeight);
					Vec2i tilePos = V2i(tileX, tileY);

					uint32_t tileId = (gid > 0) ? ((gid - entitiesTileset->firstGid) + 1) : 0;
					EntityType entityType = TilesetEntitiesToTypeMapping[tileId];

					ObjectData tmpObj = {};
					tmpObj.tilePos = tilePos;
					const char *typeName = fxmlGetAttributeValue(childTag, "type");
					const char *objName = fxmlGetAttributeValue(childTag, "name");
					if (strcmp(typeName, "Spawn") == 0) {
						tmpObj.type = ObjectType::Spawn;
						fplCopyAnsiString(objName, tmpObj.spawn.spawnId, FPL_ARRAYCOUNT(tmpObj.spawn.spawnId));
						switch (entityType) {
							case EntityType::SpawnDown:
								tmpObj.spawn.direction = V2f(0.0f, -1.0f);
								break;
							case EntityType::SpawnUp:
								tmpObj.spawn.direction = V2f(0.0f, 1.0f);
								break;
							case EntityType::SpawnLeft:
								tmpObj.spawn.direction = V2f(-1.0f, 0.0f);
								break;
							case EntityType::SpawnRight:
								tmpObj.spawn.direction = V2f(1.0f, 0.0f);
								break;
							default:
								assert(!"Unsupported entity type for spawn!");
								break;
						}

						fxmlTag *propertiesTag = fxmlFindTagByName(childTag, "properties");
						if (propertiesTag != nullptr) {
							fxmlTag *propTag = propertiesTag->firstChild;
							while (propTag != nullptr) {
								const char *name = fxmlGetAttributeValue(propTag, "name");
								const char *value = fxmlGetAttributeValue(propTag, "value");
								propTag = propTag->nextSibling;
							}
						}
					} else if (strcmp(typeName, "Waypoint") == 0) {
						tmpObj.type = ObjectType::Waypoint;
						switch (entityType) {
							case EntityType::WaypointDown:
								tmpObj.waypoint.direction = V2f(0.0f, -1.0f);
								break;
							case EntityType::WaypointUp:
								tmpObj.waypoint.direction = V2f(0.0f, 1.0f);
								break;
							case EntityType::WaypointLeft:
								tmpObj.waypoint.direction = V2f(-1.0f, 0.0f);
								break;
							case EntityType::WaypointRight:
								tmpObj.waypoint.direction = V2f(1.0f, 0.0f);
								break;
							default:
								assert(!"Unsupported entity type for waypoint!");
								break;
						}
					} else if (strcmp(typeName, "Goal") == 0) {
						tmpObj.type = ObjectType::Goal;
					}

					if (tmpObj.type != ObjectType::None) {
						assert(level.objectCount < FPL_ARRAYCOUNT(level.objects));
						ObjectData &obj = level.objects[level.objectCount++];
						obj = tmpObj;
					}
				}
			}
			childTag = childTag->nextSibling;
		}
	}

	static bool ParseTileset(fxmlTag *tilesetTag, LevelTileset &outTileset) {
		const char *name = fxmlGetAttributeValue(tilesetTag, "name");
		fplCopyAnsiString(name, outTileset.name, FPL_ARRAYCOUNT(outTileset.name));
		outTileset.firstGid = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "firstgid"));
		outTileset.tileWidth = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "tilewidth"));
		outTileset.tileHeight = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "tileheight"));
		outTileset.tileCount = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "tilecount"));
		outTileset.columns = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "columns"));
		outTileset.tileUVs = nullptr;
		fxmlTag *imageTag = fxmlFindTagByName(tilesetTag, "image");
		if (imageTag != nullptr) {
			const char *source = fxmlGetAttributeValue(imageTag, "source");
			fplCopyAnsiString(source, outTileset.image.source, FPL_ARRAYCOUNT(outTileset.image.source));
			outTileset.image.width = utils::StringToInt(fxmlGetAttributeValue(imageTag, "width"));
			outTileset.image.height = utils::StringToInt(fxmlGetAttributeValue(imageTag, "height"));
		}
		if ((outTileset.tileCount > 0 && outTileset.columns > 0) &&
			(outTileset.image.width > 0 && outTileset.image.height > 0) &&
			(outTileset.tileWidth > 0 && outTileset.tileHeight > 0)) {
			outTileset.tileUVs = (UVRect *)fplMemoryAllocate(sizeof(UVRect) * outTileset.tileCount);
			Vec2i tileSize = V2i(outTileset.tileWidth, outTileset.tileHeight);
			Vec2i imageSize = V2i(outTileset.image.width, outTileset.image.height);
			UVRect *p = outTileset.tileUVs;
			int rowCount = (int)outTileset.tileCount / outTileset.columns;
			for (size_t tileIndex = 0; tileIndex < outTileset.tileCount; ++tileIndex) {
				int tileY = (int)(tileIndex / outTileset.columns);
				int tileX = (int)(tileIndex % outTileset.columns);
				*p++ = UVRectFromTile(imageSize, tileSize, 0, V2i(tileX, rowCount - 1 - tileY));
			}
		}
		return(true);
	}

	static bool ParseLevel(fxmlTag *root, LevelData &level) {
		bool result = false;
		fxmlTag *mapTag = fxmlFindTagByName(root, "map");
		if (mapTag == nullptr) {
			return false;
		}
		level.mapWidth = utils::StringToInt(fxmlGetAttributeValue(mapTag, "width"));
		level.mapHeight = utils::StringToInt(fxmlGetAttributeValue(mapTag, "height"));

		const char *orientation = fxmlGetAttributeValue(mapTag, "orientation");
		if (strcmp(orientation, "orthogonal") != 0) {
			return false;
		}

		int tileWidth = utils::StringToInt(fxmlGetAttributeValue(mapTag, "tilewidth"));
		int tileHeight = utils::StringToInt(fxmlGetAttributeValue(mapTag, "tileheight"));
		if ((tileWidth == 0) || (tileHeight == 0)) {
			return false;
		}

		level.tileWidth = tileWidth;
		level.tileHeight = tileHeight;

		level.tilesetCount = 0;
		level.layerCount = 0;
		level.objectCount = 0;

		fxmlTag *childTag = mapTag->firstChild;
		while (childTag) {
			if (childTag->type == fxmlTagType_Element) {
				if (strcmp(childTag->name, "tileset") == 0) {
					assert(level.tilesetCount < FPL_ARRAYCOUNT(level.tilesets));
					LevelTileset *targetTileset = &level.tilesets[level.tilesetCount++];
					ParseTileset(childTag, *targetTileset);
				} else if (strcmp(childTag->name, "layer") == 0) {
					assert(level.layerCount < MAX_LAYER_COUNT);
					LevelLayer *targetLayer = &level.layers[level.layerCount++];
					ParseLevelLayer(childTag, targetLayer);
				} else if (strcmp(childTag->name, "objectgroup") == 0) {
					const char *objectGroupName = fxmlGetAttributeValue(childTag, "name");
					if (strcmp(objectGroupName, "objects") == 0) {
						ParseLevelObjects(childTag, level);
					}
				}
			}
			childTag = childTag->nextSibling;
		}

		return(true);
	}

	static FileContents LoadEntireFile(const char *filePath) {
		FileContents result = {};
		fplFileHandle file;
		if (fplOpenAnsiBinaryFile(filePath, &file)) {
			result.size = fplGetFileSizeFromHandle32(&file);
			result.data = (uint8_t *)fplMemoryAllocate(result.size);
			fplReadFileBlock32(&file, (uint32_t)result.size, result.data, (uint32_t)result.size);
			fplCloseFile(&file);
		}
		return(result);
	}

	static const char *FindNodeValue(fxmlTag *rootTag, const char *nodeName) {
		fxmlTag *foundTag = fxmlFindTagByName(rootTag, nodeName);
		if (foundTag != nullptr) {
			return foundTag->value;
		}
		return nullptr;
	}

	static void LoadCreepDefinitions(Assets &assets, const char *filename) {
		assets.creepDefinitionCount = 0;
		char filePath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 3, assets.dataPath, "levels", filename);
		FileContents fileData = LoadEntireFile(filePath);
		if (fileData.data != nullptr) {
			fxmlContext ctx = {};
			if (fxmlInitFromMemory(fileData.data, fileData.size, &ctx)) {
				fxmlTag root = {};
				if (fxmlParse(&ctx, &root)) {
					fxmlTag *creepDefinitionsTag = fxmlFindTagByName(&root, "CreepDefinitions");
					if (creepDefinitionsTag != nullptr) {
						for (fxmlTag *creepTag = creepDefinitionsTag->firstChild; creepTag; creepTag = creepTag->nextSibling) {
							if (strcmp("CreepData", creepTag->name) == 0) {
								const char *creepId = fxmlGetAttributeValue(creepTag, "id");
								assert(assets.creepDefinitionCount < FPL_ARRAYCOUNT(assets.creepDefinitions));
								CreepData *creepData = &assets.creepDefinitions[assets.creepDefinitionCount++];
								*creepData = {};
								fplCopyAnsiString(creepId, creepData->id, FPL_ARRAYCOUNT(creepData->id));
								creepData->renderRadius = MaxTileSize * utils::StringToFloat(FindNodeValue(creepTag, "renderRadius"));
								creepData->collisionRadius = MaxTileSize * utils::StringToFloat(FindNodeValue(creepTag, "collisionRadius"));
								creepData->speed = utils::StringToFloat(FindNodeValue(creepTag, "speed"));
								creepData->hp = utils::StringToInt(FindNodeValue(creepTag, "hp"));
								creepData->bounty = utils::StringToInt(FindNodeValue(creepTag, "bounty"));
								creepData->color = V4f(1, 1, 1, 1);
							}
						}
					}
				}
			}
			fplMemoryFree(fileData.data);
		}
	}

	static void LoadTowerDefinitions(Assets &assets, const char *filename) {
		assets.towerDefinitionCount = 0;
		char filePath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 3, assets.dataPath, "levels", filename);
		FileContents fileData = LoadEntireFile(filePath);
		if (fileData.data != nullptr) {
			fxmlContext ctx = {};
			if (fxmlInitFromMemory(fileData.data, fileData.size, &ctx)) {
				fxmlTag root = {};
				if (fxmlParse(&ctx, &root)) {
					fxmlTag *towerDefinitionsTag = fxmlFindTagByName(&root, "TowerDefinitions");
					if (towerDefinitionsTag != nullptr) {
						for (fxmlTag *towerTag = towerDefinitionsTag->firstChild; towerTag; towerTag = towerTag->nextSibling) {
							if (strcmp("TowerData", towerTag->name) == 0) {
								const char *towerId = fxmlGetAttributeValue(towerTag, "id");
								assert(assets.towerDefinitionCount < FPL_ARRAYCOUNT(assets.towerDefinitions));
								TowerData *towerData = &assets.towerDefinitions[assets.towerDefinitionCount++];
								*towerData = {};
								fplCopyAnsiString(towerId, towerData->id, FPL_ARRAYCOUNT(towerData->id));
								towerData->detectionRadius = MaxTileSize * utils::StringToFloat(FindNodeValue(towerTag, "detectionRadius"));
								towerData->unlockRadius = MaxTileSize * utils::StringToFloat(FindNodeValue(towerTag, "unlockRadius"));
								towerData->structureRadius = MaxTileSize * utils::StringToFloat(FindNodeValue(towerTag, "structureRadius"));
								towerData->gunTubeLength = MaxTileSize * utils::StringToFloat(FindNodeValue(towerTag, "gunTubeLength"));
								towerData->gunCooldown = utils::StringToFloat(FindNodeValue(towerTag, "gunCooldown"));
								towerData->gunTubeThickness = MaxTileSize * utils::StringToFloat(FindNodeValue(towerTag, "gunTubeThickness"));
								towerData->gunRotationSpeed = utils::StringToFloat(FindNodeValue(towerTag, "gunRotationSpeed"));
								towerData->enemyRangeTestType = FireRangeTestType::InSight;
								towerData->enemyPredictionFlags = EnemyPredictionFlags::All;
								towerData->enemyLockOnMode = EnemyLockTargetMode::LockedOn;
								towerData->costs = utils::StringToInt(fxmlGetAttributeValue(towerTag, "costs"));
								fxmlTag *bulletTag = fxmlFindTagByName(towerTag, "bullet");
								if (bulletTag != nullptr) {
									towerData->bullet.renderRadius = MaxTileSize * utils::StringToFloat(FindNodeValue(bulletTag, "renderRadius"));
									towerData->bullet.collisionRadius = MaxTileSize * utils::StringToFloat(FindNodeValue(bulletTag, "collisionRadius"));
									towerData->bullet.speed = utils::StringToFloat(FindNodeValue(bulletTag, "speed"));
									towerData->bullet.damage = utils::StringToInt(FindNodeValue(bulletTag, "damage"));
								}
							}
						}
					}
				}
			}
			fplMemoryFree(fileData.data);
		}
	}

	static void LoadWaveDefinitions(Assets &assets, const char *filename) {
		assets.waveDefinitionCount = 0;
		char filePath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 3, assets.dataPath, "levels", filename);
		FileContents fileData = LoadEntireFile(filePath);
		if (fileData.data != nullptr) {
			fxmlContext ctx = {};
			if (fxmlInitFromMemory(fileData.data, fileData.size, &ctx)) {
				fxmlTag root = {};
				if (fxmlParse(&ctx, &root)) {
					fxmlTag *waveDefinitionsTag = fxmlFindTagByName(&root, "WaveDefinitions");
					if (waveDefinitionsTag != nullptr) {
						for (fxmlTag *waveTag = waveDefinitionsTag->firstChild; waveTag; waveTag = waveTag->nextSibling) {
							if (strcmp("WaveData", waveTag->name) == 0) {
								const char *levelId = fxmlGetAttributeValue(waveTag, "level");
								assert(assets.waveDefinitionCount < FPL_ARRAYCOUNT(assets.waveDefinitions));
								WaveData *waveData = &assets.waveDefinitions[assets.waveDefinitionCount++];
								*waveData = {};
								fplCopyAnsiString(levelId, waveData->levelId, FPL_ARRAYCOUNT(waveData->levelId));
								waveData->startupCooldown = utils::StringToFloat(FindNodeValue(waveTag, "startupCooldown"));
								waveData->spawnerCount = 0;
								waveData->completionBounty = utils::StringToInt(FindNodeValue(waveTag, "completionBounty"));
								fxmlTag *spawnersTag = fxmlFindTagByName(waveTag, "spawners");
								if (spawnersTag != nullptr) {
									for (fxmlTag *spawnTag = spawnersTag->firstChild; spawnTag; spawnTag = spawnTag->nextSibling) {
										if (strcmp("SpawnData", spawnTag->name) == 0) {
											assert(waveData->spawnerCount < FPL_ARRAYCOUNT(waveData->spawners));
											SpawnData *spawnData = &waveData->spawners[waveData->spawnerCount++];
											const char *spawnId = fxmlGetAttributeValue(spawnTag, "id");
											const char *enemyId = fxmlGetAttributeValue(spawnTag, "enemy");
											fplCopyAnsiString(spawnId, spawnData->spawnId, FPL_ARRAYCOUNT(spawnData->spawnId));
											fplCopyAnsiString(enemyId, spawnData->enemyId, FPL_ARRAYCOUNT(spawnData->enemyId));
											spawnData->initialCooldown = utils::StringToFloat(FindNodeValue(spawnTag, "initialCooldown"));
											spawnData->cooldown = utils::StringToFloat(FindNodeValue(spawnTag, "cooldown"));
											spawnData->enemyCount = utils::StringToInt(FindNodeValue(spawnTag, "enemyCount"));
											const char *startModeString = FindNodeValue(spawnTag, "startMode");
											if (strcmp("AfterTheLast", startModeString) == 0) {
												spawnData->startMode = SpawnerStartMode::AfterTheLast;
											} else {
												spawnData->startMode = SpawnerStartMode::Fixed;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			fplMemoryFree(fileData.data);
		}
	}

	static LevelLayer *FindLayerByName(LevelData &level, const char *name) {
		for (size_t i = 0; i < level.layerCount; ++i) {
			if (fplIsStringEqual(level.layers[i].name, name)) {
				return &level.layers[i];
			}
		}
		return nullptr;
	}

	static bool LoadLevel(GameState &state, const char *dataPath, const char *filename, LevelData &outLevel) {
		bool result = false;

		char filePath[1024];
		fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 3, dataPath, "levels", filename);

		gamelog::Verbose("Loading level '%s'", filePath);

		FileContents fileData = LoadEntireFile(filePath);
		if (fileData.data != nullptr) {
			fxmlContext ctx = {};
			if (fxmlInitFromMemory(fileData.data, fileData.size, &ctx)) {
				fxmlTag root = {};
				if (fxmlParse(&ctx, &root)) {
					outLevel = {};
					if (ParseLevel(&root, outLevel)) {
						LevelLayer *wayLayer = FindLayerByName(outLevel, "way");
						assert(wayLayer != nullptr);

						// Tiles
						LevelTileset *wayTileset = FindLevelTileset(outLevel, "way");
						assert(wayTileset != nullptr);
						assert(state.level.tiles == nullptr);
						state.level.dimension.tileCountX = outLevel.mapWidth;
						state.level.dimension.tileCountY = outLevel.mapHeight;
						state.level.dimension.gridWidth = outLevel.mapWidth * TileWidth;
						state.level.dimension.gridHeight = outLevel.mapHeight * TileHeight;
						state.level.dimension.gridOriginX = -WorldRadiusW + ((WorldWidth - state.level.dimension.gridWidth) * 0.5f);
						state.level.dimension.gridOriginY = -WorldRadiusH + ControlsHeight;
						state.level.tiles = (Tile *)fplMemoryAllocate(sizeof(Tile) * outLevel.mapWidth * outLevel.mapHeight);
						for (size_t y = 0; y < outLevel.mapHeight; ++y) {
							for (size_t x = 0; x < outLevel.mapWidth; ++x) {
								size_t tileIndex = y * outLevel.mapWidth + x;
								uint32_t wayValue = wayLayer->data[tileIndex] > 0 ? ((wayLayer->data[tileIndex] - wayTileset->firstGid) + 1) : 0;
								Tile tile = {};
								tile.wayType = TilesetWayToTypeMapping[wayValue];
								tile.entityType = EntityType::None;
								state.level.tiles[tileIndex] = tile;
							}
						}

						// Make waypoints/goal
						for (size_t objIndex = 0; objIndex < outLevel.objectCount; ++objIndex) {
							const ObjectData &obj = outLevel.objects[objIndex];
							if (IsValidTile(state.level.dimension, obj.tilePos)) {
								int tileIndex = obj.tilePos.y * outLevel.mapWidth + obj.tilePos.x;
								switch (obj.type) {
									case ObjectType::Goal:
										state.level.tiles[tileIndex].entityType = EntityType::Goal;
										break;
									case ObjectType::Waypoint:
										AddWaypoint(state.waypoints, state.level.dimension, obj.tilePos, obj.waypoint.direction);
										break;
									default:
										break;
								}
							}
						}

						result = true;
					} else {
						gamelog::Error("Level file '%s' is not valid!", filePath);
					}
				} else {
					gamelog::Error("Level file '%s' is not a valid XML file!", filePath);
				}
				fxmlFree(&ctx);
			}
			fplMemoryFree(fileData.data);
		} else {
			gamelog::Error("Level file '%s' could not be found!", filePath);
		}

		return(result);
	}

	static void ClearWave(GameState &state) {
		gamelog::Verbose("Clear Wave");
		state.wave.totalEnemyCount = 0;
		state.wave.isActive = false;
		state.enemies.count = 0;
		state.spawners.count = 0;
		for (size_t towerIndex = 0; towerIndex < state.towers.activeCount; ++towerIndex) {
			Tower &tower = state.towers.activeList[towerIndex];
			tower.hasTarget = false;
			tower.targetEnemy = nullptr;
			tower.targetId = 0;
		}
	}

	static void FreeLevel(Level &level) {
		for (size_t i = 0; i < level.data.layerCount; ++i) {
			if (level.data.layers[i].data != nullptr) {
				fplMemoryFree(level.data.layers[i].data);
				level.data.layers[i].data = nullptr;
			}
		}
		for (size_t i = 0; i < level.data.tilesetCount; ++i) {
			if (level.data.tilesets[i].tileUVs != nullptr) {
				fplMemoryFree(level.data.tilesets[i].tileUVs);
				level.data.tilesets[i].tileUVs = nullptr;
			}
		}
		if (level.tiles != nullptr) {
			fplMemoryFree(level.tiles);
			level.tiles = nullptr;
		}
		level.data.layerCount = 0;
		level.data.tilesetCount = 0;
		level.data.objectCount = 0;
	}

	static void ClearLevel(GameState &state) {
		gamelog::Verbose("Clear Level");
		state.towers.activeCount = 0;
		state.towers.selectedIndex = -1;
		ClearWave(state);
		ClearWaypoints(state.waypoints);
		FreeLevel(state.level);
	}

	static const ObjectData *FindSpawnObjectById(const Level &level, const char *spawnId) {
		for (size_t objectIndex = 0; objectIndex < level.data.objectCount; ++objectIndex) {
			const ObjectData *obj = &level.data.objects[objectIndex];
			if (obj->type == ObjectType::Spawn) {
				if (fplIsStringEqual(obj->spawn.spawnId, spawnId)) {
					return(obj);
				}
			}
		}
		return(nullptr);
	}

	static void LoadWave(GameState &state, const int waveIndex) {
		const WaveData &wave = state.assets.waveDefinitions[waveIndex];

		state.wave.state = WaveState::Stopped;

		gamelog::Verbose("Setup wave '%d'", waveIndex);

		if (state.level.activeId == nullptr || strcmp(state.level.activeId, wave.levelId) != 0) {
			gamelog::Verbose("Active level '%s' is different from '%s'", state.level.activeId, wave.levelId);
			ClearLevel(state);
			char levelFilename[1024];
			fplCopyAnsiString(wave.levelId, levelFilename, FPL_ARRAYCOUNT(levelFilename));
			fplChangeFileExtension(levelFilename, ".tmx", levelFilename, FPL_ARRAYCOUNT(levelFilename));
			if (LoadLevel(state, state.assets.dataPath, levelFilename, state.level.data)) {
				fplCopyAnsiString(wave.levelId, state.level.activeId, FPL_ARRAYCOUNT(state.level.activeId));
			} else {
				gamelog::Error("Failed loading level '%s'!", levelFilename);
				return;
			}
			state.towers.selectedIndex = 0;
		}

		if (state.wave.totalEnemyCount > 0 || state.waypoints.first != nullptr || state.spawners.count > 0) {
			ClearWave(state);
		}

		if (fplGetAnsiStringLength(state.level.activeId) == 0) {
			gamelog::Error("No level loaded!");
			return;
		}

		Vec2i goalTilePos = level::FindTilePosByEntityType(state.level, EntityType::Goal);
		if (!IsValidTile(state.level.dimension, goalTilePos)) {
			gamelog::Error("No goal entity in level '%s' found!", state.level.activeId);
			return;
		}

		state.wave.activeIndex = waveIndex;
		state.wave.totalEnemyCount = 0;
		for (size_t objectIndex = 0; objectIndex < state.level.data.objectCount; ++objectIndex) {
			ObjectData &obj = state.level.data.objects[objectIndex];
			Vec2i objTilePos = obj.tilePos;
			if (!IsValidTile(state.level.dimension, objTilePos)) {
				gamelog::Warning("Invalid tile position '%d x %d for Object '%zu:%s'!", objTilePos.x, objTilePos.y, objectIndex, ObjectTypeToString(obj.type));
				continue;
			}
		}

		for (size_t spawnerIndex = 0; spawnerIndex < wave.spawnerCount; ++spawnerIndex) {
			const SpawnData &spawnerFromWave = wave.spawners[spawnerIndex];
			if (spawnerFromWave.enemyCount == 0) {
				continue;
				gamelog::Warning("No enemies for Spawner '%s'!", spawnerFromWave.spawnId);
			}
			const ObjectData *spawnObj = FindSpawnObjectById(state.level, spawnerFromWave.spawnId);
			if (spawnObj == nullptr) {
				continue;
				gamelog::Warning("Spawner by id '%s' does not exists!", spawnerFromWave.spawnId);
			}
			Vec2i objTilePos = spawnObj->tilePos;
			if (!IsValidTile(state.level.dimension, objTilePos)) {
				gamelog::Warning("Invalid tile position '%d x %d for Spawner '%s'!", objTilePos.x, objTilePos.y, spawnObj->spawn.spawnId);
				continue;
			}
			const CreepData *creepData = creeps::FindEnemyById(state, spawnerFromWave.enemyId);
			if (creepData == nullptr) {
				continue;
				gamelog::Warning("Enemy by id '%s' does not exists!", spawnerFromWave.enemyId);
			}
			creeps::AddSpawner(state.spawners, state.level.dimension, objTilePos, goalTilePos, spawnerFromWave.initialCooldown, spawnerFromWave.cooldown, spawnerFromWave.enemyCount, spawnerFromWave.startMode, creepData);
			state.wave.totalEnemyCount += spawnerFromWave.enemyCount;
		}

		state.wave.state = WaveState::Starting;
		state.wave.isActive = true;
		state.wave.warmupTimer = wave.startupCooldown;
	}
}

namespace towers {
	enum class CanPlaceTowerResult {
		Success = 0,
		NoTowerSelected,
		TooManyTowers,
		TileOccupied,
		NotEnoughMoney,
	};

	inline CanPlaceTowerResult CanPlaceTower(GameState &state, const Vec2i &tilePos, const TowerData *tower) {
		if ((state.towers.selectedIndex < 0) || !(state.towers.selectedIndex < state.assets.towerDefinitionCount)) {
			return CanPlaceTowerResult::NoTowerSelected;
		}
		if (state.towers.activeCount == FPL_ARRAYCOUNT(state.towers.activeList)) {
			return CanPlaceTowerResult::TooManyTowers;
		}
		Tile *tile = level::GetTile(state.level, tilePos);
		if (tile == nullptr) {
			return CanPlaceTowerResult::TileOccupied;
		}
		if (tile->isOccupied || tile->entityType != EntityType::None || tile->wayType != WayType::None) {
			return CanPlaceTowerResult::TileOccupied;
		}
		if (state.stats.money < tower->costs) {
			return CanPlaceTowerResult::NotEnoughMoney;
		}
		return(CanPlaceTowerResult::Success);
	}

	static Tower *PlaceTower(GameState &state, const Vec2i &tilePos, const TowerData *data) {
		assert(state.towers.activeCount < FPL_ARRAYCOUNT(state.towers.activeList));
		Tower *tower = &state.towers.activeList[state.towers.activeCount++];
		*tower = {};
		tower->data = data;
		tower->position = TileToWorld(state.level.dimension, tilePos, TileExt);
		tower->facingAngle = (float)M_PI * 0.5f; // Face north by default

		Tile *tile = level::GetTile(state.level, tilePos);
		assert(!tile->isOccupied);
		tile->isOccupied = true;

		assert(state.stats.money >= data->costs);
		state.stats.money -= data->costs;

		return(tower);
	}

	static Vec2f PredictEnemyPosition(const Tower &tower, const Creep &enemy, const float deltaTime) {
		Vec2f result;

		// First we compute how many frames we need until we can actually fire (Weapon cooldown)
		if (tower.data->enemyPredictionFlags != EnemyPredictionFlags::None) {
			float framesRequiredToFire;
			if ((tower.data->enemyPredictionFlags & EnemyPredictionFlags::WeaponCooldown) == EnemyPredictionFlags::WeaponCooldown) {
				framesRequiredToFire = (tower.gunTimer / deltaTime);
			} else {
				framesRequiredToFire = 0;
			}
			float timeScale = 1.0f / Max(framesRequiredToFire, 1.0f);

			// Now we predict the enemy position based on the enemy speed and the number of frames required to fire
			Vec2f velocity = enemy.facingDirection * (enemy.speed * 0.5f * deltaTime);
			Vec2f predictedPosition = enemy.position + velocity / deltaTime;
			Vec2f distanceToEnemy = predictedPosition - tower.position;

			// Second we compute how many frames we need the bullet to move to the predicted position
			float framesRequiredForBullet;
			if ((tower.data->enemyPredictionFlags & EnemyPredictionFlags::BulletDistance) == EnemyPredictionFlags::BulletDistance) {
				assert(tower.data->bullet.speed > 0);
				float bulletDistance = Vec2Length(distanceToEnemy) / (tower.data->bullet.speed / deltaTime);
				framesRequiredForBullet = (bulletDistance / deltaTime);
			} else {
				framesRequiredForBullet = 0;
			}

			// Now we recompute the time scale and the predicted enemy position
			timeScale = 1.0f / Max(framesRequiredToFire + framesRequiredForBullet, 1.0f);
			velocity = enemy.facingDirection * (enemy.speed * 0.5f * deltaTime);
			result = enemy.position + velocity / deltaTime;
		} else {
			result = enemy.position;
		}
		return(result);
	}

	static bool InFireRange(const Tower &tower, const Creep &enemy, const float deltaTime) {
		Vec2f lookDirection = Vec2AngleToAxis(tower.facingAngle);
		Vec2f predictedEnemyPosition = PredictEnemyPosition(tower, enemy, deltaTime);
		Vec2f distanceToEnemy = predictedEnemyPosition - tower.position;
		bool result;
		if (tower.data->enemyRangeTestType == FireRangeTestType::LineTrace) {
			float maxDistance = Vec2Length(distanceToEnemy) + enemy.data->collisionRadius;
			LineCastInput input = {};
			input.p1 = tower.position + lookDirection * tower.data->gunTubeLength;
			input.p2 = input.p1 + lookDirection * maxDistance;
			input.maxFraction = 1.0f;
			LineCastOutput output = {};
			result = LineCastCircle(input, enemy.position, enemy.data->collisionRadius, output);
		} else {
			float projDistance = Vec2Dot(distanceToEnemy, lookDirection);
			if (projDistance > 0) {
				Vec2f lookPos = tower.position + lookDirection * projDistance;
				float dot = Vec2Dot(predictedEnemyPosition, lookPos);
				float det = predictedEnemyPosition.x * lookPos.y - predictedEnemyPosition.y * lookPos.x;
				float angle = ArcTan2(det, dot);
				result = angle >= -ShotAngleTolerance && angle <= ShotAngleTolerance;
			} else {
				result = false;
			}
		}
		return(result);
	}

	static void ShootBullet(Bullets &bullets, Tower &tower) {
		assert(bullets.count < FPL_ARRAYCOUNT(bullets.list));
		Bullet *bullet = &bullets.list[bullets.count++];
		*bullet = {};
		Vec2f targetDir = V2f(Cosine(tower.facingAngle), Sine(tower.facingAngle));
		bullet->position = bullet->prevPosition = tower.position + targetDir * tower.data->gunTubeLength;
		bullet->data = &tower.data->bullet;
		bullet->velocity = targetDir * bullet->data->speed;
		tower.canFire = false;
		tower.gunTimer = tower.data->gunCooldown;
	}

	static void UpdateTower(GameState &state, Tower &tower, const float deltaTime) {
		// Remove lost or dead target
		// @NOTE(final): Dead enemies can be immediately reused in the next frame, so we cannot use isDead only
		if (tower.hasTarget) {
			assert(tower.targetEnemy != nullptr);
			Vec2f distance = tower.targetEnemy->position - tower.position;
			assert(tower.data->unlockRadius >= tower.data->detectionRadius);
			if (tower.targetEnemy->isDead ||
				(tower.targetEnemy->id != tower.targetId) ||
				(Vec2Length(distance) > tower.data->unlockRadius)) {
				tower.targetEnemy = nullptr;
				tower.hasTarget = false;
				tower.targetId = 0;
			}
		}

		// Detect a new target
		if (!tower.hasTarget) {
			float bestEnemyDistance = FLT_MAX;
			Creep *bestEnemy = nullptr;
			for (size_t enemyIndex = 0; enemyIndex < state.enemies.count; ++enemyIndex) {
				Creep *testEnemy = &state.enemies.list[enemyIndex];
				if (!testEnemy->isDead) {
					float distanceRadius = Vec2Length(testEnemy->position - tower.position);
					if (distanceRadius < bestEnemyDistance) {
						bestEnemy = testEnemy;
						bestEnemyDistance = distanceRadius;
					}
				}
			}
			if (bestEnemy != nullptr && bestEnemyDistance <= tower.data->detectionRadius) {
				tower.targetEnemy = bestEnemy;
				tower.targetId = bestEnemy->id;
				tower.hasTarget = true;
			}
		}

		// Weapon cooldown
		if (!tower.canFire && tower.gunTimer > 0) {
			tower.gunTimer -= deltaTime;
		} else {
			tower.gunTimer = 0;
			tower.canFire = true;
		}

		//
		// Rotate gun
		//
		if (tower.hasTarget) {
			assert(tower.targetEnemy != nullptr);
			Creep *enemy = tower.targetEnemy;
			assert(enemy->id == tower.targetId);
			Vec2f predictedEnemyPosition = towers::PredictEnemyPosition(tower, *enemy, deltaTime);
			Vec2f directionToEnemy = Vec2Normalize(predictedEnemyPosition - tower.position);
			float angleToEnemy = Vec2AxisToAngle(directionToEnemy);
			tower.facingAngle = AngleLerp(tower.facingAngle, deltaTime * tower.data->gunRotationSpeed, angleToEnemy);
		}

		//
		// Shoot
		//
		if (tower.data->enemyLockOnMode == EnemyLockTargetMode::Any) {
			for (size_t enemyIndex = 0; enemyIndex < state.enemies.count; ++enemyIndex) {
				Creep &enemy = state.enemies.list[enemyIndex];
				if (!enemy.isDead) {
					bool inFireRange = towers::InFireRange(tower, enemy, deltaTime);
					if (inFireRange && tower.canFire) {
						ShootBullet(state.bullets, tower);
					}
				}
			}
		} else if (tower.data->enemyLockOnMode == EnemyLockTargetMode::LockedOn) {
			if (tower.hasTarget) {
				assert(tower.targetEnemy != nullptr);
				Creep *enemy = tower.targetEnemy;
				assert(!enemy->isDead);
				bool inFireRange = towers::InFireRange(tower, *enemy, deltaTime);
				if (inFireRange && tower.canFire) {
					ShootBullet(state.bullets, tower);
				}
			}
		}
	}

	static void DrawTower(RenderState &renderState, const Assets &assets, const Camera2D &camera, const TowerData &tower, const Vec2f &pos, const Vec2f &maxRadius, const float angle, const float alpha, const bool drawRadius) {
		assert(MaxTileRadius > 0);
		float scale = FPL_MAX(maxRadius.x, maxRadius.y) / MaxTileRadius;

		PushRectangleCenter(renderState, pos, V2f(tower.structureRadius * scale), V4f(1, 1, 0.5f, alpha), true, 0.0f);

		Vec4f gunColor = V4f(1, 0.85f, 0.5f, alpha);
		float gunLineWidth = camera.worldToPixels * tower.gunTubeThickness * scale;

		Mat4f m = Mat4Translation(pos) * Mat4RotationZ(angle);
		PushMatrix(renderState, m);
		Vec2f gunVerts[] = {
			V2f(tower.gunTubeLength * scale, 0),
			V2f(0, 0),
		};
		PushVertices(renderState, gunVerts, 2, true, gunColor, DrawMode::Lines, false, gunLineWidth);
		PopMatrix(renderState);

		if (drawRadius) {
			const TextureAsset &radiantTexture = assets.radiantTexture;
			PushSprite(renderState, pos, V2f(tower.detectionRadius * scale, tower.detectionRadius * scale), radiantTexture.texture, V4f(0.2f, 1, 0.2f, alpha*0.25f), V2f(0, 0), V2f(1, 1));
			PushSprite(renderState, pos, V2f(tower.unlockRadius * scale, tower.unlockRadius * scale), radiantTexture.texture, V4f(1, 0.25f, 0.25f, alpha*0.25f), V2f(0, 0), V2f(1, 1));
		}
	}
}

namespace game {
	static void SetSlowdown(GameState &state, const float duration, const WaveState nextState) {
		assert(!state.isSlowDown);
		state.isSlowDown = true;
		state.slowdownScale = 0.0f;
		state.slowdownTimer[0] = state.slowdownTimer[1] = duration;
		state.waveStateAfterSlowdown = nextState;
	}

	static void ReleaseAssets(Assets &assets) {
		ReleaseFontAsset(assets.overlayFont);
		ReleaseFontAsset(assets.hudFont);
	}

	static void LoadTextureAsset(RenderState &renderState, const char *dataPath, const char *filename, const bool isTopDown, TextureAsset *outAsset) {
		char filePath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 2, dataPath, filename);
		int width, height, comp;
		stbi_set_flip_vertically_on_load(isTopDown ? 0 : 1);
		uint8_t *data = stbi_load(filePath, &width, &height, &comp, 4);
		if (data != nullptr) {
			outAsset->data.data = data;
			outAsset->data.components = 4;
			outAsset->data.width = width;
			outAsset->data.height = height;
			const TextureData &texData = outAsset->data;
			PushTexture(renderState, &outAsset->texture, texData.data, texData.width, texData.height, 4, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
		}
	}

	static void LoadAssets(Assets &assets, RenderState &renderState) {
		// Towers/Enemies/Waves
		level::LoadCreepDefinitions(assets, "creeps.xml");
		level::LoadTowerDefinitions(assets, "towers.xml");
		level::LoadWaveDefinitions(assets, "waves.xml");

		// Fonts
		char fontDataPath[1024];
		const char *fontFilename = "SulphurPoint-Bold.otf";
		fplPathCombine(fontDataPath, FPL_ARRAYCOUNT(fontDataPath), 2, assets.dataPath, "fonts");
		FontAsset &hudFont = assets.hudFont;
		if (LoadFontFromFile(fontDataPath, fontFilename, 0, 36.0f, 32, 128, 512, 512, false, &hudFont.desc)) {
			PushTexture(renderState, &hudFont.texture, hudFont.desc.atlasAlphaBitmap, hudFont.desc.atlasWidth, hudFont.desc.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
		}
		FontAsset &overlayFont = assets.overlayFont;
		if (LoadFontFromFile(fontDataPath, fontFilename, 0, 240.0f, 32, 128, 4096, 4096, false, &overlayFont.desc)) {
			PushTexture(renderState, &overlayFont.texture, overlayFont.desc.atlasAlphaBitmap, overlayFont.desc.atlasWidth, overlayFont.desc.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
		}

		// Textures
		char texturesDataPath[1024];
		char levelsDataPath[1024];
		fplPathCombine(texturesDataPath, FPL_ARRAYCOUNT(texturesDataPath), 2, assets.dataPath, "textures");
		fplPathCombine(levelsDataPath, FPL_ARRAYCOUNT(levelsDataPath), 2, assets.dataPath, "levels");
		LoadTextureAsset(renderState, texturesDataPath, "radiant.png", false, &assets.radiantTexture);
		LoadTextureAsset(renderState, levelsDataPath, "way_tileset.png", false, &assets.wayTilesetTexture);
		LoadTextureAsset(renderState, levelsDataPath, "entities_tileset.png", false, &assets.entitiesTilesetTexture);
		LoadTextureAsset(renderState, levelsDataPath, "ground_tileset.png", false, &assets.groundTilesetTexture);
	}

	static void ReleaseGame(GameState &state) {
		gamelog::Verbose("Release Game");
		level::ClearLevel(state);
		ReleaseAssets(state.assets);
	}

	static void NewGame(GameState &state) {
		// Reset camera
		state.camera.scale = 1.0f;
		state.camera.offset.x = 0;
		state.camera.offset.y = 0;

		// @TODO(final): Read from game.xml
		state.stats.money = 50;
		state.stats.lifes = 10;

		// Load initial wave
		level::LoadWave(state, 0);
	}

	static bool InitGame(GameState &state, GameMemory &gameMemory) {
		gamelog::Verbose("Initialize Game");

		fplGetExecutableFilePath(state.assets.dataPath, FPL_ARRAYCOUNT(state.assets.dataPath));
		fplExtractFilePath(state.assets.dataPath, state.assets.dataPath, FPL_ARRAYCOUNT(state.assets.dataPath));
		fplPathCombine(state.assets.dataPath, FPL_ARRAYCOUNT(state.assets.dataPath), 2, state.assets.dataPath, "data");
		gamelog::Info("Using assets path: %s", state.assets.dataPath);

		LoadAssets(state.assets, *gameMemory.render);

		NewGame(state);

		return(true);
	}

	static void DrawHUD(GameState &state, RenderState &renderState) {
		constexpr float hudPadding = MaxTileSize * 0.075f;
		constexpr float hudOriginX = -WorldRadiusW;
		constexpr float hudOriginY = WorldRadiusH;
		constexpr float hudFontHeight = TileHeight * 0.4f;
		constexpr float outlineOffset = hudFontHeight * 0.05f;
		const FontAsset &font = state.assets.hudFont;
		{
			char text[256];
			fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "%s", state.level.activeId);
			Vec2f textPos = V2f(hudOriginX + WorldRadiusW, hudOriginY - hudPadding - hudFontHeight * 0.5f);
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, 0.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, 0.0f, 0.0f, TextForeColor);

			fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Wave: %d / %zu", (state.wave.activeIndex + 1), state.assets.waveDefinitionCount);
			textPos.y -= hudFontHeight;
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, 0.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, 0.0f, 0.0f, TextForeColor);

			fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Enemies: %zu / %zu", state.enemies.count, state.wave.totalEnemyCount);
			textPos.y -= hudFontHeight;
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, 0.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, 0.0f, 0.0f, TextForeColor);
		}
		{
			char text[256];
			fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "$: %d", state.stats.money);
			Vec2f textPos = V2f(hudOriginX + hudPadding, hudOriginY - hudPadding - hudFontHeight * 0.5f);
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, 1.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, 1.0f, 0.0f, TextForeColor);
		}
		{
			char text[256];
			fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "HP: %d", state.stats.lifes);
			Vec2f textPos = V2f(hudOriginX + WorldWidth - hudPadding, hudOriginY - hudPadding - hudFontHeight * 0.5f);
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, -1.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, -1.0f, 0.0f, TextForeColor);
		}
	}

	static void DrawTowerControl(GameState &gameState, RenderState &renderState, const Vec2f &pos, const Vec2f &radius, const ui::UIButtonState buttonState, void *userData) {
		int towerDataIndex = (int)(uintptr_t)(userData);
		assert(towerDataIndex >= 0 && towerDataIndex < gameState.assets.towerDefinitionCount);
		const TowerData *towerData = &gameState.assets.towerDefinitions[towerDataIndex];
		float alpha = 0.75f;
		if (buttonState == ui::UIButtonState::Hover) {
			alpha = 1.0f;
		}
		towers::DrawTower(renderState, gameState.assets, gameState.camera, *towerData, pos, radius, Pi32 * 0.5f, alpha, false);

		// Draw selection frame
		if (gameState.towers.selectedIndex == towerDataIndex) {
			Vec2f borderVecs[] = {
				V2f(pos.x + radius.w, pos.y + radius.h),
				V2f(pos.x - radius.w, pos.y + radius.h),
				V2f(pos.x - radius.w, pos.y - radius.h),
				V2f(pos.x + radius.w, pos.y - radius.h),
			};
			float stippleWidth = (FPL_MIN(radius.x, radius.y) * 2.0f) / 10.0f;
			Vec4f stippleColor = V4f(1.0f, 1.0f, 1.0f, alpha);
			float stippleLineWidth = 1.0f;
			render::DrawLineLoopStipple(renderState, borderVecs, 4, stippleWidth, 3, stippleColor, stippleLineWidth);
		}
	}

	static void DrawControls(GameState &state, RenderState &renderState) {
		//
		// Controls Background
		//
		Vec4f backgroundColor = V4f(0.2f, 0.2f, 0.2f, 1.0f);
		PushRectangle(renderState, V2f(ControlsOriginX, ControlsOriginY), V2f(ControlsWidth, ControlsHeight), backgroundColor, true, 0.0f);

		// Controls Border
		float lineWidth = 2.0f;
		float lineWidthWorld = lineWidth * state.camera.pixelsToWorld * 0.5f;
		Vec4f controlsBorderColor = V4f(0.5f, 0.5f, 0.5f, 1.0f);
		Vec2f controlsBottomLeft = V2f();
		Vec2f controlsVerts[] = {
			V2f(ControlsOriginX + ControlsWidth - lineWidthWorld, ControlsOriginY + ControlsHeight - lineWidthWorld),
			V2f(ControlsOriginX + lineWidthWorld, ControlsOriginY + ControlsHeight - lineWidthWorld),
			V2f(ControlsOriginX + lineWidthWorld, ControlsOriginY + lineWidthWorld),
			V2f(ControlsOriginX + ControlsWidth - lineWidthWorld, ControlsOriginY + lineWidthWorld),
		};
		PushVertices(renderState, controlsVerts, 4, true, controlsBorderColor, DrawMode::Lines, true, lineWidth);

		// Tower buttons
		float buttonPadding = MaxTileSize * 0.1f;
		float buttonMargin = lineWidthWorld + (MaxTileSize * 0.15f);
		float buttonHeight = ControlsHeight - buttonMargin * 2.0f;
		Vec2f buttonRadius = V2f(buttonHeight * 0.5f);
		Vec2f buttonOutputRadius = ui::GetUIButtonExt(buttonRadius);
		for (int towerIndex = 0; towerIndex < state.assets.towerDefinitionCount; ++towerIndex) {
			void *buttonId = (void *)&state.assets.towerDefinitions[towerIndex]; // Totally dont care about const removal here
			float buttonX = ControlsOriginX + buttonMargin + (towerIndex * (buttonOutputRadius.w * 2.0f) + (FPL_MAX(0, towerIndex - 1) * buttonPadding));
			float buttonY = ControlsOriginY + buttonMargin;
			if (ui::UIButton(state.ui, buttonId, V2f(buttonX + buttonRadius.w, buttonY + buttonRadius.h), buttonRadius, DrawTowerControl, (void *)(uintptr_t)towerIndex)) {
				state.towers.selectedIndex = towerIndex;
			}
		}

		if (state.towers.selectedIndex > -1) {
			const FontAsset &font = state.assets.hudFont;
			float fontHeight = MaxTileSize * 0.4f;
			const TowerData &towerData = state.assets.towerDefinitions[state.towers.selectedIndex];
			Vec2f textPos = V2f(ControlsOriginX + ControlsWidth - lineWidthWorld - buttonMargin, ControlsOriginY + ControlsHeight * 0.5f);
			char textBuffer[256];
			fplFormatAnsiString(textBuffer, FPL_ARRAYCOUNT(textBuffer), "[%s / $%d]", towerData.id, towerData.costs);
			PushText(renderState, textBuffer, fplGetAnsiStringLength(textBuffer), &font.desc, &font.texture, V2f(textPos.x, textPos.y), fontHeight, -1.0f, 0.0f, TextForeColor);
		}

	}
}

extern bool GameInit(GameMemory &gameMemory) {
	gamelog::Verbose("Init Game");
	GameState *state = (GameState *)fmemPush(&gameMemory.persistentMemory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory.game = state;
	if (!game::InitGame(*state, gameMemory)) {
		gamelog::Fatal("Failed initializing Game!");
		GameRelease(gameMemory);
		return(false);
	}
	return(true);
}

extern void GameRelease(GameMemory &gameMemory) {
	gamelog::Verbose("Destroy Game");
	GameState *state = gameMemory.game;
	if (state != nullptr) {
		game::ReleaseGame(*state);
		state->~GameState();
	}
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}
	GameState *state = gameMemory.game;
	assert(state != nullptr);
	RenderState *renderState = gameMemory.render;

	// Debug input
	const Controller &keyboardController = input.controllers[0];
	if (WasPressed(keyboardController.debugToggle)) {
		state->isDebugRendering = !state->isDebugRendering;
	}

	// Camera
	float scale = state->camera.scale;
	state->viewport = ComputeViewportByAspect(input.windowSize, GameAspect);
	state->camera.worldToPixels = (state->viewport.w / (float)WorldWidth) * scale;
	state->camera.pixelsToWorld = 1.0f / state->camera.worldToPixels;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	float invScale = 1.0f / state->camera.scale;
	Mat4f proj = Mat4Ortho(-w * invScale, w * invScale, -h * invScale, h * invScale, 0.0f, 1.0f);
	Mat4f view = Mat4Translation(state->camera.offset);
	state->viewProjection = proj * view;

	ui::UIBegin(state->ui, state, renderState, input, state->mouseWorldPos);

	// Mouse
	int mouseCenterX = (input.mouse.pos.x - input.windowSize.w / 2);
	int mouseCenterY = (input.windowSize.h - 1 - input.mouse.pos.y) - input.windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.offset.y;

	if (state->wave.state == WaveState::Running || state->wave.state == WaveState::Starting) {
		// Update tile position from mouse
		state->mouseTilePos = WorldToTile(state->level.dimension, state->mouseWorldPos);

		// Tower placement
		if (WasPressed(input.mouse.left) && !ui::UIIsHot(state->ui)) {
			if (state->towers.selectedIndex > -1) {
				const TowerData *tower = &state->assets.towerDefinitions[state->towers.selectedIndex];
				if (towers::CanPlaceTower(*state, state->mouseTilePos, tower) == towers::CanPlaceTowerResult::Success) {
					towers::PlaceTower(*state, state->mouseTilePos, tower);
				}
			}
		}
	}
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}

	GameState *state = gameMemory.game;
	assert(state != nullptr);

	float dtScale = 1.0f;
	if (state->isSlowDown) {
		assert(state->slowdownTimer[1] > 0);
		if (state->slowdownTimer[0] > 0.0f) {
			state->slowdownTimer[0] -= input.deltaTime;
		} else {
			state->slowdownTimer[0] = 0;
			if (state->wave.state != state->waveStateAfterSlowdown) {
				state->wave.state = state->waveStateAfterSlowdown;
			}
		}
		float t = 1.0f - (state->slowdownTimer[0] / state->slowdownTimer[1]);
		dtScale = ScalarLerp(1.0f, t, state->slowdownScale);
	}
	const float dt = input.deltaTime * dtScale;

	state->deltaTime = dt;
	state->framesPerSecond = input.framesPerSeconds;

	// Startup wave
	if (state->wave.state == WaveState::Starting) {
		state->wave.warmupTimer -= dt;
		if (state->wave.warmupTimer <= 0.0f) {
			state->wave.warmupTimer = 0;
			state->wave.state = WaveState::Running;
			for (size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
				CreepSpawner &spawner = state->spawners.list[spawnerIndex];
				if (spawner.startMode == SpawnerStartMode::Fixed) {
					spawner.isActive = true;
				}
			}
		}
	}

	bool updateGameCode = state->wave.state == WaveState::Running;

	if (state->wave.state != WaveState::Stopped) {
		//
		// Move enemies
		//
		for (size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
			Creep &enemy = state->enemies.list[enemyIndex];
			if (!enemy.isDead && enemy.hasTarget) {
				Vec2f distance = enemy.targetPos - enemy.position;
				float minRadius = MaxTileSize * 0.05f;
				enemy.position += enemy.facingDirection * enemy.speed * dt;
				if (Vec2Dot(distance, distance) <= minRadius * minRadius) {
					creeps::SetCreepNextTarget(*state, enemy);
				}
			}
		}

		// Update spawners
		for (size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
			CreepSpawner &spawner = state->spawners.list[spawnerIndex];
			creeps::UpdateSpawner(*state, spawner, dt);
		}

		// Update towers
		if (updateGameCode) {
			for (size_t towerIndex = 0; towerIndex < state->towers.activeCount; ++towerIndex) {
				Tower &tower = state->towers.activeList[towerIndex];
				towers::UpdateTower(*state, tower, dt);
			}
		}

		//
		// Move and collide bullets
		//
		for (size_t bulletIndex = 0; bulletIndex < state->bullets.count; ++bulletIndex) {
			Bullet &bullet = state->bullets.list[bulletIndex];
			if (!bullet.isDestroyed) {
				bullet.position += bullet.velocity * dt;
				if (!bullet.hasHit) {
					for (size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
						Creep &enemy = state->enemies.list[enemyIndex];
						if (!enemy.isDead) {
							Vec2f distance = enemy.position - bullet.position;
							float bothRadi = bullet.data->collisionRadius + enemy.data->collisionRadius;
							float d = Vec2Dot(distance, distance);
							if (d < bothRadi * bothRadi) {
								bullet.hasHit = true;
								if (updateGameCode) {
									creeps::CreepHit(*state, enemy, bullet);
								}
								break;
							}
						}
					}
				}
				if (!bullet.hasHit) {
					if (((bullet.position.x + bullet.data->renderRadius) > WorldRadiusW) ||
						((bullet.position.y + bullet.data->renderRadius) > WorldRadiusH) ||
						((bullet.position.y - bullet.data->renderRadius) < -WorldRadiusH) ||
						((bullet.position.x - bullet.data->renderRadius) < -WorldRadiusW)) {
						bullet.isDestroyed = true;
					}
				}
			}
		}

		//
		// Remove dead enemies and destroyed bullets
		//
		for (size_t bulletIndex = 0; bulletIndex < state->bullets.count; ++bulletIndex) {
			Bullet &bullet = state->bullets.list[bulletIndex];
			if (bullet.hasHit) {
				bullet.isDestroyed = true;
			}
			if (bullet.isDestroyed) {
				if (bulletIndex < state->bullets.count - 1) {
					const Bullet &lastBullet = state->bullets.list[state->bullets.count - 1];
					bullet = lastBullet;
				}
				--state->bullets.count;
			}
		}
		size_t deadEnemyCount = 0;
		size_t nonDeadEnemyCount = 0;
		for (size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
			Creep &enemy = state->enemies.list[enemyIndex];
			if (enemy.isDead) {
				++deadEnemyCount;
			} else {
				++nonDeadEnemyCount;
			}
		}

		if (updateGameCode) {
			if (state->wave.totalEnemyCount == deadEnemyCount) {
				creeps::AllEnemiesKilled(*state);
			} else {
				if (state->stats.lifes <= 0) {
					state->stats.lifes = 0;
					state->wave.isActive = false;
					game::SetSlowdown(*state, 6.0f, WaveState::Lost);
				} else {
					bool hasActiveSpawners = false;
					CreepSpawner *nextSpawner = nullptr;
					for (size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
						CreepSpawner *spawner = &state->spawners.list[spawnerIndex];
						if (spawner->isActive) {
							hasActiveSpawners = true;
							break;
						} else {
							if (nextSpawner == nullptr && spawner->startMode == SpawnerStartMode::AfterTheLast) {
								nextSpawner = spawner;
							}
						}
					}
					if (nonDeadEnemyCount == 0 && !hasActiveSpawners) {
						// All enemies but not all from all spawners has been killed
						if (nextSpawner != nullptr) {
							nextSpawner->isActive = true;
							nextSpawner->spawnTimer = nextSpawner->cooldown;
							nextSpawner->remainingCount = nextSpawner->totalCount;
						}
					}
				}
			}
		}
	}
}

extern void GameRender(GameMemory &gameMemory, const float alpha) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);
	RenderState &renderState = *gameMemory.render;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4f(0, 0, 0, 1), ClearFlags::Color | ClearFlags::Depth);
	SetMatrix(renderState, state->viewProjection);

	const Level &level = state->level;
	const LevelDimension &dim = level.dimension;

	//
	// Tiles
	//
	// @TODO(final): Do this gid-to-tileset mapping once and not everytime on render
	const LevelTileset *gidToTileset[256 + 1] = {};
	const TextureAsset *tilesetToTexture[MAX_TILESET_COUNT] = {};
	for (size_t tilesetIndex = 0; tilesetIndex < state->level.data.tilesetCount; ++tilesetIndex) {
		const LevelTileset *tileset = &state->level.data.tilesets[tilesetIndex];
		for (size_t i = tileset->firstGid; i < (tileset->firstGid + tileset->tileCount); ++i) {
			gidToTileset[i] = tileset;
		}
		uintptr_t tilesetTextureIndex = (uintptr_t)(tileset - state->level.data.tilesets);
		assert(tilesetTextureIndex >= 0 && tilesetTextureIndex < MAX_TILESET_COUNT);
		if (fplIsStringEqual("way", tileset->name)) {
			tilesetToTexture[tilesetTextureIndex] = &state->assets.wayTilesetTexture;
		} else if (fplIsStringEqual("ground", tileset->name)) {
			tilesetToTexture[tilesetTextureIndex] = &state->assets.groundTilesetTexture;
		} else if (fplIsStringEqual("entities", tileset->name)) {
			tilesetToTexture[tilesetTextureIndex] = &state->assets.entitiesTilesetTexture;
		}
	}

	// Tile layers
	// @SLOW(final): This is slowest way you imagine how to render multiple tile layers
	for (size_t layerIndex = 0; layerIndex < state->level.data.layerCount; ++layerIndex) {
		const LevelLayer &layer = state->level.data.layers[layerIndex];
		for (size_t y = 0; y < layer.mapHeight; ++y) {
			for (size_t x = 0; x < layer.mapWidth; ++x) {
				uint32_t tileData = layer.data[y * layer.mapWidth + x];
				if (tileData > 0 && tileData < FPL_ARRAYCOUNT(gidToTileset)) {
					const LevelTileset *tileset = gidToTileset[tileData];
					assert(tileset != nullptr);
					assert(tileData >= tileset->firstGid);
					size_t indexToTilesheet = tileData - tileset->firstGid;
					uintptr_t tilesetTextureIndex = (uintptr_t)(tileset - state->level.data.tilesets);
					const TextureAsset *texAsset = tilesetToTexture[tilesetTextureIndex];
					const UVRect &uvRect = tileset->tileUVs[indexToTilesheet];
					if (texAsset != nullptr) {
						Vec2f pos = TileToWorld(state->level.dimension, V2i((int)x, (int)y), TileExt);
						PushSprite(renderState, pos, TileExt, texAsset->texture, V4f(1, 1, 1, layer.opacity), uvRect);
					}
				}
			}
		}
	}


	if (state->isDebugRendering) {
		for (int y = 0; y < dim.tileCountY; ++y) {
			for (int x = 0; x < dim.tileCountX; ++x) {
				const Tile &tile = state->level.tiles[y * dim.tileCountX + x];
				if (tile.wayType != WayType::None) {
					render::DrawTile(renderState, dim, x, y, true, V4f(0.0f, 0.0f, 1.0f, 0.5f));
				}
			}
		}

		// Draw tile entities
		for (int y = 0; y < dim.tileCountY; ++y) {
			for (int x = 0; x < dim.tileCountX; ++x) {
				const Tile &tile = state->level.tiles[y * dim.tileCountX + x];
				switch (tile.entityType) {
					case EntityType::Goal:
					{
						render::DrawTile(renderState, dim, x, y, true, V4f(0.1f, 1.0f, 0.2f, 1.0f));
					} break;
				}
			}
		}

		// Draw spawners tile
		for (size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
			const CreepSpawner &spawner = state->spawners.list[spawnerIndex];
			Vec2i tilePos = WorldToTile(dim, spawner.spawnPosition);
			render::DrawTile(renderState, dim, tilePos.x, tilePos.y, true, V4f(0.0f, 1.0f, 1.0f, 1.0f));
		}
	}


	//
	// Grid
	//
	Vec4f gridColor = V4f(1.0f, 1.0f, 1.0f, 0.25f);
	float gridLineWidth = DefaultLineWidth;
	size_t totalGridVerts = (dim.tileCountX + 1) * 2 + (dim.tileCountY + 1) * 2;
	VertexAllocation vertAlloc = AllocateVertices(renderState, totalGridVerts, gridColor, DrawMode::Lines, false, gridLineWidth);
	Vec2f *gridVertex = vertAlloc.verts;
	size_t count = 0;
	for (int y = 0; y <= dim.tileCountY; ++y) {
		*gridVertex++ = V2f(dim.gridOriginX, dim.gridOriginY + y * TileHeight);
		*gridVertex++ = V2f(dim.gridOriginX + dim.tileCountX * TileWidth, dim.gridOriginY + y * TileHeight);
		count += 2;
	}
	for (int x = 0; x <= dim.tileCountX; ++x) {
		*gridVertex++ = V2f(dim.gridOriginX + x * TileWidth, dim.gridOriginY);
		*gridVertex++ = V2f(dim.gridOriginX + x * TileWidth, dim.gridOriginY + dim.tileCountY * TileHeight);
		count += 2;
	}
	assert(count == totalGridVerts);
	*vertAlloc.count = count;

	if (state->isDebugRendering) {
		// Waypoints
		for (Waypoint *waypoint = state->waypoints.first; waypoint != nullptr; waypoint = waypoint->next) {
			PushRectangleCenter(renderState, waypoint->position, V2f(MaxTileSize * 0.15f), V4f(1, 0, 1, 1), true, 0.0f);
			PushLine(renderState, waypoint->position, waypoint->position + waypoint->direction * level::WaypointDirectionWidth, V4f(1, 1, 1, 1), 1.0f);
		}
	}

	// Hover tile
	if (state->towers.selectedIndex > -1 && IsValidTile(dim, state->mouseTilePos)) {
		const TowerData *tower = &state->assets.towerDefinitions[state->towers.selectedIndex];

		towers::CanPlaceTowerResult placeRes = towers::CanPlaceTower(*state, state->mouseTilePos, tower);
		Vec4f hoverColor;
		if (placeRes == towers::CanPlaceTowerResult::Success) {
			hoverColor = V4f(0.1f, 1.0f, 0.1f, 1.0f);
		} else {
			hoverColor = V4f(1.0f, 0.1f, 0.1f, 1.0f);
		}

		if (placeRes == towers::CanPlaceTowerResult::Success || placeRes == towers::CanPlaceTowerResult::NotEnoughMoney) {
			float alpha = placeRes == towers::CanPlaceTowerResult::Success ? 0.5f : 0.2f;
			Vec2f towerCenter = TileToWorld(dim, state->mouseTilePos, TileExt);
			towers::DrawTower(renderState, state->assets, state->camera, *tower, towerCenter, V2f(MaxTileRadius), Pi32 * 0.5f, alpha, true);
		}

		render::DrawTile(renderState, dim, state->mouseTilePos.x, state->mouseTilePos.y, false, hoverColor);
	}

	//
	// Enemies
	//
	for (size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
		Creep &enemy = state->enemies.list[enemyIndex];
		if (!enemy.isDead && enemy.id > 0) {
			Vec2f enemyPos = Vec2Lerp(enemy.prevPosition, alpha, enemy.position);

			// Mesh
			PushRectangleCenter(renderState, enemyPos, V2f(enemy.data->renderRadius, enemy.data->renderRadius), enemy.data->color, true, 0.0f);

			// HP Bar
			{
				float barWidth = TileWidth * 0.85f;
				float barHeight = TileHeight * 0.1625f;
				float barSpaceUnder = TileHeight * 0.15f;
				float barX = enemy.position.x - barWidth * 0.5f;
				float barY = enemy.position.y - enemy.data->renderRadius - barSpaceUnder - barHeight;
				float barScale = enemy.hp / (float)enemy.data->hp;

				float colorGreen = barScale;
				float colorRed = 1.0f - colorGreen;
				Vec4f progressColor = V4f(colorRed, colorGreen, 0.0f, 1.0f);
				Vec2f progressVerts[] = {
					V2f(barX + barWidth * barScale, barY + barHeight),
					V2f(barX, barY + barHeight),
					V2f(barX, barY),
					V2f(barX + barWidth * barScale, barY),
				};
				PushVertices(renderState, progressVerts, FPL_ARRAYCOUNT(progressVerts), true, progressColor, DrawMode::Polygon, true, 0.0f);

				Vec4f borderColor = V4f(0.25f, 0.25f, 0.25f, 1.0f);
				float borderLineWidth = 2.0f;
				Vec2f borderVerts[] = {
					V2f(barX + barWidth, barY + barHeight),
					V2f(barX, barY + barHeight),
					V2f(barX, barY),
					V2f(barX + barWidth, barY),
				};
				PushVertices(renderState, borderVerts, FPL_ARRAYCOUNT(borderVerts), true, borderColor, DrawMode::Lines, true, borderLineWidth);
			}

			enemy.prevPosition = enemy.position;
		}
	}

	//
	// Towers
	//
	for (size_t towerIndex = 0; towerIndex < state->towers.activeCount; ++towerIndex) {
		const Tower &tower = state->towers.activeList[towerIndex];
		towers::DrawTower(renderState, state->assets, state->camera, *tower.data, tower.position, V2f(MaxTileRadius), tower.facingAngle, 1.0f, false);

		if (state->isDebugRendering) {
			if (tower.hasTarget) {
				assert(tower.targetEnemy != nullptr);
				const Creep *target = tower.targetEnemy;
				if ((target->id > 0) && (target->id == tower.targetId)) {
					PushCircle(renderState, target->position, target->data->collisionRadius, 32, V4f(1, 0, 0, 1), false, 1.0f);

					Vec2f lookDirection = Vec2AngleToAxis(tower.facingAngle);
					Vec2f distanceToEnemy = target->position - tower.position;
					float projDistance = Vec2Dot(distanceToEnemy, lookDirection);
					Vec2f lookPos = tower.position + lookDirection * projDistance;

					PushCircle(renderState, lookPos, MaxTileSize * 0.25f, 16, V4f(1, 1, 0, 1), false, 1.0f);

					float dot = Vec2Dot(target->position, lookPos);
					float det = Vec2Cross(target->position, lookPos);
					float angle = ArcTan2(det, dot);

					if (angle >= -ShotAngleTolerance && angle <= ShotAngleTolerance) {
						Vec2f sightPos1 = tower.position + Vec2AngleToAxis(tower.facingAngle - ShotAngleTolerance) * projDistance;
						Vec2f sightPos2 = tower.position + Vec2AngleToAxis(tower.facingAngle + ShotAngleTolerance) * projDistance;
						Vec4f sightColor = V4f(1, 0, 0, 0.5);
						float sightLineWidth = 1.0f;
						Vec2f sightVec2[] = {
							V2f(tower.position.x, tower.position.y),
							V2f(sightPos1.x, sightPos1.y),
							V2f(tower.position.x, tower.position.y),
							V2f(sightPos2.x, sightPos2.y),
						};
						PushVertices(renderState, sightVec2, FPL_ARRAYCOUNT(sightVec2), true, sightColor, DrawMode::Lines, false, sightLineWidth);
					}
				}
			}
		}
	}

	//
	// Bullets
	//
	for (size_t bulletIndex = 0; bulletIndex < state->bullets.count; ++bulletIndex) {
		Bullet &bullet = state->bullets.list[bulletIndex];
		if (!bullet.isDestroyed) {
			Vec2f bulletPos = Vec2Lerp(bullet.prevPosition, alpha, bullet.position);
			// @TODO(final): Use sprites for bullets
			PushCircle(renderState, bulletPos, bullet.data->renderRadius, 32, V4f(1, 0, 0, 1), true, 0.0f);
			bullet.prevPosition = bullet.position;
		}
	}

	//
	// Selected tower text
	//

	//
	// Overlay
	//
	if (state->wave.state == WaveState::Starting) {
		const FontAsset &font = state->assets.overlayFont;
		char text[128];
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "%d", (int)ceilf(state->wave.warmupTimer));
		Vec2f textPos = V2f(0, 0);
		float overlayFontHeight = WorldWidth * 0.25f;
		float foffset = overlayFontHeight * 0.01f;
		PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), overlayFontHeight, 0.0f, 0.0f, TextBackColor);
		PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + foffset, textPos.y - foffset), overlayFontHeight, 0.0f, 0.0f, TextForeColor);
	} else if (state->wave.state == WaveState::Won || state->wave.state == WaveState::Lost) {
		const FontAsset &font = state->assets.overlayFont;
		const char *text = state->wave.state == WaveState::Won ? "You Win!" : "Game Over!";
		Vec2f textPos = V2f(0, 0);
		float overlayFontHeight = WorldWidth * 0.15f;
		float foffset = overlayFontHeight * 0.01f;
		PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), overlayFontHeight, 0.0f, 0.0f, TextBackColor);
		PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + foffset, textPos.y - foffset), overlayFontHeight, 0.0f, 0.0f, TextForeColor);
	}

	if (state->isDebugRendering) {
		const FontAsset &font = state->assets.hudFont;
		char text[256];
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Enemies: %03zu/%03zu, Bullets: %03zu, Towers: %03zu, Spawners: %03zu", state->enemies.count, state->wave.totalEnemyCount, state->bullets.count, state->towers.activeCount, state->spawners.count);
		Vec4f textColor = V4f(1, 1, 1, 1);
		float padding = MaxTileSize * 0.1f;
		Vec2f textPos = V2f(dim.gridOriginX + padding, dim.gridOriginY + padding);
		float fontHeight = MaxTileSize * 0.5f;
		PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), fontHeight, 1.0f, 1.0f, textColor);

		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Game Memory: %zu / %zu", gameMemory.persistentMemory.used, gameMemory.persistentMemory.size);
		PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + dim.gridWidth - padding * 2.0f, textPos.y + fontHeight * 2), fontHeight, -1.0f, 1.0f, textColor);
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Render Memory: %zu / %zu", gameMemory.render->lastMemoryUsage, gameMemory.render->memory.size);
		PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + dim.gridWidth - padding * 2.0f, textPos.y + fontHeight * 1), fontHeight, -1.0f, 1.0f, textColor);
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Fps: %.5f, Delta: %.5f", state->framesPerSecond, state->deltaTime);
		PushText(renderState, text, fplGetAnsiStringLength(text), &font.desc, &font.texture, V2f(textPos.x + dim.gridWidth - padding * 2.0f, textPos.y), fontHeight, -1.0f, 1.0f, textColor);
	}

	//
	// HUD & Controls
	//
	game::DrawHUD(*state, renderState);
	game::DrawControls(*state, renderState);
}

extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, const float alpha) {
	GameInput(gameMemory, input);
	GameUpdate(gameMemory, input);
	GameRender(gameMemory, alpha);
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = {};
	config.title = "FPL Demo | Towadev";
	config.disableInactiveDetection = true;
	config.noUpdateRenderSeparation = true;
	gamelog::Verbose("Startup game application '%s'", config.title);
	int result = GameMain(config);
	return(result);
}