#include "FGameDialogueEditor.h"
#include "ScopedTransaction.h"
#include "Toolkits/IToolkit.h"
#include "StoryGraph.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphUtilities.h"
#include "SNodePanel.h"
#include "StoryGraphCommands.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Editor/PropertyEditor/Public/IDetailsView.h"
#include "SDockTab.h"
#include "GenericCommands.h"
#include "Editor.h"
#include "MultiBoxBuilder.h"
#include "HAL/PlatformApplicationMisc.h"
//#include "GameDialogue.h"

#define LOCTEXT_NAMESPACE "GameDialogueEditor"

const FName FGameDialogueEditor::GraphCanvasTabId(TEXT("GameDialogueEditor_GraphCanvas"));
const FName FGameDialogueEditor::PropertiesTabId(TEXT("GameDialogueEditor_Properties"));
const FName FGameDialogueEditor::PaletteTabId(TEXT("GameDialogueEditor_Palette"));

void
FGameDialogueEditor::RegisterTabSpawners
(const TSharedRef<class FTabManager> &TabManager)
{
	this->WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_GameDialogueEditor", "Game Dialogue Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(TabManager);

	TabManager->RegisterTabSpawner(GraphCanvasTabId, FOnSpawnTab::CreateSP(this, &FGameDialogueEditor::SpawnTab_GraphCanvas))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.EventGraph_16x"));

	TabManager->RegisterTabSpawner(PropertiesTabId, FOnSpawnTab::CreateSP(this, &FGameDialogueEditor::SpawnTab_Properties))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	TabManager->RegisterTabSpawner(PaletteTabId, FOnSpawnTab::CreateSP(this, &FGameDialogueEditor::SpawnTab_Palette))
		.SetDisplayName(LOCTEXT("PaletteTab", "Palette"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.Palette"));
}

void
FGameDialogueEditor::UnregisterTabSpawners
(const TSharedRef<class FTabManager> &TabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(TabManager);
	TabManager->UnregisterTabSpawner(GraphCanvasTabId);
	TabManager->UnregisterTabSpawner(PropertiesTabId);
	TabManager->UnregisterTabSpawner(PaletteTabId);
}

FGameDialogueEditor::~FGameDialogueEditor
()
{
	GEditor->UnregisterForUndo(this);
}

void
FGameDialogueEditor::InitGameDialogueEditor
(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost> &InitToolkitHost, UObject *ObjectToEdit)
{
	//Cast game dialogue object from ObjectToEdit here
	//SoundCue = CastChecked<USoundCue>(ObjectToEdit);

	// Support undo/redo
	//SoundCue->SetFlags(RF_Transactional);

	GEditor->RegisterForUndo(this);

	//Register our commands for use in the UI
	FGraphEditorCommands::Register();
	FStoryGraphCommands::Register();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_GameDialogueEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.225f)
					->AddTab(PropertiesTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.65f)
					->AddTab(GraphCanvasTabId, ETabState::OpenedTab)->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.125f)
					->AddTab(PaletteTabId, ETabState::OpenedTab)
				)
			)
			
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, GameDialogueEditorAppIdentifier, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectToEdit, false);

	IStoryGraphModule *myModule = &FModuleManager::LoadModuleChecked<IStoryGraphModule>("GameDialogueEditor");
	AddMenuExtender(myModule->GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

	ExtendToolbar();
	RegenerateMenusAndToolbars();

	// @todo toolkit world centric editing
	/*if(IsWorldCentricAssetEditor())
	{
	SpawnToolkitTab(GetToolbarTabId(), FString(), EToolkitTabSpot::ToolBar);
	SpawnToolkitTab(GraphCanvasTabId, FString(), EToolkitTabSpot::Viewport);
	SpawnToolkitTab(PropertiesTabId, FString(), EToolkitTabSpot::Details);
	}*/
}

/*
//Our reference to the object we're editing
USoundCue* FSoundCueEditor::GetSoundCue() const
{
return SoundCue;
}
*/

