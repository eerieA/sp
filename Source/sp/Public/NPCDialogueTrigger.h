// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "DialogueDataLoader.h"
#include "NPCDialogueTrigger.generated.h"

UCLASS()
class SP_API ANPCDialogueTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANPCDialogueTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// The box that triggers dialogue
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	UBoxComponent* TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString DialogueFilePath;  // e.g. "Data/Dialogue/NPC_Bob.json"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString StartingNodeID;    // the node ID in that JSON to begin

	UPROPERTY()
	TMap<FString, FDialogueNode> DialogueData;
	
	// Overlap logic functions
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

};
