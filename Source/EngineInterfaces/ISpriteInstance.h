#pragma once

#include "ISprite.h"
#include "IPhysShapeInstance.h"
#include "ISpritePolygonInstance.h"

namespace EngineInterface
{
	class IBaluSpriteInstance
	{
	public:
		//virtual void SetTransform(TBaluTransform local)=0;
		//virtual TBaluTransform GetTransform() = 0;
		virtual IBaluSprite* GetSourceSprite() = 0;
		virtual IBaluPhysShapeInstance* GetPhysShape() = 0;

		virtual IBaluSpritePolygonInstance* GetPolygon() = 0;
	};
}