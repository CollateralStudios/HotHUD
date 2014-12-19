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
 * Thanks for checking out HotHud, a window manager for the UE 4.6 HUD. With HotHud,
 * you can easily create complex user-interfaces via blueprints *at runtime* with all
 * of the window manager complexity taken care of for you. I developed (and am 
 * continuing to develop) HotHud mainly as a way of exploring UE and well.. because I
 * needed a good HUD for my project :).
 *
 * Some of HotHuds notable features include:
 *  - Dynamic player movable windows with title-bars, borders..
 *  - Single / Multi-line player editable text-boxes.
 *	- TileGrid controls for displaying lists of items which can be dragged & dropped
 *	  by the player.
 *	- Easy(?) to deal with co-ordinate system (controls are relative to their parent).
 *
 * List of things I'm working on and hoping to get done soon:
 *  - Smart automatic scroll-bars.
 *  - More controls
 *  - Texture support for window chrome.
 *
 *
 * CAVEATS / LIMITATIONS / BUGS:
 *  - The UE object model is pretty new to me, so I'm likely not interfacing with the
 *    engine in 'the best' way, but with tinkering comes wisdom so I have no doubt it
 *    will improve over time. Constructive feedback and suggestions as to better 
 *    use the object model is appreciated.
 *
 *  - Textures are not yet supported for window chrome. This will be changed once
 *    i have some good textures to use - as it turns out I suck at drawing ;)
 *  
 *  - Editable text boxes do not work yet.
 *
 *  - TextBoxes do not scroll properly yet.
 *  - There are lots of internal code cleanups needed - this is very much a work in
 *    progress :).
 *
 */
#pragma once

#include "GameFramework/HUD.h"
#include "HotHud.generated.h"

class AHotHud;

// Contains visual-cfg options for a Window.
USTRUCT(BlueprintType)
struct FManagedWindowBuildOptions {
	GENERATED_USTRUCT_BODY()

	// Background color for the window.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		FLinearColor BackgroundColor;
	
	// Height of the title bar in pixels.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		int32 TitleBarHeight;

	// Color of the title bar.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		FLinearColor TitleBarColor;

	// Font to use for the title bar.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		UFont* TitleFont;

	// Font scale to apply to the title bar font.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		float TitleFontScale;

	// Color to draw the title bar text.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		FLinearColor TitleTextColor;

	// Title text. May be an empty string for no title-bar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		FString Title;

	// Set if the window can be moved by the user.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		bool IsMovable;

	// Set if the horizontal scrollbar should be shown at all times.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		bool AlwaysShowHBar;

	// Set if the vertical scrollbar should be shown at all times.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		bool AlwaysShowVBar;

	FManagedWindowBuildOptions() {
		// TODO(san): Find some prettier defaults.
		BackgroundColor = FLinearColor(0, 0, 0, 0.5f);
		TitleBarHeight = 18;
		TitleBarColor = FLinearColor(0.2, 0.2, 0.2, 0.75f);
		TitleFont = nullptr;  // Use the UE default font.
		TitleFontScale = 1.0f;
		TitleTextColor = FLinearColor(1, 1, 1, 1.0f);
		Title = "My Window";
		IsMovable = true;
		AlwaysShowHBar = false;
		AlwaysShowVBar = false;
	}
};

// Contains visual-cfg options for a TextBox.
USTRUCT(BlueprintType)
struct FTextBoxBuildOptions {
	GENERATED_USTRUCT_BODY()

	// Background color for the text box.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		FLinearColor BackgroundColor;

	// Font to use when drawing text in the text box.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		UFont* Font;

	// Scaling factor to be applied to the font when drawing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		float FontScale;

	// Default Color to draw the text in.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		FLinearColor DefaultTextColor;

	// Set if the box is editable by the user.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		bool Editable;

	// Default text for the textbox.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		FString Text;

