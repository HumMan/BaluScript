
#ifndef BALU_ENGINE_DISABLE_PRAGMA_ONCE
#pragma once
#endif

#include "..\Source\Layers.h"

#ifndef BALU_ENGINE_SCRIPT_CLASSES

#ifndef BALU_ENGINE_DISABLE_PRAGMA_ONCE
#include "IScene.h"
#include "ISceneInstance.h"
#include "IWorld.h"
#endif

namespace EngineInterface
{
	class IResources;
	class IComposer;
}
#endif

namespace EngineInterface
{

#ifndef BALU_ENGINE_SCRIPT_CLASSES
	class TMouseEventListener
	{
	public:
		virtual void OnMouseMove(TMouseEventArgs e){}
		virtual void OnMouseDown(TMouseEventArgs e){}
		virtual void OnMouseUp(TMouseEventArgs e){}
	};

	class IBaluWorldInstance
	{
	public:

		virtual void AddMouseEventListener(TMouseEventListener*)=0;
		virtual void RemoveMouseEventListener(TMouseEventListener*) = 0;

		virtual IBaluWorld* GetSource()=0;
		virtual IBaluSceneInstance* RunScene(IBaluScene* scene_source)=0;
		virtual IBaluSceneInstance* RunScene() = 0;
		virtual IBaluSceneInstance* RunScene(TLayersManager* scene_layers) = 0;
		virtual void StopScene(IBaluSceneInstance*)=0;

		virtual int GetSceneInstancesCount()=0;
		virtual IBaluSceneInstance* GetSceneInstance(int index)=0;

		virtual void MouseDown(TMouseEventArgs e) = 0;
		virtual void MouseMove(TMouseEventArgs e) = 0;
		virtual void MouseUp(TMouseEventArgs e) = 0;
		virtual void MouseVerticalWheel(int amount)=0;
		virtual IComposer* GetComposer() = 0;
	};
#endif

#ifdef BALU_ENGINE_SCRIPT_CLASSES
	BALU_ENGINE_SCRIPT_BEGIN_CLASS(WrapInterface, IBaluWorldInstance, "IWorldInstance");
	MUnpackRA1(WrapPointer<IBaluSceneInstance>, WrapInterface<IBaluWorldInstance>, GetSceneInstance, WrapValue<int>);
	MUnpackRA1(WrapPointer<IBaluSceneInstance>, WrapInterface<IBaluWorldInstance>, RunScene, WrapInterface<IBaluScene>);
	MUnpackRA0(WrapPointer<IBaluWorld>, WrapInterface<IBaluWorldInstance>, GetSource);
	BALU_ENGINE_SCRIPT_END_CLASS;
#endif



#ifndef BALU_ENGINE_SCRIPT_CLASSES
	BALUENGINEDLL_API EngineInterface::IBaluWorldInstance* CreateWorldInstance(EngineInterface::IBaluWorld* source, EngineInterface::IResources* resources);
	BALUENGINEDLL_API void DestroyWorldInstance(EngineInterface::IBaluWorldInstance* world);
#endif

}


