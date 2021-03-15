#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../../include/k_util.h"

#include "lua_wrapper.h"
#include "renderer.h"


//---- private functions -----------------------------------------------------//

void push_material(lua_State *l, Material material) {
	lua_newtable(l);

	lua_pushinteger(l, material.type);
	lua_setfield(l, -2, "type");

	lua_newtable(l);
	lua_pushnumber(l, material.color.x);
	lua_setfield(l, -2, "x");
	lua_pushnumber(l, material.color.y);
	lua_setfield(l, -2, "y");
	lua_pushnumber(l, material.color.z);
	lua_setfield(l, -2, "z");
	lua_setfield(l, -2, "color");

	lua_pushnumber(l, material.tint);
	lua_setfield(l, -2, "tint");

	lua_pushnumber(l, material.fuzzyness);
	lua_setfield(l, -2, "fuzzyness");

	lua_pushnumber(l, material.refIdx);
	lua_setfield(l, -2, "refIdx");
}

Material to_material(lua_State *l, int idx) {
	Material material;

	lua_getfield(l, idx, "type");
	material.type = luaL_checkinteger(l, -1);
	lua_pop(l, 1);

	lua_getfield(l, idx, "color");
	lua_getfield(l, -1, "x");
	material.color.x = luaL_checknumber(l, -1);
	lua_pop(l, 1);
	lua_getfield(l, -1, "y");
	material.color.y = luaL_checknumber(l, -1);
	lua_pop(l, 1);
	lua_getfield(l, -1, "z");
	material.color.z = luaL_checknumber(l, -1);
	lua_pop(l, 1);
	lua_pop(l, 1);

	lua_getfield(l, idx, "tint");
	material.tint = luaL_checknumber(l, -1);
	lua_pop(l, 1);

	lua_getfield(l, idx, "fuzzyness");
	material.fuzzyness = luaL_checknumber(l, -1);
	lua_pop(l, 1);

	lua_getfield(l, idx, "refIdx");
	material.refIdx = luaL_checknumber(l, -1);
	lua_pop(l, 1);

	return material;
}

//---- lua function wrappers -------------------------------------------------//

int l_create_renderer(lua_State *l) {
	Renderer *renderer = create_renderer();
	lua_pushlightuserdata(l, renderer);

	return 1;
}

int l_set_image_properties(lua_State *l) {
	Renderer *renderer = lua_touserdata(l, 1);
	int width = luaL_checkinteger(l, 2);
	int height = luaL_checkinteger(l, 3);

	set_image_properties(renderer, width, height);

	return 0;
}

int l_set_background_color(lua_State *l) {
	Renderer *renderer = lua_touserdata(l, 1);
	float r = luaL_checknumber(l, 2);
	float g = luaL_checknumber(l, 3);
	float b = luaL_checknumber(l, 4);

	set_background_color(renderer, r, g, b);

	return 0;
}

int l_create_lambertian_material(lua_State *l) {
	float r = luaL_checknumber(l, 1);
	float g = luaL_checknumber(l, 2);
	float b = luaL_checknumber(l, 3);

	Material material = create_lambertian_material(r, g, b);

	push_material(l, material);

	return 1;
}

int l_create_metal_material(lua_State *l) {
	float r = luaL_checknumber(l, 1);
	float g = luaL_checknumber(l, 2);
	float b = luaL_checknumber(l, 3);
	float tint = luaL_checknumber(l, 4);
	float fuzzyness = luaL_checknumber(l, 5);

	Material material = create_metal_material(r, g, b, tint, fuzzyness);

	push_material(l, material);

	return 1;
}

int l_create_dielectric_material(lua_State *l) {
	float r = luaL_checknumber(l, 1);
	float g = luaL_checknumber(l, 2);
	float b = luaL_checknumber(l, 3);
	float tint = luaL_checknumber(l, 4);
	float fuzzyness = luaL_checknumber(l, 5);
	float refIdx = luaL_checknumber(l, 6);

	Material material = create_dielectric_material(r, g, b, tint, fuzzyness, refIdx);

	push_material(l, material);

	return 1;
}

