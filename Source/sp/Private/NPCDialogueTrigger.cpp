// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCDialogueTrigger.h"

#include "DialogueManager.h"
#include "Components/BillboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ANPCDialogueTrigger::ANPCDialogueTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a root scene component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Add a billboard so we can see it in the editor
	UBillboardComponent* Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(RootComponent);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(100, 100, 100));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

// Called when the game starts or when spawned
void ANPCDialogueTrigger::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind overlap event
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ANPCDialogueTrigger::OnOverlapBegin);
	}
}

// Called every frame
void ANPCDialogueTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANPCDialogueTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
										 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
										 bool bFromSweep, const FHitResult & SweepResult)
{
	if (!OtherActor) return;

	// Check if overlapping actor is the player character
	if (ACharacter* PlayerChar = Cast<ACharacter>(OtherActor))
	{
		if (APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController()))
		{
			if (UDialogueManager* DM = PC->FindComponentByClass<UDialogueManager>())
			{
				DM->StartDialogue("luka_problem_entry"); // The node ID defined in dialogue data
			}
		}
	}
}


