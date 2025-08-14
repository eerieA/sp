// Fill out your copyright notice in the Description page of Project Settings.


#include "spPlayerController.h"

AspPlayerController::AspPlayerController()
{
	// Create and attach the DialogueManager component
	DialogueManager = CreateDefaultSubobject<UDialogueManager>(TEXT("DialogueManager"));
}