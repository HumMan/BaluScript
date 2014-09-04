
#pragma once

#include "../baluEditorDefs.h"

class TBaluRender;
class TEditorResourses;

class TDrawingHelper
{
	TBaluRender* render;
	TEditorResourses* resources;
public:
	TDrawingHelper(TBaluRender* render, TEditorResourses* resources);
	void DrawSprite(TBaluSpriteDef* sprite);
	void DrawLine( TVec2 p0, TVec2 p1);
	void DrawPoint( TVec2 p);
	void DrawSpriteContour(TBaluSpriteDef* sprite);
	void ActivateMaterial( TBaluMaterialDef* material);
	void DeactivateMaterial( TBaluMaterialDef* material);
	void DrawBoundary(TOBB<float, 2> boundary);

	void SetSelectedPointColor();
	void SetSelectedBoundaryColor();
	void UnsetColor();
};