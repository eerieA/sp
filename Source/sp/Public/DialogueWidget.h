// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueNode.h" // For FDialogueNode, FDialogueChoice
#include "DialogueWidget.generated.h"

/**
 * 
 */
UCLASS()
class SP_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	// Properties for the blueprint subclass
	UPROPERTY(BlueprintReadOnly)
	FText CurrentSpeaker;
	UPROPERTY(BlueprintReadOnly)
	FText CurrentLine;
	UPROPERTY(BlueprintReadOnly)
	TArray<FDialogueChoice> CurrentChoices;
	
	// Called to show or hide the widget
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void ShowWidget(bool bShow);

	// Process updated dialogue data
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void UpdateDialogue(const FString& Line, const TArray<FDialogueChoice>& Choices);

	// Called when player selects a choice (index) through a button
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void NotifyChoiceSelected(int32 ChoiceIndex);

	// Invoked when NotifyChoiceSelected is called
	UFUNCTION(BlueprintImplementableEvent, Category="Dialogue")
	void OnChoiceSelected(int32 ChoiceIndex);

	// Invoked when UpdateDialogue is called
	// just a hook the Blueprint will implement to rebuild the visible widgets
	UFUNCTION(BlueprintImplementableEvent, Category="Dialogue")
	void OnDialogueUpdated_BP();
};
