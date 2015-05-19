
#ifndef BALU_ENGINE_DISABLE_PRAGMA_ONCE
#pragma once
#endif


#ifndef BALU_ENGINE_SCRIPT_CLASSES

#ifndef BALU_ENGINE_DISABLE_PRAGMA_ONCE
#include "ICallbacks.h"
#include "IPhysShape.h"
#include "ISpritePolygon.h"
#endif

namespace EngineInterface
{
	class IBaluPhysShapeInstance;
	class IBaluInstance;
	class IBaluClass;
}

#include <string>
#endif

namespace EngineInterface
{

#ifndef BALU_ENGINE_SCRIPT_CLASSES
	class IBaluSprite
	{
	public:
		static std::string GetDefaultName()
		{
			return "sprite";
		}
		virtual std::string GetName() = 0;
		virtual void SetName(std::string name) = 0;
		virtual void SetPhysShape(IBaluPhysShape* shape) = 0;
		virtual IBaluPhysShape* GetPhysShape() = 0;
		virtual void SetPhysShapeFromGeometry() = 0;
		virtual IBaluSpritePolygon* GetPolygon() = 0;
		virtual void AddOnCollide(IBaluClass* obstancle_class, CallbackWithData<CollideCallback> callback) = 0;
		virtual std::vector<std::pair<IBaluClass*, CallbackWithData<CollideCallback>>>& GetOnCollide() = 0;
		virtual void RemoveOnCollide(int index)=0;
	};
#endif

#ifdef BALU_ENGINE_SCRIPT_CLASSES
	BALU_ENGINE_SCRIPT_BEGIN_CLASS(WrapInterface, IBaluSprite, "ISprite");
	MUnpackA1(TYPE, SetPhysShape, WrapInterface<IBaluPhysShape>);
	MUnpackRA0(WrapPointer<IBaluPhysShape>, TYPE, GetPhysShape);
	MUnpackRA0(WrapPointer<IBaluSpritePolygon>, TYPE, GetPolygon);
	BALU_ENGINE_SCRIPT_END_CLASS;
#endif
}