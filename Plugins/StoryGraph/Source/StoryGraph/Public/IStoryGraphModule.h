#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"
#include "GraphEditor.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkit.h"
#include "IGameDialogueEditor.h"
#include "ModuleInterface.h"

class IStoryGraphModule : public IModuleInterface, public IHasMenuExtensibility, public IHasToolBarExtensibility
{
public:
	/** Creates a new Game Dialogue editor for some new dialogue */
	//virtual TSharedRef<IGameDialogueEditor> CreateGameDialogueEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, USoundCue* SoundCue) = 0;
};

