
#include <baluLib.h>

#include "../Source/EngineInterfaces.h"

using namespace EngineInterface;

std::string base_path;

IBaluWorldInstance* demo_world_instance;
IBaluSceneInstance* scene_instance;

void PlayerJump(IBaluInstance* object)
{

	if (object->GetProperties()->GetBool("can_jump"))
	{
		auto speed = object->GetPhysBody()->GetLinearVelocity();
		speed[1] = 4;
		object->GetPhysBody()->SetLinearVelocity(speed);
	}
}
void PlayerLeft(IBaluInstance* object)
{
	float mult = (object->GetBool("can_jump")) ? 1 : 0.8;
	auto speed = object->GetPhysBody()->GetLinearVelocity();
	speed[0] = -1.4*mult;
	object->GetPhysBody()->SetLinearVelocity(speed);
}
void PlayerRight(IBaluInstance* object)
{
	float mult = (object->GetBool("can_jump")) ? 1 : 0.8;
	auto speed = object->GetPhysBody()->GetLinearVelocity();
	speed[0] = 1.4*mult;
	object->GetPhysBody()->SetLinearVelocity(speed);
}

void BonesPlayerLeft(IBaluInstance* object)
{
	object->GetSkeletonAnimation().PlayAnimation("walk", 1);
}


void InitDemoWorld()
{
	auto demo_world = CreateDemoWorld();

	demo_world_instance = new TBaluWorldInstance(demo_world, resources);
	scene_instance = demo_world_instance->RunScene(demo_world->GetScene("scene0"));
}

void PlayerCustomDraw(TBaluInstance* object, NVGcontext* vg)
{

}

//void PlayerJumpSensorCollide(TBaluInstance* source, TSensorInstance* sensor, TBaluInstance* obstacle, TBaluPhysShapeInstance* obstacle_shape)
//{
//	source->SetBool("can_jump", true);
//}

std::vector<TBaluPhysShapeInstance*> obstacle_shapes;

void PlayerJumpSensorBeginCollide(IBaluInstance* source, TSensorInstance* sensor, IBaluInstance* obstacle, TBaluPhysShapeInstance* obstacle_shape)
{
	auto it = std::find(obstacle_shapes.begin(), obstacle_shapes.end(), obstacle_shape);
	if (it == obstacle_shapes.end())
	{
		obstacle_shapes.push_back(obstacle_shape);
	}
	source->SetBool("can_jump", obstacle_shapes.size()>0);
}

void PlayerJumpSensorEndCollide(IBaluInstance* source, TSensorInstance* sensor, IBaluInstance* obstacle, TBaluPhysShapeInstance* obstacle_shape)
{

	auto it = std::find(obstacle_shapes.begin(), obstacle_shapes.end(), obstacle_shape);
	if (it != obstacle_shapes.end())
	{
		obstacle_shapes.erase(it);
	}
	source->SetBool("can_jump", obstacle_shapes.size()>0);
}
void PlayerPrePhysStep(TBaluInstance* object)
{
	auto can_jump = object->GetBool("can_jump");

	auto speed = object->GetPhysBody()->GetLinearVelocity();
	std::string hor_anim, v_anim;
	if (speed[0] > 0)
		hor_anim = "_right";
	//
	else
		hor_anim = "_left";
	if (can_jump)
	{
		if (abs(speed[0]) > 0.5)
			v_anim = "run";
		else
			v_anim = "stay";
	}
	else
	{
		if (speed[1] > 0)
			v_anim = "jump_up";
		else
			v_anim = "jump_down";
	}
	object->GetSprite(0)->GetPolygon().SetActiveAnimation((v_anim + hor_anim).c_str());
}

void BonesPlayerPrePhysStep(TBaluInstance* object)
{
	auto can_jump = object->GetBool("can_jump");

	auto speed = object->GetPhysBody()->GetLinearVelocity();
	std::string hor_anim, v_anim;
	if (abs(speed[0]) > 0)
		object->GetSkeletonAnimation().PlayAnimation("walk", 1);
	else
		object->GetSkeletonAnimation().StopAnimation("walk");
}

