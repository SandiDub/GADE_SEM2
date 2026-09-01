#include "TdPlayerController.h"
#include "TdPawn.h"
#include "TdGameMode.h"
#include "Actors/TdClickable.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ATdPlayerController::ATdPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ATdPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (MappingContext)
			{
				Sub->AddMappingContext(MappingContext, 0);
			}
		}
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		SetViewTarget(ControlledPawn);
	}
}

void ATdPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		return;
	}

	if (ClickAction)
	{
		EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &ATdPlayerController::HandleClick);
	}
	if (PauseAction)
	{
		EIC->BindAction(PauseAction, ETriggerEvent::Started, this, &ATdPlayerController::HandlePause);
	}
	if (PanAction)
	{
		EIC->BindAction(PanAction, ETriggerEvent::Triggered, this, &ATdPlayerController::HandlePan);
	}
}

void ATdPlayerController::HandleClick(const FInputActionValue& Value)
{
	(void)Value;

	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit) || !Hit.GetActor())
	{
		return;
	}

	if (ITdClickable* Clickable = Cast<ITdClickable>(Hit.GetActor()))
	{
		Clickable->OnClickedByPlayer(this);
	}
}

void ATdPlayerController::HandlePause(const FInputActionValue& Value)
{
	(void)Value;

	ATdGameMode* GM = Cast<ATdGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GM)
	{
		return;
	}

	const bool bNowPaused = UGameplayStatics::IsGamePaused(this);
	GM->SetPausedMatch(!bNowPaused);
}

void ATdPlayerController::HandlePan(const FInputActionValue& Value)
{
	if (ATdPawn* TdPawn = Cast<ATdPawn>(GetPawn()))
	{
		TdPawn->Pan(Value.Get<FVector2D>());
	}
}