	FTextBoxBuildOptions() {
		BackgroundColor = FLinearColor(0, 0, 0, 0.5f);
		Font = nullptr;
		FontScale = 1.0f;
		DefaultTextColor = FLinearColor(1, 1, 1, 1.0f);
		Editable = false;
		Text = "My new textbox";
	}
};

// Contains visual-cfg options for a TileGrid.
USTRUCT(BlueprintType)
struct FTileGridBuildOptions {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		int32 TileWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		int32 TileHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		int32 MaxCols;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		int32 MaxRows;

	// Background color.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		FLinearColor BackgroundColor;

	FTileGridBuildOptions() {
		TileWidth = 64;
		TileHeight = 64;
		MaxCols = 8;
		MaxRows = 0;
		BackgroundColor = FLinearColor(0, 0, 0, 0.5f);
	}
};

// Structure which contains control geometry.
USTRUCT(BlueprintType)
struct FControlGeometry {
	GENERATED_USTRUCT_BODY()

	// Co-ordinate of the control.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
	FVector2D Location;

	// The width of the control.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		int32 Width;

	// The height of the control.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
		int32 Height;

	FControlGeometry() {
		Location = FVector2D(0, 0);
		Width = 320;
		Height = 240;
	}
};

typedef enum HotHudControlType {
	HotHudControl_Window = 1,
	HotHudControl_TextBox = 2,
	HotHudControl_Tile = 3,
	HotHudControl_TileGrid = 4,
};

// Base implementation for a HotHud HUD control.
class HotHudControl {
public:
	// Constructs a HotHudControl.
	// Name is the name of the control.
	// Parent is the parent for this control. May be nullptr. Ownership not taken.
	// Geometry specifies the location and size of the control, relative to its parent.
	// IsMovable is set if this control is movable.
	// IsDraggable is set if this control can be the source of a drag/drop.
	HotHudControl(
		HotHudControlType type, const FName& name, HotHudControl* parentName,
		const FControlGeometry& geometry, bool isMovable, bool isDraggable);

	virtual ~HotHudControl() {}

	virtual void Draw(AHotHud* hud, UCanvas* canvas) = 0;

	virtual void AddChildControl(HotHudControl* child);
	virtual void MoveToRelative(const FVector2D& location);
	virtual void Resize(int32 width, int32 height);
	virtual UTexture2D* GetDragTexture() { return nullptr; }

	// Returns the top-most (highest on the Z-order) control at the specified location.
	// May return NULL.
	virtual HotHudControl* FindTopMostControlAt(const FVector2D& location);
	bool ContainsCoord(const FVector2D& coord);

	virtual void NotifyOnValidDrop(HotHudControl* sourceControl);

	HotHudControlType Type() const { return type_; }
	const FName& Name() const { return name_; }
	bool IsMovable() const { return isMovable_; }
	bool IsDraggable() const { return isDraggable_; }
	const FVector2D& ScreenCoords() const { return screenCoords_;  }
	int32 ChildOffsetTop() const { return childOffsetTop_; }
	int32 ChildOffsetRight() const { return childOffsetRight_; }
	int32 ChildOffsetBottom() const { return childOffsetBottom_; }
	int32 ChildOffsetLeft() const { return childOffsetLeft_; }
	const FControlGeometry& Geometry() const { return geometry_; }
	bool IsMoving() const { return isMoving_; }
	HotHudControl* ValidDragSource() const { return validDragSource_; }

	void SetIsMoving(bool isMoving) { isMoving_ = isMoving; }
	void SetIsDragging(bool isDragging) { isDragging_ = isDragging; }
	void SetIsHovered(bool isHovered) { isHovered_ = isHovered; }
	void SetValidDragSource(HotHudControl* validDragSource) { validDragSource_ = validDragSource; }

protected:
	void RecomputeAbsolutePosition();
	bool IsValidMove(const FVector2D& location);

