#include "ComponentTransform.h"
#include "Globals.h"

ComponentTransform::ComponentTransform()
{
	position = float3(0.f, 0.f, 0.f);
	shown_rotation = float3(0.f, 0.f, 0.f);
	rotation = Quat(0.f, 0.f, 0.f, 0.f);
	scale = float3(1.f, 1.f, 1.f);
	global_pos = float3(0.f, 0.f, 0.f);
	global_rot = float3(0.f, 0.f, 0.f);
	global_scale = float3(1.f, 1.f, 1.f);
	transform_matrix.SetIdentity();
	parent_transform = nullptr;
}

ComponentTransform::~ComponentTransform()
{
}

void ComponentTransform::SetPosition(float3 position)
{
	this->position = position;
	UpdateGlobalMatrix();
}

float3 ComponentTransform::GetGlobalPosition() const
{
	return global_pos;
}

float3 ComponentTransform::GetLocalPosition() const
{
	/*if (GetGameObject()->GetParent() != nullptr)
	{
		ComponentTransform* parent_transform = (ComponentTransform*)GetGameObject()->GetParent()->GetComponent(ComponentType::CompTransform);
		return GetGlobalPosition() - parent_transform->GetGlobalPosition();
	}*/
	return position; //If it's the parent. local position = global position
}

void ComponentTransform::SetRotation(float3 rotation)
{
	this->shown_rotation = rotation;
	this->rotation = Quat::FromEulerXYZ(rotation.x * DEGTORAD, rotation.y * DEGTORAD, rotation.z * DEGTORAD);
	UpdateGlobalMatrix();
}

float3 ComponentTransform::GetGlobalRotation() const
{
	return global_rot;
}

float3 ComponentTransform::GetLocalRotation() const
{
	/*if (GetGameObject()->GetParent() != nullptr)
	{
		ComponentTransform* parent_transform = (ComponentTransform*)GetGameObject()->GetParent()->GetComponent(ComponentType::CompTransform);
		return GetGlobalRotation() - parent_transform->GetGlobalRotation();
	}*/
	return shown_rotation; //If it's the parent. local rotation = global rotation
}

void ComponentTransform::SetScale(float3 scale)
{
	this->scale = scale;
	UpdateGlobalMatrix();
}

float3 ComponentTransform::GetGlobalScale() const
{
	return global_scale;
}

float3 ComponentTransform::GetLocalScale() const
{
	/*if (GetGameObject()->GetParent() != nullptr)
	{
		ComponentTransform* parent_transform = (ComponentTransform*)GetGameObject()->GetParent()->GetComponent(ComponentType::CompTransform);
		return GetGlobalScale() - parent_transform->GetGlobalScale();
	}*/
	return scale; //If it's the parent. local scale = global scale
}

void ComponentTransform::UpdateGlobalMatrix()
{

	//if (!this->GetGameObject()->IsRoot())
	//{
	//	ComponentTransform* parent_transform = (ComponentTransform*)this->GetGameObject()->GetParent()->GetComponent(Component::CompTransform);

	//	transform_matrix = transform_matrix.FromTRS(position, rotation, scale);
	//	transform_matrix = parent_transform->transform_matrix * transform_matrix;

	//	float3 _pos, _scale;
	//	Quat _rot;
	//	transform_matrix.Decompose(_pos, _rot, _scale);
	//	global_pos = _pos;
	//	global_rot = _rot.ToEulerXYZ() * RADTODEG;
	//	global_scale = _scale;
	//}
	//else
	//{
	//	transform_matrix = float4x4::FromTRS(position, rotation, scale);
	//	for (std::list<GameObject*>::iterator it = this->GetGameObject()->childs.begin(); it != this->GetGameObject()->childs.end(); it++)
	//	{
	//		ComponentTransform* child_transform = (ComponentTransform*)(*it)->GetComponent(Component::CompTransform);
	//		child_transform->UpdateGlobalMatrix();
	//	}

	//	global_pos = position;
	//	global_rot = shown_rotation;
	//	global_scale = scale;
	//}

	//GetGameObject()->UpdateBoundingBox();
	////If gameobject has a camera component
	//GetGameObject()->UpdateCamera();

	if (!parent_transform)
	{
		transform_matrix = float4x4::FromTRS(position, rotation, scale);

		global_pos = position;
		global_rot = shown_rotation;
		global_scale = scale;
	}
	else
	{
		transform_matrix = transform_matrix.FromTRS(position, rotation, scale);
		transform_matrix = parent_transform->transform_matrix * transform_matrix;

		float3 _pos, _scale;
		Quat _rot;
		transform_matrix.Decompose(_pos, _rot, _scale);
		global_pos = _pos;
		global_rot = _rot.ToEulerXYZ() * RADTODEG;
		global_scale = _scale;
	}
}

const float4x4 ComponentTransform::GetMatrix() const
{
	return transform_matrix;
}

const float * ComponentTransform::GetOpenGLMatrix() const
{
	return transform_matrix.Transposed().ptr();
}

void ComponentTransform::SetParentTransform(ComponentTransform* transform)
{
	parent_transform = transform;
}
