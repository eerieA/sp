#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "DialogueNode.h" // FDialogueNode struct
#include "DialogueManager.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SP_API UDialogueManager : public UActorComponent
{
	GENERATED_BODY()

public:    
	UDialogueManager();

protected:
	virtual void BeginPlay() override;

public:    
	// DataTable reference
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* DialogueDataTable;

	// Current node ID
	FString CurrentNodeID;

	// Simple player state
	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	int32 Trust = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FString LastTopic;

	// Start a dialogue at a node
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(const FString& NodeID);

	// Get the current line (evaluates alt lines)
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString GetCurrentLine() const;

	// List available choices (ignores complex requirements for now)
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FDialogueChoice> GetAvailableChoices() const;

	// Select a choice
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectChoice(int32 ChoiceIndex);
};