TBaluWorld* CreateDemoWorld()
{
	auto world = new TBaluWorld();

	auto brick_mat = world->CreateMaterial("brick");

	brick_mat->SetImagePath(base_path + "\\textures\\brick.png");
	brick_mat->SetColor(TVec4(1, 1, 1, 1));

	auto box_sprite = world->CreateSprite("box0");
	box_sprite->GetPolygone().SetMaterial(brick_mat);
	box_sprite->GetPolygone().SetAsBox(1, 1);
	box_sprite->GetPolygone().SetTexCoordsFromVertices(TVec2(-0.5, -0.5), TVec2(1, 1));
	box_sprite->SetPhysShape(new TBaluBoxShape(1, 1));

	auto box_class = world->CreateClass("box");
	auto box_class_instance = box_class->AddSprite(box_sprite);
	box_class->GetPhysBody().Enable(true);
	box_class->GetPhysBody().SetPhysBodyType(TPhysBodyType::Static);

	auto player_mat = world->CreateMaterial("player_skin");
	player_mat->SetImagePath(base_path + "\\textures\\player.png");
	auto player_sprite = world->CreateSprite("player");

	player_sprite->GetPolygone().SetMaterial(player_mat);
	//player_sprite->GetPolygone().SetAsBox(20.0/8, 20.0/8);
	player_sprite->GetPolygone().SetAsBox(6, 6);
	//player_sprite->GetPolygone().SetTexCoordsFromVertices(TVec2(0, 0), TVec2(1, 1));
	player_sprite->SetPhysShape(new TBaluCircleShape(2.5));
	//player_sprite->SetPhysShape(new TBaluBoxShape(0.5,2));

	TGridFrames* grid_frames = new TGridFrames(TVec2(0, 0), TVec2(1, 1), 8, 4);

	player_sprite->GetPolygone().AddAnimDesc(grid_frames);

	player_sprite->GetPolygone().CreateAnimationLine("run_right", grid_frames, FramesRange(0, 7));
	player_sprite->GetPolygone().CreateAnimationLine("jump_up_right", grid_frames, FramesRange(9, 9));
	player_sprite->GetPolygone().CreateAnimationLine("jump_down_right", grid_frames, FramesRange(10, 10));
	player_sprite->GetPolygone().CreateAnimationLine("stay_right", grid_frames, FramesRange(11, 11));
	player_sprite->GetPolygone().CreateAnimationLine("run_right", grid_frames, FramesRange(0, 7));
	player_sprite->GetPolygone().CreateAnimationLine("run_left", grid_frames, FramesRange(16, 16 + 7));
	player_sprite->GetPolygone().CreateAnimationLine("jump_up_left", grid_frames, FramesRange(16 + 7 + 2, 16 + 7 + 2));
	player_sprite->GetPolygone().CreateAnimationLine("jump_down_left", grid_frames, FramesRange(16 + 7 + 3, 16 + 7 + 3));
	player_sprite->GetPolygone().CreateAnimationLine("stay_left", grid_frames, FramesRange(16 + 7 + 4, 16 + 7 + 4));

	auto player_class = world->CreateClass("player");
	auto player_class_instance = player_class->AddSprite(player_sprite);
	player_class_instance->tag = "character_sprite";
	player_class->GetPhysBody().Enable(true);
	player_class->GetPhysBody().SetPhysBodyType(TPhysBodyType::Dynamic);
	player_class->GetPhysBody().SetFixedRotation(true);

	auto sensor = player_class->GetPhysBody().CreateSensor(new TBaluCircleShape(0.4, TVec2(0, -2.5)));

	player_class->OnKeyDown(TKey::Up, PlayerJump);
	player_class->OnKeyDown(TKey::Left, PlayerLeft);
	player_class->OnKeyDown(TKey::Right, PlayerRight);

	player_class->OnBeforePhysicsStep(PlayerPrePhysStep);
	//player_class->OnSensorCollide(sensor, PlayerJumpSensorCollide);
	player_class->OnBeginContact(sensor, PlayerJumpSensorBeginCollide);
	player_class->OnEndContact(sensor, PlayerJumpSensorEndCollide);

	auto bones_player = world->CreateClass("bones");
	bones_player->OnKeyDown(TKey::Left, BonesPlayerLeft);
	bones_player->OnBeforePhysicsStep(BonesPlayerPrePhysStep);
	{
		auto bones_mat = world->CreateMaterial("zombie");

		bones_mat->SetImagePath(base_path + "\\textures\\zombie.png");
		bones_mat->SetColor(TVec4(1, 1, 1, 1));

		//special phys sprite
		auto bones_phys_sprite = world->CreateSprite("bones_phys_sprite");
		bones_phys_sprite->SetPhysShape(new TBaluCircleShape(1.0, TVec2(0, -1.5)));
		bones_phys_sprite->GetPolygone().SetEnable(false);

		auto bones_player_class_instance = bones_player->AddSprite(bones_phys_sprite);
		bones_player->GetPhysBody().Enable(true);
		bones_player->GetPhysBody().SetPhysBodyType(TPhysBodyType::Dynamic);
		bones_player->GetPhysBody().SetFixedRotation(true);

		//sprites
		auto bones_head = world->CreateSprite("bones_head");
		bones_head->GetPolygone().SetMaterial(bones_mat);
		bones_head->GetPolygone().SetAsBox(2, 2);
		bones_head->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(0, 256 - 256) / 256, TVec2(73, 256 - 192) / 256);

		auto bones_torso = world->CreateSprite("bones_torso");
		bones_torso->GetPolygone().SetMaterial(bones_mat);
		bones_torso->GetPolygone().SetAsBox(1.6, 2.5);
		bones_torso->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(0, 256 - 189) / 256, TVec2(53, 256 - 112) / 256);

		//hands

		auto bones_left_shoulder = world->CreateSprite("bones_left_shoulder");
		bones_left_shoulder->GetPolygone().SetMaterial(bones_mat);
		bones_left_shoulder->GetPolygone().SetAsBox(1, 1);
		bones_left_shoulder->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(113, 256 - 190) / 256, TVec2(152, 256 - 142) / 256);

		auto bones_left_hand = world->CreateSprite("bones_left_hand");
		bones_left_hand->GetPolygone().SetMaterial(bones_mat);
		bones_left_hand->GetPolygone().SetAsBox(1, 1);
		bones_left_hand->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(75, 256 - 256) / 256, TVec2(138, 256 - 193) / 256);

		auto bones_right_shoulder = world->CreateSprite("bones_right_shoulder");
		bones_right_shoulder->GetPolygone().SetMaterial(bones_mat);
		bones_right_shoulder->GetPolygone().SetAsBox(1, 1);
		bones_right_shoulder->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(152, 256 - 192) / 256, TVec2(183, 256 - 143) / 256);

		auto bones_right_hand = world->CreateSprite("bones_right_hand");
		bones_right_hand->GetPolygone().SetMaterial(bones_mat);
		bones_right_hand->GetPolygone().SetAsBox(1, 1);
		bones_right_hand->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(58, 256 - 190) / 256, TVec2(113, 256 - 129) / 256);

		//legs

		auto bones_left_leg = world->CreateSprite("bones_left_leg");
		bones_left_leg->GetPolygone().SetMaterial(bones_mat);
		bones_left_leg->GetPolygone().SetAsBox(1, 1);
		bones_left_leg->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(55, 256 - 72) / 256, TVec2(94, 256 - 14) / 256);

		auto bones_left_foot = world->CreateSprite("bones_left_foot");
		bones_left_foot->GetPolygone().SetMaterial(bones_mat);
		bones_left_foot->GetPolygone().SetAsBox(1, 1);
		bones_left_foot->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(54, 256 - 131) / 256, TVec2(94, 256 - 75) / 256);

		auto bones_right_leg = world->CreateSprite("bones_right_leg");
		bones_right_leg->GetPolygone().SetMaterial(bones_mat);
		bones_right_leg->GetPolygone().SetAsBox(1, 1);
		bones_right_leg->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(0, 256 - 55) / 256, TVec2(44, 256 - 7) / 256);

		auto bones_right_foot = world->CreateSprite("bones_right_foot");
		bones_right_foot->GetPolygone().SetMaterial(bones_mat);
		bones_right_foot->GetPolygone().SetAsBox(1, 1);
		bones_right_foot->GetPolygone().SetTexCoordsFromVerticesByRegion(TVec2(9, 256 - 112) / 256, TVec2(39, 256 - 56) / 256);

		auto& bones_player_skel = bones_player->GetSkeleton();

		//bones
		auto root_bone = bones_player_skel.CreateBone(nullptr);

		auto right_leg_bone = bones_player_skel.CreateBone(root_bone);
		right_leg_bone->SetTransform(TBaluTransform(TVec2(-0.5, -1.5), b2Rot(0)));
		auto right_foot_bone = bones_player_skel.CreateBone(right_leg_bone);
		right_foot_bone->SetTransform(TBaluTransform(TVec2(0, -1), b2Rot(0)));

		auto left_leg_bone = bones_player_skel.CreateBone(root_bone);
		left_leg_bone->SetTransform(TBaluTransform(TVec2(0.5, -1), b2Rot(0)));
		auto left_foot_bone = bones_player_skel.CreateBone(left_leg_bone);
		left_foot_bone->SetTransform(TBaluTransform(TVec2(0.0, -0.5), b2Rot(0)));

		auto right_shoulder_bone = bones_player_skel.CreateBone(root_bone);
		auto right_hand_bone = bones_player_skel.CreateBone(right_shoulder_bone);

		auto left_shoulder_bone = bones_player_skel.CreateBone(root_bone);
		auto left_hand_bone = bones_player_skel.CreateBone(left_shoulder_bone);

		//create skin
		auto skin = bones_player_skel.CreateSkin();
		skin->SetBoneSprite(bones_player_skel.GetBoneIndex(root_bone), bones_head, TBaluTransform(TVec2(-0.3, 2), b2Rot(0)));

		skin->SetBoneSprite(bones_player_skel.GetBoneIndex(root_bone), bones_torso, TBaluTransform(TVec2(0, 0), b2Rot(0)));

		skin->SetBoneSprite(bones_player_skel.GetBoneIndex(right_leg_bone), bones_right_leg, TBaluTransform(TVec2(0, 0), b2Rot(0)));
		skin->SetBoneSprite(bones_player_skel.GetBoneIndex(right_foot_bone), bones_right_foot, TBaluTransform(TVec2(0, 0), b2Rot(0)));

		skin->SetBoneSprite(bones_player_skel.GetBoneIndex(left_leg_bone), bones_left_leg, TBaluTransform(TVec2(0, -0.5), b2Rot(0)));
		skin->SetBoneSprite(bones_player_skel.GetBoneIndex(left_foot_bone), bones_left_foot, TBaluTransform(TVec2(0, -1), b2Rot(0)));

		//walk animation
		auto& skel_anim = bones_player->GetSkeletonAnimation();

		auto walk_anim = skel_anim.CreateAnimation("walk");
		float timeline_size = 0.5;
		walk_anim->SetTimelineSize(timeline_size);
		auto left_leg_bone_track = walk_anim->CreateTrack(left_leg_bone);

		left_leg_bone_track->CreateFrame(0, 0);
		left_leg_bone_track->CreateFrame(0.25* timeline_size, DegToRad(40.0));
		left_leg_bone_track->CreateFrame(0.75* timeline_size, DegToRad(-40.0));

		auto left_foot_bone_track = walk_anim->CreateTrack(left_foot_bone);

		left_foot_bone_track->CreateFrame(0, 0);
		left_foot_bone_track->CreateFrame(0.25* timeline_size, DegToRad(40.0));
		left_foot_bone_track->CreateFrame(0.75* timeline_size, DegToRad(-40.0));

		auto right_leg_bone_track = walk_anim->CreateTrack(right_leg_bone);

		right_leg_bone_track->CreateFrame(0, 0);
		right_leg_bone_track->CreateFrame(0.25* timeline_size, DegToRad(-40.0));
		right_leg_bone_track->CreateFrame(0.75* timeline_size, DegToRad(40.0));

		auto right_foot_bone_track = walk_anim->CreateTrack(right_foot_bone);

		right_foot_bone_track->CreateFrame(0, 0);
		right_foot_bone_track->CreateFrame(0.25* timeline_size, DegToRad(-40.0));
		right_foot_bone_track->CreateFrame(0.75* timeline_size, DegToRad(40.0));
	}

	auto scene0 = world->CreateScene("scene0");

	for (int i = 0; i < 1; i++)
	{
		auto inst0 = scene0->CreateInstance(player_class);
		inst0->transform = TBaluTransform(TVec2(0, 0 + i), b2Rot(0));

		auto inst1 = scene0->CreateInstance(bones_player);
		inst1->transform = TBaluTransform(TVec2(3, 0 + i), b2Rot(0));
	}

	for (int i = -10; i < 40; i++)
	{
		auto inst0 = scene0->CreateInstance(box_class);
		inst0->transform = TBaluTransform(TVec2(-5 + i*0.9 + 0.3, -7 + sinf(i*0.3) * 1), b2Rot(i));
	}

	auto viewport = scene0->CreateViewport("main_viewport");
	viewport->SetTransform(TBaluTransform(TVec2(0, 0), b2Rot(0)));
	viewport->SetAspectRatio(1);
	viewport->SetWidth(5);

	return world;
}