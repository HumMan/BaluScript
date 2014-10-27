#include "baluEditor.h"

#include "Editors\abstractEditor.h"
//#include "Editors\BoundaryEditor.h"
#include "Editors\editorResourses.h"

#include "Editors\SpritePolygonEditor\spritePolygonEditor.h"
#include "Editors\SpriteEditor\spriteEditor.h"
#include "Editors\PhysBodyEditor\physBodyEditor.h"
#include "Editors\MaterialEditor\materialEditor.h"
#include "Editors\ClassEditor\classEditor.h"
//#include "Editors\SceneEditor\sceneEditor.h"

#include <baluRender.h>
#include <pugixml.hpp>

//enum class TCurrEditor
//{
//	SPRITE,
//	PHYS_SHAPE,
//	PHYS_BODY,
//	CLASS,
//	SCENE,
//
//	//testing only
//	BOUNDARY
//};

class TBaluEditorInternal
{
public:
	TBaluEditor* editor;

	std::unique_ptr<TBaluRender> render;
	std::unique_ptr<TBaluWorldDef> world;
	
	std::unique_ptr<TDrawingHelper> drawing_helper;

	std::unique_ptr<TEditorResourses> resourses;

	//TCurrEditor curr_editor_mode;

	//TSpriteEditor sprite_editor;
	//TBoundaryEditor boundary_editor;
	TAbstractEditor* active_editor;

	TVec2 screen_pos;
	TVec2 screen_size;

	bool screen_moving;
	TVec2i old_cursor_pos;

	TBaluEditorInternal(TBaluEditor* editor)// :sprite_editor(world.get())
	{
		this->editor = editor;
		screen_moving = false;
		active_editor = nullptr;
		//curr_editor_mode = TCurrEditor::BOUNDARY;
		//curr_editor_mode = TCurrEditor::SPRITE;
		//boundary_editor.objects.push_back(TBoundaryObjectBehaivor());
		//auto ed = new TPhysBodyEditor();
		//ed->SetActiveTool(new TCreatePolygonTool(ed));
		//active_editor = ed;
		
	}
};

void TBaluEditor::SetWorld(TBaluWorldDef* world)
{
	p->world.reset(world);
}
TBaluWorldDef* TBaluEditor::GetWorld()
{
	return p->world.get();
}

void TBaluEditor::AddSelectionChangedCallback(void* calle, SelectionChangedCallbackRefType MyCallback)
{
	SelectionChangedCallbackRef = MyCallback;
	SelectionChangedCallbackRef_calle = calle;
}

void TBaluEditor::RemoveSelectionChangedCallback(SelectionChangedCallbackRefType MyCallback)
{
	SelectionChangedCallbackRef = NULL;
	SelectionChangedCallbackRef_calle = NULL;
}

void TBaluEditor::AddPropertiesChangedCallback(void* calle, PropertiesChangedCallbackRefType MyCallback)
{
	PropertiesChangedCallbackRef = MyCallback;
	PropertiesChangedCallbackRef_calle = calle;
}
void TBaluEditor::RemovePropertiesChangedCallback(PropertiesChangedCallbackRefType MyCallback)
{
	PropertiesChangedCallbackRef = NULL;
	PropertiesChangedCallbackRef_calle = NULL;
}

void TBaluEditor::AddObjectCreatedCallback(void* calle, ObjectCreatedCallbackRefType MyCallback)
{
	ObjectCreatedCallbackRef = MyCallback;
	ObjectCreatedCallbackRef_calle = calle;
}
void TBaluEditor::RemoveObjectCreatedCallback(ObjectCreatedCallbackRefType MyCallback)
{
	ObjectCreatedCallbackRef = NULL;
	ObjectCreatedCallbackRef_calle = NULL;
}

TBaluEditor::TBaluEditor(int hWnd, TVec2i use_size)
{
	p.reset(new TBaluEditorInternal(this));
	p->screen_size = TVec2(10,10);
	p->screen_pos = TVec2(0, 0);
	p->render.reset(new TBaluRender(hWnd, use_size));
	p->world.reset(new TBaluWorldDef());
	p->resourses.reset(new TEditorResourses(p->render.get()));
	p->drawing_helper.reset(new TDrawingHelper(p->render.get(), p->resourses.get()));
	p->render->BeginScene();
}

