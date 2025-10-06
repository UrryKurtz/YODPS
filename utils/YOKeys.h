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

inline std::string yo_key(const std::string &name) {

	auto it = yo_keys.find(name);

	if(it != yo_keys.end())
	{
		return yo_keys[name];
	}       // "vts" -> "vertices"
    return name;
}

#define YO_KEY(name, str) inline const char* name = yo_register_key(#name, str);

namespace yo::k {
    YO_KEY(address,    "adr")
	YO_KEY(advertise,  "adv")
    YO_KEY(apply,      "apl")
    YO_KEY(broker,     "bkr")
	YO_KEY(bearing,    "brg")
	YO_KEY(camera,     "cam")
	YO_KEY(cameras,    "cms")
    YO_KEY(color,      "clr")
    YO_KEY(colors,     "cls")
    YO_KEY(comment,    "com")
    YO_KEY(coord,      "crd")
    YO_KEY(config,     "cfg")
    YO_KEY(counter,    "cnt")
    YO_KEY(data,       "dat")
    YO_KEY(disabled,   "dis")
	YO_KEY(direction,  "dir")
    YO_KEY(enabled,    "enb")
	YO_KEY(file,       "fle")
    YO_KEY(fill,       "fll")
    YO_KEY(fov,        "fov")
    YO_KEY(font,       "fnt")
    YO_KEY(frame,      "frm")
	YO_KEY(frames,     "frs")
    YO_KEY(frame_id,   "fid")
    YO_KEY(geometry,   "geo")
    YO_KEY(geometries, "ges")
	YO_KEY(geometry_type, "gtp")
    YO_KEY(global,     "glb")
    YO_KEY(height,     "hgt")
    YO_KEY(id,         "id")
	YO_KEY(image,      "img")
    YO_KEY(internal,   "int")
    YO_KEY(input,      "inp")
	YO_KEY(inputs,     "ins")
    YO_KEY(ip,         "ip")
    YO_KEY(length,     "len")
    YO_KEY(line,       "ln")
    YO_KEY(limits,     "lim")
    YO_KEY(material,   "mtl")
    YO_KEY(material_id,"mid")
    YO_KEY(materials,  "mts")
    YO_KEY(max,        "max")
	YO_KEY(message,    "msg")
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
	YO_KEY(plugin,     "plu")
	YO_KEY(plugins,    "pls")
    YO_KEY(pointer,    "ptr")
    YO_KEY(port,       "prt")
    YO_KEY(position,   "pos")
    YO_KEY(projection, "prj")
	YO_KEY(publish,    "pub")
    YO_KEY(receiver,   "rcv")
	YO_KEY(record,     "rec")
    YO_KEY(replace,    "rps")
	YO_KEY(request,    "req")
	YO_KEY(requests,   "rqs")
	YO_KEY(response,   "res")
    YO_KEY(rotation,   "rot")
    YO_KEY(scale,      "scl")
    YO_KEY(sender,     "sdr")
	YO_KEY(select,     "sel")
	YO_KEY(settings,   "sts")
    YO_KEY(screen,     "scr")
    YO_KEY(size,       "sz")
    YO_KEY(step,       "stp")
    YO_KEY(step_fast,  "stf")
    YO_KEY(style_id,   "sid")
	YO_KEY(subscribe,  "sub")
    YO_KEY(text,       "txt")
	YO_KEY(texture,    "txr")
	YO_KEY(tile,       "til")
	YO_KEY(topic,      "tpc")
    YO_KEY(transform,  "tfm")
    YO_KEY(type,       "typ")
    YO_KEY(type_id,    "tid")
    YO_KEY(types,      "tps")
	YO_KEY(value,      "val")
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
	YOGeomery,
	YOModel,
};

enum YOGeomType
{
	YOTriangleList,
	YOLineList,
	YOPointList,
	YOTriangleStrip,
    YOLineStrip,
    YOTriangleFan
};

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
  <model />
  <geom />
  <data />
  <text />
</object>

<geom>
  <geom_type id int32/>
  <style id int32/>
  <color line />
  <color fill />
  <YOVector3List vertices />
  <YOColor4CList colors opt />
</geom>

<model>
    <file model/>
    <materials />
</model>

 */
