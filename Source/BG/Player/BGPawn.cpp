// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BGPawn.h"
#include "BG.h"

// Sets default values
ABGPawn::ABGPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABGPawn::BeginPlay()
{
	Super::BeginPlay();

	FString NetRoleString = BGFunctionLibrary::GetRoleString(this);
	FString CombindedString = FString::Printf(TEXT("BGPawn""BeginPlay() %s [%s]"), *BGFunctionLibrary::GetNetModeString(this), *NetRoleString);
	BGFunctionLibrary::MyPrintString(this, CombindedString, 10.0f);
}

void ABGPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

FString NetRoleString = BGFunctionLibrary::GetRoleString(this);
FString CombindedString = FString::Printf(TEXT("BGPawn::PossessedBy() %s [%s]"), *BGFunctionLibrary::GetNetModeString(this), *NetRoleString);
BGFunctionLibrary::MyPrintString(this, CombindedString, 10.0f);

}
// Called every frame
void ABGPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABGPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

