#include "ModuleEditor.h"
#include "Application.h"
#include "imgui-1.53/imgui.h"
#include "imgui-1.53/imgui_impl_sdl_gl3.h"
#include "tinyfiledialogs.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "ModuleBlast.h"
#include "BlastMesh.h"

ModuleEditor::ModuleEditor(Application* app) : Module(app)
{
	name = "Editor";
	open_options = false;
}

ModuleEditor::~ModuleEditor()
{
}

bool ModuleEditor::Init()
{
	ImGui_ImplSdlGL3_Init(App->window->window);

	// (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
	/*ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromFileTTF(EDITOR_FONTS_FOLDER"Cousine-Regular.ttf", 15.0f);
	io.Fonts->AddFontFromFileTTF(EDITOR_FONTS_FOLDER"DroidSans.ttf", 16.0f);
	io.Fonts->AddFontFromFileTTF(EDITOR_FONTS_FOLDER"ProggyClean.ttf", 13.0f);
	io.Fonts->AddFontFromFileTTF(EDITOR_FONTS_FOLDER"ProggyTiny.ttf", 10.0f);
	font = io.Fonts->AddFontFromFileTTF(EDITOR_FONTS_FOLDER"OpenSans-Semibold.ttf", 16.0f);*/

	return true;
}

update_status ModuleEditor::PreUpdate(float delta_time)
{
	ImGui_ImplSdlGL3_NewFrame(App->window->window);
	return UPDATE_CONTINUE;
}

