
#ifndef BALU_ENGINE_DISABLE_PRAGMA_ONCE
#pragma once
#endif


#ifndef BALU_ENGINE_SCRIPT_CLASSES

#ifndef BALU_ENGINE_DISABLE_PRAGMA_ONCE
#include "ICallbacks.h"
#include "IMaterial.h"
#include "ISprite.h"
#include "IClass.h"
#include "IScene.h"
#endif

namespace EngineInterface
{
	class IDirector;
	class IBaluWorldInstance;
	class TRender;
}
#endif

namespace EngineInterface
{

#ifndef BALU_ENGINE_SCRIPT_CLASSES
	enum class TMouseButton
	{
		Left,
		Right,
		Middle
	};
#endif

#ifdef BALU_ENGINE_SCRIPT_CLASSES
	void TMouseButton_register(TClassRegistryParams& params)
	{
		auto scl = RegisterEnum(params,
			"enum TMouseButton\n"
			"{\n"
			"	Left,\n"
			"	Right,\n"
			"	Middle,\n"
			"}\n");
	}
	static bool TMouseButton_registered = TScriptClassesRegistry::RegisterEnum("TMouseButton", TMouseButton_register);
#endif

#ifndef BALU_ENGINE_SCRIPT_CLASSES
	class TMouseEventArgs
	{
	public:
		TMouseButton button;
		TVec2i location;
		TMouseEventArgs(){}
		TMouseEventArgs(TMouseButton button, TVec2i location)
		{
			this->button = button;
			this->location = location;
		}
	};
#endif

#ifdef BALU_ENGINE_SCRIPT_CLASSES
	BALU_ENGINE_SCRIPT_BEGIN_CLASS(WrapInterface, TMouseEventArgs, "TMouseEventArgs");
	BALU_ENGINE_SCRIPT_END_CLASS;
#endif

#ifndef BALU_ENGINE_SCRIPT_CLASSES
	class IBaluWorld
	{
	public:
		virtual TCallbacksActiveType& GetCallbacksActiveType() = 0;

		virtual bool TryFind(const char* material_name, EngineInterface::IBaluMaterial*& result) = 0;
		virtual bool TryFind(const char* sprite_name, EngineInterface::IBaluSprite*& result) = 0;
		virtual bool TryFind(const char* scene_name, EngineInterface::IBaluScene*& result) = 0;
		virtual bool TryFind(const char* class_name, IBaluClass*& result) = 0;

		void Create(const char* name, IBaluMaterial*& item)
		{
			item = CreateMaterial(name);
		}
		void Create(const char* name, IBaluSprite*& item)
		{
			item = CreateSprite(name);
		}
		void Create(const char* name, IBaluClass*& item)
		{
			item = CreateClass(name);
		}
		void Create(const char* name, IBaluScene*& item)
		{
			item = CreateScene(name);
		}

		virtual IBaluMaterial* CreateMaterial(const char* mat_name) = 0;
		virtual IBaluSprite* CreateSprite(const char* sprite_name) = 0;
		virtual IBaluClass* CreateClass(const char* class_name) = 0;
		virtual IBaluScene* CreateScene(const char* scene_name) = 0;

		virtual void DestroySprite(const char* class_name) = 0;
		virtual void DestroyClass(const char* class_name) = 0;
		virtual void DestroyScene(const char* scene_name) = 0;

		virtual std::vector<std::pair<std::string, EngineInterface::IBaluMaterial*>> GetMaterials() = 0;
		virtual std::vector<std::pair<std::string, EngineInterface::IBaluSprite*>> GetSprites() = 0;
		virtual std::vector<std::pair<std::string, EngineInterface::IBaluClass*>> GetClasses() = 0;
		virtual std::vector<std::pair<std::string, EngineInterface::IBaluScene*>> GetScenes() = 0;

		virtual IBaluScene* GetScene(const char* scene_name) = 0;
		virtual IBaluScene* GetScene(int index) = 0;

		virtual IBaluPhysShapeFactory* GetPhysShapeFactory() = 0;

		virtual void AddOnMouseDown(CallbackWithData<MouseCallback>) = 0;
		virtual void AddOnMouseUp(CallbackWithData<MouseCallback>) = 0;
		virtual void AddOnMouseMove(CallbackWithData<MouseCallback>) = 0;

		virtual std::vector<CallbackWithData<MouseCallback>>& GetOnMouseDown()=0;
		virtual std::vector<CallbackWithData<MouseCallback>>& GetOnMouseUp() = 0;
		virtual std::vector<CallbackWithData<MouseCallback>>& GetOnMouseMove() = 0;

		virtual void AddOnWorldStart(CallbackWithData<OnStartWorldCallback> callback) = 0;
		virtual std::vector<CallbackWithData<OnStartWorldCallback>>& GetOnWorldStart() = 0;
		virtual void RemoveOnWorldStart(int index) = 0;

		virtual void AddOnViewportResize(CallbackWithData<ViewportResizeCallback> callback) = 0;
		virtual std::vector<CallbackWithData<ViewportResizeCallback>>& GetOnViewportResize() = 0;
		virtual void RemoveOnViewportResize(int index) = 0;

		virtual void RemoveOnMouseDown(int index) = 0;
		virtual void RemoveOnMouseUp(int index) = 0;
		virtual void RemoveOnMouseMove(int index) = 0;

		virtual void SaveToXML(std::string path) = 0;
		virtual void LoadFromXML(std::string path) = 0;

		

	};
#endif

#ifdef BALU_ENGINE_SCRIPT_CLASSES
	BALU_ENGINE_SCRIPT_BEGIN_CLASS(WrapInterface, IBaluWorld, "IBaluWorld");
	MUnpackRA1(WrapPointer<IBaluScene>, WrapInterface<IBaluWorld>, GetScene, WrapValue<int>);
	BALU_ENGINE_SCRIPT_END_CLASS;
#endif

#ifndef BALU_ENGINE_SCRIPT_CLASSES
	BALUENGINEDLL_API EngineInterface::IBaluWorld* CreateWorld();
	BALUENGINEDLL_API void DestroyWorld(EngineInterface::IBaluWorld* world);
#endif
}
