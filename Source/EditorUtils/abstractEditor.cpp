#include "abstractEditor.h"

void OnMouseMove(TCallbackData* data, TMouseEventArgs e)
{
	TAbstractEditor* ed = (TAbstractEditor*)data->GetUserData();
	if (ed->current_local_editor != nullptr)
	{
	}
	else
	{
		auto tool = ed->GetActiveTool();
		if (tool != nullptr)
			tool->OnMouseMove(e);
	}
}

void OnMouseDown(TCallbackData* data, TMouseEventArgs e)
{
	TAbstractEditor* ed = (TAbstractEditor*)data->GetUserData();
	if (ed->current_local_editor != nullptr)
	{
	}
	else
	{
		auto tool = ed->GetActiveTool();
		if (tool != nullptr)
			tool->OnMouseDown(e);
	}
}

void OnMouseUp(TCallbackData* data, TMouseEventArgs e)
{
	TAbstractEditor* ed = (TAbstractEditor*)data->GetUserData();
	if (ed->current_local_editor != nullptr)
	{
	}
	else
	{
		auto tool = ed->GetActiveTool();
		if (tool != nullptr)
			tool->OnMouseUp(e);
	}
}

void TAbstractEditor::InitializeControls(IBaluWorld* world)
{
	this->world = world;
	world->AddOnMouseMove(CallbackWithData<MouseUpDownCallback>(OnMouseMove, &world->GetCallbacksActiveType(), this, TCallbacksActiveType::EDITOR));
	world->AddOnMouseDown(CallbackWithData<MouseUpDownCallback>(OnMouseDown, &world->GetCallbacksActiveType(), this, TCallbacksActiveType::EDITOR));
	world->AddOnMouseUp(CallbackWithData<MouseUpDownCallback>(OnMouseUp, &world->GetCallbacksActiveType(), this, TCallbacksActiveType::EDITOR));
}

void TAbstractEditor::DeinitializeControls()
{
	world->RemoveOnMouseMove(CallbackWithData<MouseUpDownCallback>(OnMouseMove, &world->GetCallbacksActiveType(), this, TCallbacksActiveType::EDITOR));
	world->RemoveOnMouseDown(CallbackWithData<MouseUpDownCallback>(OnMouseDown, &world->GetCallbacksActiveType(), this, TCallbacksActiveType::EDITOR));
	world->RemoveOnMouseUp(CallbackWithData<MouseUpDownCallback>(OnMouseUp, &world->GetCallbacksActiveType(), this, TCallbacksActiveType::EDITOR));
}

void TAbstractEditor::SetActiveTool(IEditorTool* tool)
{
	if (current_local_editor != nullptr)
	{
		return current_local_editor->SetActiveTool(tool);
	}
	else
		active_tool = tool;
}