#include "Render.h"

#include "baluRender.h"


//#include <GL\glew.h>
//#include <GL\wglew.h>

#include "nanovg_support.h"


using namespace TBaluRenderEnums;

TRender::TRender(TBaluRender* internal_render)
{
	render = internal_render;
	nanovg_init();
}

void TRender::Render(std::vector<TRenderCommand>& render_commands)
{
	render->Set.ModelView(TMatrix4::GetOrtho(TVec2(0, 0), TVec2(20, 20), -1, 1));
	//render->AlphaTest.Enable(true);
	//render->AlphaTest.Func(TAlphaTestFunc::AT_GREATER, 0.9);
	render->Blend.Enable(true);
	render->Blend.Func(TBlendEquation::BE_SRC_ALPHA, TBlendFunc::BF_ADD, TBlendEquation::BE_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < render_commands.size(); i++)
	{
		auto& c = render_commands[i];
		auto tex = c.material_id->GetTexture();
		render->Texture.Bind(*(TTextureId*)&tex);
		TStreamsDesc streams;
		streams.AddStream(TStream::Vertex, TDataType::Float, 2, c.vertices);
		streams.AddStream(TStream::TexCoord, TDataType::Float, 2, c.tex_coords);
		//streams.AddStream(TStream::Color, TDataType::Float, 4, c.colors);
		render->Draw(streams, TPrimitive::Triangles, c.vertices_count);
	}
	//glDisable(GL_DEPTH_TEST);

	render_test();
}