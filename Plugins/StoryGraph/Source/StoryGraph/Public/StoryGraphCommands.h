// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "StoryGraphStyle.h"

class FStoryGraphCommands : public TCommands<FStoryGraphCommands>
{
public:

	FStoryGraphCommands()
		: TCommands<FStoryGraphCommands>(TEXT("StoryGraph"), NSLOCTEXT("Contexts", "StoryGraph", "StoryGraph Plugin"), NAME_None, FStoryGraphStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};