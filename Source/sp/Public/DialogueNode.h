#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueNode.generated.h"

// Simple operation enum for effects
UENUM(BlueprintType)
enum class EDialogueEffectOp : uint8
{
    Add     UMETA(DisplayName = "Add"),   // numeric add, e.g., trust +1
    Set     UMETA(DisplayName = "Set"),   // set to a value, e.g., last_topic = "autonomy"
    Toggle  UMETA(DisplayName = "Toggle") // toggle boolean flag
};

// A single alternate line that appears if Condition (string) evaluates true.
// Condition is a human-friendly expression string (e.g., "trust <= -1", "last_topic == \"autonomy\"")
USTRUCT(BlueprintType)
struct SP_API FDialogueAltLine
{
    GENERATED_BODY()

    // A condition string to be evaluated by your condition parser/evaluator.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Condition;

    // The text to display if Condition is true
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Text;
};

// Alternate text for a choice (same pattern as alt lines)
USTRUCT(BlueprintType)
struct SP_API FDialogueAltText
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Condition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Text;
};

// Effects that happen when a choice is selected.
// Value is stored as string so it can represent numbers or strings; parse it when applying.
USTRUCT(BlueprintType)
struct SP_API FDialogueEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Attribute; // e.g., "trust", "last_topic", "Clue_Cinema_MainDoor"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EDialogueEffectOp Operation = EDialogueEffectOp::Set;

    // The value to use. For Add operation, use numeric string like "1". For Set, any string like "autonomy" or "true".
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Value;
};

// Choice structure authored by writers
USTRUCT(BlueprintType)
struct SP_API FDialogueChoice
{
    GENERATED_BODY()

    // Default text for the choice
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Text;

    // Optional alternate texts that overwrite default if their Condition is true
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FDialogueAltText> AltTexts;

    // Requirements expressed as string conditions. All must pass (AND) to be unlocked.
    // e.g., ["trust >= 2", "skill.observation >= 1"]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> Requirements;

    // Effects to apply when this choice is selected
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FDialogueEffect> Effects;

    // If unlocked, go to this node
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextNodeID;

    // (Optional) Node to go to if Requirements fail (failure branch)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString FailureNodeID;
};

// Top-level node (DataTable row)
USTRUCT(BlueprintType)
struct SP_API FDialogueNode : public FTableRowBase
{
    GENERATED_BODY()

    // explicit id (optional if you rely on DataTable row name, but convenient)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Speaker;

    // The primary line (base)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString BaseLine;

    // Alternate lines that *replace* the BaseLine if their Condition is true.
    // Evaluate in order — first match wins (you decide order in data).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FDialogueAltLine> AltLines;

    // Lines that *append* to the baseline when their Condition is true (useful for notes/extra info)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FDialogueAltLine> AppendLines;

    // Player choices for this node (empty -> auto-advance via NextNodeID)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FDialogueChoice> Choices;

    // If present and Choices is empty, automatically continue to this node after showing line
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextNodeID;
};
