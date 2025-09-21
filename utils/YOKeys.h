/*
 * YOKeys.h
 *
 *  Created on: Sep 6, 2025
 *      Author: kurtz
 */

#ifndef UTILS_YOKEYS_H_
#define UTILS_YOKEYS_H_

#include <sstream>
#include <YOVariant.h>
#include <YOTypes.h>

#define YO_CAMERA_NUM 10
#define YO_VIDEO_NUM 10
#define YO_INPUT_NUM 10
#define YO_TYPE_NUM 16

inline std::map<std::string, std::string > yo_keys;
inline std::map<std::string, std::string > yo_keys_rev;

inline const char* yo_register_key(const char* name, const char* str) {
    yo_keys[str]   = name;       // "vts" -> "vertices"
    yo_keys_rev[name]   = str;   // "vertices" -> "vts"
    return str;
}

#define YO_KEY(name, str) inline const char* name = yo_register_key(#name, str);

namespace yo::k {
    YO_KEY(address,    "adr")
    YO_KEY(apply,      "apl")
    YO_KEY(broker,     "bkr")
	YO_KEY(camera,     "cam")
    YO_KEY(color,      "clr")
    YO_KEY(colors,     "cls")
    YO_KEY(comment,    "com")
    YO_KEY(coord,      "crd")
    YO_KEY(config,     "cfg")
    YO_KEY(counter,    "cnt")
    YO_KEY(data,       "dat")
    YO_KEY(disabled,   "dis")
    YO_KEY(enabled,    "enb")
    YO_KEY(fill,       "fll")
    YO_KEY(fov,        "fov")
    YO_KEY(font,       "fnt")
    YO_KEY(frame,      "frm")
	YO_KEY(frames,     "frs")
    YO_KEY(frame_id,   "fid")
    YO_KEY(geometry,   "geo")
    YO_KEY(geometries, "ges")
    YO_KEY(global,     "glb")
    YO_KEY(height,     "hgt")
    YO_KEY(id,         "id")
    YO_KEY(internal,   "int")
    YO_KEY(input,      "inp")
    YO_KEY(ip,         "ip")
    YO_KEY(length,     "len")
    YO_KEY(line,       "ln")
    YO_KEY(limits,     "lim")
    YO_KEY(material,   "mtl")
    YO_KEY(material_id,"mid")
    YO_KEY(materials,  "mts")
    YO_KEY(max,        "max")
    YO_KEY(min,        "min")
    YO_KEY(model,      "mdl")
    YO_KEY(models,     "mds")
    YO_KEY(name,       "nam")
    YO_KEY(network,    "net")
    YO_KEY(object,     "obj")
    YO_KEY(object_id,  "oid")
    YO_KEY(object_type,"oty")
    YO_KEY(objects,    "obs")
	YO_KEY(opacity,    "opa")
    YO_KEY(orthogonal, "ort")
    YO_KEY(overlay,    "oly")
	YO_KEY(plane,      "pln")
    YO_KEY(pointer,    "ptr")
    YO_KEY(port,       "prt")
    YO_KEY(position,   "pos")
    YO_KEY(projection, "prj")
    YO_KEY(receiver,   "rcv")
	YO_KEY(record,     "rec")
    YO_KEY(replace,    "rps")
    YO_KEY(rotation,   "rot")
    YO_KEY(scale,      "scl")
    YO_KEY(sender,     "sdr")
	YO_KEY(select,     "sel")
    YO_KEY(screen,     "scr")
    YO_KEY(size,       "sz")
    YO_KEY(step,       "stp")
    YO_KEY(step_fast,  "stf")
    YO_KEY(style_id,   "sid")
    YO_KEY(text,       "txt")
    YO_KEY(transform,  "tfm")
    YO_KEY(type,       "typ")
    YO_KEY(type_id,    "tid")
    YO_KEY(types,      "tps")
    YO_KEY(version,    "ver")
    YO_KEY(vertex,     "vtx")
    YO_KEY(vertices,   "vts")
	YO_KEY(video,      "vid")
    YO_KEY(width,      "wdt")
    YO_KEY(world,      "wld")
#undef YO_KEY
}

enum YOProjectionType
{
    YOPerspective,
    YOOrthogonal
};

enum YOCoordType
{
    YOWorld,
    YOOverlay
};

enum YOObjectType
{
    YOPointCloud,
    YOLineList,
    YOLineStrip,
    YOTriangleList,
    YOTriangleFan,
    YOModel
};

