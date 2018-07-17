// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "StoryGraph.h"
#include "StoryGraphStyle.h"
#include "StoryGraphCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

static const FName StoryGraphTabName("StoryGraph");

#define LOCTEXT_NAMESPACE "FStoryGraphModule"

void FStoryGraphModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolbarExtensibilityManager = MakeShareable(new FExtensibilityManager);
}

void FStoryGraphModule::ShutdownModule()
{
	MenuExtensibilityManager.Reset();
	ToolbarExtensibilityManager.Reset();
}




TSharedRef<SGraphEditor> FStoryGraphModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FStoryGraphModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("StoryGraph.cpp"))
		);

	return SNew(SGraphEditor);
		
}

void FStoryGraphModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(StoryGraphTabName);
}

void FStoryGraphModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FStoryGraphCommands::Get().OpenPluginWindow);
}

void FStoryGraphModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FStoryGraphCommands::Get().OpenPluginWindow);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStoryGraphModule, StoryGraph)