	// Internal type identifier for this control.
	HotHudControlType type_;
	// Unique name for this control.
	FName name_;
	// Parent control to which this control is constrained. May be nullptr. Not owned.
	HotHudControl* parent_;
	// Relative Geometry of the control.
	FControlGeometry geometry_;
	// True if this control can be moved with the mouse.
	bool isMovable_;
	// True if this control can be dragged with the mouse.
	bool isDraggable_;
	// Absolute screen co-ordinates of this control.
	FVector2D screenCoords_;
	// Number of pixels this control needs for chrome on the top.
	int childOffsetTop_;
	// Number of pixels this control needs for chrome on the right.
	int childOffsetRight_;
	// Number of pixels this control needs for chrome on the bottom.
	int childOffsetBottom_;
	// Number of pixels this control needs for chrome on the left.
	int childOffsetLeft_;
	// Set if the control is currently being moved.
	bool isMoving_;
	// Set if the control is currently being dragged
	bool isDragging_;
	// Set if the control is currently being hovered over with the mouse.
	bool isHovered_;
	// Array of child controls. Pointers are owned.
	TArray<HotHudControl*> childControls_;
	// Pointer to the control which is currently being dragged over this one. It as assumed that
	// the dragSource has already been validated to be a valid source. Pointer not owned.
	HotHudControl* validDragSource_;
};

// A HotHudWindow is the top level object which contains controls to be displayed on the HUD.
// The window consists of a frame, title-bar, title-text, and optional horizontal and vertical 
// scrollbars (depending on whether the window size is insufficient to contain the contents). 
class HotHudWindow : public HotHudControl {
public:
	// Construct a HotHudWindow.
	// Name is the name of the window.
	// Parent is the parent for this control. May be nullptr. Ownership not taken.
	// IsMovable should be set if the window should allow being relocated by the user.
	// Title to be displayed on the titlebar of the window.
	// TitleColor is the color to render the titlebar text in.
	HotHudWindow(const FName& name, HotHudControl* parent, const FControlGeometry& geometry, const FManagedWindowBuildOptions& cfg);

	virtual ~HotHudWindow() {}

	virtual void Draw(AHotHud* hud, UCanvas* canvas) override;

private:
	static const int kWindowBorderWidth = 2;

	void DrawTitlebar(AHUD* hud);
	void DrawBorder(AHUD* hud);
	void DrawBox(AHUD* hud, float x1, float y1, float x2, float y2, const FLinearColor& color);

	// Window cfg as provided by the BP.
	FManagedWindowBuildOptions cfg_;
};

// A n-line text-box..
class HotHudTextBox : public HotHudControl {
public:
	// Construct a HotHudTextBox
	// Name is the name of the TextBox.
	// ParentControl is the control which this panel is parented to. MUST NOT BE NULL.
	// Geometry is the geometry of the TextBox.
	HotHudTextBox(const FName& name, HotHudControl* parent, const FControlGeometry& geometry, const FTextBoxBuildOptions& cfg);
	virtual ~HotHudTextBox() {}

	void PrintLine(const FString& line);
	void Clear();
	virtual void Draw(AHotHud* hud, UCanvas* canvas) override;
	virtual void Resize(int32 width, int32 height) override;

private:
	static const int kTextBoxBorderTopHeight = 2;
	static const int kTextBoxBorderLeftWidth = 2;
	static const int kTextBoxBorderRightWidth = 2;
	static const int kTextBoxBorderBottomHeight = 2;

	// TextBox cfg as provided by the BP.
	FTextBoxBuildOptions cfg_;
	// Total number of columns we can fit given our font and size.
	int numColumns_;
	// Total number of rows we can fit given our font and size.
	int numRows_;
	// Height in pixels of a row of text.
	int rowHeight_;
	// Virtual cursor row.
	int virtualCursorRow_;
	// Virtual cursor column.
	int virtualCursorColumn_;
	// An array of strings used as backing buffers (one string per row).
	TArray<FString> rowBuffer_;
};