int l_create_light_source_material(lua_State *l) {
	float r = luaL_checknumber(l, 1);
	float g = luaL_checknumber(l, 2);
	float b = luaL_checknumber(l, 3);

	Material material = create_light_source_material(r, g, b);

	push_material(l, material);

	return 1;
}

int l_set_camera_properties(lua_State *l) {
	Renderer *renderer = lua_touserdata(l, 1);
	float x = luaL_checknumber(l, 2);
	float y = luaL_checknumber(l, 3);
	float z = luaL_checknumber(l, 4);
	float rotX = luaL_checknumber(l, 5);
	float rotY = luaL_checknumber(l, 6);
	float rotZ = luaL_checknumber(l, 7);
	float sensorWidth = luaL_checknumber(l, 8);
	float focalLength = luaL_checknumber(l, 9);
	float aperture = luaL_checknumber(l, 10);
	float exposure = luaL_checknumber(l, 11);

	set_camera_properties(renderer, x, y, z, rotX, rotY, rotZ, sensorWidth, focalLength, aperture, exposure);

	return 0;
}

int l_render(lua_State *l) {
	Renderer *renderer = lua_touserdata(l, 1);
	int samples = luaL_checkinteger(l, 2);
	int verbose = luaL_checkinteger(l, 3);

	Image *image = render(renderer, samples, verbose);

	lua_pushlightuserdata(l, image);

	return 1;
}

int l_render_to_file(lua_State *l) {
	Renderer *renderer = lua_touserdata(l, 1);
	int samples = luaL_checkinteger(l, 2);
	char *fileName = (char *)luaL_checkstring(l, 3);
	int verbose = luaL_checkinteger(l, 4);

	render_to_file(renderer, samples, fileName, verbose);

	return 0;
}

int l_destroy_renderer(lua_State *l) {
	Renderer *renderer = lua_touserdata(l, 1);

	destroy_renderer(renderer);

	return 0;
}

int l_write_image(lua_State *l) {
	Image *image = lua_touserdata(l, 1);
	char *fileName = (char *)luaL_checkstring(l, 2);

	write_image(image, fileName);

	return 0;
}

int l_destroy_image(lua_State *l) {
	Image *image = lua_touserdata(l, 1);

	destroy_image(image);

	return 0;
}
//---- main function ---------------------------------------------------------//

lua_State *create_script(char *fileName) {
	lua_State *l = luaL_newstate();
	luaL_openlibs(l);

	const struct luaL_Reg luaFuncs[] = {
		{"create_renderer", l_create_renderer},
		{"set_image_properties", l_set_image_properties},
		{"set_background_color", l_set_background_color},
		{"create_lambertian_material", l_create_lambertian_material},
		{"create_metal_material", l_create_metal_material},
		{"create_dielectric_material", l_create_dielectric_material},
		{"create_light_source_material", l_create_light_source_material},
		{"set_camera_properties", l_set_camera_properties},
		{"render", l_render},
		{"render_to_file", l_render_to_file},
		{"destroy_renderer", l_destroy_renderer},
		{"write_image", l_write_image},
		{"destroy_image", l_destroy_image},
		{NULL, NULL}
	};

	lua_newtable(l);
	luaL_setfuncs(l, luaFuncs, 0);
	lua_setglobal(l, "renderer");

	if(luaL_loadfile(l, fileName) != LUA_OK)
		msg("%s\n", lua_tostring(l, -1));

	if(lua_pcall(l, 0, 0, 0) != LUA_OK)
		msg("%s\n", lua_tostring(l, -1));

	return l;
}

void run_script(lua_State *l, char *functionName) {
	lua_getglobal(l, functionName);

	if(lua_pcall(l, 0, 0, 0) != LUA_OK)
		msg("%s\n", lua_tostring(l, -1));
}

void destroy_script(lua_State *l) {
	lua_close(l);
}