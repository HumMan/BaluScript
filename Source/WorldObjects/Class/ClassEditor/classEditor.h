#pragma once

#include <EditorUtils\abstractEditor.h>

#include "classEditorScene.h"
#include "classEditorTools.h"

#include <World/IWorld.h>

using namespace EngineInterface;

#include <EditorUtils\DrawingHelper.h>

class TClassEditor :public TAbstractEditor
{

	TClassEditorScene scene;
	TClassEditorToolsRegistry tools_registry;
	std::unique_ptr<TDrawingHelper> drawing_helper;
public:
	TClassEditor();

	void Initialize(TDrawingHelperContext drawing_context, IBaluWorld* world, IBaluClass* edited_class, IBaluSceneInstance* editor_scene_instance);

	void Deinitialize()
	{
		scene.Deinitialize();
		drawing_helper.reset();
		DeinitializeControls();
	}

	bool CanSetSelectedAsWork();
	void SetSelectedAsWork();

	bool CanEndSelectedAsWork();
	bool EndSelectedAsWork();

	const std::vector<TToolWithDescription>& GetAvailableTools();
};