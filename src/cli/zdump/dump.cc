// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#include "dump.hh"
#include "phoenix/Mesh.hh"
#include "phoenix/ModelScript.hh"
#include "phoenix/vobs/VirtualObject.hh"
#include "phoenix/world/BspTree.hh"
#include "phoenix/world/WayNet.hh"

#include <unordered_map>

static const std::unordered_map<phoenix::VobType, std::string> VobType_map = {
    {phoenix::VobType::zCVob, "zCVob"},
    {phoenix::VobType::zCVobLevelCompo, "zCVobLevelCompo:zCVob"},
    {phoenix::VobType::oCItem, "oCItem:zCVob"},
    {phoenix::VobType::oCNpc, "oCNpc:zCVob"},
    {phoenix::VobType::oCMOB, "oCMOB:zCVob"},
    {phoenix::VobType::oCMobInter, "oCMobInter:oCMOB:zCVob"},
    {phoenix::VobType::oCMobBed, "oCMobBed:oCMobInter:oCMOB:zCVob"},
    {phoenix::VobType::oCMobFire, "oCMobFire:oCMobInter:oCMOB:zCVob"},
    {phoenix::VobType::oCMobLadder, "oCMobLadder:oCMobInter:oCMOB:zCVob"},
    {phoenix::VobType::oCMobSwitch, "oCMobSwitch:oCMobInter:oCMOB:zCVob"},
    {phoenix::VobType::oCMobWheel, "oCMobWheel:oCMobInter:oCMOB:zCVob"},
    {phoenix::VobType::oCMobContainer, "oCMobContainer:oCMobInter:oCMOB:zCVob"},
    {phoenix::VobType::oCMobDoor, "oCMobDoor:oCMobInter:oCMOB:zCVob"},
    {phoenix::VobType::zCPFXController, "zCPFXControler:zCVob"},
    {phoenix::VobType::zCVobAnimate, "zCVobAnimate:zCVob"},
    {phoenix::VobType::zCVobLensFlare, "zCVobLensFlare:zCVob"},
    {phoenix::VobType::zCVobLight, "zCVobLight:zCVob"},
    {phoenix::VobType::zCVobSpot, "zCVobSpot:zCVob"},
    {phoenix::VobType::zCVobStartpoint, "zCVobStartpoint:zCVob"},
    {phoenix::VobType::zCVobSound, "zCVobSound:zCVob"},
    {phoenix::VobType::zCVobSoundDaytime, "zCVobSoundDaytime:zCVobSound:zCVob"},
    {phoenix::VobType::oCZoneMusic, "oCZoneMusic:zCVob"},
    {phoenix::VobType::oCZoneMusicDefault, "oCZoneMusicDefault:oCZoneMusic:zCVob"},
    {phoenix::VobType::zCZoneZFog, "zCZoneZFog:zCVob"},
    {phoenix::VobType::zCZoneZFogDefault, "zCZoneZFogDefault:zCZoneZFog:zCVob"},
    {phoenix::VobType::zCZoneVobFarPlane, "zCZoneVobFarPlane:zCVob"},
    {phoenix::VobType::zCZoneVobFarPlaneDefault, "zCZoneVobFarPlaneDefault:zCZoneVobFarPlane:zCVob"},
    {phoenix::VobType::zCMessageFilter, "zCMessageFilter:zCVob"},
    {phoenix::VobType::zCCodeMaster, "zCCodeMaster:zCVob"},
    {phoenix::VobType::zCTrigger, "zCTrigger:zCVob"},
    {phoenix::VobType::zCTriggerList, "zCTriggerList:zCTrigger:zCVob"},
    {phoenix::VobType::oCTriggerScript, "oCTriggerScript:zCTrigger:zCVob"},
    {phoenix::VobType::zCMover, "zCMover:zCTrigger:zCVob"},
    {phoenix::VobType::oCTriggerChangeLevel, "oCTriggerChangeLevel:zCTrigger:zCVob"},
    {phoenix::VobType::zCTriggerWorldStart, "zCTriggerWorldStart:zCVob"},
    {phoenix::VobType::zCTriggerUntouch, "zCTriggerUntouch:zCVob"},
    {phoenix::VobType::zCCSCamera, "zCCSCamera:zCVob"},
    {phoenix::VobType::zCCamTrj_KeyFrame, "zCCamTrj_KeyFrame:zCVob"},
    {phoenix::VobType::oCTouchDamage, "oCTouchDamage:zCTouchDamage:zCVob"},
    {phoenix::VobType::zCEarthquake, "zCEarthquake:zCVob"},
    {phoenix::VobType::zCMoverController, "zCMoverControler:zCVob"},
    {phoenix::VobType::zCVobScreenFX, "zCVobScreenFX:zCVob"},
    {phoenix::VobType::zCVobStair, "zCVobStair:zCVob"},
    {phoenix::VobType::oCCSTrigger, "oCCSTrigger:zCTrigger:zCVob"},
    {phoenix::VobType::ignored, "\xA7"}, // some sort of padding object, probably. seems to be always empty
};

