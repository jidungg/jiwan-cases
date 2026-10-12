#include "RTChessPlayerController.h"
#include "../Board/ChessBoard.h"
#include "../Pieces/ChessPieceBase.h"
#include "../Tile/TileIndicatorManagerComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "RTChessPlayerState.h"

constexpr ECollisionChannel BoardTraceChannel = ECollisionChannel::ECC_GameTraceChannel1;
constexpr ECollisionChannel PieceTraceChannel = ECollisionChannel::ECC_GameTraceChannel2;
constexpr float ClickMaxDragDistance = 100.f;

void ARTChessPlayerController::OnSetTargetReleased()
{
	if (ClickMaxDragDistance >= FVector::Distance(CachedMouseHitLocation, CachedCursorLocation))
	{

		if (IsPieceSelected())
		{
			FTileCoord ClickedTile = GetCursorTileOnBoard();
			FCommandData Command = MakeCommandData(ClickedTile);
			if (IsLegalCommand(SelectedPiece.Get(), Command, TryGetBoard()))
			{
				if (ARTChessPlayerState* PS = GetPlayerState<ARTChessPlayerState>())
					PS->ApplyLocalPredictedConsumeEnergy();
				CommandRPC(SelectedPiece.Get(), Command, TryGetBoard());
			}
			HandleSelection(nullptr);
		}
		else
		{
			AChessPieceBase* Piece = GetPieceUnderCursor();
			HandleSelection(Piece);
		}
	}
}

bool ARTChessPlayerController::IsPieceSelected()const
{
	return SelectedPiece.IsValid();
}

void ARTChessPlayerController::HandleSelection(AChessPieceBase* NewSelected)
{
	if (SelectedPiece == NewSelected)
		return;
	if (SelectedPiece.IsValid())
		SelectedPiece->Deselect(this);
	TileIndicatorManager->HideAllIndicators();
	if (nullptr == NewSelected)
	{
		SelectedPiece = nullptr;
		return;
	}
	if (false == NewSelected->IsSelectablePiece(this))
		return;

	SelectedPiece = NewSelected;
	SelectedPiece->Select(this);
	TileIndicatorManager->ShowIndicatorsForPiece(TryGetBoard(), SelectedPiece.Get());
}


void ARTChessPlayerController::CommandRPC_Implementation(AChessPieceBase* Commandee, FCommandData Command, const AChessBoard* Board)
{
	if (false == IsLegalCommand(Commandee, Command, Board))
		return;
	if (ARTChessPlayerState* PS = GetPlayerState<ARTChessPlayerState>())
	{
		PS->ConsumeEnergy();
		Commandee->ProcessCommand(Command, Board);
	}
}

bool ARTChessPlayerController::IsLegalCommand(AChessPieceBase* Commandee, FCommandData Command, const AChessBoard* Board) const
{
	if (nullptr == Commandee || false == Commandee->IsMyPiece(this))
		return false;
	if (false == Command.IsValid())
		return false;
	ARTChessPlayerState* PS = GetPlayerState<ARTChessPlayerState>();
	if (nullptr == PS)
		return false;
	if (false == PS->HasEnoughEnergy())
		return false;

	switch (Command.Type)
	{
	case ECommandType::Move:
		if (EMoveType::Invalid == Commandee->GetMoveTypeAtTile(Command.Destination, Board))
			return false;
		break;
	default:
		break;
	}

	return true;
}

AChessPieceBase* ARTChessPlayerController::GetPieceUnderCursor()const
{
	FHitResult OutHit;
	bool bHitSuccessful = false;
	if (bIsTouch)
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, PieceTraceChannel, true, OutHit);
	else
		bHitSuccessful = GetHitResultUnderCursor(PieceTraceChannel, true, OutHit);
	if (false == bHitSuccessful)
		return nullptr;

	return Cast<AChessPieceBase>(OutHit.GetActor());
}

FCommandData ARTChessPlayerController::MakeCommandData(FTileCoord Tile)
{
	FCommandData Command;
	Command.Type = ECommandType::Move;
	Command.Destination = Tile;

	return Command;
}

AChessBoard* ARTChessPlayerController::TryGetBoard()
{
	if (ChessBoard)
		return ChessBoard;
	ChessBoard = Cast<AChessBoard>(
		UGameplayStatics::GetActorOfClass(
			GetWorld(),
			AChessBoard::StaticClass()
		)
	);
	return ChessBoard;
}

FTileCoord ARTChessPlayerController::GetCursorTileOnBoard()
{
	return TryGetBoard()->WorldToTile(GetCursorLocationOnBoard());
}
FVector ARTChessPlayerController::GetCursorLocationOnBoard()
{
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, BoardTraceChannel, true, Hit);
	else
		bHitSuccessful = GetHitResultUnderCursor(BoardTraceChannel, true, Hit);
	if (false == bHitSuccessful)
		return FVector::ZeroVector;

	return Hit.Location;
}


