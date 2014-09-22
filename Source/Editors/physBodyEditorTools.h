#pragma once

#include "BoundaryEditor.h"

class TPhysBodyEditor;

class TCreateFixtureTool : public TEditorTool
{
};

class TCreatePolygonTool : public TCreateFixtureTool
{
	TPhysBodyEditor* phys_body_editor;
public:
	TCreatePolygonTool(TPhysBodyEditor* phys_body_editor);
	void OnMouseDown(TMouseEventArgs e, TVec2 world_cursor_location);
	void OnMouseMove(TMouseEventArgs e, TVec2 wolrd_cursor_location);
	void OnMouseUp(TMouseEventArgs e, TVec2 wolrd_cursor_location);
	void Render(TDrawingHelper* drawing_helper);
};

class TCreateJointTool :public  TEditorTool
{

};

class TCreatePrismaticJointTool :public TCreateJointTool
{

};

class TPolygonShapeAdornment : public  TBoundaryBoxAdornment
{
public:
	TPolygonShapeAdornment(TVec2 pos);
	void OnBoxChange(TOBB<float, 2> old_box, TOBB<float, 2> new_box);
	void Render(TDrawingHelper* drawing_helper);
};
