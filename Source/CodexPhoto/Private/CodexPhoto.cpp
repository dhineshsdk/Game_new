#include "CodexPhoto.h"

DEFINE_LOG_CATEGORY(LogCodexPhoto);

void FCodexPhotoModule::StartupModule()
{
	UE_LOG(LogCodexPhoto, Log, TEXT("CodexPhoto module initialized (stub)."));
}

void FCodexPhotoModule::ShutdownModule()
{
	UE_LOG(LogCodexPhoto, Log, TEXT("CodexPhoto module shutdown."));
}

IMPLEMENT_MODULE(FCodexPhotoModule, CodexPhoto);
