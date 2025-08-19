#include "DialogueManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DialogueDataLoader.h"

UDialogueManager::UDialogueManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueManager::BeginPlay()
{
    Super::BeginPlay();

    if (!DialogueJSONPath.IsEmpty())
    {
        bool bSuccess = LoadDialogueFromJSON(DialogueJSONPath);
        if (!bSuccess && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                FString::Printf(TEXT("Failed to load dialogue JSON at: %s"), *DialogueJSONPath));
        }
    } else if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
            TEXT("DialogueJSONPath is not set!"));
    }
    
    LoadDialogueFromJSON(DialogueJSONPath);
}

void UDialogueManager::StartDialogue(const FString& NodeID)
{
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Entered StartDialogue()."));
    CurrentNodeID = NodeID;
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: StartDialogue(), NodeID."));

    FString Line = GetCurrentLine();
    const FDialogueNode* Node = DialogueNodeMap.Find(CurrentNodeID);
    FString Speaker = Node ? Node->Speaker : TEXT("???");

    OnDialogueUpdated.Broadcast(Speaker, Line);

    TArray<FDialogueChoice> Choices = GetAvailableChoices();
    OnChoicesUpdated.Broadcast(Choices);
}

const FDialogueNode* UDialogueManager::GetCurrentNode() const
{
    if (CurrentNodeID.IsEmpty())
    {
        return nullptr;
    }

    return DialogueNodeMap.Find(CurrentNodeID);
}

FString UDialogueManager::GetCurrentLine() const
{
    const FDialogueNode* Node = DialogueNodeMap.Find(CurrentNodeID);    
    if (!Node)
        return FString("Node not found!");

    // Check alt lines (replacement)
    for (const FDialogueAltLine& Alt : Node->AltLines)
    {
        if (Alt.Condition.IsEmpty())
            continue;

        if (EvaluateConditionString(Alt.Condition))
        {
            // Found a replacement alt line
            FString Result = Alt.Text;

            // Append any append lines that match
            for (const FDialogueAltLine& App : Node->AppendLines)
            {
                if (App.Condition.IsEmpty())
                    continue;
                if (EvaluateConditionString(App.Condition))
                {
                    Result += " ";
                    Result += App.Text;
                }
            }

            return Result;
        }
    }

    // No replacement found; use BaseLine and append appendLines that match
    FString Base = Node->BaseLine;
    for (const FDialogueAltLine& App : Node->AppendLines)
    {
        if (App.Condition.IsEmpty())
            continue;
        if (EvaluateConditionString(App.Condition))
        {
            Base += " ";
            Base += App.Text;
        }
    }

    return Base;
}

TArray<FDialogueChoice> UDialogueManager::GetAvailableChoices() const
{
    TArray<FDialogueChoice> Result;

    const FDialogueNode* Node = DialogueNodeMap.Find(CurrentNodeID);
    if (!Node) return Result;

    for (const FDialogueChoice& Choice : Node->Choices)
    {
        // Check requirements (all must pass). Empty requirements => unlocked.
        bool bUnlocked = true;
        for (const FString& Req : Choice.Requirements)
        {
            if (!Req.IsEmpty() && !EvaluateConditionString(Req))
            {
                bUnlocked = false;
                break;
            }
        }

        // If not unlocked, we skip adding it to available list (alternatively you could add disabled entries)
        if (!bUnlocked) continue;

        // Resolve alt text for choice
        FString FinalText = Choice.Text;
        for (const FDialogueAltText& AltText : Choice.AltTexts)
        {
            if (!AltText.Condition.IsEmpty() && EvaluateConditionString(AltText.Condition))
            {
                FinalText = AltText.Text;
                break;
            }
        }

        FDialogueChoice Resolved = Choice;
        Resolved.Text = FinalText;
        Result.Add(Resolved);
    }

    return Result;
}

void UDialogueManager::SelectChoice(int32 ChoiceIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager::SelectChoice(%d)  CurrentNode=%s"), ChoiceIndex, *CurrentNodeID);

    TArray<FDialogueChoice> Choices = GetAvailableChoices();
    if (!Choices.IsValidIndex(ChoiceIndex)) return;

    const FDialogueChoice& Choice = Choices[ChoiceIndex];

    // Apply effects
    ApplyEffects(Choice.Effects);

    // Advance to next node
    if (!Choice.NextNodeID.IsEmpty())
    {
        CurrentNodeID = Choice.NextNodeID;
        StartDialogue(CurrentNodeID);
    }
    else
    {
        // No next node - end of dialogue
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Cyan, TEXT("Dialogue end."));
        OnDialogueEnded.Broadcast();
    }
}

