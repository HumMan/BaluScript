#pragma once

#include "Material.h"
#include "Sprite.h"
#include "Class.h"
#include "Scene.h"

#include <map>

#include "EngineInterfaces.h"

class TBaluPhysShapeFactory : public EngineInterface::IBaluPhysShapeFactory
{
public:
	TBaluPolygonShape* CreatePolygonShape();
	TBaluCircleShape* CreateCircleShape(float radius);
	TBaluCircleShape* CreateCircleShape(float radius, TVec2 pos);
	TBaluBoxShape* CreateBoxShape(float width, float height);
};


class TBaluWorld : public EngineInterface::IBaluWorld
{
private:
	friend class TBaluWorldInstance;

	std::map<std::string, TBaluMaterial> materials;
	std::map<std::string, TBaluSprite> sprites;
	std::map<std::string, TBaluClass> classes;
	std::map<std::string, TBaluScene> scenes;

	TBaluPhysShapeFactory shape_factory;

	std::vector<CallbackWithData<MouseUpDownCallback>> mouse_down_callbacks;
	std::vector<CallbackWithData<MouseUpDownCallback>> mouse_up_callbacks;
	std::vector<CallbackWithData<MouseMoveCallback>> mouse_move_callbacks;

	//TScreen screen;
	TCallbacksActiveType callback_active_type;
public:
	TBaluWorld();
	TCallbacksActiveType& GetCallbacksActiveType()
	{
		return callback_active_type;
	}
	//void SetScreen(const TScreen& screen);
	
	bool TryFindClass(char* class_name, TBaluClass*& result);
	bool TryFindClass(char* class_name, EngineInterface::IBaluClass*& result);

	TBaluMaterial* CreateMaterial(char* mat_name);
	TBaluSprite* CreateSprite(char* sprite_name);
	TBaluClass* CreateClass(char* class_name);
	TBaluScene* CreateScene(char* scene_name);

	TBaluScene* GetScene(char* scene_name);

	TBaluPhysShapeFactory* GetPhysShapeFactory();

	//void OnKeyDown(TKey key, KeyDownCallback callback);

	void OnMouseDown(CallbackWithData<MouseUpDownCallback>);
	void OnMouseUp(CallbackWithData<MouseUpDownCallback>);
	void OnMouseMove(CallbackWithData<MouseUpDownCallback>);
};