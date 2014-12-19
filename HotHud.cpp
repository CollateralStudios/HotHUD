/*
* This file is part of HotHud. HotHud is free software : you can
* redistribute it and / or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, version 2.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program; if not, write to the Free Software Foundation, Inc., 51
* Franklin Street, Fifth Floor, Boston, MA 02110 - 1301 USA.
*
* Copyright 2014, San Mehat <san.mehat@gmail.com>
* =================================================================================
*
* SEE HOTHUD.H FOR DOCUMENTATION & RELEASE NOTES.
*
*/

#include "Shiver.h"
#include "HotHud.h"

/*****************************************************************************/

DEFINE_LOG_CATEGORY_STATIC(LogHUD, Log, All);

#define LOCTEXT_NAMESPACE "HUD"


AHotHud::AHotHud(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	SupressHud(true),
	localPlayerController_(nullptr),
	lastLeftMouseButtonDown_(false),
	controlBeingHovered_(nullptr),
	controlBeingDragged_(nullptr),
	controlBeingMoved_(nullptr),
	dragDropTargetValid_(false) {
}

void AHotHud::PostInitializeComponents() {
	Super::PostInitializeComponents();

	localPlayerController_ = GetOwningPlayerController();
}

HotHudControl* AHotHud::FindTopMostControlAt(const FVector2D& location) {
	for (HotHudWindow* window : rootWindows_) {
		HotHudControl* control = window->FindTopMostControlAt(location);
		if (control != nullptr) {
			return control;
		}
	}
	return nullptr;
}

HotHudControl* AHotHud::FindControlByName(const FName& name) {
	HotHudControl** tmp = controlMap_.Find(name);
	if (!tmp) {
		return nullptr;
	}
	return *tmp;
}

HotHudControl* AHotHud::HandleControlLookup(
	const FName& name, HotHudControlType type, bool& bpReturnCode) {
	HotHudControl* control = FindControlByName(name);
	if (!control) {
		UE_LOG(LogHUD, Error, TEXT("HandleControlLookup(%s): Unable to find control."), *name.ToString());
		bpReturnCode = true;
		return NULL;
	}
	if (control->Type() != type) {
		UE_LOG(LogHUD, Error, TEXT("HandleControlLookup(%s): Control type mismatch."), *name.ToString());
		bpReturnCode = true;
		return NULL;
	}
	bpReturnCode = false;
	return control;
}

void AHotHud::CreateManagedWindow(FName name, FName parentName, const FControlGeometry& geometry, const FManagedWindowBuildOptions& buildOptions, FName& nameThru, bool& error) {
	nameThru = name;
	// Validate the name hasn't already been used.
	if (controlMap_.Contains(name)) {
		UE_LOG(LogHUD, Error, TEXT("Handle '%s' already in use"), *name.ToString());
		error = true;
		return;
	}

	// Create and register the new Window.
	UE_LOG(LogHUD, Warning, TEXT("Creating window '%s'"), *name.ToString());
	HotHudControl* parent = FindControlByName(parentName);
	HotHudWindow* newWindow = new HotHudWindow(name, parent, geometry, buildOptions);
	if (parent != nullptr) {
		parent->AddChildControl(newWindow);
	}
	else {
		rootWindows_.Add(newWindow);
	}
	controlMap_.Add(name, newWindow);
	error = false;
}

void AHotHud::DeleteControl(const FName& name, bool& error) {
	error = true;
}

void AHotHud::DeleteAllControls() {
}

void AHotHud::CreateTextBox(FName name, const FName& parentName, const FControlGeometry& geometry, const FTextBoxBuildOptions& buildOptions, FName& nameThru, bool& error) {
	nameThru = name;
	// Validate the name hasn't already been used.
	if (controlMap_.Contains(name)) {
		UE_LOG(LogHUD, Error, TEXT("Handle '%s' already in use"), *name.ToString());
		error = true;
		return;
	}

	// Lookup & validate the parent.
	HotHudControl* parent = HandleControlLookup(parentName, HotHudControl_Window, error);
	if (parent == nullptr) {
		return;
	}

	// Create and register the new TextBox.
	HotHudTextBox* newTextBox = new HotHudTextBox(name, parent, geometry, buildOptions);
	parent->AddChildControl(newTextBox);
	controlMap_.Add(name, newTextBox);
	error = false;
}


void AHotHud::AddTilesToTileGrid(
	const FName& tileGridName, const TArray<FName>& tileNames, bool& error) {
	HotHudTileGrid* parent = static_cast<HotHudTileGrid*>(HandleControlLookup(tileGridName, HotHudControl_TileGrid, error));
	if (parent == nullptr) {
		return;
	}

	// Validate names haven't been used.
	for (const FName& name : tileNames) {
		if (controlMap_.Contains(name)) {
			UE_LOG(LogHUD, Error, TEXT("Handle '%s' already in use"), *name.ToString());
			error = true;
			return;
		}
	}

	parent->AddTiles(&controlMap_, tileNames);

	error = false;
}

void AHotHud::CreateTileGrid(
	const FName name, const FName& parentName, const FControlGeometry& geometry, const FTileGridBuildOptions& buildOptions, FName& nameThru, bool& error) {
	nameThru = name;

	// Validate the name hasn't already been used.
	if (controlMap_.Contains(name)) {
		UE_LOG(LogHUD, Error, TEXT("Handle '%s' already in use"), *name.ToString());
		error = true;
		return;
	}

	// Lookup & validate the parent.
	HotHudControl* parent = HandleControlLookup(parentName, HotHudControl_Window, error);
	if (parent == nullptr) {
		return;
	}

	HotHudTileGrid* newTileGrid = new HotHudTileGrid(name, parent, geometry, buildOptions);
	parent->AddChildControl(newTileGrid);
	controlMap_.Add(name, newTileGrid);
	error = false;
}

void AHotHud::PrintLineToTextBox(const FName& textboxHandle, const FString& text, bool& error) {
	HotHudControl* control = HandleControlLookup(textboxHandle, HotHudControl_TextBox, error);
	if (control == nullptr) {
		return;
	}
	HotHudTextBox* textbox = static_cast<HotHudTextBox*>(control);
	textbox->PrintLine(text);
	error = false;
}

void AHotHud::ClearTextBox(const FName& textboxHandle, bool& error) {
	HotHudControl* control = HandleControlLookup(textboxHandle, HotHudControl_TextBox, error);
	if (control == nullptr) {
		return;
	}
	HotHudTextBox* textbox = static_cast<HotHudTextBox*>(control);
	textbox->Clear();
	error = false;
}

void AHotHud::DrawHUD() {
	// Call our base DrawHUD first which will allow the blueprint layer to draw first.
	Super::DrawHUD();

	if (!localPlayerController_) {
		localPlayerController_ = GetOwningPlayerController();
		if (!localPlayerController_) {
			return;
		}
	}

	if (!localPlayerController_->PlayerInput || SupressHud || !Canvas) {
		return;
	}

	// Figure out which control is currently under the mouse and if it's different from
	// the last time we checked.
	FVector2D mouseLocation;
	localPlayerController_->GetMousePosition(mouseLocation.X, mouseLocation.Y);

	HotHudControl* controlUnderMouse = FindTopMostControlAt(mouseLocation);
	bool hoverTargetChanged = false;
	if (controlUnderMouse != controlBeingHovered_) {
		// Notify any previously hovered control it's no longer hovered over
		if (controlBeingHovered_ != nullptr) {
			UE_LOG(LogHUD, Warning, TEXT("Control '%s' is no longer being hovered."), *controlBeingHovered_->Name().ToString());
			controlBeingHovered_->SetIsHovered(false);
		}

		// Invalidate drag & drop target validity state.
		dragDropTargetValid_ = false;

		// Notify the newly hovered control and do drag & drop checks.
		if (controlUnderMouse != nullptr) {
			controlUnderMouse->SetIsHovered(true);
			UE_LOG(LogHUD, Warning, TEXT("Control '%s' is now being hovered."), *controlUnderMouse->Name().ToString());
			if (controlBeingDragged_ != nullptr) {
				// Clear out any drag state left on the last hovered control.
				if (controlBeingHovered_ != nullptr) {
					controlBeingHovered_->SetValidDragSource(nullptr);
				}

				ReceiveValidateDropTargetRequest(controlBeingDragged_->Name(), controlUnderMouse->Name(), dragDropTargetValid_);

				UE_LOG(
					LogHUD, Warning, TEXT("DropTargetValidation = %d for %s -> %s"),
					dragDropTargetValid_, *controlBeingDragged_->Name().ToString(), *controlUnderMouse->Name().ToString());

				// TODO(san): Refactor.
				if (dragDropTargetValid_) {
					controlUnderMouse->SetValidDragSource(controlBeingDragged_);
				}
				else {
					controlUnderMouse->SetValidDragSource(nullptr);
				}
			}
		}
		// 
		controlBeingHovered_ = controlUnderMouse;
		hoverTargetChanged = true;
	}

	// Get the mouse button state.
	FVector leftMouseButtonVector = localPlayerController_->GetInputVectorKeyState(
		FKey(EKeys::LeftMouseButton.GetFName()));
	bool leftMouseButtonDown = (leftMouseButtonVector.X != 0);

	if (leftMouseButtonDown != lastLeftMouseButtonDown_) {
		// Left button state has changed.
		if (leftMouseButtonDown) {
			if (controlBeingHovered_ != nullptr) {
				if (controlBeingHovered_->IsMovable()) {
					// Begin movable control move.
					controlBeingHovered_->SetIsMoving(true);
					controlBeingMoved_ = controlBeingHovered_;
					mouseControlOffset_ = mouseLocation - controlBeingMoved_->ScreenCoords();
					UE_LOG(LogHUD, Warning, TEXT("Control '%s' is now being moved."), *controlBeingMoved_->Name().ToString());
				}
				else if (controlBeingHovered_->IsDraggable()) {
					// Begin draggable control drag.
					controlBeingHovered_->SetIsDragging(true);
					controlBeingDragged_ = controlBeingHovered_;
					mouseControlOffset_ = mouseLocation - controlBeingDragged_->ScreenCoords();
					UE_LOG(LogHUD, Warning, TEXT("Control '%s' is now being dragged."), *controlBeingDragged_->Name().ToString());
				}
			}
		}
		else {
			if (controlBeingMoved_ != nullptr) {
				UE_LOG(LogHUD, Warning, TEXT("Control '%s' is no longer being moved."), *controlBeingMoved_->Name().ToString());
				controlBeingMoved_->SetIsMoving(false);
				controlBeingMoved_ = nullptr;
			}

			if (controlBeingDragged_ != nullptr) {
				UE_LOG(LogHUD, Warning, TEXT("Control '%s' is no longer being dragged."), *controlBeingDragged_->Name().ToString());
				controlBeingDragged_->SetIsDragging(false);
				if (controlBeingHovered_ != nullptr && (controlBeingHovered_->ValidDragSource())) {
					controlBeingHovered_->NotifyOnValidDrop(controlBeingDragged_);
					controlBeingHovered_->SetValidDragSource(nullptr);
				}
				controlBeingDragged_ = nullptr;
			}
		}

		lastLeftMouseButtonDown_ = leftMouseButtonDown;
	}

	// Draw any windows *not* being moved.
	HotHudWindow* movingWindow = nullptr;
	for (HotHudWindow* window : rootWindows_) {
		if (!window->IsMoving()) {
			window->Draw(this, this->Canvas);
		}
		else {
			movingWindow = window;
		}
	}

	// If there's a window being moved then update it's position and draw it.
	if (movingWindow != nullptr) {
		movingWindow->MoveToRelative(mouseLocation - mouseControlOffset_);
		movingWindow->Draw(this, this->Canvas);
	}

	// Finally draw any dragging going on.
	if (controlBeingDragged_ != nullptr) {
		UTexture2D* texture = controlBeingDragged_->GetDragTexture();
		if (texture != nullptr) {
			FLinearColor color;
			// TODO(san): Make this prettier.
			if (dragDropTargetValid_) {
				color = FLinearColor::Green;
			}
			else {
				color = FLinearColor::Red;
			}
			FCanvasTileItem dragCursor((mouseLocation - mouseControlOffset_), texture->Resource, color);
			dragCursor.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(dragCursor);
		}
	}
}

/*****************************************************************************/
HotHudControl::HotHudControl(
	HotHudControlType type, const FName& name, HotHudControl* parent,
	const FControlGeometry& geometry, bool isMovable, bool isDraggable)
	: type_(type),
	name_(name),
	parent_(parent),
	geometry_(geometry),
	isMovable_(isMovable),
	isDraggable_(isDraggable),
	screenCoords_(FVector2D(0, 0)),
	childOffsetTop_(0),
	childOffsetRight_(0),
	childOffsetBottom_(0),
	childOffsetLeft_(0),
	isMoving_(false),
	isDragging_(false),
	isHovered_(false),
	validDragSource_(nullptr) {
	RecomputeAbsolutePosition();
}

void HotHudControl::AddChildControl(HotHudControl* child) {
	childControls_.Add(child);
	child->RecomputeAbsolutePosition();

	// Clamp the size of the child to our child viewport.
	// TODO(san): Revisit for scrollbars.
	int32 maxChildWidth = geometry_.Width - childOffsetLeft_ - childOffsetRight_;
	int32 maxChildHeight = geometry_.Height - childOffsetTop_ - childOffsetBottom_;

	if ((child->Geometry().Width > maxChildWidth) || (child->Geometry().Height > maxChildHeight)) {
		UE_LOG(LogHUD, Warning, TEXT("Control %s being resized from %d,%d to %d,%d for fit to %s"),
			*child->Name().ToString(), child->Geometry().Width, child->Geometry().Height, maxChildWidth, maxChildHeight, *name_.ToString());
		child->Resize(maxChildWidth, maxChildHeight);
	}
}

bool HotHudControl::ContainsCoord(const FVector2D& coord) {
	if ((coord.X >= screenCoords_.X) && (coord.X <= (screenCoords_.X + geometry_.Width)) &&
		(coord.Y >= screenCoords_.Y) && (coord.Y <= (screenCoords_.Y + geometry_.Height))) {
		return true;
	}
	return false;
}

HotHudControl* HotHudControl::FindTopMostControlAt(const FVector2D& location) {
	// Fast-path check.
	if (!ContainsCoord(location)) {
		return nullptr;
	}

	HotHudControl* childHit = nullptr;
	for (HotHudControl* child : childControls_) {
		childHit = child->FindTopMostControlAt(location);
		if (childHit) {
			break;
		}
	}

	return childHit == nullptr ? this : childHit;
}

bool HotHudControl::IsValidMove(const FVector2D& location) {
	// TODO(san): This is where we'd check for sub-window constraints.
	return true;
}

void HotHudControl::Resize(int32 width, int32 height) {
	geometry_.Width = width;
	geometry_.Height = height;

	// TODO(san): Notify children? Soon.
}

void HotHudControl::MoveToRelative(const FVector2D& location) {
	if (IsValidMove(location)) {
		geometry_.Location = location;

		RecomputeAbsolutePosition();

		for (HotHudControl* child : this->childControls_) {
			child->MoveToRelative(child->geometry_.Location);
		}
	}
}

void HotHudControl::RecomputeAbsolutePosition() {
	if (parent_ == nullptr) {
		screenCoords_ = geometry_.Location;
	}
	else {
		screenCoords_.X = parent_->ScreenCoords().X + parent_->ChildOffsetLeft() + geometry_.Location.X;
		screenCoords_.Y = parent_->ScreenCoords().Y + parent_->ChildOffsetTop() + geometry_.Location.Y;
	}
}

void HotHudControl::NotifyOnValidDrop(HotHudControl* sourceControl) {

}

/*****************************************************************************/

HotHudWindow::HotHudWindow(const FName& name, HotHudControl* parent, const FControlGeometry& geometry, const FManagedWindowBuildOptions& cfg)
: HotHudControl(HotHudControl_Window, name, parent, geometry, cfg.IsMovable, false),
    cfg_(cfg ) {
	childOffsetLeft_ = kWindowBorderWidth;
	childOffsetTop_ = kWindowBorderWidth + cfg_.TitleBarHeight;
	childOffsetRight_ = childOffsetLeft_;
	childOffsetBottom_ = childOffsetLeft_;
}

void HotHudWindow::DrawBox(AHUD* hud, float x1, float y1, float x2, float y2, const FLinearColor& color) {
	hud->DrawLine(x1, y1, x2, y1, color);
	hud->DrawLine(x2, y1, x2, y2, color);
	hud->DrawLine(x2, y2, x1, y2, color);
	hud->DrawLine(x1, y2, x1, y1, color);
}

void HotHudWindow::DrawBorder(AHUD* hud) {
	FLinearColor colorBlack(0, 0, 0, 1.0);
	FLinearColor colorLessBlack(0, 0, 0, 0.75);

	DrawBox(hud, screenCoords_.X, screenCoords_.Y, screenCoords_.X + geometry_.Width, screenCoords_.Y + geometry_.Height, colorBlack);
	DrawBox(hud, screenCoords_.X + 1, screenCoords_.Y + 1, screenCoords_.X + geometry_.Width - 1, screenCoords_.Y + geometry_.Height - 1, colorLessBlack);
}

void HotHudWindow::DrawTitlebar(AHUD* hud) {
	if (cfg_.Title.Len() != 0) {
		int32 x, y, w;

		x = screenCoords_.X + kWindowBorderWidth;
		y = screenCoords_.Y + kWindowBorderWidth;
		w = geometry_.Width - (kWindowBorderWidth * 2);

		hud->DrawRect(cfg_.TitleBarColor, x, y, w, cfg_.TitleBarHeight);
		hud->DrawText(cfg_.Title, cfg_.TitleTextColor, x, y, cfg_.TitleFont, cfg_.TitleFontScale, false);
	}
}

void HotHudWindow::Draw(AHotHud* hud, UCanvas* canvas) {
	// Draw window background.
	hud->DrawRect(cfg_.BackgroundColor, screenCoords_.X, screenCoords_.Y, geometry_.Width, geometry_.Height);

	// Draw window border.
	DrawBorder(hud);

	// Draw titlebar (if any).
	DrawTitlebar(hud);

	// Draw children.
	for (HotHudControl* child : childControls_) {
		child->Draw(hud, canvas);
	}
}

/*****************************************************************************/

HotHudTextBox::HotHudTextBox(
	const FName& name, HotHudControl* parent, const FControlGeometry& geometry, const FTextBoxBuildOptions& cfg)
	: HotHudControl(HotHudControl_TextBox, name, parent, geometry, false, false),
	cfg_(cfg),
	numColumns_(-1),
	numRows_(-1),
	rowHeight_(0),
	virtualCursorRow_(0),
	virtualCursorColumn_(0) {
	childOffsetLeft_ = kTextBoxBorderLeftWidth;
	childOffsetTop_ = kTextBoxBorderTopHeight;
	childOffsetRight_ = kTextBoxBorderRightWidth;
	childOffsetBottom_ = kTextBoxBorderBottomHeight;
	if (cfg_.Text.Len()) {
		PrintLine(cfg_.Text);
	}
}

void HotHudTextBox::Resize(int32 width, int32 height) {
	HotHudControl::Resize(width, height);
	numColumns_ = -1;
	numRows_ = -1;
}

void HotHudTextBox::Clear() {
	rowBuffer_.Empty();
	virtualCursorRow_ = 0;
	virtualCursorColumn_ = 0;
}

void HotHudTextBox::PrintLine(const FString& line) {
	rowBuffer_.Add(line);
	virtualCursorRow_++;
}

void HotHudTextBox::Draw(AHotHud* hud, UCanvas* canvas) {
	// Re-calculate the number of rows and columns we can display if needed.
	// TODO(san): Investigate the cost of creating a temporary Canvas so we don't need to do this in Draw().
	if (numColumns_ == -1) {
		FString testString = "XgGZP";

		float testStringWidth;
		float testStringHeight;
		canvas->TextSize(cfg_.Font, testString, testStringWidth, testStringHeight);
		rowHeight_ = testStringHeight;
		numColumns_ = geometry_.Width / (testStringWidth / testString.Len());
		numRows_ = geometry_.Height / testStringHeight;

		UE_LOG(LogHUD, Warning, TEXT("Textbox can contain %d columns and %d rows, font %x"), numColumns_, numRows_, cfg_.Font);
	}
	
	// Draw background.
	hud->DrawRect(cfg_.BackgroundColor, screenCoords_.X, screenCoords_.Y, geometry_.Width, geometry_.Height);

	// Draw text.
	int numItemsToDraw = (rowBuffer_.Num() > numRows_) ? numRows_ : rowBuffer_.Num();
	for (int i = 0; i < numItemsToDraw; i++) {
		const FString& rowString = rowBuffer_[i];
		hud->DrawText(rowString, cfg_.DefaultTextColor, screenCoords_.X, screenCoords_.Y + (rowHeight_ * i), cfg_.Font, cfg_.FontScale, false);
	}
}
/*****************************************************************************/

HotHudTile::HotHudTile(
	const FName& name, HotHudControl* parent, const FControlGeometry& geometry)
	: HotHudControl(HotHudControl_Tile, name, parent, geometry, false, false),
	tileDataFetched_(false),
	tileImage_(nullptr),
	tileText_("") {
}

void HotHudTile::Draw(AHotHud* hud, UCanvas* canvas) {
	if (!tileDataFetched_) {
		hud->ReceiveTileInfoRequest(name_, tileImage_, isDraggable_);
		tileDataFetched_ = true;
	}

	if (tileImage_ != nullptr) {
		FCanvasTileItem canvasTile(screenCoords_, tileImage_->Resource, FLinearColor::White);
		canvasTile.BlendMode = SE_BLEND_Translucent;
		canvas->DrawItem(canvasTile);
	}
	else {
		// TODO(san): Stock image.
	}
	if (tileText_ != "") {

	}
}

UTexture2D* HotHudTile::GetDragTexture() {
	return tileImage_;
}

/*****************************************************************************/

HotHudTileGrid::HotHudTileGrid(
	const FName& name, HotHudControl* parent, const FControlGeometry& geometry,
	const FTileGridBuildOptions& cfg)
	: HotHudControl(HotHudControl_TileGrid, name, parent, geometry, false, false),
	cfg_(cfg),
	numColumns_(-1),
	numRows_(-1),
	tilePositionsNeedRecalc_(true) {
	childOffsetLeft_ = kTileSeparation;
	childOffsetTop_ = kTileSeparation;
	childOffsetRight_ = kTileSeparation;
	childOffsetBottom_ = kTileSeparation;
}

void HotHudTileGrid::Resize(int32 width, int32 height) {
	HotHudControl::Resize(width, height);
	numColumns_ = -1;
	numRows_ = -1;
	tilePositionsNeedRecalc_ = true;
}

void HotHudTileGrid::AddTiles(TMap<FName, HotHudControl*>* controlMap, const TArray<FName>& tileNames) {
	for (const FName& tileName : tileNames) {
		FControlGeometry geom;
		geom.Height = cfg_.TileHeight;
		geom.Width = cfg_.TileWidth;
		HotHudTile* newTile = new HotHudTile(tileName, this, geom);
		AddChildControl(newTile);
		controlMap->Add(tileName, newTile);
	}
	tilePositionsNeedRecalc_ = true;
}

void HotHudTileGrid::AddChildControl(HotHudControl* child) {
	HotHudControl::AddChildControl(child);
	tilePositionsNeedRecalc_ = true;
}

void HotHudTileGrid::Draw(AHotHud* hud, UCanvas* canvas) {
	if (numColumns_ == -1) {
		numColumns_ = geometry_.Width / (cfg_.TileWidth + kTileSeparation);
		numRows_ = geometry_.Height / (cfg_.TileHeight + kTileSeparation);
		UE_LOG(LogHUD, Warning, TEXT("TileGrid can contain %d columns and %d rows"), numColumns_, numRows_);
	}

	// Draw background.
	hud->DrawRect(cfg_.BackgroundColor, screenCoords_.X, screenCoords_.Y, geometry_.Width, geometry_.Height);

	// TODO(san): Draw border.

	// TODO(san): Draw grid.

	// Recalculate tile positions if needed. This can happen when one of the following occurs:
	//   1. A tile is added to or removed from the grid.
	//   2. The grid is re-sized.
	if (tilePositionsNeedRecalc_ && childControls_.Num() > 0) {
		for (int i = 0; i < childControls_.Num(); i++) {
			int childRow = i / numColumns_;
			int childCol = i % numColumns_;
			int childX = childCol * (cfg_.TileWidth + kTileSeparation);
			int childY = childRow * (cfg_.TileHeight + kTileSeparation);

			FVector2D childRelCoord(childX, childY);
			childControls_[i]->MoveToRelative(childRelCoord);
		}
		tilePositionsNeedRecalc_ = false;
	}

	// Draw children.
	for (HotHudControl* child : childControls_) {
		child->Draw(hud, canvas);
	}
}