void
FGameDialogueEditor::SetSelection
(TArray<UObject *> SelectedObjects)
{
	if (GameDialogueProperties.IsValid())
	{
		GameDialogueProperties->SetObjects(SelectedObjects);
	}
}

bool
FGameDialogueEditor::GetBoundsForSelectedNodes
(class FSlateRect &Rect, float Padding)
{
	return GameDialogueGraphEditor->GetBoundsForSelectedNodes(Rect, Padding);
}

int32 
FGameDialogueEditor::GetNumberOfSelectedNodes
() const
{
	return GameDialogueGraphEditor->GetSelectedNodes().Num();
}

FName
FGameDialogueEditor::GetToolkitFName
() const
{
	return FName("GameDialogueEditor");
}

FText
FGameDialogueEditor::GetBaseToolkitName
() const
{
	return LOCTEXT("AppLabel", "GameDialogue Editor");
}

FString
FGameDialogueEditor::GetWorldCentricTabPrefix
() const
{
	return LOCTEXT("WorldCentricTabPrefix", "GameDialogue").ToString();
}

FLinearColor
FGameDialogueEditor::GetWorldCentricTabColorScale
() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

TSharedRef<SDockTab> 
FGameDialogueEditor::SpawnTab_GraphCanvas
(const FSpawnTabArgs &Args)
{
	//check(Args.GetTabId() == GraphCanvasTabId);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab).Label(LOCTEXT("GameDialogueGraphCanvasTitle", "Viewport"));
	if (GameDialogueGraphEditor.IsValid())
	{
		SpawnedTab->SetContent(GameDialogueGraphEditor.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> 
FGameDialogueEditor::SpawnTab_Properties
(const FSpawnTabArgs &Args)
{
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("GameDialogueDetailsTitle", "Details"))
		[
			GameDialogueProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab>
FGameDialogueEditor::SpawnTab_Palette
(const FSpawnTabArgs &Args)
{
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
		.Label(LOCTEXT("GameDialogueDetailsTitle", "Details"))
		[
			GameDialogueProperties.ToSharedRef()
		];
		//.Label(LOCTEXT("GamedialoguePaletteTitle"));
		//[
			//Palette.toSharedRef()
		//];
}

void
FGameDialogueEditor::AddReferencedObjects
(FReferenceCollector &Collector)
{
	//Collector.AddReferencedObjects(SoundCue);
}

void
FGameDialogueEditor::PostUndo
(bool bSuccess)
{
	if (GameDialogueGraphEditor.IsValid())
	{
		GameDialogueGraphEditor->ClearSelectionSet();
		GameDialogueGraphEditor->NotifyGraphChanged();
	}
}

void
FGameDialogueEditor::NotifyPostChange
(const FPropertyChangedEvent &PropertyChangedEvent, class UProperty *PropertyThatChanged)
{
	if (GameDialogueGraphEditor.IsValid() && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		GameDialogueGraphEditor->NotifyGraphChanged();
	}
}

void
FGameDialogueEditor::CreateInternalWidgets
()
{
	GameDialogueGraphEditor = CreateGraphEditorWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule &PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	GameDialogueProperties = PropertyModule.CreateDetailView(Args);
	//todo: set to game dialogue object
	//GameDialogueProperties->SetObject(SoundCue);
	//Palette = SNew(SSoundCuePalette);
}

void
FGameDialogueEditor::ExtendToolbar
()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder &ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("Toolbar");
			{
				/*ToolbarBuilder.AddToolBarButton(FSoundCueGraphEditorCommands::Get().PlayCue);

				ToolbarBuilder.AddToolBarButton(FSoundCueGraphEditorCommands::Get().PlayNode);

				ToolbarBuilder.AddToolBarButton(FSoundCueGraphEditorCommands::Get().StopCueNode);*/
			}
			ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension
	("Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);

	IStoryGraphModule *StoryGraphModule = &FModuleManager::LoadModuleChecked<IStoryGraphModule>("StoryGraphEditor");
	AddToolbarExtender(StoryGraphModule->GetToolBarExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));
}

void
FGameDialogueEditor::BindGraphCommands
()
{
	const FStoryGraphCommands Commands = FStoryGraphCommands::Get();

	ToolkitCommands->MapAction(
		FGenericCommands::Get().Undo,
		FExecuteAction::CreateSP(this, &FGameDialogueEditor::UndoGraphAction));

	ToolkitCommands->MapAction(
		FGenericCommands::Get().Redo,
		FExecuteAction::CreateSP(this, &FGameDialogueEditor::RedoGraphAction));
}

void
FGameDialogueEditor::SyncInBrowser
()
{
	TArray<UObject *> ObjectToSync;
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		//Iterate through nodes
		/*
		USoundCueGraphNode* SelectedNode = Cast<USoundCueGraphNode>(*NodeIt);

		if (SelectedNode)
		{
			USoundNodeWavePlayer* SelectedWave = Cast<USoundNodeWavePlayer>(SelectedNode->SoundNode);
			if (SelectedWave && SelectedWave->GetSoundWave())
			{
				ObjectsToSync.AddUnique(SelectedWave->GetSoundWave());
			}
			//ObjectsToSync.AddUnique(SelectedNode->SoundNode);
		}
		*/
	}

	if (ObjectToSync.Num() > 0)
	{
		GEditor->SyncBrowserToObjects(ObjectToSync);
	}
}

bool
FGameDialogueEditor::CanSyncInBrowser
() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	
	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		/*
		if (SelectedNode)
		{
			USoundNodeWavePlayer* WavePlayer = Cast<USoundNodeWavePlayer>(SelectedNode->SoundNode);

			if (WavePlayer && WavePlayer->GetSoundWave())
			{
				return true;
			}
		}
		*/
	}
	return false;
}

