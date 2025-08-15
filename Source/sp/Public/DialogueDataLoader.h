// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueNode.h"
#include "DialogueDataLoader.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SP_API UDialogueDataLoader : public UObject
{
	GENERATED_BODY()

public:

	// Load from JSON file; returns a map keyed by NodeID
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	bool LoadDialogueFromFile(const FString& RelativePath, TMap<FString, FDialogueNode>& OutNodes);

};