namespace glm {
	void to_json(nlohmann::json& j, const glm::vec2& obj) {
		j["x"] = obj.x;
		j["y"] = obj.y;
	}

	void to_json(nlohmann::json& j, const glm::vec3& obj) {
		j["x"] = obj.x;
		j["y"] = obj.y;
		j["z"] = obj.z;
	}

	void to_json(nlohmann::json& j, const glm::vec4& obj) {
		j["x"] = obj.x;
		j["y"] = obj.y;
		j["z"] = obj.z;
		j["w"] = obj.w;
	}

	void to_json(nlohmann::json& j, const glm::quat& obj) {
		j["x"] = obj.x;
		j["y"] = obj.y;
		j["z"] = obj.z;
		j["w"] = obj.w;
	}

	void to_json(nlohmann::json& j, const glm::mat4x4& obj) {
		j = {obj[0], obj[1], obj[2], obj[3]};
	}

	void to_json(nlohmann::json& j, const glm::u8vec4& obj) {
		j["r"] = obj.r;
		j["g"] = obj.g;
		j["b"] = obj.b;
		j["a"] = obj.a;
	}
} // namespace glm

namespace phoenix {
	void to_json(nlohmann::json& j, const phoenix::FontGlyph& obj) {
		j["width"] = obj.width;
		j["uv"] = {obj.uv[0], obj.uv[1]};
	}

	void to_json(nlohmann::json& j, const phoenix::Font& obj) {
		j = nlohmann::json {{"type", "font"}, {"name", obj.name}, {"height", obj.height}, {"glyphs", obj.glyphs}};
	}