void
FGameDialogueEditor::AddInput
()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	// Iterator used but should only contain one node
	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		/*USoundCueGraphNode* SelectedNode = Cast<USoundCueGraphNode>(*NodeIt);

		if (SelectedNode)
		{
			SelectedNode->AddInputPin();
			break;
		}*/
	}
}

bool
FGameDialogueEditor::CanAddInput
() const
{
	return GetSelectedNodes().Num() == 1;
}

void
FGameDialogueEditor::DeleteInput
()
{
	UEdGraphPin *SelectedPin = GameDialogueGraphEditor->GetGraphPinForMenu();
	/*
	USoundCueGraphNode* SelectedNode = Cast<USoundCueGraphNode>(SelectedPin->GetOwningNode());

	if (SelectedNode && SelectedNode == SelectedPin->GetOwningNode())
	{
		SelectedNode->RemoveInputPin(SelectedPin);
	}
	*/
}

bool
FGameDialogueEditor::CanDeleteInput
() const
{
	return true;
}

void
FGameDialogueEditor::OnCreateComment
()
{
	/*
	FSoundCueGraphSchemaAction_NewComment CommentAction;
	CommentAction.PerformAction(SoundCue->SoundCueGraph, NULL, SoundCueGraphEditor->GetPasteLocation());
	*/
}

