#include "DialogueManager.h"
#include "DialogueNode.h"
#include "Engine/Engine.h"

UDialogueManager::UDialogueManager()
    : DialogueDataTable(nullptr) // explicit initialization
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueManager::BeginPlay()
{
    Super::BeginPlay();
}

void UDialogueManager::StartDialogue(const FString& NodeID)
{
    CurrentNodeID = NodeID;

    FString Line = GetCurrentLine();
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%s"), *Line));

    TArray<FDialogueChoice> Choices = GetAvailableChoices();
    for(int i = 0; i < Choices.Num(); ++i)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("%d: %s"), i, *Choices[i].Text));
    }
}

FString UDialogueManager::GetCurrentLine() const
{
    if (!DialogueDataTable) return FString("No DataTable set!");

    FDialogueNode* Node = DialogueDataTable->FindRow<FDialogueNode>(*CurrentNodeID, TEXT("GetCurrentLine"));
    if (!Node) return FString("Node not found!");

    for (const FDialogueAltLine& AltLine : Node->AltLineConditions)
    {
        for (const FDialogueCondition& Cond : AltLine.Conditions)
        {
            // Example: check "trust <= -1"
            if (Cond.Attribute == "trust" && Cond.Comparator == "<=")
            {
                int32 Value = FCString::Atoi(*Cond.Value);
                if (Trust <= Value)
                {
                    return AltLine.Text; // Use the line text from AltLine
                }
            }

            // Will add other attribute checks (last_topic, skills, etc.)
        }
    }

    return Node->BaseLine;
}

TArray<FDialogueChoice> UDialogueManager::GetAvailableChoices() const
{
    if (!DialogueDataTable) return {};
    FDialogueNode* Node = DialogueDataTable->FindRow<FDialogueNode>(*CurrentNodeID, TEXT("GetAvailableChoices"));
    if (!Node) return {};
    
    return Node->Choices; // for now, ignoring requirement checks
}

void UDialogueManager::SelectChoice(int32 ChoiceIndex)
{
    TArray<FDialogueChoice> Choices = GetAvailableChoices();
    if (!Choices.IsValidIndex(ChoiceIndex)) return;

    const FDialogueChoice& Choice = Choices[ChoiceIndex];

    // Apply effects (very simple: trust +1 if attribute is "trust")
    for (const FDialogueEffect& Effect : Choice.Effects)
    {
        if (Effect.Attribute == "trust")
        {
            if (Effect.bSetDirectly)
                Trust = Effect.ChangeValue;
            else
                Trust += Effect.ChangeValue;
        }
        if (Effect.Attribute == "last_topic")
        {
            LastTopic = FString::FromInt(Effect.ChangeValue); // adapt as needed
        }
    }

    // Advance to next node
    CurrentNodeID = Choice.NextNodeID;
    StartDialogue(CurrentNodeID);
}