// A drag & droppable tile.
class HotHudTile : public HotHudControl {
public:
	// Construct a HotHudTile.
	// Name is the name of the Tile.
	// ParentControl is the control which this panel is parented to. MUST NOT BE NULL.
	// Geometry is the geometry of the tile..
	HotHudTile(const FName& name, HotHudControl* parent, const FControlGeometry& geometry);
	virtual ~HotHudTile() {}

	virtual void Draw(AHotHud* hud, UCanvas* canvas) override;
	virtual UTexture2D* GetDragTexture() override;

private:
	bool tileDataFetched_;
	UTexture2D* tileImage_;
	FString tileText_;
};

// A panel which contains a grid of tiles which are HotHud by col/row instead of x,y.
class HotHudTileGrid : public HotHudControl {
public:
	// Construct a HotHudTileGrid.
	// Name is the name of the TileGrid.
	// ParentControl is the control which this panel is parented to. MUST NOT BE NULL.
	// 
	HotHudTileGrid(const FName& name, HotHudControl* parent, const FControlGeometry& geometry, const FTileGridBuildOptions& cfg);
	virtual ~HotHudTileGrid() {}

	virtual void AddTiles(TMap<FName, HotHudControl*>* controlMap, const TArray<FName>& tileNames);

	virtual void Draw(AHotHud* hud, UCanvas* canvas) override;
	virtual void Resize(int32 width, int32 height) override;
	virtual void AddChildControl(HotHudControl* child) override;

	const FTileGridBuildOptions& Cfg() const { return cfg_;  }

private:
	const static int kTileSeparation = 2;

	// TileGrid cfg as provided by the BP.
	FTileGridBuildOptions cfg_;
	// Total number of columns we can fit.
	int numColumns_;
	// Total number of rows we can fit.
	int numRows_;
	// Set when something changes that requires us to recompute child tile locations.
	bool tilePositionsNeedRecalc_;
};

/**
*
*/
UCLASS()
class SHIVER_API AHotHud : public AHUD
{
	GENERATED_BODY()
public:
	AHotHud(const FObjectInitializer& ObjectInitializer);
	virtual ~AHotHud() {}

	//// 
	//// Public properties exposed to blueprints.
	////

	// Blueprint read/write exported property which when set indicates we shouldn't draw any of the HotHud HUD controls.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HotHud)
	bool SupressHud;


	//// 
	//// Public methods exposed to blueprints.
	////

	// Creates a managed window which will be drawn on the HUD.
	// Name is a BP provided name for the new window.
	// ParentName is the parent for this window. May be 'None'. If parent is not 'None' then the newly created
	//            window is a sub-window which cannot be moved outside of the parent.
	// Geometry is a copy of a structure which specifies the geometry for the new window. If Parent was
	//          specified then the X and Y co-ordinates are relative to the parent.
	// BuildOptions contain window-specific build options.
	// NameThru is set to a copy of the passed in Name.
	// Error is set if the operation failed. Logs will have more details on the failure.
	UFUNCTION(BlueprintCallable, Category = HotHud)
	void CreateManagedWindow(FName name, FName parentName, const FControlGeometry& geometry, const FManagedWindowBuildOptions& buildOptions, FName& nameThru, bool& error);

	// Deletes a previously created control. Control will not get shown on the next draw event.
	// Name is the name of the control to delete.
	// Error is set if the operation failed. Logs will have more details on the failure.
	UFUNCTION(BlueprintCallable, Category = HotHud)
	void DeleteControl(const FName& name, bool& error);

	// Delete all controls HotHud is managing.
	UFUNCTION(BlueprintCallable, Category = HotHud)
	void DeleteAllControls();

	// Creates a TextBox suitable for displaying rows of text.
	// Name is the BP provided name of the new TextBox.
	// Parent is the parent control. Cannot be 'None'.
	// Geometry specifies the location and size of the TextBox in pixels. Location is relative to Parent.
	// BuildOptions contain textbox-specific build options.
	// NameThru is set to a copy of the passed in Name.
	// Error is set if the operation failed. Logs will have more details on the failure.
	UFUNCTION(BlueprintCallable, Category = HotHud)
	void CreateTextBox(FName name, const FName& parentName, const FControlGeometry& geometry, const FTextBoxBuildOptions& buildOptions, FName& nameThru, bool& error);

