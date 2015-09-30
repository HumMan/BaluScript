#pragma once

#include <EditorUtils/abstractEditor.h>

#include "spriteEditorScene.h"
#include "spriteEditorTools.h"

#include "../../Scene/IScene.h"
#include <World\IWorld.h>

using namespace EngineInterface;

#include <EditorUtils/DrawingHelper.h>

class TSpriteEditor :public TAbstractEditor
{

	TSpriteEditorScene scene;
	TSpriteEditorToolsRegistry tools_registry;
	std::unique_ptr<TDrawingHelper> drawing_helper;
public:
	TSpriteEditor();
	//void StartEdit(TBaluSpriteDef* use_sprite);
	//void EndEdit();

	void Initialize(TDrawingHelperContext drawing_context, IBaluWorld* world, IBaluSprite* edited_sprite, IBaluSceneInstance* editor_scene_instance);
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
	//void SetActiveTool(TEditorTool* tool);
};