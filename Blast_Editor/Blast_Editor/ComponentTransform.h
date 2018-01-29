#pragma once
#include "Math/float3.h"
#include "Math/Quat.h"
#include "Math/float4x4.h"

class ComponentTransform
{
public:
	ComponentTransform();
	virtual ~ComponentTransform();

	void SetPosition(float3 position);
	float3 GetGlobalPosition() const;
	float3 GetLocalPosition() const;
	void SetRotation(float3 rotation);
	float3 GetGlobalRotation() const;
	float3 GetLocalRotation() const;
	void SetScale(float3 scale);
	float3 GetGlobalScale() const;
	float3 GetLocalScale() const;
	void UpdateGlobalMatrix();
	const float4x4 GetMatrix() const;
	const float* GetOpenGLMatrix() const;
	void SetParentTransform(ComponentTransform* transform);

private:
	float3 position;
	Quat rotation;
	float3 shown_rotation;
	float3 scale;

	float3 global_pos;
	float3 global_rot;
	float3 global_scale;

	float4x4 transform_matrix;
	ComponentTransform* parent_transform;
};

