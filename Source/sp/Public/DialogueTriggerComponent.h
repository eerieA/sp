#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "DialogueDataLoader.h"
#include "DialogueTriggerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SP_API UDialogueTriggerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDialogueTriggerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

public:
	// Collision box, a sub commponent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	UBoxComponent* TriggerBox;
	
private:
	// Dialogue config (exposed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(AllowPrivateAccess="true"))
	FString DialogueFilePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(AllowPrivateAccess="true"))
	FString StartingNodeID;

	// Your dialogue map type (keep same as your project)
	UPROPERTY()
	TMap<FString, FDialogueNode> DialogueData;

	// Overlap and end handlers
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	                    AActor* OtherActor,
	                    UPrimitiveComponent* OtherComp,
	                    int32 OtherBodyIndex,
	                    bool bFromSweep,
	                    const FHitResult& SweepResult);

	UFUNCTION()
	void HandleDialogueEnded();
};