TBaluEditor::~TBaluEditor()
{

}

void TBaluEditor::SetViewport(TVec2i use_size)
{

	p->render->Set.Viewport(use_size);
}

void TBaluEditor::Render()
{
	TMatrix<float, 4> ortho_m = TMatrix<float, 4>::GetOrtho(p->screen_pos, p->screen_size, -10, 10);
	TMatrix<float, 4> ortho_m_inv = ortho_m.GetInverted();
	p->render->Set.Projection(ortho_m);

	//p->render->BeginScene();
	if(true){
		p->render->Set.ClearColor(0, 0, 0);
		p->render->Set.Color(1, 1, 1, 1);
		p->render->Clear(true);

		//p->render->Texture.Enable(true);

		//p->render->AlphaTest.Enable(true);
		//p->render->Blend.Enable(true);

		//p->render->AlphaTest.Func(">=", 0.1);
		//p->render->Blend.Func("dc*(1-sa)+sc*sa");
		if (p->active_editor!=nullptr)
			p->active_editor->Render(p->drawing_helper.get());

		

		//p->render->Texture.Enable(false);
		//p->render->AlphaTest.Enable(false);
		//p->render->Blend.Enable(false);
	}
	p->render->EndScene();
}

TVec2 TBaluEditor::ScreenToWorld(const TVec2& v)
{
	return p->screen_pos + (v + TVec2(-0.5, -0.5)).ComponentMul(p->screen_size);
}

TVec2 TBaluEditor::DirScreenToWorld(const TVec2& v)
{
	return (v).ComponentMul(p->screen_size);
}

TVec2 TBaluEditor::WorldToScreen(const TVec2& v)
{
	return (v - p->screen_pos) / p->screen_size + TVec2(-0.5f, -0.5f);
}

TVec2 TBaluEditor::ScreenToWorld(const TVec2i& screen)
{
	TVec2i use_client_mouse_pos = screen;
	TVec2i screen_size = p->render->ScreenSize();
	TVec2 pos = TVec2(use_client_mouse_pos[0] / (float)screen_size[0], 1 - use_client_mouse_pos[1] / (float)screen_size[1]);
	TVec2 world_pos = ScreenToWorld(pos);
	return world_pos;
}

TVec2 TBaluEditor::DirScreenToWorld(const TVec2i& screen)
{
	TVec2i use_client_mouse_pos = screen;
	TVec2i screen_size = p->render->ScreenSize();
	TVec2 pos = TVec2(use_client_mouse_pos[0] / (float)screen_size[0], - use_client_mouse_pos[1] / (float)screen_size[1]);
	TVec2 world_pos = DirScreenToWorld(pos);
	return world_pos;
}

void TBaluEditor::OnMouseMove(TMouseEventArgs e)
{
	TVec2 world_pos = ScreenToWorld(e.location);
	if (p->screen_moving)
	{
		p->screen_pos -= DirScreenToWorld( e.location - p->old_cursor_pos);
		p->old_cursor_pos = e.location;
	}
	else
	{
		if (p->active_editor != nullptr)
		p->active_editor->OnMouseMove(e, world_pos);
	}
}

void TBaluEditor::OnMouseDown(TMouseEventArgs e)
{
	if (e.button == TMouseButton::Middle)
	{
		p->screen_moving = true;
		TVec2 world_pos = ScreenToWorld(e.location);
		p->old_cursor_pos = e.location;
	}
	else if (e.button == TMouseButton::Left)
	{
		TVec2 world_pos = ScreenToWorld(e.location);
		if (p->active_editor != nullptr)
		p->active_editor->OnMouseDown(e, world_pos);
	}
}

void TBaluEditor::OnMouseUp(TMouseEventArgs e)
{
	if (e.button == TMouseButton::Middle)
	{
		p->screen_moving = false;
	}
	else if (e.button == TMouseButton::Left)
	{
		TVec2 world_pos = ScreenToWorld(e.location);
		if (p->active_editor != nullptr)
		p->active_editor->OnMouseUp(e, world_pos);
	}
}

