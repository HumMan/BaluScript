#pragma once

#include "IMaterial.h"
#include "IAnimationFrames.h"

#include "../../BaluLib/Source/Math/vec.h"

#include "../RenderCommand.h"

#include <vector>

namespace EngineInterface
{
	class IBaluSpritePolygon
	{
	public:

		virtual bool IsEnable() = 0;
		virtual void SetEnable(bool enable)=0 ;
		virtual void AddAnimDesc(TAnimDesc* desc) = 0;
		virtual void CreateAnimationLine(std::string line_name, std::vector<TAnimationFrames> frames) = 0;
		virtual void CreateAnimationLine(std::string line_name, TAnimDesc* desc, std::vector<int> frames) = 0;

		virtual IBaluMaterial* GetMaterial() = 0;
		virtual void SetMaterial(IBaluMaterial* material) = 0;
		virtual void SetPolygonVertices(std::vector<TVec2> polygon_vertices) = 0;
		virtual void SetAsBox(float width, float height) = 0;

		virtual void SetVertices(std::vector<TVec2> vertices) = 0;
		virtual std::vector<TVec2> GetVertices() = 0;

		virtual std::vector<TVec2> GetTexCoords() = 0;

		virtual int GetVerticesCount() = 0;
		virtual void SetVertex(int id, TVec2 pos) = 0;
		virtual TVec2 GetPolygonVertex(int id) = 0;
		virtual TVec2 GetVertex(int id) = 0;

		virtual void SetTexCoordsFromVertices(TVec2 origin, TVec2 scale) = 0;
		virtual void SetTexCoordsFromVerticesByRegion(TVec2 left_bottom, TVec2 right_top) = 0;

		virtual void OnCustomDraw(TCustomDrawCallback callback) = 0;
		virtual void OnCustomDraw(TCustomDrawCallback callback, void* user_data)=0;

		virtual bool IsCustomDraw() = 0;

	};
}