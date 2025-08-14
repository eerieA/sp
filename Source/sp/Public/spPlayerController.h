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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UDialogueManager* DialogueManager;
	
};
