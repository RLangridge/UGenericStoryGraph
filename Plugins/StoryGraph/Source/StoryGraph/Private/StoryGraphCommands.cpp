// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "StoryGraphCommands.h"

#define LOCTEXT_NAMESPACE "FStoryGraphModule"

void FStoryGraphCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "StoryGraph", "Bring up StoryGraph window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