update_status ModuleEditor::Update(float deltaTime)
{
	ms_timer.Start();
	bool ret = true;
	if (ImGui::BeginMainMenuBar())
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4 previous_color = style.Colors[ImGuiCol_Text];
		style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		if (ImGui::BeginMenu("File"))
		{
			style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

			if (ImGui::MenuItem("Save File")) {
				App->blast->CreateBlastFile();
			}
			if (ImGui::MenuItem("Exit")) {
				return UPDATE_STOP;
			}

			ImGui::EndMenu();
			style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		}


		if (ImGui::BeginMenu("Help"))
		{
			style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		
			if (ImGui::MenuItem("Documentation"))
			{
				OpenBrowserPage("https://github.com/sergipa/The-Creator-3D/wiki");
			}
			if (ImGui::MenuItem("Download Latest"))
			{
				OpenBrowserPage("https://github.com/sergipa/The-Creator-3D/releases");
			}
			if (ImGui::MenuItem("Report a bug"))
			{
				OpenBrowserPage("https://github.com/sergipa/The-Creator-3D/issues");
			}
			ImGui::EndMenu();
			style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		}
		ImGui::EndMainMenuBar();
		style.Colors[ImGuiCol_Text] = previous_color;
	}

	ImGuiIO& io = ImGui::GetIO();
	bool fracture_active = true;
	ImGui::SetNextWindowPos({ io.DisplaySize.x / 2 - 200, 20 });
	ImGui::Begin("Blast Fracture", &fracture_active,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	float explosion_amount = App->blast->explosion_amount;
	if (ImGui::SliderFloat("Explosion Amount", &explosion_amount, 0, 2))
	{
		App->renderer3D->SetExplosionDisplacement(explosion_amount);
		App->blast->explosion_amount = explosion_amount;
	}
	ImGui::End();

	bool hierarchy_active = true;
	//ImGui::SetNextWindowSize({ 200, io.DisplaySize.y });
	ImGui::SetNextWindowPos({ 0,20 });
	ImGui::Begin("Blast Hierarchy", &hierarchy_active,
		ImGuiWindowFlags_AlwaysAutoResize);

	std::vector<BlastMesh*> chunks = App->blast->blast_meshes;
	if (!chunks.empty())
	{
		if (ImGui::Button("Fracture"))
		{
			if (App->blast->current_selected_mesh)
			{
				App->blast->ApplyFracture();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			if (App->blast->current_selected_mesh)
			{
				App->blast->ResetMesh();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Optimize"))
		{
			if (App->blast->current_selected_mesh)
			{
				App->blast->ResetMesh();
			}
		}
		ImGui::Separator();
		for (std::vector<BlastMesh*>::iterator it = chunks.begin(); it != chunks.end(); it++)
		{
			if (ImGui::TreeNodeEx((*it)->name.c_str()))
			{
				DrawChunkTree((*it));
				ImGui::TreePop();
			}
			
		}
	}
	else
	{
		ImGui::Text("No Mesh loaded.");
		ImGui::Text("Drag & Drop an .fbx to load a mesh.");
	}

	if (ImGui::BeginPopupContextWindow("Options"))
	{
		if (ImGui::MenuItem("Remove Childs")) 
		{
			App->blast->RemoveChildChunks(App->blast->current_selected_mesh);
		}
		ImGui::EndPopup();
	}
	
	ImGui::End();

	bool options_active = true;
	ImGui::SetNextWindowPos({ io.DisplaySize.x - 225,20 });
	ImGui::Begin("Fracture Options", &options_active,
		ImGuiWindowFlags_AlwaysAutoResize);

	const char* fracture_names[6] = { "Voronoi Uniform", "Voronoi Clustered", "Voronoi Radial", "Voronoi In Sphere", "Voronoi Remove In Sphere", "Uniform Slicing" };
	static const char* current_item = fracture_names[0];
	ImGui::Text("Fracture Type:");
	if (ImGui::BeginCombo("##fracture_type", current_item))
	{
		for (int i = 0; i < 6; i++)
		{
			if (ImGui::Selectable(fracture_names[i]))
			{
				App->blast->SetFractureType((ModuleBlast::FractureType)i);
				current_item = fracture_names[i];
			}
		}
		ImGui::EndCombo();
	}
	
	int cell_counts = App->blast->GetCellCounts();
	ImGui::Text("Cell Count:");
	if (ImGui::DragInt("##Cell Count", &cell_counts, 1, 2))
	{
		App->blast->SetCellCounts(cell_counts);
	}

	ModuleBlast::FractureType type = App->blast->GetFractureType();
	switch (type)
	{
	case ModuleBlast::VoronoiClustered:
	{
		int cluster_count = App->blast->GetClusterCount();
		ImGui::Text("Cluster Count:");
		if (ImGui::DragInt("##Cluster Count", &cluster_count, 1, 2))
		{
			App->blast->SetClusterCount(cluster_count);
		}
		float cluster_radius = App->blast->GetClusterRadius();
		ImGui::Text("Cluster Radius:");
		if (ImGui::DragFloat("##Cluster Radius", &cluster_radius, 0.25f, 0))
		{
			App->blast->SetClusterRadius(cluster_radius);
		}
	}
		break;
	case ModuleBlast::VoronoiRadial:
	{
		float3 radial_center = App->blast->GetRadialCenter();
		ImGui::Text("Center:");
		if (ImGui::DragFloat3("##Radial_Center", (float*)&radial_center, 0.25f))
		{
			App->blast->SetRadialCenter(radial_center);
		}
		float3 radial_normal = App->blast->GetRadialNormal();
		ImGui::Text("Normal:");
		if (ImGui::DragFloat3("##Radial_Normal", (float*)&radial_normal, 0.25f))
		{
			App->blast->SetRadialNormal(radial_normal);
		}
		float radial_radius = App->blast->GetRadialRadius();
		ImGui::Text("Radius:");
		if (ImGui::DragFloat("##Radial_Radius", &radial_radius, 0.25f))
		{
			App->blast->SetRadialRadius(radial_radius);
		}
		int radial_angular_steps = App->blast->GetRadialAngularSteps();
		ImGui::Text("Angular Steps:");
		if (ImGui::DragInt("##Radial_Angular Steps", &radial_angular_steps))
		{
			App->blast->SetRadialAngularSteps(radial_angular_steps);
		}
		int radial_steps = App->blast->GetRadialSteps();
		ImGui::Text("Steps:");
		if (ImGui::DragInt("##Radial_Steps", &radial_steps))
		{
			App->blast->SetRadialSteps(radial_steps);
		}
		float radial_angle_offset = App->blast->GetRadialAngleOffset();
		ImGui::Text("Angle Offset:");
		if (ImGui::DragFloat("##Radial_Angle Offset", &radial_angle_offset, 0.25f))
		{
			App->blast->SetRadialAngleOffset(radial_angle_offset);
		}
		float radial_variability = App->blast->GetRadialVariability();
		ImGui::Text("Variability:");
		if (ImGui::DragFloat("##Radial_Variability", &radial_variability, 0.25f))
		{
			App->blast->SetRadialVariability(radial_variability);
		}
	}
		break;
	case ModuleBlast::VoronoiInSphere:
	{
		float3 sphere_center = App->blast->GetSphereCenter();
		ImGui::Text("Center:");
		if (ImGui::DragFloat3("##Sphere_Center", (float*)&sphere_center, 0.25f))
		{
			App->blast->SetSphereCenter(sphere_center);
		}
		float sphere_radius = App->blast->GetSphereRadius();
		ImGui::Text("Radius:");
		if (ImGui::DragFloat("##Sphere_Radius", &sphere_radius, 0.25f))
		{
			App->blast->SetSphereRadius(sphere_radius);
		}
	}
		break;
	case ModuleBlast::VoronoiRemove:
	{
		float3 sphere_center = App->blast->GetRemoveSphereCenter();
		ImGui::Text("Center:");
		if (ImGui::DragFloat3("Remove_Center", (float*)&sphere_center, 0.25f))
		{
			App->blast->SetRemoveSphereCenter(sphere_center);
		}
		float sphere_radius = App->blast->GetRemoveSphereRadius();
		ImGui::Text("Radius:");
		if (ImGui::DragFloat("##Remove_Radius", &sphere_radius, 0.25f))
		{
			App->blast->SetRemoveSphereRadius(sphere_radius);
		}
		float sphere_probability = App->blast->GetRemoveSphereEraserProbability();
		ImGui::Text("Probability:");
		if (ImGui::DragFloat("##Remove_Probability", &sphere_probability, 0.25f))
		{
			App->blast->SetRemoveSphereEraserProbability(sphere_probability);
		}
	}
		break;
	case ModuleBlast::Slice:
	{
		int slicing_X = App->blast->GetSlicingX();
		ImGui::Text("Slices X:");
		if (ImGui::DragInt("##Slices X", &slicing_X))
		{
			App->blast->SetSlicingX(slicing_X);
		}
		int slicing_Y = App->blast->GetSlicingY();
		ImGui::Text("Slices Y:");
		if (ImGui::DragInt("##Slices Y", &slicing_Y))
		{
			App->blast->SetSlicingX(slicing_Y);
		}
		int slicing_Z = App->blast->GetSlicingZ();
		ImGui::Text("Slices Z:");
		if (ImGui::DragInt("##Slices Z", &slicing_Z))
		{
			App->blast->SetSlicingZ(slicing_Z);
		}
		float slicing_angle_variation = App->blast->GetSlicingAngleVariation();
		ImGui::Text("Angle Variation:");
		if (ImGui::DragFloat("##Angle Variation", &slicing_angle_variation, 0.25f))
		{
			App->blast->SetSlicingAngleVariation(slicing_angle_variation);
		}
		float slicing_offset_variation = App->blast->GetSlicingOffsetVariations();
		ImGui::Text("Offset Variation:");
		if (ImGui::DragFloat("##Offset Variation", &slicing_offset_variation, 0.25f))
		{
			App->blast->SetSlicingOffsetVariations(slicing_offset_variation);
		}
		float slicing_noise_amplitude = App->blast->GetSlicingNoiseAmplitude();
		ImGui::Text("Noise Amplitude:");
		if (ImGui::DragFloat("##Noise Amplitude", &slicing_noise_amplitude, 0.25f))
		{
			App->blast->SetSlicingNoiseAmplitude(slicing_noise_amplitude);
		}
		float slicing_noise_frequency = App->blast->GetSlicingNoiseFrequency();
		ImGui::Text("Noise Frequency:");
		if (ImGui::DragFloat("##Noise Frequency", &slicing_noise_frequency, 0.25f))
		{
			App->blast->SetSlicingNoiseFrequency(slicing_noise_frequency);
		}
		int slicing_noise_octave_number = App->blast->GetSlicingNoiseOctaveNumber();
		ImGui::Text("Noise Octave Number:");
		if (ImGui::DragInt("##Noise Octave Number", &slicing_noise_octave_number))
		{
			App->blast->SetSlicingNoiseOctaveNumber(slicing_noise_octave_number);
		}
		int slicing_surface_resolution = App->blast->GetSlicingSurfaceResolution();
		ImGui::Text("Surface Resolution:");
		if (ImGui::DragInt("##Surface Resolution", &slicing_surface_resolution))
		{
			App->blast->SetSlicingSurfaceResolution(slicing_surface_resolution);
		}
	}
		break;
	}

	ImGui::End();

	return UPDATE_CONTINUE;
}

bool ModuleEditor::DrawEditor()
{
	ImGui::Render();
	return true;
}

void ModuleEditor::DrawChunkTree(BlastMesh * mesh)
{
	uint flag = 0;
	std::string name = "Chunk " + std::to_string(mesh->chunk_id) + " : Depth " + std::to_string(mesh->chunk_depth);
	if (!mesh->is_root) flag |= ImGuiTreeNodeFlags_Leaf;
	flag |= ImGuiTreeNodeFlags_OpenOnArrow;
	if (mesh == App->blast->current_selected_mesh) flag |= ImGuiTreeNodeFlags_Selected;

	if (ImGui::TreeNodeEx(name.c_str(), flag))
	{
		if (ImGui::IsItemHoveredRect())
		{
			if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1))
			{
				App->blast->SetCurrentMesh(mesh->blast_mesh);
				App->blast->current_selected_mesh = mesh;
				ImGui::OpenPopupOnItemClick("Options");
			}
		}
		
		for (std::vector<BlastMesh*>::iterator it = mesh->childs.begin(); it != mesh->childs.end(); it++)
		{
			DrawChunkTree(*it);
		}
		ImGui::TreePop();
	}
}

bool ModuleEditor::CleanUp()
{
	/*for (std::list<Window*>::iterator it = editor_windows.begin(); it != editor_windows.end(); ++it) {
		RELEASE(*it);
	}
	RELEASE(drag_data);
	editor_windows.clear();
	ImGui::SaveDocks();*/
	ImGui_ImplSdlGL3_Shutdown();
	return true;
}

void ModuleEditor::HandleInput(SDL_Event * event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}

void ModuleEditor::OpenBrowserPage(const char * url)
{
	//ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

