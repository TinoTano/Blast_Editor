#include "ComponentCamera.h"
#include "Math/Plane.h"
#include "Application.h"
#include <algorithm>
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Math/MathFunc.h"
#include "Math/float4x4.h"

ComponentCamera::ComponentCamera()
{
	camera_frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
	camera_frustum.SetPos(float3(0, 0, 0));
	camera_frustum.SetFront(float3::unitZ);
	camera_frustum.SetUp(float3::unitY);
	camera_frustum.SetHorizontalFovAndAspectRatio(60.f*DEGTORAD, 1.3f);
	camera_frustum.SetViewPlaneDistances(0.1f, 1000.0f);

	background_color = Black;
}

ComponentCamera::~ComponentCamera()
{
	/*if (GetGameObject() != nullptr)
	{
		if (App->renderer3D)
		{
			if (std::find(App->renderer3D->rendering_cameras.begin(), App->renderer3D->rendering_cameras.end(), this) != App->renderer3D->rendering_cameras.end())
			{
				App->renderer3D->rendering_cameras.remove(this);
				if (GetGameObject()->GetTag() == "Main Camera")
				{
					App->renderer3D->game_camera = nullptr;
				}
			}
		}
	}*/
}

math::Frustum ComponentCamera::GetFrustum() const
{
	return camera_frustum;
}

void ComponentCamera::UpdatePosition()
{
	/*camera_frustum.pos = GetGameObject()->GetGlobalTransfomMatrix().TranslatePart();
	camera_frustum.front = GetGameObject()->GetGlobalTransfomMatrix().WorldZ().Normalized();
	camera_frustum.up = GetGameObject()->GetGlobalTransfomMatrix().WorldY().Normalized();*/
}

float * ComponentCamera::GetProjectionMatrix() const
{
	static float4x4 matrix;

	matrix = camera_frustum.ProjectionMatrix();
	matrix.Transpose();

	return (float*)matrix.v;
}

float * ComponentCamera::GetViewMatrix()
{
	//ViewMatrix is 3x4 and Transposed3 is not working
	static float4x4 matrix;
	matrix = camera_frustum.ViewMatrix();
	matrix.Transpose();

	return (float*)matrix.v;
}

void ComponentCamera::SetFOV(float fov)
{
	/*camera_frustum.SetHorizontalFovAndAspectRatio(angle*DEGTORAD, aspect_ratio);
	fov = angle;
	camera_frustum.ComputeProjectionMatrix();*/
}

float ComponentCamera::GetFOV() const
{
	return 0;// camera_frustum.verticalFov * RADTODEG;
}

Color ComponentCamera::GetBackgroundColor() const
{
	return background_color;
}

void ComponentCamera::SetNearPlaneDistance(float distance)
{
	//camera_frustum.SetViewPlaneDistances(distance);
}

float ComponentCamera::GetNearPlaneDistance() const
{
	return camera_frustum.NearPlaneDistance();
}

void ComponentCamera::SetFarPlaneDistance(float distance)
{
	//camera_frustum.farPlaneDistance = distance;
}

float ComponentCamera::GetFarPlanceDistance() const
{
	return camera_frustum.FarPlaneDistance();
}

void ComponentCamera::SetAspectRatio(float ratio)
{
	/*float r = DEGTORAD * ratio;

	if (r > 0.0f)
	{
		camera_frustum.SetHorizontalFovAndAspectRatio(fov*DEGTORAD, aspect);
		aspect_ratio = aspect;
	}*/
}

float ComponentCamera::GetAspectRatio() const
{
	return camera_frustum.AspectRatio();
}

void ComponentCamera::SetBackgroundColor(Color color)
{
	background_color = color;
}