	void to_json(nlohmann::json& j, const phoenix::CutsceneBlock& obj) {
		j["name"] = obj.name;
		j["message"] = {
		    {"type", obj.message.type},
		    {"name", obj.message.name},
		    {"text", obj.message.text},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::CutsceneLibrary& obj) {
		j["type"] = "messages";
		j["blocks"] = obj.blocks;
	}

	void to_json(nlohmann::json& j, const phoenix::AxisAlignedBoundingBox& obj) {
		j["min"] = obj.min;
		j["max"] = obj.max;
	}

	void to_json(nlohmann::json& j, const phoenix::OrientedBoundingBox& obj) {
		j = {
		    {"center", obj.center},
		    {"axes", {obj.axes[0], obj.axes[1], obj.axes[2]}},
		    {"center", obj.half_width},
		    {"children", obj.children},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::Date& obj) {
		j = {
		    {"year", obj.year},
		    {"month", obj.month},
		    {"day", obj.day},
		    {"hour", obj.hour},
		    {"minute", obj.minute},
		    {"second", obj.second},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::AnimationSample& obj) {
		j["position"] = obj.position;
		j["rotation"] = obj.rotation;
	}

	void to_json(nlohmann::json& j, const phoenix::AnimationEvent& obj) {
		j["type"] = obj.type;
		j["frame"] = obj.frame;
		j["tag"] = obj.tag;
		j["content"] = obj.content;
		j["values"] = obj.values;
		j["probability"] = obj.probability;
	}

	void to_json(nlohmann::json& j, const phoenix::Animation& obj) {
		j = {
		    {"type", "animation"},
		    {"name", obj.name},
		    {"next", obj.next},
		    {"layer", obj.layer},
		    {"frameCount", obj.frame_count},
		    {"nodeCount", obj.node_count},
		    {"fps", obj.fps},
		    {"fpsSource", obj.fps_source},
		    {"bbox", obj.bbox},
		    {"checksum", obj.checksum},
		    {"sourcePath", obj.source_path},
		    {"sourceScript", obj.source_script},
		    {"samples", obj.samples},
		    {"events", obj.events},
		    {"nodeIndices", obj.node_indices},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::ModelHierarchyNode& obj) {
		j["parentIndex"] = obj.parent_index;
		j["name"] = obj.name;
		j["transform"] = obj.transform;
	}

	void to_json(nlohmann::json& j, const phoenix::ModelHierarchy& obj) {
		j = {
		    {"type", "hierarchy"},
		    {"nodes", obj.nodes},
		    {"bbox", obj.bbox},
		    {"collisionBbox", obj.collision_bbox},
		    {"rootTranslation", obj.root_translation},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::Texture& obj) {
		j = {
		    {"format", obj.format()},
		    {"width", obj.width()},
		    {"height", obj.height()},
		    {"referenceWidth", obj.ref_width()},
		    {"referenceHeight", obj.ref_height()},
		    {"mipmapCount", obj.mipmaps()},
		    {"averageColor", obj.average_color()},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MaterialGroup& obj) {
		switch (obj) {

		case MaterialGroup::UNDEFINED:
			j = "undefined";
			break;
		case MaterialGroup::METAL:
			j = "metal";
			break;
		case MaterialGroup::STONE:
			j = "stone";
			break;
		case MaterialGroup::WOOD:
			j = "wood";
			break;
		case MaterialGroup::EARTH:
			j = "earth";
			break;
		case MaterialGroup::WATER:
			j = "water";
			break;
		case MaterialGroup::SNOW:
			j = "snow";
			break;
		case MaterialGroup::NONE:
			j = "none";
			break;
		}
	}

	void to_json(nlohmann::json& j, const phoenix::AnimationMapping& obj) {
		switch (obj) {
		case AnimationMapping::NONE:
			j = "none";
			break;
		case AnimationMapping::LINEAR:
			j = "linear";
			break;
		}
	}

	void to_json(nlohmann::json& j, const WaveType& obj) {
		switch (obj) {
		case WaveType::NONE:
			j = "none";
			break;
		case WaveType::GROUND_AMBIENT:
			j = "ambientGround";
			break;
		case WaveType::GROUND:
			j = "ground";
			break;
		case WaveType::WALL_AMBIENT:
			j = "ambientWall";
			break;
		case WaveType::WALL:
			j = "wall";
			break;
		case WaveType::ENVIRONMENT:
			j = "env";
			break;
		case WaveType::WIND_AMBIENT:
			j = "ambientWind";
			break;
		case WaveType::WIND:
			j = "wind";
			break;
		}
	}

	void to_json(nlohmann::json& j, const phoenix::WaveSpeed& obj) {
		switch (obj) {
		case WaveSpeed::NONE:
			j = "none";
			break;
		case WaveSpeed::SLOW:
			j = "slow";
			break;
		case WaveSpeed::NORMAL:
			j = "normal";
			break;
		case WaveSpeed::FAST:
			j = "fast";
			break;
		}
	}

	void to_json(nlohmann::json& j, const phoenix::AlphaFunction& obj) {
		switch (obj) {
		case AlphaFunction::DEFAULT:
			j = "default";
			break;
		case AlphaFunction::NONE:
			j = "none";
			break;
		case AlphaFunction::BLEND:
			j = "blend";
			break;
		case AlphaFunction::ADD:
			j = "add";
			break;
		case AlphaFunction::SUBTRACT:
			j = "sub";
			break;
		case AlphaFunction::MULTIPLY:
			j = "mul";
			break;
		case AlphaFunction::MULTIPLY_ALT:
			j = "mul2";
			break;
		}
	}

	void to_json(nlohmann::json& j, const phoenix::Material& obj) {
		j = {
		    {"name", obj.name},
		    {"group", obj.group},
		    {"color", obj.color},
		    {"smoothAngle", obj.smooth_angle},
		    {"texture", obj.texture},
		    {"textureScale", obj.texture_scale},
		    {"textureAnimFps", obj.texture_anim_fps},
		    {"textureAnimMapMode", obj.texture_anim_map_mode},
		    {"textureAnimMapDir", obj.texture_anim_map_dir},
		    {"disableCollision", obj.disable_collision},
		    {"disableLightmap", obj.disable_lightmap},
		    {"dontCollapse", obj.dont_collapse},
		    {"detailObject", obj.detail_object},
		    {"detailTextureScale", obj.detail_texture_scale},
		    {"forceOccluder", obj.force_occluder},
		    {"environmentMapping", obj.environment_mapping},
		    {"environmentMappingStrength", obj.environment_mapping_strength},
		    {"waveMode", obj.wave_mode},
		    {"waveSpeed", obj.wave_speed},
		    {"waveMaxAmplitude", obj.wave_max_amplitude},
		    {"waveGridSize", obj.wave_grid_size},
		    {"ignoreSun", obj.ignore_sun},
		    {"alphaFunc", obj.alpha_func},
		    {"defaultMapping", obj.default_mapping},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::VertexFeature& obj) {
		j = {
		    {"texture", obj.texture},
		    {"light", obj.light},
		    {"normal", obj.normal},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::LightMap& obj) {
		j = {
		    {"image", *obj.image},
		    {"normals", {obj.normals[0], obj.normals[1]}},
		    {"origin", obj.origin},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::PolygonFlags& obj) {
		j = {
		    {"isPortal", obj.is_portal},
		    {"isOcclude", obj.is_occluder},
		    {"isSector", obj.is_sector},
		    {"shouldRelight", obj.should_relight},
		    {"isOutdoor", obj.is_outdoor},
		    {"isGhostOccluder", obj.is_ghost_occluder},
		    {"isDynamicallyLit", obj.is_dynamically_lit},
		    {"sectorIndex", obj.sector_index},
		    {"isLod", obj.is_lod},
		    {"normalAxis", obj.normal_axis},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::PolygonList& obj) {
		j = {
		    {"materialIndices", obj.material_indices},
		    {"lightmapIndices", obj.lightmap_indices},
		    {"featureIndices", obj.feature_indices},
		    {"vertexIndices", obj.vertex_indices},
		    {"flags", obj.flags},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::Mesh& obj) {
		j = {{"date", obj.date},
		     {"name", obj.name},
		     {"bbox", obj.bbox},
		     {"obb", obj.obb},
		     {"materials", obj.materials},
		     {"vertices", obj.vertices},
		     {"features", obj.features},
		     {"lightmaps", obj.lightmaps},
		     {"polygons", obj.polygons}};
	}

	void to_json(nlohmann::json& j, const phoenix::WayPoint& obj) {
		j = {
		    {"name", obj.name},
		    {"waterDepth", obj.water_depth},
		    {"underWater", obj.under_water},
		    {"position", obj.position},
		    {"direction", obj.direction},
		    {"freePoint", obj.free_point},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::WayEdge& obj) {
		j = {
		    {"a", obj.a},
		    {"b", obj.b},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::WayNet& obj) {
		j = {
		    {"waypoints", obj.waypoints},
		    {"edges", obj.edges},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::BspSector& obj) {
		j = {
		    {"name", obj.name},
		    {"nodeIndices", obj.node_indices},
		    {"portalPolygonIndices", obj.portal_polygon_indices},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::BspNode& obj) {
		j = {
		    {"plane", obj.plane},
		    {"bbox", obj.bbox},
		    {"polygonIndex", obj.polygon_index},
		    {"polygonCount", obj.polygon_count},
		    {"frontIndex", obj.front_index},
		    {"backIndex", obj.back_index},
		    {"parentIndex", obj.parent_index},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::BspTree& obj) {
		j = {
		    {"mode", obj.mode == BspTreeType::INDOOR ? "indoor" : "outdoor"},
		    {"leafPolygons", obj.leaf_polygons},
		    {"lightPoints", obj.light_points},
		    {"sectors", obj.sectors},
		    {"portalPolygonIndices", obj.portal_polygon_indices},
		    {"nodes", obj.nodes},
		    {"leafNodeIndices", obj.leaf_node_indices},
		};
	}

	void to_json(nlohmann::json& j, phoenix::VobType obj) {
		j = VobType_map.at(obj);
	}

	void to_json(nlohmann::json& j, const phoenix::VirtualObject& obj) {
		// TODO
		j = {
		    {"type", obj.type},
		};
	}

	void to_json(nlohmann::json& j, const std::unique_ptr<phoenix::VirtualObject>& obj) {
		// TODO
		to_json(j, *obj);
	}

	void to_json(nlohmann::json& j, const phoenix::World& obj) {
		j = {
		    {"vobTree", obj.world_vobs},
		    {"mesh", obj.world_mesh},
		    {"bspTree", obj.world_bsp_tree},
		    {"wayNet", obj.world_way_net},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsSkeleton& obj) {
		j = {{"name", obj.name}, {"disableMesh", obj.disable_mesh}};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsAnimationCombine& obj) {
		j = {
			{"name", obj.name},
			{"layer", obj.layer},
			{"next", obj.next},
			{"blendIn", obj.blend_in},
			{"blendOut", obj.blend_out},
			{"blendFlags", obj.flags},
			{"model", obj.model},
			{"lastFrame", obj.last_frame},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsEventTag& obj) {
		j = {
			{"frame", obj.frame},
			{"type", obj.type},
			{"slot", obj.slot},
			{"slot2", obj.slot2},
			{"item", obj.item},
			{"frames", obj.frames},
			{"fightMode", obj.fight_mode},
			{"attached", obj.attached},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsParticleEffect& obj) {
		j = {
			{"frame", obj.frame},
			{"index", obj.index},
			{"name", obj.name},
			{"position", obj.position},
			{"attached", obj.attached},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsParticleEffectStop& obj) {
		j = {
			{"frame", obj.frame},
			{"index", obj.index},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsSoundEffect& obj) {
		j = {
			{"frame", obj.frame},
			{"name", obj.name},
			{"range", obj.range},
			{"emptySlot", obj.empty_slot},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsSoundEffectGround& obj) {
		j = {
			{"frame", obj.frame},
			{"name", obj.name},
			{"range", obj.range},
			{"emptySlot", obj.empty_slot},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsMorphAnimation& obj) {
		j = {
			{"frame", obj.frame},
			{"animation", obj.animation},
			{"node", obj.node},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsCameraTremor& obj) {
		j = {
			{"frame", obj.frame},
			{"field1", obj.field1},
			{"field2", obj.field2},
			{"field3", obj.field3},
			{"field4", obj.field4},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsAnimation& obj) {
		j = {
		    {"name", obj.name},
		    {"layer", obj.layer},
		    {"next", obj.next},
		    {"blendIn", obj.blend_in},
		    {"blendOut", obj.blend_out},
		    {"blendFlags", obj.flags},
		    {"model", obj.model},
		    {"direction", obj.direction},
		    {"firstFrame", obj.first_frame},
		    {"lastFrame", obj.last_frame},
		    {"fps", obj.fps},
		    {"speed", obj.speed},
		    {"collisionVolumeScale", obj.collision_volume_scale},
		    {"events", obj.events},
		    {"pfx", obj.pfx},
		    {"pfxStop", obj.pfx_stop},
		    {"sfx", obj.sfx},
		    {"sfxGround", obj.sfx_ground},
		    {"morph", obj.morph},
		    {"tremors", obj.tremors},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsAnimationBlend& obj) {
		j = {
			{"name", obj.name},
			{"next", obj.next},
			{"blendIn", obj.blend_in},
			{"blendOut", obj.blend_out},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsAnimationAlias& obj) {
		j = {
			{"name", obj.name},
			{"layer", obj.layer},
			{"next", obj.next},
			{"blendIn", obj.blend_in},
			{"blendOut", obj.blend_out},
			{"blendFlags", obj.flags},
			{"alias", obj.alias},
			{"direction", obj.direction},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::MdsModelTag& obj) {
		j = {
			{"bone", obj.bone},
		};
	}

	void to_json(nlohmann::json& j, const phoenix::ModelScript& obj) {
		j = {
		    {"skeleton", obj.skeleton},
		    {"meshes", obj.meshes},
		    {"disabledAnimations", obj.disabled_animations},
		    {"combinations", obj.combinations},
		    {"blends", obj.blends},
		    {"aliases", obj.aliases},
		    {"modelTags", obj.model_tags},
		    {"animations", obj.animations},
		};
	}
} // namespace phoenix