TSharedRef<SGraphEditor> 
FGameDialogueEditor::CreateGraphEditorWidget
()
{
	if (!GraphEditorCommands.IsValid())
	{
		GraphEditorCommands = MakeShareable(new FUICommandList);

		//Graph editor commands
		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().CreateComment,
			FExecuteAction::CreateSP(this, &FGameDialogueEditor::OnCreateComment));

		//Editing commands
		GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateSP(this, &FGameDialogueEditor::SelectAllNodes),
			FCanExecuteAction::CreateSP(this, &FGameDialogueEditor::CanSelectAllNodes));

		GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
			FExecuteAction::CreateSP(this, &FGameDialogueEditor::DeleteSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FGameDialogueEditor::CanDeleteNodes));

		GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP(this, &FGameDialogueEditor::CopySelectedNodes),
			FCanExecuteAction::CreateSP(this, &FGameDialogueEditor::CanCopyNodes));

		GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
			FExecuteAction::CreateSP(this, &FGameDialogueEditor::CutSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FGameDialogueEditor::CanCutNodes));

		GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP(this, &FGameDialogueEditor::PasteNodes),
			FCanExecuteAction::CreateSP(this, &FGameDialogueEditor::CanPasteNodes));

		GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateSP(this, &FGameDialogueEditor::DuplicateNodes),
			FCanExecuteAction::CreateSP(this, &FGameDialogueEditor::CanDuplicateNodes));
	}

	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_GameDialogue", "GAME DIALOGUE");

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FGameDialogueEditor::OnSelectedNodesChanged);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FGameDialogueEditor::OnNodeTitleCommitted);
	
	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		//.GraphToEdit(Soundcue->GetGraph())
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}

FGraphPanelSelectionSet
FGameDialogueEditor::GetSelectedNodes
() const
{
	FGraphPanelSelectionSet CurrentSelection;
	if (GameDialogueGraphEditor.IsValid())
	{
		CurrentSelection = GameDialogueGraphEditor->GetSelectedNodes();
	}
	return CurrentSelection;
}

void
FGameDialogueEditor::OnSelectedNodesChanged
(const TSet<class UObject *> &NewSelection)
{
	TArray<UObject *> Selection;

	if (NewSelection.Num())
	{
		for (TSet<class UObject *>::TConstIterator SetIt(NewSelection); SetIt; ++SetIt)
		{
			/*
			if (Cast<USoundCueGraphNode_Root>(*SetIt))
			{
				Selection.Add(GetSoundCue());
			}
			else if (USoundCueGraphNode* GraphNode = Cast<USoundCueGraphNode>(*SetIt))
			{
				Selection.Add(GraphNode->SoundNode);
			}
			else
			{
				Selection.Add(*SetIt);
			}
			*/
		}
	}
	else
	{
		//Selection.Add(GetSoundCue());
	}

	SetSelection(Selection);
}

void
FGameDialogueEditor::OnNodeTitleCommitted
(const FText &NewText, ETextCommit::Type CommitInfo, UEdGraphNode *NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameNode", "Rename Node"));
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

void 
FGameDialogueEditor::SelectAllNodes
()
{
	GameDialogueGraphEditor->SelectAllNodes();
}

bool
FGameDialogueEditor::CanSelectAllNodes
() const
{
	return true;
}

void
FGameDialogueEditor::DeleteSelectedNodes
()
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GameDialogueDeleteSelectedNode", "Delete Selected Game Dialogue Node"));

	GameDialogueGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	GameDialogueGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode *Node = CastChecked<UEdGraphNode>(*NodeIt);

		if (Node->CanUserDeleteNode())
		{
			/*
			if (USoundCueGraphNode* SoundGraphNode = Cast<USoundCueGraphNode>(Node))
			{
				USoundNode* DelNode = SoundGraphNode->SoundNode;

				FBlueprintEditorUtils::RemoveNode(NULL, SoundGraphNode, true);

				// Make sure SoundCue is updated to match graph
				SoundCue->CompileSoundNodesFromGraphNodes();

				// Remove this node from the SoundCue's list of all SoundNodes
				SoundCue->AllNodes.Remove(DelNode);
				SoundCue->MarkPackageDirty();
			}
			else
			{
				FBlueprintEditorUtils::RemoveNode(NULL, Node, true);
			}
			*/
		}
	}
}

bool 
FGameDialogueEditor::CanDeleteNodes
() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	if (SelectedNodes.Num() == 1)
	{
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			/*
			if (Cast<USoundCueGraphNode_Root>(*NodeIt))
			{
				// Return false if only root node is selected, as it can't be deleted
				return false;
			}
			*/
		}
	}

	return SelectedNodes.Num() > 0;
}