void UDialogueManager::AdvanceDialogue()
{    
    const FDialogueNode* Node = DialogueNodeMap.Find(CurrentNodeID);
    if (!Node)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("AdvanceDialogue: current node not found"));
        return;
    }

    // If current node has no choices but has a NextNodeID, jump to it.
    if (Node->Choices.Num() == 0)
    {
        if (!Node->NextNodeID.IsEmpty())
        {
            StartDialogue(Node->NextNodeID);
            return;
        }
        else
        {
            // If no choice and no NextNodeID, we assume it is the end
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("AdvanceDialogue: end of dialogue"));
            OnDialogueEnded.Broadcast();
            return;
        }
    }

    // If current node has choices, prompt player to choose
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("AdvanceDialogue: node has choices — press a number key"));
}

bool UDialogueManager::LoadDialogueFromJSON(const FString& RelativePath)
{
    UDialogueDataLoader* Loader = NewObject<UDialogueDataLoader>(this);
    if (!Loader) return false;

    return Loader->LoadDialogueFromFile(RelativePath, DialogueNodeMap);
}

bool UDialogueManager::EvaluateConditionString(const FString& Condition) const
{
    if (Condition.IsEmpty()) return false;

    // Handle OR groups split by "||"
    TArray<FString> OrParts;
    SplitBySubstring(Condition, TEXT("||"), OrParts);

    for (const FString& OrPartRaw : OrParts)
    {
        // Each OrPart may contain && clauses
        TArray<FString> AndParts;
        SplitBySubstring(OrPartRaw, TEXT("&&"), AndParts);

        bool bAndAllTrue = true;
        for (FString AndPart : AndParts)
        {
            FString Expr = Trim(AndPart);
            if (Expr.IsEmpty()) continue;
            if (!EvaluateSingleExpression(Expr))
            {
                bAndAllTrue = false;
                break;
            }
        }

        if (bAndAllTrue)
        {
            // One OR branch succeeded
            return true;
        }
    }

    // No OR branch succeeded
    return false;
}

bool UDialogueManager::EvaluateSingleExpression(const FString& Expr) const
{
    // Find comparator
    static const TArray<FString> Comparators = { TEXT("=="), TEXT("!="), TEXT(">="), TEXT("<="), TEXT(">"), TEXT("<") };

    int32 FoundPos = INDEX_NONE;
    FString FoundComp;

    for (const FString& Comp : Comparators)
    {
        int32 Pos = Expr.Find(Comp, ESearchCase::IgnoreCase, ESearchDir::FromStart);
        if (Pos != INDEX_NONE)
        {
            FoundPos = Pos;
            FoundComp = Comp;
            break;
        }
    }

    if (FoundPos == INDEX_NONE)
    {
        // If no comparator found, treat as boolean/flag check (e.g., "FlagName" or "flag == true")
        FString Key = Trim(Expr);
        bool bVal = false;
        const bool* FoundFlag = Flags.Find(Key);
        if (FoundFlag) return *FoundFlag;
        // also check equality to string 'true'
        if (Key.Equals(TEXT("true"), ESearchCase::IgnoreCase)) return true;
        if (Key.Equals(TEXT("false"), ESearchCase::IgnoreCase)) return false;
        return false;
    }

    FString Left = Trim(Expr.Left(FoundPos));
    FString Right = Trim(Expr.Mid(FoundPos + FoundComp.Len()));

    // Remove surrounding quotes on Right if present
    if (Right.StartsWith("\"") && Right.EndsWith("\"") && Right.Len() >= 2)
    {
        Right = Right.Mid(1, Right.Len() - 2);
    }

    // Handle left attribute cases
    // trust (int)
    if (Left.Equals(TEXT("trust"), ESearchCase::IgnoreCase))
    {
        int32 RightInt = FCString::Atoi(*Right);
        if (FoundComp == "==") return Trust == RightInt;
        if (FoundComp == "!=") return Trust != RightInt;
        if (FoundComp == ">=") return Trust >= RightInt;
        if (FoundComp == "<=") return Trust <= RightInt;
        if (FoundComp == ">") return Trust > RightInt;
        if (FoundComp == "<") return Trust < RightInt;
        return false;
    }

    // last_topic (string)
    if (Left.Equals(TEXT("last_topic"), ESearchCase::IgnoreCase))
    {
        if (FoundComp == "==") return LastTopic == Right;
        if (FoundComp == "!=") return LastTopic != Right;
        // numeric comparisons for strings not supported
        return false;
    }

    // skill.<name>
    if (Left.StartsWith(TEXT("skill."), ESearchCase::IgnoreCase))
    {
        FString SkillName = Left.RightChop(6); // remove "skill."
        const int32* Found = Skills.Find(SkillName);
        int32 SkillValue = Found ? *Found : 0;
        int32 RightInt = FCString::Atoi(*Right);
        if (FoundComp == "==") return SkillValue == RightInt;
        if (FoundComp == "!=") return SkillValue != RightInt;
        if (FoundComp == ">=") return SkillValue >= RightInt;
        if (FoundComp == "<=") return SkillValue <= RightInt;
        if (FoundComp == ">") return SkillValue > RightInt;
        if (FoundComp == "<") return SkillValue < RightInt;
        return false;
    }

    // flags (boolean)
    {
        const bool* Found = Flags.Find(Left);
        if (Found)
        {
            bool RightBool = Right.Equals(TEXT("true"), ESearchCase::IgnoreCase);
            if (FoundComp == "==") return *Found == RightBool;
            if (FoundComp == "!=") return *Found != RightBool;
        }
    }

    // fallback: try compare as integers (Left may be an int attr you add later)
    int32 LeftVal = 0;
    const int32* Lfound = nullptr;
    // no general int map in header, so fallback false
    return false;
}