inline void createStyleCfg(uint32_t i, YOVariant &style, const std::string &name)
{
    //std::cout << " createStyleCfg " << name << std::endl;
    style[yo::k::enabled] = true;
    style[yo::k::replace] = true;

    style[yo::k::name] = (yo_keys[name] + " " + std::to_string(i)).c_str();
    style[yo::k::comment] = "";

    style[yo::k::id] = i;

    style[yo::k::line][yo::k::enabled] = true;
    style[yo::k::line][yo::k::color] = YOColor4F{1.0f, 1.0f, 1.0f, 1.0f};
    style[yo::k::line][yo::k::width] = YOLimitF{1.0f, 0.0f, 16.0f, 0.1f};

    style[yo::k::fill][yo::k::enabled] = true;
    style[yo::k::fill][yo::k::color] = YOColor4F{1.0f, 1.0f, 1.0f, 1.0f};

    style[yo::k::text][yo::k::enabled] = true;
    style[yo::k::text][yo::k::color] = YOColor4F{1.0f, 1.0f, 1.0f, 1.0f};
    style[yo::k::text][yo::k::size]  = YOLimitF{12.0f, 1.0f, 32.0f, 0.1f};
    style[yo::k::text][yo::k::position] = YOVector3{0.0f, 0.0f, 0.0f};
    style[yo::k::text][yo::k::font]  = YOStringList{ .items = {"default", "Arial", "Courier", "Times", "Robo"}, .select = 0};

    style.m_name = name;
}

inline void createInputCfg(uint32_t i, YOVariant &input)
{
    input[yo::k::transform][yo::k::position] = YOVector3{};
    input[yo::k::transform][yo::k::rotation] = YOVector3{};
    input[yo::k::transform][yo::k::scale] = YOVector3{1.0f, 1.0f, 1.0f};
    input[yo::k::transform][yo::k::apply] = YOStringList {.items = {"Ignore", "From Config", "From Data", "Combine"}, .select = 1};

    createStyleCfg(i, input, yo::k::input);

    input[yo::k::types] = YOArray(YO_TYPE_NUM);

    for(int j = 0; j < YO_TYPE_NUM; j++)
    {
        YOVariant &type = input[yo::k::types][j];
        createStyleCfg(j, type, yo::k::type);
    }
}

inline void createCameraCfg(uint32_t i, YOVariant &config, const std::string &name)
{
	config.m_name = name;
	config[yo::k::select] = false;
	config[yo::k::record] = false;
	config[yo::k::frames] = 20u;
	config[yo::k::id] = i;
	config[yo::k::name] = (yo_keys[name] + " " + std::to_string(i)).c_str();
	config[yo::k::comment] = "";
	config[yo::k::overlay] = true;
	config[yo::k::orthogonal] = false;
	config[yo::k::position] = YOVector3{};
    config[yo::k::rotation] = YOVector3{};
    config[yo::k::fov] = YOLimitF { .value = 55.0f, .min = 0.0f, .max = 180.f, .speed = 0.1f};
}

inline void createVideoCfg(uint32_t i, YOVariant &config, const std::string &name)
{
	config.m_name = name;
	config[yo::k::enabled] = true;
	config[yo::k::id] = i;
	config[yo::k::name] = (yo_keys[name] + " " + std::to_string(i)).c_str();
	config[yo::k::comment] = "";
	config[yo::k::overlay] = true;
	config[yo::k::opacity] = YOLimitF{.value = 1.0f, .min=0.0f, .max=1.0f, .speed=0.01};

	config[yo::k::size][yo::k::width] = (uint16_t) 0;
	config[yo::k::size][yo::k::height] = (uint16_t) 0;

	config[yo::k::plane][yo::k::width] = 3.0f;
	config[yo::k::plane][yo::k::height] = 4.0f;

    config[yo::k::transform][yo::k::position] = YOVector3{};
    config[yo::k::transform][yo::k::rotation] = YOVector3{};
    config[yo::k::transform][yo::k::scale] = YOVector3{1.0f, 1.0f, 1.0f};
}

inline bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

#endif /* UTILS_YOKEYS_H_ */

/*
//TODO finalize data types
<frame>
  <frame_params />
  <objects />
</frame>

<frame_params>
    <id type=int32" opt />
    <ts type=int64 opt />
    <transform opt />
</params>

<transform>
    <position />
    <rotation />
    <scale />
</transform>

<objects>
    <object />
    <object />
    ...
    <object />
</objects>

<object>
  <type model/geom />
  <coord type world/onscreen />
  <style_ids int32 />
  <transform >

  <ref pt />

  <model />
  <geoms />
</object>

<geoms>
  <geom />
  <geom />
  ..
  <geom />
</geoms>

<geom>
  <style id int32/>
  <color line />
  <color fill />
  <vector vertices />
  <vector normals />
  <vector colors />
</geom>

<model>
    <file model/>
    <materials />
</model>

 */
