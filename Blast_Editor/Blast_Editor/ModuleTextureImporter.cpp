#include "ModuleTextureImporter.h"
#include "Texture.h"
#include "Application.h"

#include "Devil/include/il.h"
#include "Devil/include/ilut.h"
#pragma comment ( lib, "Devil/libx86/DevIL.lib" )
#pragma comment ( lib, "Devil/libx86/ILU.lib" )
#pragma comment ( lib, "Devil/libx86/ILUT.lib" )

ModuleTextureImporter::ModuleTextureImporter(Application* app) : Module(app)
{
	name = "Texture_Importer";

	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);
}

ModuleTextureImporter::~ModuleTextureImporter()
{
}

bool ModuleTextureImporter::CleanUp()
{
	ilShutDown();
	return true;
}

Texture * ModuleTextureImporter::ImportTexture(const char* path)
{
	ILuint image_id;
	ilGenImages(1, &image_id);
	ilBindImage(image_id);

	Texture* tmp_texture = nullptr;

	if (ilLoad(IL_TYPE_UNKNOWN, path))
	{
		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);

		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		{
			iluFlipImage();
		}

		if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
		{
			CONSOLE_LOG("DeviL: Failed to convert image %s. Error: %s", path, iluErrorString(ilGetError()));
		}

		int width = ilGetInteger(IL_IMAGE_WIDTH);
		int height = ilGetInteger(IL_IMAGE_HEIGHT);
		int format = ilGetInteger(IL_IMAGE_FORMAT);
		int type = ilGetInteger(IL_IMAGE_TYPE);
		byte* data = new byte[width * height * 4];
		ilCopyPixels(0, 0, 0, width, height, 1, IL_RGBA, IL_UNSIGNED_BYTE, data);

		tmp_texture = new Texture();
		tmp_texture->SetWidth(width);
		tmp_texture->SetHeight(height);
		tmp_texture->SetCompression(ilGetInteger(IL_DXTC_FORMAT));
		tmp_texture->SetImageData(data);
		tmp_texture->SetFormat(Texture::rgba);
		tmp_texture->LoadToMemory();

		ilDeleteImage(ilGetInteger(IL_ACTIVE_IMAGE));
		CONSOLE_DEBUG("Image loaded from library: %s", path);

		tmp_texture->RecreateTexture();
	}
	else
	{
		CONSOLE_DEBUG("Cannot load image %s. Error: %s", path, iluErrorString(ilGetError()));
	}

	return tmp_texture;
}

