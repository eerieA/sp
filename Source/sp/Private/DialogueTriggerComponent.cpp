#include "DialogueTriggerComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DialogueManager.h"
#include "spPlayerController.h"
#include "Engine/Engine.h"

UDialogueTriggerComponent::UDialogueTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	TriggerBox = nullptr;
}

void UDialogueTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueTriggerComponent: No owner found."));
		return;
	}

	// Create BoxComponent at runtime and attach to owner's root
	if (!TriggerBox)
	{
		TriggerBox = NewObject<UBoxComponent>(Owner, TEXT("DialogueTriggerBox"));
		if (TriggerBox)
		{
			TriggerBox->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
			TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
			TriggerBox->SetGenerateOverlapEvents(true);
			TriggerBox->RegisterComponent(); // registers with the world
		}
	}

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &UDialogueTriggerComponent::OnOverlapBegin);
	}

	// Load dialogue data (same loader you used)
	if (!DialogueFilePath.IsEmpty())
	{
		UDialogueDataLoader* Loader = NewObject<UDialogueDataLoader>(this);
		if (Loader)
		{
			Loader->LoadDialogueFromFile(DialogueFilePath, DialogueData);
		}
	}
}

void UDialogueTriggerComponent::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("DialogueTriggerComponent::OnOverlapBegin"));
	}

	if (!OtherActor || OtherActor == GetOwner()) return;

	// Only allow player character
	ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
	if (!PlayerChar) return;

	APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
	if (!PC) return;

	UDialogueManager* DM = PC->FindComponentByClass<UDialogueManager>();
	if (!DM)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueTriggerComponent: DialogueManager not found on PlayerController"));
		return;
	}

	if (DialogueData.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueTriggerComponent: DialogueData empty, cannot start dialogue."));
		return;
	}

	// Disable player movement
	if (PlayerChar->GetCharacterMovement())
	{
		PlayerChar->GetCharacterMovement()->DisableMovement();
	}

	// Switch input mode if using AspPlayerController and the widget instance
	if (AspPlayerController* spPC = Cast<AspPlayerController>(PC))
	{
		if (spPC->DialogueWidgetInstance)
		{
			PC->bShowMouseCursor = true;
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(spPC->DialogueWidgetInstance->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
		}
	}

	// Bind to DM end event
	DM->OnDialogueEnded.AddDynamic(this, &UDialogueTriggerComponent::HandleDialogueEnded);

	UE_LOG(LogTemp, Log, TEXT("DialogueTriggerComponent: Starting dialogue."));
	DM->StartDialogue(StartingNodeID, &DialogueData);
}

void UDialogueTriggerComponent::HandleDialogueEnded()
{
	UE_LOG(LogTemp, Log, TEXT("DialogueTriggerComponent: HandleDialogueEnded called."));

	// Unbind from DialogueManager on the player controller (safe remove)
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		if (UDialogueManager* DM = PC->FindComponentByClass<UDialogueManager>())
		{
			DM->OnDialogueEnded.RemoveDynamic(this, &UDialogueTriggerComponent::HandleDialogueEnded);
			UE_LOG(LogTemp, Log, TEXT("DialogueTriggerComponent: Unbound from DM %p"), DM);
		}
	}

	// Restore movement
	if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		if (PlayerChar->GetCharacterMovement())
		{
			PlayerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
}

void UDialogueTriggerComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.RemoveAll(this);
		TriggerBox->DestroyComponent();
		TriggerBox = nullptr;
	}
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
