// Copyright 2020 Stephen Maloney

#include "Core/CeremonyFunctionLibrary.h"

void UCeremonyFunctionLibrary::LogRoleAndMode(APawn* Pawn, const FString InfoString)
{
	const ENetMode NetMode = Pawn->GetNetMode();

	FString NetModeString = "NoNetMode";
	switch(NetMode)
	{
	case NM_ListenServer:
		NetModeString = "ListenServer";
		break;
	case NM_Client:
		NetModeString = "Client";
		break;
	case NM_DedicatedServer:
		NetModeString = "DedicatedServer";
		break;
	case NM_Standalone:
		NetModeString = "Standalone";
		break;
	default:
		break;
	}

	const ENetRole NetRole = Pawn->GetLocalRole();

	FString NetRoleString = "NoNetRole";
	switch(NetRole)
	{
	case ROLE_Authority:
		NetRoleString = "Authority";
		break;
	case ROLE_AutonomousProxy:
		NetRoleString = "AutonomousProxy";
		break;
	case ROLE_SimulatedProxy:
		NetRoleString = "SimulatedProxy";
		break;
	default:
		break;
	}

	const FString Message = FString::Printf(TEXT("NetMode %s - NetRole %s - IsLocallyControlled %d : %s"), *NetModeString, *NetRoleString, Pawn->IsLocallyControlled(), *InfoString);
	
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.0f, FColor::Red, Message);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}
