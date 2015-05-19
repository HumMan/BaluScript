#include "Composer.h"

#include "Render.h"

#include "baluRender.h"

using namespace EngineInterface;

class TComposerPrivate
{
public:
	std::vector<TComposerStage> stages;
};

TComposerStage* TComposer::AddToRender(EngineInterface::IBaluSceneInstance* scene_instance, EngineInterface::IViewport* viewport)
{
	p->stages.emplace_back();
	p->stages.back().scene_instance = scene_instance;
	p->stages.back().viewport = viewport;
	return nullptr;
}

void TComposer::RemoveFromRender(EngineInterface::IComposerStage* stage)
{

}

void TComposer::Render(EngineInterface::TRender* render)
{
	auto internal_render = render->GetInternalRender();
	auto screen = TScreen(internal_render->Get.Viewport());

	for (auto& v : p->stages)
	{
		auto main_viewport = v.viewport;

		std::vector<TRenderCommand> render_commands;
		std::vector<TCustomDrawCommand> custom_draw_commands;
		auto viewport_aabb = main_viewport->GetAABB();
		v.scene_instance->QueryAABB(viewport_aabb, render_commands, custom_draw_commands);

		//TODO ��� �� ����� ������� viewport_view
		auto main_viewport_view = TView(TVec2(0.5, 0.5), TVec2(1, 1));

		for (auto& v : custom_draw_commands)
		{
			v.screen = &screen;
			v.view = &main_viewport_view;
			v.viewport = &viewport_aabb;
		}

		//render->EnableScissor(true);
		//render->SetScissorRect(*screen, main_viewport_view);
		render->Render(render_commands, custom_draw_commands, main_viewport);
		//render->EnableScissor(false);
	}		
}

TComposer::TComposer()
{
	p = std::make_unique<TComposerPrivate>();
}

TComposer::~TComposer()
{

}