	// Adds a line of text to the textbox which will be shown on the next Draw.
	// Contents of the current window is scrolled up if necessary.
	// TextBoxName is the name of a previously created TextBox.
	// Text is the line of text to display.
	// Error is set if the operation failed. Logs will have more details on the failure.
	UFUNCTION(BlueprintCallable, Category = HotHud)
		void PrintLineToTextBox(const FName& textBoxName, const FString& text, bool& error);

	// Clears the contents of a TextBox and resets the cursor to the first row.
	// TextBoxName is the name of a previously created TextBox.
	// Error is set if the operation failed. Logs will have more details on the failure.
	UFUNCTION(BlueprintCallable, Category = HotHud)
		void ClearTextBox(const FName& textBoxName, bool& error);

	// Creates a grid of tiles
	// Name is the BP provided name of the new TileGrid.
	// Parent is the parent control. Cannot be 'None'.
	// Geometry specifies the location and size of the TileGrid in pixels. Location is relative to Parent.
	// BuildOptions contain tilegrid specific build options.
	// NameThru is set to a copy of the passed in Name.
	// Error is set if the operation failed. Logs will have more details on the failure.
	//
	// Note: Structures are copied by value to make it more convenient for BP authors.
	// Note: This implementation expects all tiles to be of the same size. A future refactoring
	//       will remove this restriction.
	UFUNCTION(BlueprintCallable, Category = HotHud)
		void CreateTileGrid(FName name, const FName& parentName, const FControlGeometry& geometry, const FTileGridBuildOptions& buildOptions, FName& nameThru, bool& error);


	// Add multiple tiles to a TileGrid. Each tile is backed by a 2D texture, emits visual cues when
	// hovered, and can be optionally dragged and dropped by the player.
	// TileGridName is the Name of the TileGrid to which we're adding tiles.
	// TileNames is an array of Names to be used for the new tiles.
	// Error is set if the operation failed. Logs will have more details on the failure.
	//
	// Blueprints using this method must implement the 'ReceiveTileInfoRequest' event in order to provide
	// HotHud with per-tile configuration.
	UFUNCTION(BlueprintCallable, Category = HotHud)
		void AddTilesToTileGrid(const FName& tileGridName, const TArray<FName>& tileNames, bool& error);


	//// 
	//// Functions which are overridden in blueprints that we call out to.
	////

	UFUNCTION(BlueprintImplementableEvent, Category = HotHud)
		void ReceiveTileInfoRequest(const FName& tileName, UTexture2D*& tileImage, bool& isDraggable);


	UFUNCTION(BlueprintImplementableEvent, Category = HotHud)
		void ReceiveValidateDropTargetRequest(
		const FName& sourceControl, const FName& targetControl,
		bool& okToDrop);

protected:
	virtual void DrawHUD() override;
	virtual void PostInitializeComponents() override;

private:
	HotHudControl* HandleControlLookup(
		const FName& name, HotHudControlType type, bool& bpReturnCode);
	HotHudControl* FindControlByName(const FName& name);
	HotHudControl* FindTopMostControlAt(const FVector2D& location);

	// Map of all controls.
	TMap<FName, HotHudControl*> controlMap_;

	// Array windows which are parented to the screen.
	TArray<HotHudWindow*> rootWindows_;

	// PlayerController for the current client machine. Pointer not owned.
	APlayerController* localPlayerController_;

	bool lastLeftMouseButtonDown_;
	HotHudControl* controlBeingHovered_;
	HotHudControl* controlBeingDragged_;
	HotHudControl* controlBeingMoved_;
	FVector2D mouseControlOffset_;
	bool dragDropTargetValid_;
};
