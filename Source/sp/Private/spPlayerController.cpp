// Fill out your copyright notice in the Description page of Project Settings.


#include "spPlayerController.h"
#include "DialogueManager.h"
#include "DialogueWidget.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h" // for EKeys
#include "Blueprint/UserWidget.h"

AspPlayerController::AspPlayerController()
{
	// Create and attach the DialogueManager component
	DialogueManager = CreateDefaultSubobject<UDialogueManager>(TEXT("DialogueManager"));
}

void AspPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent) return;

	// Bind number keys (1..9)
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AspPlayerController::OnChoice0);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AspPlayerController::OnChoice1);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AspPlayerController::OnChoice2);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AspPlayerController::OnChoice3);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AspPlayerController::OnChoice4);
	InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &AspPlayerController::OnChoice5);
	InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &AspPlayerController::OnChoice6);
	InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &AspPlayerController::OnChoice7);
	InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &AspPlayerController::OnChoice8);

	// Bind space to advance (or continue auto-next)
	InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AspPlayerController::OnAdvance);
}

void AspPlayerController::OnChoice0() { SelectChoiceByIndex(0); }
void AspPlayerController::OnChoice1() { SelectChoiceByIndex(1); }
void AspPlayerController::OnChoice2() { SelectChoiceByIndex(2); }
void AspPlayerController::OnChoice3() { SelectChoiceByIndex(3); }
void AspPlayerController::OnChoice4() { SelectChoiceByIndex(4); }
void AspPlayerController::OnChoice5() { SelectChoiceByIndex(5); }
void AspPlayerController::OnChoice6() { SelectChoiceByIndex(6); }
void AspPlayerController::OnChoice7() { SelectChoiceByIndex(7); }
void AspPlayerController::OnChoice8() { SelectChoiceByIndex(8); }

void AspPlayerController::OnAdvance()
{
	if (UDialogueManager* DM = FindComponentByClass<UDialogueManager>())
	{
		DM->AdvanceDialogue();
	}
}

void AspPlayerController::HandleDialogueEnded()
{
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		
		// Return control back to game
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}

void AspPlayerController::HandleOnDialogueUpdated(const FString& Speaker, const FString& Line)
{
	UpdateDialogueUI();
}

void AspPlayerController::HandleOnChoicesUpdated(const TArray<FDialogueChoice>& Choices)
{
	// Rebuild the choice buttons
	if (UDialogueWidget* DW = Cast<UDialogueWidget>(DialogueWidgetInstance))
	{
		DW->UpdateDialogue(DW->CurrentLine.ToString(), Choices);
	}
}

void AspPlayerController::SelectChoiceByIndex(int32 Index)
{
	if (UDialogueManager* DM = FindComponentByClass<UDialogueManager>())
	{
		// Print what player is attempting to select
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Trying choice %d"), Index));

		DM->SelectChoice(Index);
	}
	else
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("No DialogueManager component found on PlayerController"));
	}
}

void AspPlayerController::UpdateDialogueUI()
{
	if (!DialogueWidgetInstance || !DialogueManager) return;

	UDialogueWidget* DW = Cast<UDialogueWidget>(DialogueWidgetInstance);
	if (!DW) return;

	// Example data pull
	const FDialogueNode* CurrentNode = DialogueManager->GetCurrentNode();
	if (!CurrentNode)
	{
		DW->ShowWidget(false);
		return;
	}

	// Pass speaker + text + choices to widget
	DW->ShowWidget(true);
	DW->CurrentSpeaker = FText::FromString(CurrentNode->Speaker);
	DW->UpdateDialogue(CurrentNode->BaseLine, CurrentNode->Choices);
}

void AspPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (DialogueWidgetClass)
	{
		DialogueWidgetInstance = CreateWidget<UUserWidget>(this, DialogueWidgetClass);
		if (DialogueWidgetInstance)
		{
			DialogueWidgetInstance->AddToViewport();
			DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		}		
	}

	if (DialogueManager)
	{
		// Bind a delegate to handle necessary processes after a dialogue ends
		DialogueManager->OnDialogueEnded.AddDynamic(this, &AspPlayerController::HandleDialogueEnded);
		// Bind update delegates so UI refreshes automatically
		DialogueManager->OnDialogueUpdated.AddDynamic(this, &AspPlayerController::HandleOnDialogueUpdated);
		DialogueManager->OnChoicesUpdated.AddDynamic(this, &AspPlayerController::HandleOnChoicesUpdated);
    }
}
