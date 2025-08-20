#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueNode.h"   // All structs related to a dialogue node
#include "DialogueManager.generated.h"

// Delegate for UI updates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueUpdated, const FString&, Speaker, const FString&, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChoicesUpdated, const TArray<FDialogueChoice>&, Choices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SP_API UDialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    // Holds all loaded nodes by ID in memory
    UPROPERTY(BlueprintReadOnly, Category="Dialogue")
    TMap<FString, FDialogueNode> DialogueNodeMap;

    // Current node id
    UPROPERTY(BlueprintReadOnly, Category="Dialogue")
    FString CurrentNodeID;

    // Simple state
    UPROPERTY(BlueprintReadWrite, Category="Dialogue")
    int32 Trust = 0;

    UPROPERTY(BlueprintReadWrite, Category="Dialogue")
    FString LastTopic;

    // Relative path to JSON, e.g., "Dialogues/sample_dlg.json"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
    FString DialogueJSONPath = TEXT("Dialogues/change_me.json");;

    // Skills map (e.g., skill.observation -> int)
    UPROPERTY(BlueprintReadWrite, Category="Dialogue")
    TMap<FString,int32> Skills;

    // Flags map
    UPROPERTY(BlueprintReadWrite, Category="Dialogue")
    TMap<FString,bool> Flags;

    // Start dialogue at node
    UFUNCTION(BlueprintCallable, Category="Dialogue")
    void StartDialogue(const FString& NodeID, const TMap<FString, FDialogueNode>& DialogueNodeMapReplace);
    
    // Returns a pointer to the current node, or nullptr if not found
    // No need for UFUNCTION decorator
    const FDialogueNode* GetCurrentNode() const;
    
    // Get display line (resolves alt + append)
    UFUNCTION(BlueprintCallable, Category="Dialogue")
    FString GetCurrentLine() const;

    // Get list of choices (with resolved text and only unlocked)
    UFUNCTION(BlueprintCallable, Category="Dialogue")
    TArray<FDialogueChoice> GetAvailableChoices() const;

    // Select a choice index (applies effects and advances)
    UFUNCTION(BlueprintCallable, Category="Dialogue")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category="Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category="Dialogue")
    bool LoadDialogueFromJSON(const FString& RelativePath);

    UPROPERTY(BlueprintAssignable, Category="Dialogue")
    FOnDialogueUpdated OnDialogueUpdated;
    
    UPROPERTY(BlueprintAssignable, Category="Dialogue")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category="Dialogue")
    FOnChoicesUpdated OnChoicesUpdated;

protected:
    // Evaluate a full condition string. Supports "||" and "&&" (basic).
    bool EvaluateConditionString(const FString& Condition) const;

    // Evaluate a single expression like 'trust >= 1' or 'last_topic == "autonomy"'
    bool EvaluateSingleExpression(const FString& Expr) const;

    // Apply effects from a choice
    void ApplyEffects(const TArray<FDialogueEffect>& Effects);

    // Helper: split by substring (works with multi-char separators)
    void SplitBySubstring(const FString& Input, const FString& Separator, TArray<FString>& Out) const;

    // Helper: trim
    FString Trim(const FString& In) const;
};
