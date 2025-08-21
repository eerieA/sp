#include "NPCDialogueTrigger.h"

#include "DialogueManager.h"
#include "spPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ANPCDialogueTrigger::ANPCDialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    UBillboardComponent* Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    Billboard->SetupAttachment(RootComponent);

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(100, 100, 100));
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

    // Ensure overlap events are generated
    TriggerBox->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ANPCDialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    if (TriggerBox)
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ANPCDialogueTrigger::OnOverlapBegin);
    }

    // Load this NPC's dialogue JSON once at startup
    if (!DialogueFilePath.IsEmpty())
    {
        UDialogueDataLoader* Loader = NewObject<UDialogueDataLoader>(this);
        if (!Loader) return;
        
        Loader->LoadDialogueFromFile(DialogueFilePath, DialogueData);
    }
}

// Called every frame
void ANPCDialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANPCDialogueTrigger::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Quick visual debug so we know the function fired
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("NPCDialogueTrigger::OnOverlapBegin"));
    }

    if (!OtherActor || OtherActor == this)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTrigger: No OtherActor is found or OtherActor is the current actor."));
        return;
    }

    // Only continue if it is player character overlapping it
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTrigger: The OtherActor is not a player actor."));
        return;
    }

    // PlayerController is expected, but this works generically.
    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTrigger: No PlayerController is found or casted."));
        return;
    }

    // Find DialogueManager component on the controller
    UDialogueManager* DM = PC->FindComponentByClass<UDialogueManager>();
    if (!DM)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTrigger: DialogueManager not found on PlayerController"));
        return;
    }

    if (DialogueData.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTrigger: DialogueData is somehow empty, cannot start dialogue."));
        return;
    }

    // Preparation 1: disable player movement
    if (PlayerChar && PlayerChar->GetCharacterMovement())
    {
        PlayerChar->GetCharacterMovement()->DisableMovement();
    }
    // Preparation 2: change UI mode
    AspPlayerController* spPC = Cast<AspPlayerController>(PC);
    if (spPC && spPC->DialogueWidgetInstance)
    {
        // Show mouse cursor
        PC->bShowMouseCursor = true;

        // Use Game+UI mode so InputComponent bindings (Space, etc.) still fire
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(spPC->DialogueWidgetInstance->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        PC->SetInputMode(InputMode);
    }
    // Preparation 3: bind dialogue end handler to DM's dialogue end event
    DM->OnDialogueEnded.AddDynamic(this, &ANPCDialogueTrigger::HandleDialogueEnded);
    
    UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTrigger: Starting dialogue."));
    // Start dialogue (use the node id defined in the json we want to use)
    DM->StartDialogue(StartingNodeID, &DialogueData);
}

void ANPCDialogueTrigger::HandleDialogueEnded()
{    
    UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTrigger: Entered HandleDialogueEnded()."));

    // Unbind immediately so handler won't be called again by stale DM
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        if (UDialogueManager* DM = PC->FindComponentByClass<UDialogueManager>())
        {
            DM->OnDialogueEnded.RemoveDynamic(this, &ANPCDialogueTrigger::HandleDialogueEnded);
            UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTrigger: Unbound HandleDialogueEnded from DM %p"), DM);
        }
    }    
    
    // Restore movement
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (PlayerChar && PlayerChar->GetCharacterMovement())
    {
        PlayerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }

    // UI mode resume will be done in spPlayerController
}
