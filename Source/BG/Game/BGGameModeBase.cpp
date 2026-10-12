// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BGGameModeBase.h"

#include "BGGameStateBase.h"
#include "Player/BGPlayerController.h"
#include "Player/BGPlayerState.h"
#include "EngineUtils.h"

void ABGGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    SecretNumberString = GenerateSecretNumber();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Secret Number: %s"),
        *SecretNumberString
    );
}

void ABGGameModeBase::PrintChatMessageString(
    ABGPlayerController* InChattingPlayerController,
    const FString& InChatMessageString)
{
    if (!IsValid(InChattingPlayerController))
    {
        return;
    }

    // 채팅 메시지의 마지막 세 글자를 추측 숫자로 사용합니다.
    FString GuessNumberString = InChatMessageString.Right(3);

    if (IsGuessNumberString(GuessNumberString))
    {
        FString JudgeResultString =
            JudgeResult(SecretNumberString, GuessNumberString);

        IncreaseGuessCount(InChattingPlayerController);

        // 모든 플레이어에게 숫자 판정 결과를 전달합니다.
        for (TActorIterator<ABGPlayerController> It(GetWorld()); It; ++It)
        {
            ABGPlayerController* BGPlayerController = *It;

            if (IsValid(BGPlayerController))
            {
                FString CombinedMessageString =
                    InChatMessageString
                    + TEXT(" -> ")
                    + JudgeResultString;

                BGPlayerController->ClientRPCPrintChatMessageString(
                    CombinedMessageString
                );
            }
        }

        // 결과 전달이 끝나면 승패를 한 번만 판정합니다.
        int32 StrikeCount = 0;

        if (JudgeResultString != TEXT("OUT"))
        {
            StrikeCount =
                FCString::Atoi(*JudgeResultString.Left(1));
        }

        JudgeGame(InChattingPlayerController, StrikeCount);
    }
    else
    {
        // 숫자 입력이 아니면 일반 채팅으로 전달합니다.
        for (TActorIterator<ABGPlayerController> It(GetWorld()); It; ++It)
        {
            ABGPlayerController* BGPlayerController = *It;

            if (IsValid(BGPlayerController))
            {
                BGPlayerController->ClientRPCPrintChatMessageString(
                    InChatMessageString
                );
            }
        }
    }
}

void ABGGameModeBase::OnPostLogin(AController* NewPlayer)
{
    Super::OnPostLogin(NewPlayer);

    ABGPlayerController* BGPlayerController =
        Cast<ABGPlayerController>(NewPlayer);

    if (!IsValid(BGPlayerController))
    {
        return;
    }

    // 접속한 플레이어에게 보여 줄 공지입니다.
    BGPlayerController->NotificationText =
        FText::FromString(TEXT("Connected to the game server."));

    AllPlayerControllers.AddUnique(BGPlayerController);

    ABGPlayerState* BGPS =
        BGPlayerController->GetPlayerState<ABGPlayerState>();

    if (!IsValid(BGPS))
    {
        return;
    }

    BGPS->PlayerNameString =
        TEXT("Player")
        + FString::FromInt(AllPlayerControllers.Num());

    ABGGameStateBase* BGGameStateBase =
        GetGameState<ABGGameStateBase>();

    if (IsValid(BGGameStateBase))
    {
        BGGameStateBase->MulticastRPCBroadcastLoginMessage(
            BGPS->PlayerNameString
        );
    }
}

FString ABGGameModeBase::GenerateSecretNumber()
{
    TArray<int32> Numbers;

    for (int32 i = 1; i <= 9; ++i)
    {
        Numbers.Add(i);
    }

    FString Result;

    for (int32 i = 0; i < 3; ++i)
    {
        int32 Index =
            FMath::RandRange(0, Numbers.Num() - 1);

        Result.Append(FString::FromInt(Numbers[Index]));

        // 선택한 숫자를 제거해서 중복을 방지합니다.
        Numbers.RemoveAt(Index);
    }

    return Result;
}

