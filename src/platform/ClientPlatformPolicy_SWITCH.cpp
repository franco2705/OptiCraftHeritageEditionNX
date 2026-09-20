#include "platform/ClientPlatformPolicy.h"
#include "net/minecraft/src/GameSettings.h"
#include "platform/Log.h"
namespace ClientPlatformPolicy { int initialWidth(){return 1280;} int initialHeight(){return 720;} std::string minecraftDirectory(){return "sdmc:/switch/OptiCraft/.minecraft";} bool saveConverterUsesSavesSubdirectory(){return true;} void applyGameSettingsDefaults(GameSettings*s){if(s){s->renderDistance=2;s->advancedOpengl=false;s->ofChunkUpdates=1;}} void preloadStartupTextures(RenderEngine*){} void releaseWorldEntryAssets(RenderEngine*){} int panoramaSampleGrid(){return 4;} void reportCrash(const std::string&d){MC_LOG_ERROR("switch","%s\n",d.c_str());}}
