
#ifndef BALU_ENGINE_DISABLE_PRAGMA_ONCE
#pragma once
#endif

#include <World\Layers.h>

#if !defined(BALU_ENGINE_SCRIPT_CLASSES)  && !defined(BALU_ENGINE_DISABLE_PRAGMA_ONCE)
#include "../Class/IClass.h"

#include "../../BaluLib/Source/BVolumes/AABB.h"

#include <Interfaces\exportMacro.h>
#endif

namespace EngineInterface
{

#ifndef BALU_ENGINE_SCRIPT_CLASSES
	class IBaluSceneClassInstance
	{
	public:
		virtual ~IBaluSceneClassInstance(){}
		virtual void SetTransform(TBaluTransform transform) = 0;
		virtual void SetScale(TVec2 scale) = 0;
		virtual TBaluTransform GetTransform() = 0;
		virtual TVec2 GetScale() = 0;
		virtual IBaluClass* GetClass() = 0;
	};

#ifndef BALU_ENGINE_DLL_INTERFACES
	class TBaluSceneClassInstance : public IBaluSceneClassInstance, public TSceneObject
	{
		TBaluClass* balu_class;
		TBaluTransformWithScale transform;
	public:
		static const char* FactoryName()
		{
			return "ClassInstance";
		}
		const char* GetFactoryName()
		{
			return FactoryName();
		}
		TBaluSceneClassInstance()
		{
			balu_class = nullptr;
		}
		TBaluSceneClassInstance(TBaluClass* balu_class)
		{
			this->balu_class = balu_class;
		}
		void SetTransform(TBaluTransform transform)
		{
			this->transform.transform = transform;
		}
		void SetScale(TVec2 scale)
		{
			this->transform.scale = scale;
		}
		TBaluTransform GetTransform()
		{
			return transform.transform;
		}
		TVec2 GetScale()
		{
			return transform.scale;
		}
		TBaluTransformWithScale GetTransformWithScale()
		{
			return transform;
		}
		TBaluClass* GetClass()
		{
			return balu_class;
		}
		void Save(pugi::xml_node& parent_node, const int version);
		void Load(const pugi::xml_node& instance_node, const int version, TBaluWorld* world);
		static TSceneObject* Clone()
		{
			return new TBaluSceneClassInstance();
		}
	};
	REGISTER_FACTORY_CLASS(SceneObjectFactory, TBaluSceneClassInstance)
		//static bool TBaluSceneClassInstance_registered = SceneObjectFactory::Register(TBaluSceneClassInstance::FactoryName(), TBaluSceneClassInstance::Clone);

#endif
#endif

#ifndef BALU_ENGINE_SCRIPT_CLASSES

	class TViewport : public IViewport
	{
		TBaluTransform transform;
		float aspect; //��������� ������ � ������ (0.5 ������� �����)
		float width;
	public:
		TBaluTransform GetTransform()
		{
			return transform;
		}

		void SetTransform(TBaluTransform transform)
		{
			this->transform = transform;
		}
		void SetAspectRatio(float aspect)
		{
			this->aspect = aspect;
		}
		void SetWidth(float width)
		{
			this->width = width;
		}
		TAABB2 GetAABB()
		{
			TAABB2 aabb(TVec2(0, 0), TVec2(width, width * aspect)*0.5);

			return TOBB<float, 2>(transform.position, transform.GetOrientation(), aabb).GetAABB();
		}
		TVec2 GetSize()
		{
			return TVec2(width, width * aspect);
		}
		void SetSize(TVec2 size)
		{
			width = size[0];
			aspect = size[1] / size[0];
		}

		void Save(pugi::xml_node& parent_node, const int version);
		void Load(const pugi::xml_node& instance_node, const int version, TBaluWorld* world);
	};

	class IBaluScene
	{
	public:
		BALUENGINEDLL_API static TVec2 FromViewportToScene(IViewport* viewport, TVec2 viewport_coord);
		BALUENGINEDLL_API static TVec2 FromSceneToViewport(IViewport* viewport, TVec2 scene_coord);

		static std::string GetDefaultName()
		{
			return "scene";
		}

		virtual TLayersManager* GetLayers() = 0;

		virtual IViewport* CreateViewport(std::string name)=0;
		virtual IViewport* FindViewport(std::string name)=0;

		virtual std::string GetName() = 0;
		virtual void SetName(std::string name) = 0;

		virtual int GetInstancesCount() = 0;
		virtual TSceneObject* GetInstance(int index) = 0;

		virtual TSceneObject* CreateInstance(IBaluClass* balu_class) = 0;
		virtual void DestroyInstance(TSceneObject* instance) = 0;
	};

#ifndef BALU_ENGINE_DLL_INTERFACES
	class TBaluScene : public IBaluScene, public TBaluWorldObject
	{
	private:
		std::vector<std::unique_ptr<TSceneObject>> instances;
		std::string scene_name;

		std::map<std::string, TViewport> viewports;

		TLayersManager layers;

		TProperties properties;
		TBaluWorld* world;
	public:
		TLayersManager* GetLayers()
		{
			return &layers;
		}
		TBaluScene(const char* name, TBaluWorld* world);
		IProperties* GetProperties();

		TViewport* CreateViewport(std::string name);
		TViewport* FindViewport(std::string name);

		std::string GetName();
		void SetName(std::string name);

		int GetInstancesCount();
		TSceneObject* GetInstance(int index);

		TSceneObject* CreateInstance(TBaluClass* balu_class);
		TSceneObject* CreateInstance(IBaluClass* balu_class);

		void DestroyInstance(TSceneObject*);

		void Save(pugi::xml_node& parent_node, const int version);
		void Load(const pugi::xml_node& instance_node, const int version, TBaluWorld* world);

		IAbstractEditor* CreateEditor(TDrawingHelperContext drawing_context, IBaluSceneInstance* editor_scene_instance);
};
#endif
#endif

#ifdef BALU_ENGINE_SCRIPT_CLASSES	
	BALU_ENGINE_SCRIPT_BEGIN_CLASS(WrapInterface, IBaluScene, "IScene");
	MUnpackRA1(WrapPointer<IViewport>, TYPE, FindViewport, TString);
	BALU_ENGINE_SCRIPT_END_CLASS;
#endif

}