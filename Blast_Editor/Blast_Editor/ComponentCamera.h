#pragma once
#include "Math/Frustum.h"
#include "Math/AABB.h"
#include "Math/Rect.h"
#include "Color.h"

class RenderTextureMSAA;

//MathGeoLib frustum info: http://clb.demon.fi/MathGeoLib/nightly/docs/Frustum_summary.php

class ComponentCamera
{
public:
	ComponentCamera();
	~ComponentCamera();

	math::Frustum GetFrustum() const;
	void UpdatePosition();
	float* GetProjectionMatrix() const;
	float* GetViewMatrix();

	void SetFOV(float fov);
	float GetFOV() const;
	void SetBackgroundColor(Color color);
	Color GetBackgroundColor() const;
	void SetNearPlaneDistance(float distance);
	float GetNearPlaneDistance() const;
	void SetFarPlaneDistance(float distance);
	float GetFarPlanceDistance() const;
	void SetAspectRatio(float ratio);
	float GetAspectRatio() const;

public:
	std::vector<std::string> layers_to_draw;
	Frustum camera_frustum;

private:
	Color background_color;
	float aspect_ratio;
};

