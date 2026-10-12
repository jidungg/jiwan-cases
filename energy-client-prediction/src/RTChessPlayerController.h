#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../Data/CommonDataHeader.h"
#include "RTChessPlayerController.generated.h"

/** Forward declaration to improve compiling times */
class AChessPieceBase;
class AChessBoard;
class UTileIndicatorManagerComponent;

UCLASS()
class ARTChessPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	void OnSetTargetReleased();

private:
	void HandleSelection(AChessPieceBase* NewSelected);
	bool IsPieceSelected() const;
	AChessPieceBase* GetPieceUnderCursor() const;
	FTileCoord GetCursorTileOnBoard();
	FVector GetCursorLocationOnBoard();
	FCommandData MakeCommandData(FTileCoord Tile);
	AChessBoard* TryGetBoard();

public://Running On Server Side
	UFUNCTION(Server, Reliable)
	void CommandRPC(AChessPieceBase* Commandee, FCommandData Command, const AChessBoard* Board);
	bool IsLegalCommand(AChessPieceBase* Commandee, FCommandData Command, const AChessBoard* Board) const;

private:
	TObjectPtr<AChessBoard> ChessBoard;
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	TObjectPtr<UTileIndicatorManagerComponent> TileIndicatorManager;
	TWeakObjectPtr<AChessPieceBase> SelectedPiece;
	bool bIsTouch; // Is it a touch device

	FVector CachedCursorLocation;
	FVector CachedMouseHitLocation;

	float ClickMaxDragDistance = 100.f;
};

