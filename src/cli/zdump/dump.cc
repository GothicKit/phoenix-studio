// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#include "dump.hh"

#include <unordered_map>

static const std::unordered_map<px::vob_type, std::string> vob_type_map = {
    {px::vob_type::zCVob, "zCVob"},
    {px::vob_type::zCVobLevelCompo, "zCVobLevelCompo:zCVob"},
    {px::vob_type::oCItem, "oCItem:zCVob"},
    {px::vob_type::oCNpc, "oCNpc:zCVob"},
    {px::vob_type::oCMOB, "oCMOB:zCVob"},
    {px::vob_type::oCMobInter, "oCMobInter:oCMOB:zCVob"},
    {px::vob_type::oCMobBed, "oCMobBed:oCMobInter:oCMOB:zCVob"},
    {px::vob_type::oCMobFire, "oCMobFire:oCMobInter:oCMOB:zCVob"},
    {px::vob_type::oCMobLadder, "oCMobLadder:oCMobInter:oCMOB:zCVob"},
    {px::vob_type::oCMobSwitch, "oCMobSwitch:oCMobInter:oCMOB:zCVob"},
    {px::vob_type::oCMobWheel, "oCMobWheel:oCMobInter:oCMOB:zCVob"},
    {px::vob_type::oCMobContainer, "oCMobContainer:oCMobInter:oCMOB:zCVob"},
    {px::vob_type::oCMobDoor, "oCMobDoor:oCMobInter:oCMOB:zCVob"},
    {px::vob_type::zCPFXController, "zCPFXControler:zCVob"},
    {px::vob_type::zCVobAnimate, "zCVobAnimate:zCVob"},
    {px::vob_type::zCVobLensFlare, "zCVobLensFlare:zCVob"},
    {px::vob_type::zCVobLight, "zCVobLight:zCVob"},
    {px::vob_type::zCVobSpot, "zCVobSpot:zCVob"},
    {px::vob_type::zCVobStartpoint, "zCVobStartpoint:zCVob"},
    {px::vob_type::zCVobSound, "zCVobSound:zCVob"},
    {px::vob_type::zCVobSoundDaytime, "zCVobSoundDaytime:zCVobSound:zCVob"},
    {px::vob_type::oCZoneMusic, "oCZoneMusic:zCVob"},
    {px::vob_type::oCZoneMusicDefault, "oCZoneMusicDefault:oCZoneMusic:zCVob"},
    {px::vob_type::zCZoneZFog, "zCZoneZFog:zCVob"},
    {px::vob_type::zCZoneZFogDefault, "zCZoneZFogDefault:zCZoneZFog:zCVob"},
    {px::vob_type::zCZoneVobFarPlane, "zCZoneVobFarPlane:zCVob"},
    {px::vob_type::zCZoneVobFarPlaneDefault, "zCZoneVobFarPlaneDefault:zCZoneVobFarPlane:zCVob"},
    {px::vob_type::zCMessageFilter, "zCMessageFilter:zCVob"},
    {px::vob_type::zCCodeMaster, "zCCodeMaster:zCVob"},
    {px::vob_type::zCTrigger, "zCTrigger:zCVob"},
    {px::vob_type::zCTriggerList, "zCTriggerList:zCTrigger:zCVob"},
    {px::vob_type::oCTriggerScript, "oCTriggerScript:zCTrigger:zCVob"},
    {px::vob_type::zCMover, "zCMover:zCTrigger:zCVob"},
    {px::vob_type::oCTriggerChangeLevel, "oCTriggerChangeLevel:zCTrigger:zCVob"},
    {px::vob_type::zCTriggerWorldStart, "zCTriggerWorldStart:zCVob"},
    {px::vob_type::zCTriggerUntouch, "zCTriggerUntouch:zCVob"},
    {px::vob_type::zCCSCamera, "zCCSCamera:zCVob"},
    {px::vob_type::zCCamTrj_KeyFrame, "zCCamTrj_KeyFrame:zCVob"},
    {px::vob_type::oCTouchDamage, "oCTouchDamage:zCTouchDamage:zCVob"},
    {px::vob_type::zCEarthquake, "zCEarthquake:zCVob"},
    {px::vob_type::zCMoverController, "zCMoverControler:zCVob"},
    {px::vob_type::zCVobScreenFX, "zCVobScreenFX:zCVob"},
    {px::vob_type::zCVobStair, "zCVobStair:zCVob"},
    {px::vob_type::oCCSTrigger, "oCCSTrigger:zCTrigger:zCVob"},
    {px::vob_type::ignored, "\xA7"}, // some sort of padding object, probably. seems to be always empty
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
	void to_json(nlohmann::json& j, const px::glyph& obj) {
		j["width"] = obj.width;
		j["uv"] = {obj.uv[0], obj.uv[1]};
	}

	void to_json(nlohmann::json& j, const px::font& obj) {
		j = nlohmann::json {{"type", "font"}, {"name", obj.name}, {"height", obj.height}, {"glyphs", obj.glyphs}};
	}

	void to_json(nlohmann::json& j, const px::message_block& obj) {
		j["name"] = obj.name;
		j["message"] = {
		    {"type", obj.message.type},
		    {"name", obj.message.name},
		    {"text", obj.message.text},
		};
	}

	void to_json(nlohmann::json& j, const px::messages& obj) {
		j["type"] = "messages";
		j["blocks"] = obj.blocks;
	}

	void to_json(nlohmann::json& j, const px::bounding_box& obj) {
		j["min"] = obj.min;
		j["max"] = obj.max;
	}

	void to_json(nlohmann::json& j, const px::obb& obj) {
		j = {
		    {"center", obj.center},
		    {"axes", {obj.axes[0], obj.axes[1], obj.axes[2]}},
		    {"center", obj.half_width},
		    {"children", obj.children},
		};
	}

	void to_json(nlohmann::json& j, const px::date& obj) {
		j = {
		    {"year", obj.year},
		    {"month", obj.month},
		    {"day", obj.day},
		    {"hour", obj.hour},
		    {"minute", obj.minute},
		    {"second", obj.second},
		};
	}

	void to_json(nlohmann::json& j, const px::animation_sample& obj) {
		j["position"] = obj.position;
		j["rotation"] = obj.rotation;
	}

	void to_json(nlohmann::json& j, const px::animation_event& obj) {
		j["type"] = obj.type;
		j["no"] = obj.no;
		j["tag"] = obj.tag;
		j["content"] = obj.content;
		j["values"] = obj.values;
		j["probability"] = obj.probability;
	}

	void to_json(nlohmann::json& j, const px::animation& obj) {
		j = {
		    {"type", "animation"},
		    {"name", obj.name},
		    {"next", obj.next},
		    {"layer", obj.layer},
		    {"frameCount", obj.frame_count},
		    {"nodeCount", obj.node_count},
		    {"fps", obj.fps},
		    {"fpsSource", obj.fps_source},
		    {"samplePositionRangeMin", obj.sample_position_range_min},
		    {"samplePositionScalar", obj.sample_position_scalar},
		    {"bbox", obj.bbox},
		    {"checksum", obj.checksum},
		    {"sourcePath", obj.source_path},
		    {"sourceScript", obj.source_script},
		    {"samples", obj.samples},
		    {"events", obj.events},
		    {"nodeIndices", obj.node_indices},
		};
	}

	void to_json(nlohmann::json& j, const px::model_hierarchy_node& obj) {
		j["parentIndex"] = obj.parent_index;
		j["name"] = obj.name;
		j["transform"] = obj.transform;
	}

	void to_json(nlohmann::json& j, const px::model_hierarchy& obj) {
		j = {
		    {"type", "hierarchy"},
		    {"nodes", obj.nodes},
		    {"bbox", obj.bbox},
		    {"collisionBbox", obj.collision_bbox},
		    {"rootTranslation", obj.root_translation},
		};
	}

	void to_json(nlohmann::json& j, const px::texture& obj) {
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

	void to_json(nlohmann::json& j, const px::material_group& obj) {
		switch (obj) {

		case material_group::undefined:
			j = "undefined";
			break;
		case material_group::metal:
			j = "metal";
			break;
		case material_group::stone:
			j = "stone";
			break;
		case material_group::wood:
			j = "wood";
			break;
		case material_group::earth:
			j = "earth";
			break;
		case material_group::water:
			j = "water";
			break;
		case material_group::snow:
			j = "snow";
			break;
		case material_group::none:
			j = "none";
			break;
		}
	}

	void to_json(nlohmann::json& j, const px::animation_mapping_mode& obj) {
		switch (obj) {
		case animation_mapping_mode::none:
			j = "none";
			break;
		case animation_mapping_mode::linear:
			j = "linear";
			break;
		}
	}

	void to_json(nlohmann::json& j, const px::wave_mode_type& obj) {
		switch (obj) {
		case wave_mode_type::none:
			j = "none";
			break;
		case wave_mode_type::ambient_ground:
			j = "ambientGround";
			break;
		case wave_mode_type::ground:
			j = "ground";
			break;
		case wave_mode_type::ambient_wall:
			j = "ambientWall";
			break;
		case wave_mode_type::wall:
			j = "wall";
			break;
		case wave_mode_type::env:
			j = "env";
			break;
		case wave_mode_type::ambient_wind:
			j = "ambientWind";
			break;
		case wave_mode_type::wind:
			j = "wind";
			break;
		}
	}

	void to_json(nlohmann::json& j, const px::wave_speed_type& obj) {
		switch (obj) {
		case wave_speed_type::none:
			j = "none";
			break;
		case wave_speed_type::slow:
			j = "slow";
			break;
		case wave_speed_type::normal:
			j = "normal";
			break;
		case wave_speed_type::fast:
			j = "fast";
			break;
		}
	}

	void to_json(nlohmann::json& j, const px::alpha_function& obj) {
		switch (obj) {
		case alpha_function::default_:
			j = "default";
			break;
		case alpha_function::none:
			j = "none";
			break;
		case alpha_function::blend:
			j = "blend";
			break;
		case alpha_function::add:
			j = "add";
			break;
		case alpha_function::sub:
			j = "sub";
			break;
		case alpha_function::mul:
			j = "mul";
			break;
		case alpha_function::mul2:
			j = "mul2";
			break;
		}
	}

	void to_json(nlohmann::json& j, const px::material& obj) {
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

	void to_json(nlohmann::json& j, const px::vertex_feature& obj) {
		j = {
		    {"texture", obj.texture},
		    {"light", obj.light},
		    {"normal", obj.normal},
		};
	}

	void to_json(nlohmann::json& j, const px::light_map& obj) {
		j = {
		    {"image", *obj.image},
		    {"normals", {obj.normals[0], obj.normals[1]}},
		    {"origin", obj.origin},
		};
	}

	void to_json(nlohmann::json& j, const px::polygon_flags& obj) {
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

	void to_json(nlohmann::json& j, const px::polygon_list& obj) {
		j = {
		    {"materialIndices", obj.material_indices},
		    {"lightmapIndices", obj.lightmap_indices},
		    {"featureIndices", obj.feature_indices},
		    {"vertexIndices", obj.vertex_indices},
		    {"flags", obj.flags},
		};
	}

	void to_json(nlohmann::json& j, const px::mesh& obj) {
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

	void to_json(nlohmann::json& j, const px::way_point& obj) {
		j = {
		    {"name", obj.name},
		    {"waterDepth", obj.water_depth},
		    {"underWater", obj.under_water},
		    {"position", obj.position},
		    {"direction", obj.direction},
		    {"freePoint", obj.free_point},
		};
	}

	void to_json(nlohmann::json& j, const px::way_edge& obj) {
		j = {
		    {"a", obj.a},
		    {"b", obj.b},
		};
	}

	void to_json(nlohmann::json& j, const px::way_net& obj) {
		j = {
		    {"waypoints", obj.waypoints},
		    {"edges", obj.edges},
		};
	}

	void to_json(nlohmann::json& j, const px::bsp_sector& obj) {
		j = {
		    {"name", obj.name},
		    {"nodeIndices", obj.node_indices},
		    {"portalPolygonIndices", obj.portal_polygon_indices},
		};
	}

	void to_json(nlohmann::json& j, const px::bsp_node& obj) {
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

	void to_json(nlohmann::json& j, const px::bsp_tree& obj) {
		j = {
		    {"mode", obj.mode == bsp_tree_mode::indoor ? "indoor" : "outdoor"},
		    {"leafPolygons", obj.leaf_polygons},
		    {"lightPoints", obj.light_points},
		    {"sectors", obj.sectors},
		    {"portalPolygonIndices", obj.portal_polygon_indices},
		    {"nodes", obj.nodes},
		    {"leafNodeIndices", obj.leaf_node_indices},
		};
	}

	void to_json(nlohmann::json& j, px::vob_type obj) {
		j = vob_type_map.at(obj);
	}

	void to_json(nlohmann::json& j, const px::vob& obj) {
		// TODO
		j = {
		    {"type", obj.type},
		};
	}

	void to_json(nlohmann::json& j, const std::unique_ptr<px::vob>& obj) {
		// TODO
		to_json(j, *obj);
	}

	void to_json(nlohmann::json& j, const px::world& obj) {
		j = {
		    {"vobTree", obj.world_vobs},
		    {"mesh", obj.world_mesh},
		    {"bspTree", obj.world_bsp_tree},
		    {"wayNet", obj.world_way_net},
		};
	}

	void to_json(nlohmann::json& j, const px::edge& obj) {
		j = {{"edges", obj.edges}};
	}

	void to_json(nlohmann::json& j, const px::triangle_edge& obj) {
		j = {{"edges", obj.edges}};
	}

	void to_json(nlohmann::json& j, const px::triangle& obj) {
		j = {{"wedges", obj.wedges}};
	}

	void to_json(nlohmann::json& j, const px::wedge& obj) {
		j = {
		    {"index", obj.index},
		    {"normal", obj.normal},
		    {"texture", obj.texture},
		};
	}

	void to_json(nlohmann::json& j, const px::sub_mesh& obj) {
		j = {
		    {"colors", obj.colors},
		    {"edgeScores", obj.edge_scores},
		    {"edges", obj.edges},
		    {"material", obj.mat},
		    {"triangleEdges", obj.triangle_edges},
		    {"trianglePlaneIndices", obj.triangle_plane_indices},
		    {"triangles", obj.triangles},
		    {"wedgeMap", obj.wedge_map},
		    {"wedges", obj.wedges},
		};
	}

	void to_json(nlohmann::json& j, const px::proto_mesh& obj) {
		j = {
		    {"materials", obj.materials},
		    {"normals", obj.normals},
		    {"alphaTest", obj.alpha_test},
		    {"bbox", obj.bbox},
		    {"obbox", obj.obbox},
		    {"positions", obj.positions},
		    {"subMeshes", obj.sub_meshes},
		};
	}

	void to_json(nlohmann::json& j, const px::wedge_normal& obj) {
		j = {
		    {"index", obj.index},
		    {"normal", obj.normal},
		};
	}
	void to_json(nlohmann::json& j, const px::weight_entry& obj) {
		j = {
		    {"position", obj.position},
		    {"nodeIndex", obj.node_index},
		    {"weight", obj.weight},
		};
	}

	void to_json(nlohmann::json& j, const px::softskin_mesh& obj) {
		j = {
		    {"bboxes", obj.bboxes},
		    {"mesh", obj.mesh},
		    {"nodes", obj.nodes},
		    {"wedgeNormals", obj.wedge_normals},
		    {"weights", obj.weights},
		};
	}

	void to_json(nlohmann::json& j, const px::model_mesh& obj) {
		j = {{"checksum", obj.checksum}, {"meshes", obj.meshes}, {"attachments", obj.attachments}};
	}

	void to_json(nlohmann::json& j, const px::morph_animation& obj) {
		j = {
		    {"samples", obj.samples},
		    {"name", obj.name},
		    {"blendIn", obj.blend_in},
		    {"blendOut", obj.blend_out},
		    {"duration", obj.duration},
		    {"flags", obj.flags},
		    {"frameCount", obj.frame_count},
		    {"layer", obj.layer},
		    {"speed", obj.speed},
		    {"vertices", obj.vertices},
		};
	}

	void to_json(nlohmann::json& j, const px::morph_source& obj) {
		j = {
		    {"fileDate", obj.file_date},
		    {"fileName", obj.file_name},
		};
	}

	void to_json(nlohmann::json& j, const px::morph_mesh& obj) {
		j = {
		    {"name", obj.name},
		    {"mesh", obj.mesh},
		    {"animations", obj.animations},
		    {"morphPositions", obj.morph_positions},
		    {"sources", obj.sources},
		};
	}

	void to_json(nlohmann::json& j, const px::model& obj) {
		j = {
		    {"mesh", obj.mesh},
		    {"hierarchy", obj.hierarchy},
		};
	}

	void to_json(nlohmann::json& j, const px::VfsNodeType& obj) {
		switch (obj) {
		case VfsNodeType::DIRECTORY:
			j = "DIRECTORY";
			break;
		case VfsNodeType::FILE:
			j = "FILE";
			break;
		}
	}

	void to_json(nlohmann::json& j, const px::VfsNode& obj) {
		if (obj.type() == VfsNodeType::FILE) {
			j = {
			    {"name", obj.name()},
			    {"type", obj.type()},
			    {"time", obj.time()},
			};
		} else {
			j = {
			    {"name", obj.name()},
			    {"type", obj.type()},
			    {"time", obj.time()},
			    {"children", obj.children()},
			};
		}
	}

	void to_json(nlohmann::json& j, const px::Vfs& obj) {
		j = {
		    {"root", obj.root()},
		};
	}

	namespace mds {
		void to_json(nlohmann::json& j, const px::mds::skeleton& obj) {
			j = {{"name", obj.name}, {"disableMesh", obj.disable_mesh}};
		}

		void to_json(nlohmann::json& j, const px::mds::animation_combination& obj) {
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

		void to_json(nlohmann::json& j, const px::mds::event_tag& obj) {
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

		void to_json(nlohmann::json& j, const px::mds::event_pfx& obj) {
			j = {
			    {"frame", obj.frame},
			    {"index", obj.index},
			    {"name", obj.name},
			    {"position", obj.position},
			    {"attached", obj.attached},
			};
		}

		void to_json(nlohmann::json& j, const px::mds::event_pfx_stop& obj) {
			j = {
			    {"frame", obj.frame},
			    {"index", obj.index},
			};
		}

		void to_json(nlohmann::json& j, const px::mds::event_sfx& obj) {
			j = {
			    {"frame", obj.frame},
			    {"name", obj.name},
			    {"range", obj.range},
			    {"emptySlot", obj.empty_slot},
			};
		}

		void to_json(nlohmann::json& j, const px::mds::event_sfx_ground& obj) {
			j = {
			    {"frame", obj.frame},
			    {"name", obj.name},
			    {"range", obj.range},
			    {"emptySlot", obj.empty_slot},
			};
		}

		void to_json(nlohmann::json& j, const px::mds::event_morph_animate& obj) {
			j = {
			    {"frame", obj.frame},
			    {"animation", obj.animation},
			    {"node", obj.node},
			};
		}

		void to_json(nlohmann::json& j, const px::mds::event_camera_tremor& obj) {
			j = {
			    {"frame", obj.frame},
			    {"field1", obj.field1},
			    {"field2", obj.field2},
			    {"field3", obj.field3},
			    {"field4", obj.field4},
			};
		}

		void to_json(nlohmann::json& j, const px::mds::animation& obj) {
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

		void to_json(nlohmann::json& j, const px::mds::animation_blending& obj) {
			j = {
			    {"name", obj.name},
			    {"next", obj.next},
			    {"blendIn", obj.blend_in},
			    {"blendOut", obj.blend_out},
			};
		}

		void to_json(nlohmann::json& j, const px::mds::animation_alias& obj) {
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

		void to_json(nlohmann::json& j, const px::mds::model_tag& obj) {
			j = {
			    {"bone", obj.bone},
			};
		}
	} // namespace mds

	void to_json(nlohmann::json& j, const px::model_script& obj) {
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
