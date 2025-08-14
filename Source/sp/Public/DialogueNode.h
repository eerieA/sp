#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueNode.generated.h"

USTRUCT(BlueprintType)
struct SP_API FDialogueCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Comparator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value;
};

USTRUCT(BlueprintType)
struct SP_API FDialogueEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSetDirectly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChangeValue = 0;
};

USTRUCT(BlueprintType)
struct SP_API FDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDialogueCondition> Requirements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDialogueEffect> Effects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NextNodeID;
};

USTRUCT(BlueprintType)
struct SP_API FDialogueAltLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDialogueCondition> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text;
};

USTRUCT(BlueprintType)
struct SP_API FDialogueNode : public FTableRowBase
{
	GENERATED_BODY()

	// The default line spoken by the NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BaseLine;

	// Alternate lines if certain conditions are met
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDialogueAltLine> AltLineConditions;

	// Choices available to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDialogueChoice> Choices;
};