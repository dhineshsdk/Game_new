#include "CodexAudio.h"

DEFINE_LOG_CATEGORY(LogCodexAudio);

void FCodexAudioModule::StartupModule()
{
	UE_LOG(LogCodexAudio, Log, TEXT("CodexAudio module initialized (stub)."));
}

void FCodexAudioModule::ShutdownModule()
{
	UE_LOG(LogCodexAudio, Log, TEXT("CodexAudio module shutdown."));
}

IMPLEMENT_MODULE(FCodexAudioModule, CodexAudio);
