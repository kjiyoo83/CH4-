// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BGPlayerController.h"
#include "UI/BGChatInput.h"
#include <Kismet/KismetSystemLibrary.h>
#include "BG/BG.h"
#include "EngineUtils.h"

#include "Kismet/GameplayStatics.h"
#include "Game/BGGameModeBase.h"

#include "BGPlayerState.h"

#include "Net/UnrealNetwork.h"

ABGPlayerController::ABGPlayerController()
{
	bReplicates = true;
}

void ABGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UBGChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
}

void ABGPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	//PrintChatMessageString(ChatMessageString);
	if (IsLocalController() == true)
	{
		//ServerRPCPrintChatMessageString(InChatMessageString);

		ABGPlayerState* BGPS = GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			//FString CombinedMessageString = BGPS->PlayerNameString + TEXT(": ") + InChatMessageString;
			FString CombinedMessageString = BGPS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString;

			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void ABGPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	//UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 5.0f);

	//FString NetModeString = BGFunctionLibrary::GetNetModeString(this);
	//FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
	//BGFunctionLibrary::MyPrintString(this, CombinedMessageString, 10.f);

	BGFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void ABGPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}

void ABGPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void ABGPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	/*for (TActorIterator<ABGPlayerController> It(GetWorld()); It; ++It)
	{
		ABGPlayerController* BGPlayerController = *It;
		if (IsValid(BGPlayerController) == true)
		{
			BGPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
		}
	}*/

	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		ABGGameModeBase* BGGM = Cast<ABGGameModeBase>(GM);
		if (IsValid(BGGM) == true)
		{
			BGGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}
