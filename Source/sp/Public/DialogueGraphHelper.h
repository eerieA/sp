// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueNode.h"
#include "DialogueGraphHelper.generated.h"

// Simple structs for node visual info
USTRUCT(BlueprintType)
struct FDialogueGraphNode
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString NodeID;

	UPROPERTY(BlueprintReadOnly)
	FVector2D Position;
};

USTRUCT(BlueprintType)
struct FDialogueGraphEdge
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString FromNodeID;

	UPROPERTY(BlueprintReadOnly)
	FString ToNodeID;
};

/**
 * 
 */
UCLASS()
class SP_API UDialogueGraphHelper : public UObject
{
	GENERATED_BODY()
	
public:

	// Generates nodes & edges from dialogue map
	UFUNCTION(BlueprintCallable, Category="Dialogue Graph")
	static void GenerateGraph(
		const TMap<FString, FDialogueNode>& DialogueNodes,
		TArray<FDialogueGraphNode>& OutNodes,
		TArray<FDialogueGraphEdge>& OutEdges
	);
};