void TBaluEditor::OnMouseWheel(float delta)
{
	p->screen_size += (p->screen_size*0.1* delta / abs(delta));
}

std::string GetNewMaterialDefaultName()
{
	int static material_id = 0;
	material_id++;
	char buf[100];
	sprintf_s(buf, "material%i", material_id);
	return std::string(buf);
}

std::string GetNewSpriteDefaultName()
{
	int static sprites_id = 0;
	sprites_id++;
	char buf[100];
	sprintf_s(buf, "sprite%i", sprites_id);
	return std::string(buf);
}

std::string GetNewPhysBodyDefaultName()
{
	int static physBody_id = 0;
	physBody_id++;
	char buf[100];
	sprintf_s(buf, "physBody%i", physBody_id);
	return std::string(buf);
}

std::string GetNewClassDefaultName()
{
	int static class_id = 0;
	class_id++;
	char buf[100];
	sprintf_s(buf, "class%i", class_id);
	return std::string(buf);
}

std::string GetNewSceneDefaultName()
{
	int static scene_id = 0;
	scene_id++;
	char buf[100];
	sprintf_s(buf, "class%i", scene_id);
	return std::string(buf);
}

void TBaluEditor::CreateMaterial()
{
	auto new_name = GetNewMaterialDefaultName();
	p->world->materials[new_name].material_name = new_name;

	ObjectCreatedCallbackRef(ObjectCreatedCallbackRef_calle, &p->world->materials[new_name]);
	p->editor->Edit(&p->world->materials[new_name]);
}

void TBaluEditor::CreateSprite()
{
	auto new_name = GetNewSpriteDefaultName();
	p->world->sprites[new_name].sprite_name = new_name;

	ObjectCreatedCallbackRef(ObjectCreatedCallbackRef_calle, &p->world->sprites[new_name]);
	p->editor->Edit(&p->world->sprites[new_name]);
}

void TBaluEditor::CreatePhysBody()
{
	auto new_name = GetNewPhysBodyDefaultName();
	p->world->phys_bodies[new_name].phys_body_name = new_name;

	ObjectCreatedCallbackRef(ObjectCreatedCallbackRef_calle, &p->world->phys_bodies[new_name]);
	p->editor->Edit(&p->world->phys_bodies[new_name]);
}

void TBaluEditor::CreateClass()
{
	auto new_name = GetNewClassDefaultName();
	p->world->classes[new_name].class_name = new_name;

	ObjectCreatedCallbackRef(ObjectCreatedCallbackRef_calle, &p->world->classes[new_name]);
	p->editor->Edit(&p->world->classes[new_name]);
}

void TBaluEditor::CreateScene()
{
	auto new_name = GetNewSceneDefaultName();
	p->world->scenes[new_name].scene_name = new_name;

	ObjectCreatedCallbackRef(ObjectCreatedCallbackRef_calle, &p->world->scenes[new_name]);
	p->editor->Edit(&p->world->scenes[new_name]);
}

TAbstractEditor* GetEditorOfWorldObject(TWorldObjectDef* obj)
{
	if ((dynamic_cast<TBaluMaterialDef*>(obj)) != nullptr)
		return new TMaterialEditor();

	if ((dynamic_cast<TBaluSpritePolygonDef*>(obj)) != nullptr)
		return new TSpritePolygonEditor();

	if ((dynamic_cast<TBaluSpriteDef*>(obj)) != nullptr)
		return new TSpriteEditor();

	if ((dynamic_cast<TBaluPhysBodyDef*>(obj)) != nullptr)
		return new TPhysBodyEditor();

	if ((dynamic_cast<TBaluClass*>(obj)) != nullptr)
		return new TClassEditor();

	//if ((dynamic_cast<TBaluSceneDef*>(obj)) != nullptr)
	//	return new TSceneEditor();

	return nullptr;
}

void TBaluEditor::OnSelectionChangedEvent(TWorldObjectDef* old_selection, TWorldObjectDef* new_selection)
{
	SelectionChangedCallbackRef(SelectionChangedCallbackRef_calle, NULL, new_selection);
}

