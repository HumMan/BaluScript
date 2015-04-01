#pragma once

#include "../../../BaluLib/Source/Math/vec.h"
#include "../../../BaluLib/Source/Math/matrix.h"
#include "../../../BaluLib/Source/BVolumes/OBB.h"

#include <string>

#include "IProperties.h"

struct TRot
{
	TRot() {}
	TRot(float s, float c)
	{
		this->s = s;
		this->c = c;
	}
	explicit TRot(float angle)
	{
		Set(angle);
	}
	explicit TRot(const TOBB2& box)
	{
		c = box.orient[0][0];
		s = box.orient[0][1];
	}
	void Set(float angle)
	{
		s = sinf(angle);
		c = cosf(angle);
	}
	void SetIdentity()
	{
		s = 0.0f;
		c = 1.0f;
	}
	float GetAngle() const
	{
		return atan2(s, c);
	}
	TVec2 GetXAxis() const
	{
		return TVec2(c, s);
	}
	TVec2 GetYAxis() const
	{
		return TVec2(-s, c);
	}
	float s, c;
};

class TBaluTransform
{
public:
	TVec2 position;
	TRot angle;
	TBaluTransform()
	{
		position = TVec2(0, 0);
		angle = TRot(0);
	}
	TBaluTransform(TVec2 position, TRot angle)
	{
		this->position = position;
		this->angle = angle;
	}
	TMatrix2 GetOrientation()
	{
		return TMatrix2(angle.GetXAxis(), angle.GetYAxis());
	}
	TVec2 ToGlobal(TVec2 p)
	{
		return GetOrientation()*p + position;
	}
	TVec2 ToLocal(TVec2 p)
	{
		return GetOrientation().TransMul((p-position));
	}

	TOBB2 ToGlobal(TOBB2 box)
	{
		//TODO
		return TOBB2(box.pos+position, GetOrientation()*box.orient, box.local);
	}
	TOBB2 ToGlobal(TAABB2 box)
	{
		//TODO
		return TOBB2(position, GetOrientation(), box);
	}
	TBaluTransform ToGlobal(TBaluTransform local)
	{
		TBaluTransform global;
		global.position = position + GetOrientation()*local.position;
		global.angle = TRot(angle.GetAngle() + local.angle.GetAngle());
		return global;
	}
};

inline TVec2 Transform(TVec2 vertex, TVec2 scale, TBaluTransform transform)
{
	return transform.ToGlobal((vertex).ComponentMul(scale));
}


namespace EngineInterface
{
	class IViewport
	{
	public:
		virtual void SetTransform(TBaluTransform transform) = 0;
		virtual TBaluTransform GetTransform() = 0;
		virtual void SetAspectRatio(float aspect) = 0;
		virtual void SetWidth(float width) = 0;
		virtual TAABB2 GetAABB() = 0;
		virtual void SetSize(TVec2 size) = 0;
		virtual TVec2 GetSize() = 0;
	};

#ifdef BALU_ENGINE_SCRIPT_CLASSES

	void IViewport_GetSize(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
	{
		*result.get_as<TVec2>() = (*object.get_as<IViewport*>())->GetSize();
	}

	void IViewport_SetSize(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
	{
		(*object.get_as<IViewport*>())->SetSize(*(formal_params[0].get_as<TVec2>()));
	}

	void IViewport_register(TClassRegistryParams& params)
	{
		auto scl = RegisterExternClass(params,
			"class extern IViewport\n"
			"{\n"
			"func GetSize():vec2;\n"
			"func SetSize(vec2 size);\n"
			"}\n",
			sizeof(IViewport*));
		RegisterMethod(params, scl, "GetSize", IViewport_GetSize);
		RegisterMethod(params, scl, "SetSize", IViewport_SetSize);
	}
	static bool IViewport_registered = TScriptClassesRegistry::Register("IViewport", IViewport_register);
#endif

	class IBaluMaterial
	{
	public:
		static std::string GetDefaultName()
		{
			return "material";
		}
		virtual std::string GetName() = 0;
		virtual void SetImagePath(std::string image_path) = 0;
		virtual std::string GetImagePath() = 0;
		virtual void SetColor(TVec4 color) = 0;
	};
}

class TScreen;
class TView;

namespace EngineInterface
{
	class IViewport;
}

struct TDrawingHelperContext
{
	TScreen* screen;
	TView* view;
	EngineInterface::IViewport* viewport;
	TDrawingHelperContext()
	{
		screen = nullptr;
		view = nullptr;
		viewport = nullptr;
	}
};