void UDialogueManager::ApplyEffects(const TArray<FDialogueEffect>& Effects)
{
    for (const FDialogueEffect& Eff : Effects)
    {
        if (Eff.Attribute.Equals(TEXT("trust"), ESearchCase::IgnoreCase))
        {
            if (Eff.Operation == EDialogueEffectOp::Add)
            {
                int32 Delta = FCString::Atoi(*Eff.Value);
                Trust += Delta;
            }
            else if (Eff.Operation == EDialogueEffectOp::Set)
            {
                Trust = FCString::Atoi(*Eff.Value);
            }
        }
        else if (Eff.Attribute.Equals(TEXT("last_topic"), ESearchCase::IgnoreCase))
        {
            if (Eff.Operation == EDialogueEffectOp::Set)
            {
                LastTopic = Eff.Value;
            }
            else if (Eff.Operation == EDialogueEffectOp::Add)
            {
                // treat Add on strings as Set
                LastTopic = Eff.Value;
            }
        }
        else
        {
            // Generic attribute handling:
            // If it's a flag, treat Set/Toggle
            if (Eff.Operation == EDialogueEffectOp::Toggle)
            {
                bool* Found = Flags.Find(Eff.Attribute);
                if (Found)
                {
                    *Found = !(*Found);
                }
                else
                {
                    Flags.Add(Eff.Attribute, true);
                }
            }
            else if (Eff.Operation == EDialogueEffectOp::Set)
            {
                // try boolean value
                if (Eff.Value.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Eff.Value.Equals(TEXT("false"), ESearchCase::IgnoreCase))
                {
                    Flags.Add(Eff.Attribute, Eff.Value.Equals(TEXT("true"), ESearchCase::IgnoreCase));
                }
                else
                {
                    // assume string attribute - for now only last_topic uses string; you can expand with a map
                }
            }
            else if (Eff.Operation == EDialogueEffectOp::Add)
            {
                // support skill increments (e.g., skill.observation)
                if (Eff.Attribute.StartsWith(TEXT("skill."), ESearchCase::IgnoreCase))
                {
                    FString SkillName = Eff.Attribute.RightChop(6);
                    int32 Delta = FCString::Atoi(*Eff.Value);
                    int32& ValRef = Skills.FindOrAdd(SkillName);
                    ValRef += Delta;
                }
            }
        }
    }
}

void UDialogueManager::SplitBySubstring(const FString& Input, const FString& Separator, TArray<FString>& Out) const
{
    Out.Empty();

    if (Separator.IsEmpty())
    {
        Out.Add(Input);
        return;
    }

    int32 Start = 0;
    while (true)
    {
        int32 Found = Input.Find(Separator, ESearchCase::IgnoreCase, ESearchDir::FromStart, Start);
        if (Found == INDEX_NONE)
        {
            Out.Add(Input.Mid(Start));
            break;
        }

        Out.Add(Input.Mid(Start, Found - Start));
        Start = Found + Separator.Len();
        if (Start >= Input.Len())
        {
            Out.Add(TEXT(""));
            break;
        }
    }

    // Trim each part
    for (FString& S : Out) S = Trim(S);
}

FString UDialogueManager::Trim(const FString& In) const
{
    FString Out = In;
    Out.TrimStartInline();
    Out.TrimEndInline();
    return Out;
}