void TBaluEditor::Edit(TWorldObjectDef* obj_to_edit)
{
	SelectionChangedCallbackRef(SelectionChangedCallbackRef_calle, NULL, obj_to_edit);
	auto ed = GetEditorOfWorldObject(obj_to_edit);
	ed->OnSelectionChanged.connect(boost::bind(&TBaluEditor::OnSelectionChangedEvent, this, _1, _2));
	assert(ed != nullptr);
	if (ed != nullptr)
	{
		ed->Initialize(obj_to_edit,TVec2(0,0));
		p->active_editor = ed;
	}
}

void TBaluEditor::SetActiveTool(TEditorTool* tool)
{
	assert(p->active_editor != nullptr);
	p->active_editor->SetActiveTool(tool);
}

const std::vector<TToolWithDescription>& TBaluEditor::GetAvailableTools()
{
	assert(p->active_editor != nullptr);//�� ������ ���������� ���� �� ������� ��������
	return p->active_editor->GetAvailableTools();
}

bool TBaluEditor::CanSetSelectedAsWork()
{
	assert(p->active_editor != nullptr);
	return p->active_editor != nullptr && p->active_editor->CanSetSelectedAsWork();
}

void TBaluEditor::SetSelectedAsWork()
{
	assert(p->active_editor != nullptr);
	if(p->active_editor != nullptr)
		p->active_editor->SetSelectedAsWork();
}

bool TBaluEditor::CanEndSelectedAsWork()
{
	assert(p->active_editor != nullptr);
	return p->active_editor != nullptr && p->active_editor->CanEndSelectedAsWork();
}
void TBaluEditor::EndSelectedAsWork()
{
	assert(p->active_editor != nullptr);
	if (p->active_editor != nullptr)
		p->active_editor->EndSelectedAsWork();
}

bool TBaluEditor::ToolNeedObjectSelect(std::vector<TWorldObjectDef*>& selection_list)
{
	selection_list.clear();
	assert(p->active_editor != nullptr);
	if (p->active_editor != nullptr)
	{
		auto need_obj = p->active_editor->active_tool->NeedObjectSelect();
		switch (need_obj)
		{
		case TWorldObjectType::Material:
			for (const auto& v : p->world->materials)
				selection_list.push_back((TWorldObjectDef*)&(v.second));
			break;
		case TWorldObjectType::Sprite:
			for (const auto& v : p->world->sprites)
				selection_list.push_back((TWorldObjectDef*)&(v.second));
			break;
		case TWorldObjectType::PhysBody:
			for (const auto& v : p->world->phys_bodies)
				selection_list.push_back((TWorldObjectDef*)&(v.second));
			break;
		case TWorldObjectType::Class:
			for (const auto& v : p->world->classes)
				selection_list.push_back((TWorldObjectDef*)&(v.second));
			break;
		default:
			assert(false);
			return false;
			break;
		}
		return true;
	}
	return false;
}

void TBaluEditor::SetToolSelectedObject(std::string obj_name)
{
	assert(p->active_editor != nullptr);
	if (p->active_editor != nullptr)
	{
		auto need_obj = p->active_editor->active_tool->NeedObjectSelect();
		switch (need_obj)
		{
		case TWorldObjectType::Material:
			p->active_editor->active_tool->SetSelectedObject(&p->world->materials[obj_name]);
			break;
		case TWorldObjectType::Sprite:
			p->active_editor->active_tool->SetSelectedObject(&p->world->sprites[obj_name]);
			break;
		case TWorldObjectType::PhysBody:
			p->active_editor->active_tool->SetSelectedObject(&p->world->phys_bodies[obj_name]);
			break;
		case TWorldObjectType::Class:
			p->active_editor->active_tool->SetSelectedObject(&p->world->classes[obj_name]);
			break;
		default:
			assert(false);
			break;
		}
	}
}

void TBaluEditor::SaveWorldTo(std::string path)
{
	pugi::xml_document doc;
	auto doc_el = doc.append_child("BaluEditorWorldFile");
	p->world->Save(doc_el,1);
	doc.save_file(path.c_str());
}

void TBaluEditor::LoadWorldFrom(std::string path)
{
	pugi::xml_document doc;
	doc.load_file(path.c_str());
	p->world->Load(doc.child("BaluEditorWorldFile"), 1);
}