// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueGraphHelper.h"

void UDialogueGraphHelper::GenerateGraph(
	const TMap<FString, FDialogueNode>& DialogueNodes,
	TArray<FDialogueGraphNode>& OutNodes,
	TArray<FDialogueGraphEdge>& OutEdges
)
{
	OutNodes.Empty();
	OutEdges.Empty();

	// Simple vertical layout: y = index * 150, x = 100
	int32 Index = 0;
	for (const auto& Pair : DialogueNodes)
	{
		FDialogueGraphNode Node;
		Node.NodeID = Pair.Key;
		Node.Position = FVector2D(100.0f, Index * 150.0f);
		OutNodes.Add(Node);
		Index++;
	}

	// Build edges
	for (const auto& Pair : DialogueNodes)
	{
		const FDialogueNode& Node = Pair.Value;

		auto AddEdgeIfValid = [&](const FString& TargetID)
		{
			if (!TargetID.IsEmpty() && DialogueNodes.Contains(TargetID))
			{
				FDialogueGraphEdge Edge;
				Edge.FromNodeID = Node.ID;
				Edge.ToNodeID = TargetID;
				OutEdges.Add(Edge);
			}
		};

		AddEdgeIfValid(Node.NextNodeID);

		for (const FDialogueChoice& Choice : Node.Choices)
		{
			AddEdgeIfValid(Choice.NextNodeID);
			AddEdgeIfValid(Choice.FailureNodeID);
		}
	}
}