bool ABGGameModeBase::IsGuessNumberString(
    const FString& InNumberString)
{
    if (InNumberString.Len() != 3)
    {
        return false;
    }

    TSet<TCHAR> UniqueDigits;

    for (TCHAR C : InNumberString)
    {
        // 1부터 9까지만 허용합니다.
        if (C < TEXT('1') || C > TEXT('9'))
        {
            return false;
        }

        // 이미 사용한 숫자라면 허용하지 않습니다.
        if (UniqueDigits.Contains(C))
        {
            return false;
        }

        UniqueDigits.Add(C);
    }

    return true;
}

FString ABGGameModeBase::JudgeResult(
    const FString& InSecretNumberString,
    const FString& InGuessNumberString)
{
    int32 StrikeCount = 0;
    int32 BallCount = 0;

    for (int32 i = 0; i < 3; ++i)
    {
        if (InSecretNumberString[i] == InGuessNumberString[i])
        {
            ++StrikeCount;
        }
        else
        {
            FString PlayerGuessChar =
                FString::Printf(TEXT("%c"), InGuessNumberString[i]);

            if (InSecretNumberString.Contains(PlayerGuessChar))
            {
                ++BallCount;
            }
        }
    }

    if (StrikeCount == 0 && BallCount == 0)
    {
        return TEXT("OUT");
    }

    return FString::Printf(
        TEXT("%dS%dB"),
        StrikeCount,
        BallCount
    );
}

void ABGGameModeBase::IncreaseGuessCount(
    ABGPlayerController* InChattingPlayerController)
{
    if (!IsValid(InChattingPlayerController))
    {
        return;
    }

    ABGPlayerState* BGPS =
        InChattingPlayerController->GetPlayerState<ABGPlayerState>();

    if (IsValid(BGPS))
    {
        ++BGPS->CurrentGuessCount;
    }
}

void ABGGameModeBase::ResetGame()
{
    SecretNumberString = GenerateSecretNumber();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("New Secret Number: %s"),
        *SecretNumberString
    );

    for (const auto& BGPlayerController : AllPlayerControllers)
    {
        if (!IsValid(BGPlayerController))
        {
            continue;
        }

        ABGPlayerState* BGPS =
            BGPlayerController->GetPlayerState<ABGPlayerState>();

        if (IsValid(BGPS))
        {
            BGPS->CurrentGuessCount = 0;
        }
    }
}

void ABGGameModeBase::JudgeGame(
    ABGPlayerController* InChattingPlayerController,
    int InStrikeCount)
{
    if (!IsValid(InChattingPlayerController))
    {
        return;
    }

    // 정답을 맞힌 경우입니다.
    if (InStrikeCount == 3)
    {
        ABGPlayerState* WinnerPlayerState =
            InChattingPlayerController->GetPlayerState<ABGPlayerState>();

        if (!IsValid(WinnerPlayerState))
        {
            return;
        }

        FString WinMessage =
            WinnerPlayerState->PlayerNameString
            + TEXT(" has won the game.");

        for (const auto& BGPlayerController : AllPlayerControllers)
        {
            if (IsValid(BGPlayerController))
            {
                BGPlayerController->NotificationText =
                    FText::FromString(WinMessage);
            }
        }

        // 모든 플레이어에게 공지를 저장한 뒤 한 번만 초기화합니다.
        ResetGame();
        return;
    }

    // 모든 플레이어가 기회를 소진했는지 확인합니다.
    bool bIsDraw = true;
    bool bHasValidPlayer = false;

    for (const auto& BGPlayerController : AllPlayerControllers)
    {
        if (!IsValid(BGPlayerController))
        {
            continue;
        }

        ABGPlayerState* BGPS =
            BGPlayerController->GetPlayerState<ABGPlayerState>();

        if (!IsValid(BGPS))
        {
            continue;
        }

        bHasValidPlayer = true;

        if (BGPS->CurrentGuessCount < BGPS->MaxGuessCount)
        {
            bIsDraw = false;
            break;
        }
    }

    if (bHasValidPlayer && bIsDraw)
    {
        for (const auto& BGPlayerController : AllPlayerControllers)
        {
            if (IsValid(BGPlayerController))
            {
                BGPlayerController->NotificationText =
                    FText::FromString(TEXT("Draw..."));
            }
        }

        // 무승부일 때도 한 번만 초기화합니다.
        ResetGame();
    }
}