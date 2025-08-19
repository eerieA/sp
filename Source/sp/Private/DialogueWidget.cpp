// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueWidget.h"
#include "spPlayerController.h"

void UDialogueWidget::ShowWidget(bool bShow)
{
	SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UDialogueWidget::UpdateDialogue(const FString& Line, const TArray<FDialogueChoice>& Choices)
{
	// Update C++ visible properties (these are BlueprintReadOnly)
	CurrentLine = FText::FromString(Line);
	CurrentChoices = Choices;

	// Let the Blueprint subclass rebuild visuals
	OnDialogueUpdated_BP();
}

void UDialogueWidget::NotifyChoiceSelected(int32 ChoiceIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("DialogueWidget: NotifyChoiceSelected(%d)"), ChoiceIndex);
	
	// Call the BP event (Blueprint will handle UI → controller hookup)
	OnChoiceSelected(ChoiceIndex);

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UDialogueManager* DM = PC->FindComponentByClass<UDialogueManager>())
		{
			DM->SelectChoice(ChoiceIndex);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("DialogueWidget: No DialogueManager found on owning player"));
}
