// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DialogueManager.h"
#include "spPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SP_API AspPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AspPlayerController();

	// Holds a DialogueManager instance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UDialogueManager* DialogueManager;
	// Let it be able to receive a widget for dialogue UI
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> DialogueWidgetClass;
	UPROPERTY()
	UUserWidget* DialogueWidgetInstance;

	virtual void SetupInputComponent() override;

	// Temp function to force starting dialogue
	void TestStartDialogue();
	
	// Handlers for number keys (map numbers 1..9 to choice indices 0..8)
	UFUNCTION()
	void OnChoice0(); // number 1 => index 0
	UFUNCTION()
	void OnChoice1();
	UFUNCTION()
	void OnChoice2();
	UFUNCTION()
	void OnChoice3();
	UFUNCTION()
	void OnChoice4();
	UFUNCTION()
	void OnChoice5();
	UFUNCTION()
	void OnChoice6();
	UFUNCTION()
	void OnChoice7();
	UFUNCTION()
	void OnChoice8();

	// Handler for no-choice advance (space)
	UFUNCTION()
	void OnAdvance();

	UFUNCTION()
	void HandleDialogueEnded();

	UFUNCTION()
	void HandleOnDialogueUpdated(const FString& Speaker, const FString& Line);

	UFUNCTION()
	void HandleOnChoicesUpdated(const TArray<FDialogueChoice>& Choices);

	// helper to reduce repetition
	void SelectChoiceByIndex(int32 Index);

	void UpdateDialogueUI();	

protected:
	virtual void BeginPlay() override;
};
