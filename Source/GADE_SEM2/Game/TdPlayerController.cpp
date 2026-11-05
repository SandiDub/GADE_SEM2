#include "TdPlayerController.h"
#include "TdPawn.h"
#include "TdGameMode.h"
#include "TdGameState.h"
#include "Actors/TdClickable.h"
#include "Actors/Defender.h"
#include "Actors/CentralTower.h"
#include "Data/DefenderData.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"
#include "Engine/Engine.h"

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
	if (ZoomAction)
	{
		EIC->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ATdPlayerController::HandleZoom);
	}

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ATdPlayerController::SelectDefender0);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ATdPlayerController::SelectDefender1);
		InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ATdPlayerController::SelectDefender2);
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

void ATdPlayerController::SetUpgradeTarget(AActor* Target)
{
	if (SelectedUpgradeTarget == Target)
	{
		return;
	}

	SelectedUpgradeTarget = Target;
	OnUpgradeTargetChanged.Broadcast(Target);
}

bool ATdPlayerController::GetUpgradeInfo(AActor* Target, int32& OutLevel, int32& OutMaxLevel, bool& bOutCanUpgrade) const
{
	OutLevel = 0;
	OutMaxLevel = 0;
	bOutCanUpgrade = false;

	if (const ADefender* Def = Cast<ADefender>(Target))
	{
		OutLevel = Def->CurrentLevel;
		OutMaxLevel = Def->MaxLevel;
		bOutCanUpgrade = Def->CanUpgrade();
		return true;
	}

	if (const ACentralTower* Tower = Cast<ACentralTower>(Target))
	{
		OutLevel = Tower->CurrentLevel;
		OutMaxLevel = Tower->MaxLevel;
		bOutCanUpgrade = Tower->CanUpgrade();
		return true;
	}

	return false;
}

bool ATdPlayerController::TryUpgradeActor(AActor* SelectedActor, int32 UpgradeCost)
{
	ADefender* Def = Cast<ADefender>(SelectedActor);
	ACentralTower* Tower = Def ? nullptr : Cast<ACentralTower>(SelectedActor);

	if (!Def && !Tower)
	{
		return false;
	}

	// Check the level cap before touching gold, or a maxed target eats the payment.
	const bool bCanUpgrade = Def ? Def->CanUpgrade() : Tower->CanUpgrade();
	if (!bCanUpgrade)
	{
		return false;
	}

	ATdGameState* GS = GetWorld() ? GetWorld()->GetGameState<ATdGameState>() : nullptr;
	if (!GS || !GS->CanAfford(UpgradeCost) || !GS->TrySpendGold(UpgradeCost))
	{
		return false;
	}

	// Upgrade() raises the stats and then fires OnUpgraded for the Blueprint visuals.
	if (Def)
	{
		Def->Upgrade();
	}
	else
	{
		Tower->Upgrade();
	}

	return true;
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
void ATdPlayerController::HandleZoom(const FInputActionValue& Value)
{
	if (ATdPawn* TdPawn = Cast<ATdPawn>(GetPawn()))
	{
		TdPawn->Zoom(Value.Get<float>());
	}
}

void ATdPlayerController::SelectDefenderSlot(int32 Index)
{
	if (ATdGameMode* GM = Cast<ATdGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->SetSelectedDefenderIndex(Index);

		// Names the asset that actually resolved, so an empty or short
		// DefenderRoster is obvious the moment you press the key.
		const UDefenderData* Data = GM->GetSelectedDefenderData();
		const FString Name = Data ? Data->GetName() : TEXT("NONE");

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
				FString::Printf(TEXT("Slot %d -> %s  (roster %d, cost %d)"),
					Index + 1, *Name, GM->DefenderRoster.Num(), GM->GetNextDefenderCost()));
		}
	}
}