void
FGameDialogueEditor::DeleteSelectedDuplicatableNodes
()
{
	// Cache off the old selection
	const FGraphPanelSelectionSet OldSelectedNodes = GetSelectedNodes();

	// Clear the selection and only select the nodes that can be duplicated
	FGraphPanelSelectionSet RemainingNodes;
	GameDialogueGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			GameDialogueGraphEditor->SetNodeSelection(Node, true);
		}
		else
		{
			RemainingNodes.Add(Node);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	// Reselect whatever's left from the original selection after the deletion
	GameDialogueGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(RemainingNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			GameDialogueGraphEditor->SetNodeSelection(Node, true);
		}
	}
}

void
FGameDialogueEditor::CutSelectedNodes
()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
}

bool
FGameDialogueEditor::CanCutNodes
() const
{
	return CanCopyNodes() && CanDeleteNodes();
}

void
FGameDialogueEditor::CopySelectedNodes
()
{
	// Export the selected nodes and place the text on the clipboard
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		/*if (USoundCueGraphNode* Node = Cast<USoundCueGraphNode>(*SelectedIter))
		{
			Node->PrepareForCopying();
		}*/
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, /*out*/ ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	// Make sure SoundCue remains the owner of the copied nodes
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		/*if (USoundCueGraphNode* Node = Cast<USoundCueGraphNode>(*SelectedIter))
		{
			Node->PostCopyNode();
		}*/
	}
}

bool
FGameDialogueEditor::CanCopyNodes
() const
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			return true;
		}
	}
	return false;
}

void
FGameDialogueEditor::PasteNodes
()
{
	PasteNodesHere(GameDialogueGraphEditor->GetPasteLocation());
}

void
FGameDialogueEditor::PasteNodesHere
(const FVector2D &Location)
{
	// Undo/Redo support
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GameDialogueEditorPaste", "Paste Game Dialogue Node"));
	/*SoundCue->GetGraph()->Modify();
	SoundCue->Modify();*/

	// Clear the selection set (newly pasted stuff will be selected)
	GameDialogueGraphEditor->ClearSelectionSet();

	// Grab the text to paste from the clipboard.
	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	// Import the nodes
	TSet<UEdGraphNode*> PastedNodes;
	//FEdGraphUtilities::ImportNodesFromText(SoundCue->GetGraph(), TextToImport, /*out*/ PastedNodes);

	//Average position of nodes so we can move them while still maintaining relative distances to each other
	FVector2D AvgNodePosition(0.0f, 0.0f);

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	if (PastedNodes.Num() > 0)
	{
		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;
	}

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;

		/*if (USoundCueGraphNode* SoundGraphNode = Cast<USoundCueGraphNode>(Node))
		{
			SoundCue->AllNodes.Add(SoundGraphNode->SoundNode);
		}*/

		// Select the newly pasted stuff
		GameDialogueGraphEditor->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

		Node->SnapToGrid(SNodePanel::GetSnapGridSize());

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}

	// Force new pasted SoundNodes to have same connections as graph nodes
	//SoundCue->CompileSoundNodesFromGraphNodes();

	// Update UI
	GameDialogueGraphEditor->NotifyGraphChanged();

	/*SoundCue->PostEditChange();
	SoundCue->MarkPackageDirty();*/
}

bool
FGameDialogueEditor::CanPasteNodes
() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return false;
	//return FEdGraphUtilities::CanImportNodesFromText(SoundCue->SoundCueGraph, ClipboardContent);
}

void
FGameDialogueEditor::DuplicateNodes
()
{
	CopySelectedNodes();
	PasteNodes();
}

bool 
FGameDialogueEditor::CanDuplicateNodes
() const
{
	return CanCopyNodes();
}

void
FGameDialogueEditor::UndoGraphAction
()
{
	GameDialogueGraphEditor->ClearSelectionSet();

	GEditor->RedoTransaction();
}

void 
FGameDialogueEditor::RedoGraphAction
()
{
}
