#pragma once

#include "IClass.h"

#include "ISpriteInstance.h"

namespace EngineInterface
{
	class ISensorInstance
	{

	};

	class IBaluClassPhysBodyIntance
	{
	public:
		virtual TVec2 GetLinearVelocity() = 0;
		virtual void SetLinearVelocity(TVec2 velocity) = 0;
	};

#ifdef BALU_ENGINE_SCRIPT_CLASSES	
	BALU_ENGINE_SCRIPT_BEGIN_CLASS(WrapInterface, IBaluClassPhysBodyIntance, "IClassPhysBodyInstance");
	MUnpackRA0(WrapValue<TVec2>, TYPE, GetLinearVelocity);
	MUnpackA1(TYPE, SetLinearVelocity, WrapValue<TVec2>);
	BALU_ENGINE_SCRIPT_END_CLASS(WrapInterface<IBaluClassPhysBodyIntance>);
#endif

	class ISkeletonAnimationInstance
	{
	public:
		virtual void Update(float step) = 0;
		virtual void PlayAnimation(std::string name, float alpha) = 0;
		virtual void StopAnimation(std::string name) = 0;
	};
#ifdef BALU_ENGINE_SCRIPT_CLASSES	
	BALU_ENGINE_SCRIPT_BEGIN_CLASS(WrapInterface, ISkeletonAnimationInstance, "ISkeletonAnimationInstance");
	MUnpackA2(TYPE, PlayAnimation, TStringWrapper<std::string>, WrapValue<float>);
	MUnpackA1(TYPE, StopAnimation, TStringWrapper<std::string>);
	BALU_ENGINE_SCRIPT_END_CLASS(WrapInterface<ISkeletonAnimationInstance>);
#endif
	class IBaluInstance
	{
	public:
		virtual void UpdateTranform() = 0;
		virtual TOBB2 GetOBB() = 0;
		virtual IBaluClass* GetClass() = 0;
		virtual TBaluTransform GetTransform() = 0;
		virtual void SetTransform(TBaluTransform) = 0;
		virtual TVec2 GetScale() = 0;
		virtual void SetScale(TVec2 scale) = 0;
		virtual int GetSpritesCount() = 0;
		virtual IBaluSpriteInstance* GetSprite(int index) = 0;
		virtual IProperties* GetProperties() = 0;
		virtual IBaluClassPhysBodyIntance* GetPhysBody() = 0;
		virtual ISkeletonAnimationInstance* GetSkeletonAnimation() = 0;
	};

#ifdef BALU_ENGINE_SCRIPT_CLASSES
	BALU_ENGINE_SCRIPT_BEGIN_CLASS(WrapInterface, IBaluInstance, "IInstance");
	MUnpackRA0(WrapPointer<IProperties>, WrapInterface<IBaluInstance>, GetProperties);
	MUnpackRA0(WrapPointer<IBaluClassPhysBodyIntance>, WrapInterface<IBaluInstance>, GetPhysBody);
	BALU_ENGINE_SCRIPT_END_CLASS(WrapInterface<IBaluInstance>);
#endif
}