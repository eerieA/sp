// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueDataLoader.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"

bool UDialogueDataLoader::LoadDialogueFromFile(const FString& RelativePath, TMap<FString, FDialogueNode>& OutNodes)
{
	const FString FullPath = FPaths::ProjectContentDir() / RelativePath;
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FullPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load dialogue JSON: %s"), *FullPath);
		return false;
	}

	TSharedPtr<FJsonObject> RootObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON in file: %s"), *FullPath);
		return false;
	}

	OutNodes.Empty();
	for (const auto& Pair : RootObj->Values)
	{
		const FString NodeID = Pair.Key;
		TSharedPtr<FJsonObject> NodeObj = Pair.Value->AsObject();
		if (!NodeObj.IsValid()) continue;

		NodeObj->SetStringField(TEXT("ID"), NodeID);

		FDialogueNode NodeStruct;
		if (FJsonObjectConverter::JsonObjectToUStruct<FDialogueNode>(NodeObj.ToSharedRef(), &NodeStruct, 0, 0))
		{
			OutNodes.Add(NodeID, MoveTemp(NodeStruct));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to convert node: %s"), *NodeID);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded %d dialogue nodes from %s"), OutNodes.Num(), *FullPath);
	return true;
}
